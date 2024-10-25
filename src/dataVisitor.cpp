#include "SysFunc.h"
#include "dataVisitor.h"
#include "Compiler.h"
#include "Annotations.h"
#include "annotationVisitor.h"
#include "commands_inc.h"

bool dataVisitor::gotDataNum = false;
bool dataVisitor::gotDataStr = false;
bool dataVisitor::gotDataBool = false;

visitRes dataVisitor::visitCommand(const AST::Exp* component) const
{
    DataRes res;

    //getting the expression type
    Type type = Compiler::getAnnotation<AST::Exp_Annotation>(component->exp_id)->exp_type;
    TYPES action_type = type.pointerDepth ? INT : type.type;
    switch (action_type)
    {
    case STR: {
        
        std::vector<std::pair<unsigned int, std::string>> names;

        //getting the names of the strings 
        ((AST::Exp_Str*)(component))->getTmpStrNames(names);

        //if the strings concat resulte buffer wasn't already sent by other string expression (if true: send the buffer)
        if(!gotDataStr)
        {
            gotDataStr = true;
            res.push_back({SPACE , "res: .space 10000\n"});
        }

        //creating and adding the data segment lines for all the string 
        for(const auto& [id, val] : names)
            res.push_back({ASCII , "str" + std::to_string(id) + ": .asciiz " + val + "\n"});
    }
        break;
    

    case BOOL: 
        if(!gotDataBool)
        {
            gotDataBool = true;
            res.push_back({SPACE, " Bvars: .space " + std::to_string((annotationVisitor::boolMaxHeight + 1)) + "\n"}); // boolean expresion buffer
        }
    case INT:
    case FLOAT: 
        if(!gotDataNum && annotationVisitor::numMaxHeight != 0)
        {
            gotDataNum = true;
            res.insert(res.begin(), {SPACE, " Nvars: .space " + std::to_string((annotationVisitor::numMaxHeight + 1) * WORD_SIZE) + "\n"});  // numeric expresion buffer
        }
        break;
    }
    
    return res;

}

visitRes dataVisitor::visitCommand(const AST::VarDecl* component) const
{
    if(Compiler::getAnnotation<AST::Var_Annotation>(component->id)->scopeId != GLOBAL_SCOPE_ID)
        return DataRes();

    DataRes res;

    dataType temp;

    // gets the data type of the decleration
    switch (component->getType().type) {
    case INT:
    case FLOAT:
        temp = WORD;
        break;
    case BOOL:
        temp = BYTE;
        break;
    }

    std::string mipsData = component->getName() + ": ";

    // gets the mips code of the data type
    switch (temp) {
    case WORD:
        mipsData += ".word";
        break;
    case BYTE:
        mipsData += ".byte";
        break;
    }
    mipsData += " 0\n";

    res.push_back({temp, mipsData});
    
    return res;

}

visitRes dataVisitor::visitCommand(const AST::VarAssign* component) const
{
    return visitCommand(component->val);
}

visitRes dataVisitor::visitCommand(const AST::VarInit* component) const
{
    DataRes declRes = visitCommand((AST::VarDecl*)component).get<DataRes>();
    DataRes assignRes = visitCommand((AST::VarAssign*)component).get<DataRes>();

    declRes.insert(declRes.end(),
     std::make_move_iterator(assignRes.begin()),
     std::make_move_iterator(assignRes.end()));
    
    return declRes;
}

visitRes dataVisitor::visitCommand(const AST::If *component) const
{
    DataRes res = visitCommand(component->condition).get<DataRes>();

    //calling the data segment visitor to visit each command. 
    DataRes commandRes = component->scope->Accept(this).get<DataRes>();
    res.insert(res.end(), commandRes.begin(), commandRes.end());
    
    if(component->elif) { // visit else if
        DataRes commandRes = visitCommand(component->elif).get<DataRes>(); 
        res.insert(res.end(), commandRes.begin(), commandRes.end());
    } 

    if(component->after) { // visit after command
        DataRes commandRes = component->after->Accept(this).get<DataRes>();
        res.insert(res.end(), commandRes.begin(), commandRes.end());
    } 

    if(component->else_scope) { // visit else 
        DataRes commandRes = component->else_scope->Accept(this).get<DataRes>();
        res.insert(res.end(), commandRes.begin(), commandRes.end());
    }


    return res;
}

visitRes dataVisitor::visitCommand(const AST::While *component) const
{
    DataRes res = visitCommand(component->condition).get<DataRes>();

    //calling the data visitor to visit each command. 
    DataRes commandRes = component->scope->Accept(this).get<DataRes>();
    res.insert(res.end(), commandRes.begin(), commandRes.end());

    return res;
}

visitRes dataVisitor::visitCommand(const AST::Scope *component) const 
{
    component->moveScopeVars(INSERT);
    DataRes res;

    //calling the data visitor to visit each command. 
    for(AST::ICommand* command : component->_scopeCode) {
        DataRes commandRes = command->Accept(this).get<DataRes>();
        res.insert(res.end(), commandRes.begin(), commandRes.end());
    }

    component->moveScopeVars(REMOVE);
    return res;
}

visitRes dataVisitor::visitCommand(const AST::SysFunc* component) const
{
    DataRes res;
    for(const AST::Exp* i : component->getArgs())
    {
        DataRes temp = visitCommand(i).get<DataRes>();
        res.insert(res.begin(), temp.begin(), temp.end());   
    }
    if(!gotDataNum && annotationVisitor::numMaxHeight != 0)
    {
        gotDataNum = true;
        res.insert(res.begin(), {SPACE, " Nvars: .space " + std::to_string((annotationVisitor::numMaxHeight + 1) * WORD_SIZE) + "\n"}); // numeric expresion buffer
    }
    return res;
}

visitRes dataVisitor::visitCommand(const AST::Function *component) const
{
    for (AST::VarDecl* decl : component->params)
        decl->moveScopeVars(INSERT);    

    auto res = component->scope->Accept(this).get<DataRes>();

    for (AST::VarDecl* decl : component->params)
        decl->moveScopeVars(REMOVE);    

    return res; 
}

visitRes dataVisitor::visitCommand(const AST::Return *component) const
{
    return component->returned->Accept(this);
}

#include "textVisitor.h"
#include "SysFunc.h"
#include "Compiler.h"
#include "Annotations.h"
#include "commands_inc.h"

const AST::ICommand* textVisitor::afterCommand = nullptr;
bool textVisitor::highestIfFlag = true; 
bool textVisitor::expIsBool =    false;
int  textVisitor::condEndLabel =     0;
int  textVisitor::condLabel =        0;
int  textVisitor::loopLabel =        0;
int  textVisitor::whileLabel =       0;

visitRes textVisitor::visitCommand(const AST::Exp* component)  const
{
    Type type = Compiler::getAnnotation<AST::Exp_Annotation>(component->exp_id)->exp_type;
    std::string mips;
    
    TYPES action_type = type.pointerDepth ? INT : type.type;
    //init the pointer to the resulte buffer. 
    switch (action_type)
    {
    case STR:
        mips += "la $t2, res\n";
        break;
    case BOOL:
        mips +=  "la $t5, Bvars\n";
    
    case INT:
    case FLOAT:
        mips += "la $t0, Nvars \n";
        break;
    }

    AST::Exp_Tree::varPullBool = false;
    mips += component->getMips();

    return mips;

}

visitRes textVisitor::visitCommand(const AST::VarAssign* component)  const
{
    std::string mips;
    Id scope_id = Compiler::getAnnotation<AST::Var_Annotation>(component->id)->scopeId;
    if(AST::Var::vars[component->getName()].isGlobal)
    {
        mips = component->pointerDepthUsed ? "lw" : "la";
        mips += " $t1, " + component->getName() + "\n";
    }
    else 
    {
        std::string name = component->getName(); 
        std::string loc = std::to_string(AST::Scope::scopes[scope_id]->getStackLoc(name));
        mips += component->pointerDepthUsed ? "lw $t1, " + loc + "($sp)\n" : "addi $t1, $sp, " + loc + "\n"; 
    }
    
    for (int i = 1; i < component->pointerDepthUsed - 1; i++) // getting into the pointer depth -> *(*(*(...))) 
            mips += "lw $t1, ($t1)\n";

    TextRes res = visitCommand(component->val).get<TextRes>();
    TYPES type = component->getType().type;

    //loading var address into $t1
    res += mips 
    //assuming return register is $t0
                    + load(type)
                    + store(type) + " ($t1)\n\n";
    return res;
}

visitRes textVisitor::visitCommand(const AST::VarInit* component) const
{
    return visitCommand((AST::VarAssign*)component);
}

std::string textVisitor::buildIfAdditional(const AST::If *component, bool highest_flag) const
{
    std::string res_mips;
    if(component->elif || component->else_scope) { res_mips += "j end_label" + std::to_string(condEndLabel) + "\n"; }
    res_mips += "condition_label" +  std::to_string(condLabel++) + ":\n\n";
   

    if(component->elif) { res_mips += visitCommand(component->elif).get<TextRes>(); } // visit else if
    if(component->else_scope) { res_mips += visitCommand(component->else_scope).get<TextRes>(); } // visit else 

    highestIfFlag = highest_flag;
    // if the current if the the highest add the 'end' label to the mips. 
    if(highest_flag && (component->elif || component->else_scope)) { res_mips += "end_label" + std::to_string(condEndLabel++) + ":\n\n"; }

    if(component->after) afterCommand = component->after; 

    if(highest_flag) // visit the after command
        if (afterCommand) res_mips += afterCommand->Accept(this).get<TextRes>(); 
    
    return res_mips;
}

visitRes textVisitor::visitCommand(const AST::If *component) const
{
    TextRes res = visitCommand(component->condition).get<TextRes>();
    bool flag_tmp = highestIfFlag; 
    highestIfFlag = false;

    // load the condition resulte 
    TYPES type = Compiler::getAnnotation<AST::Exp_Annotation>(component->condition->exp_id)->exp_type.type;
    if(type == BOOL)
        res += "lb $t1, " BOOL_BUFFER_REG "\n";
    else { // in case the condition is a numeric exp, convert the resulte of the exp to boolean value (example: if (4 + 5) { ... } )
        res += "li $t1, 0x00\nlw $t2, " NUM_BUFFER_REG "\nbeq $t2, $zero, boolLabel" + std::to_string(AST::Exp_Tree::booLable++) + 
            "\nli $t1, 0x01\nboolLabel" + std::to_string(AST::Exp_Tree::booLable) + ":\n";
    }

    // the if command 
    res += "beq $t1, $zero, condition_label" + std::to_string(condLabel) + "\n";

    res += visitCommand(component->scope).get<TextRes>();

    res += buildIfAdditional(component, flag_tmp);
    return res;
}

visitRes textVisitor::visitCommand(const AST::While *component) const
{
    TextRes res = "";

    if (!(component->doFlag)) { res += "j while_label" + std::to_string(whileLabel) + "\n"; }
    res += "loop_start" + std::to_string(loopLabel) + ":\n";
    
    res += visitCommand(component->scope).get<TextRes>();
    
    if(!(component->doFlag)) res += "while_label" + std::to_string(whileLabel++) + ":\n";

    res += visitCommand(component->condition).get<TextRes>();

    // load the condition resulte 
    TYPES type = Compiler::getAnnotation<AST::Exp_Annotation>(component->condition->exp_id)->exp_type.type;
    if(type == BOOL)
        res += "lb $t1, " BOOL_BUFFER_REG "\n";
    else { // in case the condition is a numeric exp, convert the resulte of the exp to boolean value (example: if (4 + 5) { ... } )
        res += "li $t1, 0x00\nlw $t2, " NUM_BUFFER_REG "\nbeq $t2, $zero, boolLabel" + std::to_string(AST::Exp_Tree::booLable++) + 
            "\nli $t1, 0x01\nboolLabel" + std::to_string(AST::Exp_Tree::booLable) + ":\n";
    }
    
    res += "bne $t1, $zero, loop_start" + std::to_string(loopLabel++) + "\n\n";
    return res;
}

visitRes textVisitor::visitCommand(const AST::SysFunc *component) const
{
    TextRes text;
    const std::vector<AST::Exp*>& params = component->getArgs(); 
    const std::vector<std::string> argRegs = component->getArgRegs();
    const std::vector<TYPES> argTypes = component->getArgsTypes();
    int temp = component->numIndex, len = params.size();

    for (const AST::Exp* i : params)
    {
        component->numIndex++;
        text += i->Accept(this).get<TextRes>();
    }
    component->numIndex = temp;
    for (int i = 0; i < len; i++)
    {
        switch(argTypes[i])
        {
            case INT:
                text += "lw ";
                break;
            case FLOAT:
                text += "l.s ";
                break;
            default:
                throw std::runtime_error("error: invalid syscall arg type");
        }
        text += argRegs[i] + ", " + std::to_string((temp + i + 1) * WORD_SIZE) + NUM_BUFFER_REG  + "\n"; 
    }
    text += "li $v0, " + std::to_string(component->getSyscall()) + "\n";
    text += "syscall\n";
    if(component->returnsVal())
    {
        text += "la $t0, Nvars\n";
        switch(component->getType().type)
        {
            case INT:
                text += "sw $v0, ";
                break;
            case FLOAT:
                text += "s.s $f0, ";  
                break;
        }
        text += std::to_string(temp * WORD_SIZE)+ NUM_BUFFER_REG + "\n";
    }
    return text;
}

visitRes textVisitor::visitCommand(const AST::Scope *component) const
{
    component->moveScopeVars(INSERT);
    std::string res;
    int frame_size = Compiler::getAnnotation<AST::Scope_Annotation>(component->annotationId)->stackframeSize;
    res += "sub $sp, $sp, " + std::to_string(frame_size)+ "\n"; // create stackframe

    //calling the semantic checks visitor to visit each command. 
    for(AST::ICommand* command : component->_scopeCode) 
        res += command->Accept(this).get<TextRes>();


    res += "add $sp, $sp, " + std::to_string(frame_size)+ "\n"; // close stackframe

    component->moveScopeVars(REMOVE);
    return res;
}

visitRes textVisitor::visitCommand(const AST::Function *component) const
{
    for (AST::VarDecl* decl : component->params)
        decl->moveScopeVars(INSERT);    

    std::string res = (component->funcName == "main" ? "Main" : component->funcName) + ":\n";
    
    AST::Function::currentFunc = component->funcName;
    res += visitCommand(component->scope).get<TextRes>();
    res += "jr $ra\n"; // return to caller in case the user didn't write a return command
    AST::Function::currentFunc = "";

    for (AST::VarDecl* decl : component->params)
        decl->moveScopeVars(REMOVE);    
    return res;
}

visitRes textVisitor::visitCommand(const AST::Return *component) const
{
    std::string res = component->returned->Accept(this).get<TextRes>();

    const AST::Function* func = AST::Function::functions[AST::Function::currentFunc];

    int stack_pointer = 0;
    for(AST::VarDecl* var : func->params) // sum function params sizes
        stack_pointer += AST::Var::getSizeByType(var->getName());
    stack_pointer += AST::Call::align(stack_pointer); 

    Id scope_id = Compiler::getAnnotation<AST::Var_Annotation>(component->annotationId)->scopeId;
    while(scope_id != GLOBAL_SCOPE_ID) // sum all the stackframes of the scopes that the command is in 
    {
        stack_pointer += Compiler::getAnnotation<AST::Scope_Annotation>(AST::Scope::scopes[scope_id]->annotationId)->stackframeSize;
        scope_id =  Compiler::getAnnotation<AST::Scope_Annotation>(AST::Scope::scopes[scope_id]->annotationId)->parentScopeId;
    }

    TYPES mips_type = func->returnType.pointerDepth ? INT : Compiler::getAnnotation<AST::Exp_Annotation>(component->returned->exp_id)->exp_type.type;
    switch (mips_type)
    {
    case INT:
        res += "lw $t1, " NUM_BUFFER_REG "\n";
        if(func->returnType.type != BOOL || func->returnType.pointerDepth)  // load value as int (or addres)
            res += "sw $t1"; 
        else // load value as bool
            res += "li $t2, 0x00\nbeq $t1, $zero, boolLabel" + std::to_string(AST::Exp_Tree::booLable++) + "\nli $t2, 0x01\nboolLabel" + 
                std::to_string(AST::Exp_Tree::booLable) + ":\nsb $t2"; 
        break;
    case FLOAT: 
        res += "l.s $f0, " NUM_BUFFER_REG "\n";
        if(func->returnType.type != BOOL)  // load value as float 
            res += "s.s $f0"; 
        else  // load value as bool
            res += "li $t1, 0x00\nbeq $f0, $zero, boolLabel" + std::to_string(AST::Exp_Tree::booLable++) + "\nli $t1, 0x01\nboolLabel" + 
                std::to_string(AST::Exp_Tree::booLable) + ":\nsb $t1"; 
        break;
    case BOOL:
        res += "lb $t1, " BOOL_BUFFER_REG "\nsb $t1";
    default:
        break;
    }
    res += ", " + std::to_string(stack_pointer) + "($sp)\n";

    res += "jr $ra\n";
    return res;
}

std::string textVisitor::load(TYPES type)
{
    switch(type)
    {
        case FLOAT:
            return "l.s $f2," NUM_BUFFER_REG "\n";
        case INT:
            return "lw  $t2," NUM_BUFFER_REG "\n";
        case BOOL:
            return "lb  $t2," BOOL_BUFFER_REG "\n";
        default:
            throw std::runtime_error("error: unsupported type");
    }
}

std::string textVisitor::store(TYPES type)
{
    switch(type)
    {
        case FLOAT:
            return "s.s $f2,";
        case INT:
            return "sw $t2,";
        case BOOL:
            return "sb $t2,";
        default:
            throw std::runtime_error("error: unsupported type");
    }
}


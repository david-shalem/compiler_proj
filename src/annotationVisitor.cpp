#include "annotationVisitor.h"
#include "Compiler.h"
#include "Annotations.h"
#include "commands_inc.h"

int annotationVisitor::numMaxHeight = 0;
int annotationVisitor::boolMaxHeight = 0;
unsigned int annotationVisitor::nextStrName = 0;


visitRes annotationVisitor::visitCommand(const AST::Exp* component) const
{
    Type type = Compiler::getAnnotation<AST::Exp_Annotation>(component->exp_id)->exp_type;
    TYPES action_type = type.pointerDepth ? INT : type.type;
    switch (action_type)
    {
        case STR:
            //set the 'name' (name number, for examaple 3, for 'var3' in the mips code) of the string in the annotation map
            ((AST::Exp_String*)component)->setTmpStrName(nextStrName);
            break;
        case INT:
        case FLOAT: {
            //gets the depth of the exp tree. in case it's the dippest exp tree for now, the 'dippest tree' value will change to it for use in the data visitor  
            setNumMax(((AST::Exp_Tree*)component)->getHeight());
        }
        case BOOL: {
            int height = ((AST::Exp_Tree*)component)->getHeight();
            if (height > boolMaxHeight)
                boolMaxHeight = height;
            break;
        }
    }
    
    return visitRes();
}

visitRes annotationVisitor::visitCommand(const AST::VarDecl* component) const
{
    //for now nothing happends
    
    return visitRes();
}

visitRes annotationVisitor::visitCommand(const AST::VarAssign* component) const
{
    return visitCommand(component->val);
}

visitRes annotationVisitor::visitCommand(const AST::VarInit* component) const
{
    return visitCommand((AST::VarAssign*)component);
}

visitRes annotationVisitor::visitCommand(const AST::If *component) const
{
    visitCommand(component->condition);
    visitCommand(component->scope);

    if(component->elif) { visitCommand(component->elif); } // visit else if
    else if(component->else_scope) visitCommand(component->else_scope); // visit else 
    if(component->after) { component->after->Accept(this); } // visit after command

    return visitRes(); 
}

visitRes annotationVisitor::visitCommand(const AST::While *component) const
{
    visitCommand(component->condition);
    visitCommand(component->scope);
    
    return visitRes();
}

visitRes annotationVisitor::visitCommand(const AST::SysFunc *component) const
{
    auto& params = component->getArgs();
    for (const auto* i: params)
        visitCommand(i);
    setNumMax(component->getHeight());
    Compiler::getAnnotation<AST::Exp_Annotation>(component->exp_id)->exp_type = component->getType();
    return visitRes();
}

visitRes annotationVisitor::visitCommand(const AST::Scope *component) const
{
    component->moveScopeVars(INSERT);

    std::vector<AST::VarDecl*> declerations; 

    // getting all the var decleretions in the scope
    for(AST::ICommand* command : component->_scopeCode) {
        if(dynamic_cast<AST::VarDecl*>(command)) // if command is var decleration
            declerations.push_back(dynamic_cast<AST::VarDecl*>(command)); 
    }

    //sorting the var declerations by the type (first bool/char, than int/float)
    std::sort(declerations.begin(), declerations.end(), [](AST::VarDecl* a, AST::VarDecl* b)
        { return AST::Var::getSizeByType(a->getName()) < AST::Var::getSizeByType(b->getName()); });
    
    std::string data_seq_str;
    int stack_pointer = 0;

    auto ann = Compiler::getAnnotation<AST::Scope_Annotation>(component->annotationId);
    for(AST::VarDecl* var : declerations) {
        ann->stackframe[var->getName()] = stack_pointer;
        stack_pointer += AST::Var::getSizeByType(var->getName());
    }

    // in case the parent scope decreased the $sp to unaligned addres, change the stack frame size so the $sp will be aligned correctly in the stack frame creation
    Id parent_id = Compiler::getAnnotation<AST::Scope_Annotation>(component->annotationId)->parentScopeId;
    if(parent_id != GLOBAL_SCOPE_ID) {
        stack_pointer += AST::Call::align(Compiler::getAnnotation<AST::Scope_Annotation>(AST::Scope::scopes[parent_id]->annotationId)->stackframeSize);
    }

    Compiler::getAnnotation<AST::Scope_Annotation>(component->annotationId)->stackframeSize = stack_pointer;  

    for(AST::ICommand* command : component->_scopeCode) 
        command->Accept(this);
    
    component->moveScopeVars(REMOVE);

    return visitRes();
}

visitRes annotationVisitor::visitCommand(const AST::Function *component) const
{
    for (AST::VarDecl* decl : component->params)
        decl->moveScopeVars(INSERT);    

    visitCommand(component->scope); // anotate the scope 

    int params_size = 0;
    for(AST::VarDecl* var : component->params)
        params_size += AST::Var::getSizeByType(var->getName());

    auto scopeAnn = Compiler::getAnnotation<AST::Scope_Annotation>(component->scope->annotationId);
    int frame_pointer = scopeAnn->stackframeSize + AST::Call::align(params_size); // set the frame pointer to the 
        //start of the function parameters in the stack (edge of the scope stackframe + alignment space)

    // set the location of the function parameters in the scope to their location in the stack (actualy above the stackframe)
    for(AST::VarDecl* var : component->params) {
        scopeAnn->stackframe[var->getName()] = frame_pointer;
        frame_pointer += AST::Var::getSizeByType(var->getName());
    }

    for (AST::VarDecl* decl : component->params)
        decl->moveScopeVars(REMOVE);        
    return visitRes();
}

visitRes annotationVisitor::visitCommand(const AST::Return *component) const
{
    return component->returned->Accept(this);
}

void annotationVisitor::setNumMax(int new_hight)
{
    if (new_hight > numMaxHeight)
        numMaxHeight = new_hight;
}

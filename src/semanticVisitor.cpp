#include "semanticVisitor.h"
#include "Compiler.h"
#include "commands_inc.h"


visitRes semanticVisitor::visitCommand(const AST::Exp* component) const
{
    if(dynamic_cast<const AST::Exp_Binop*>(component))
        ((AST::Exp_Binop*)component)->checkForIlligalPointers();
        
    component->getType();
    return visitRes();
}

visitRes semanticVisitor::visitCommand(const AST::VarDecl* component) const
{
    try {
        checkTypeName(component->getName());
        throw std::runtime_error("error: cannot use a saved name (\'" + component->getName() + "\') to function");
    }
    catch (std::runtime_error& e) { } 

    return visitRes();
}

visitRes semanticVisitor::visitCommand(const AST::VarAssign* component) const
{
    visitCommand(component->val);
    Type var_t = AST::Var::vars[component->getName()].varT;

    if(component->pointerDepthUsed > var_t.pointerDepth)
         throw std::runtime_error("error: the pointer depth of the var \'" + component->getName() + "\' is " + 
                std::to_string(var_t.pointerDepth) + ", and cannot be called with the depth of " + std::to_string(component->pointerDepthUsed));

    Type exp_t = Compiler::getAnnotation<AST::Exp_Annotation>(component->val->exp_id)->exp_type;
    unsigned short reciver_pointer_depth = var_t.pointerDepth - component->pointerDepthUsed;
    if(!(reciver_pointer_depth * exp_t.pointerDepth)) { // var or/and val are not pointers
        if((exp_t.pointerDepth || reciver_pointer_depth) // if var is pointer and val is not or the opposite
        || (exp_t.type != var_t.type && var_t.type != BOOL)) // if type aren't matching
           throw std::runtime_error("error: types don't match");
        }
    else if(reciver_pointer_depth != exp_t.pointerDepth)
        throw std::runtime_error("error: pointer-depthes don't match");

    return visitRes();
}

visitRes semanticVisitor::visitCommand(const AST::VarInit* component) const
{
    return visitCommand((AST::VarAssign*)component);
}

visitRes semanticVisitor::visitCommand(const AST::If *component) const
{
    if(Compiler::getAnnotation<AST::Exp_Annotation>(component->condition->exp_id)->exp_type.pointerDepth)
        throw std::runtime_error("addres is not a condition dumbfuck");

    visitCommand(component->condition);
    visitCommand(component->scope);

    if(component->elif) { visitCommand(component->elif); } // visit else if
    if(component->after) { component->after->Accept(this); } // visit after command
    if(component->else_scope) { visitCommand(component->else_scope); } // visit else

    return visitRes();
}

visitRes semanticVisitor::visitCommand(const AST::While *component) const
{
    if(Compiler::getAnnotation<AST::Exp_Annotation>(component->condition->exp_id)->exp_type.pointerDepth)
        throw std::runtime_error("addres is not a condition dumbfuck");

    visitCommand(component->condition);
    visitCommand(component->scope);

    return visitRes();
}

visitRes semanticVisitor::visitCommand(const AST::Scope *component) const 
{
    component->moveScopeVars(INSERT);

    for(AST::ICommand* command : component->_scopeCode)
        command->Accept(this);
        
    component->moveScopeVars(REMOVE);
    return visitRes();
}

visitRes semanticVisitor::visitCommand(const AST::Function* component) const
{
    for (AST::VarDecl* decl : component->params)
        decl->moveScopeVars(INSERT);    

    try {
        checkTypeName(component->funcName);
        throw std::runtime_error("error: cannot use a saved name (\'" + component->funcName + "\') to function");
    }
    catch (std::runtime_error& e) { } 

    visitCommand(component->scope);

    for(AST::VarDecl* var : component->params)  
        visitCommand(var);

    for (AST::VarDecl* decl : component->params)
        decl->moveScopeVars(REMOVE);    

    return visitRes();
}

visitRes semanticVisitor::visitCommand(const AST::Return *component) const
{
    return component->returned->Accept(this);
}

TYPES semanticVisitor::checkTypeName(std::string type_name) 
{
    if( type_name == "int")
        return INT;
    else if (type_name == "float")
        return FLOAT;
    else if(type_name == "string")
        return STR;
    else if(type_name == "bool")
        return BOOL;
    else
        throw std::runtime_error(("error: invalid type \'" + type_name + "\'").c_str());
}


visitRes semanticVisitor::visitCommand(const AST::SysFunc *component) const
{
    const std::vector<AST::Exp*>& params = component->getArgs(); 
    const std::vector<TYPES> argTypes = component->getArgsTypes();
    if(params.size() != argTypes.size())
        throw std::runtime_error("error: invalid number of argumnets for syscall");
    int len = params.size();
    for(int i = 0; i < len; i++)
    {
        params[i]->Accept(this);
		Type type = params[i]->getType();
        if(type.type != argTypes[i] || type.pointerDepth > 0)
            throw std::runtime_error("error: incompatible types in " + std::to_string(i) + "th argument of syscall");
    }
    return visitRes();
}

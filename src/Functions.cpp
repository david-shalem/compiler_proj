#include "Functions.h"
#include "Var.h"
#include "semanticVisitor.h"
#include "Compiler.h"

namespace AST
{

std::string Function::currentFunc = "";
std::map<std::string, const Function*> Function::functions; 

Function::Function(const Scope* code, std::vector<VarDecl*> parameters, const Type& return_type, const std::string name) :
    scope(code), funcName(name), returnType(return_type)
{
    if(functions.find(funcName) != functions.end()) // check if the name of the function is allready used
        throw std::runtime_error("cannot use the name \'" + funcName + "\' for two differnce functions"); 

    { // inserting the parameters in the right order to the 'params' vector, and initilaizing the 'paramsOrder' vector
    for(int i = 0; i < parameters.size(); i++) paramsOrder.push_back(i); // init the paramsOrder

    std::sort(paramsOrder.begin(), paramsOrder.end(), [&](int a, int b){ // sorting the paramsOrder vector by the sizes of the params
        return Var::getSizeByType(parameters[a]->getName()) < 
            Var::getSizeByType(parameters[b]->getName()); });

    for(int param_loc : paramsOrder) // inserting the parameters
        params.push_back(parameters[param_loc]);
    }

    // inserting the parameters to the scope  
    auto& stackframe = Compiler::getAnnotation<Scope_Annotation>(scope->annotationId)->stackframe;
    for(VarDecl* var : params)
        stackframe[var->getName()] = Var::getSizeByType(var->getName());

    functions[name] = this;
    
    for (VarDecl* decl : params)
        decl->moveScopeVars(REMOVE);    
    
}

void Function::insertAndCheckScopeId(Id scope_id) const
{
    for (VarDecl* decl : params)
        decl->moveScopeVars(INSERT);    

    if(scope_id != GLOBAL_SCOPE_ID)
        throw std::runtime_error("all functions must be defined globaly!!");

    currentFunc = funcName;
    scope->insertAndCheckScopeId(scope_id); 
    currentFunc = "";

    for (VarDecl* decl : params)
        decl->moveScopeVars(REMOVE);    
}

}
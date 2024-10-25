#include "Scopes.h"
#include "Compiler.h"

namespace AST
{

std::map<Id, const Scope*> Scope::scopes; 
Id Scope::scopeIds = 0;

void Scope::insertAndCheckScopeId(Id scope_id) const
{
    Id scopeId = ++scopeIds;
    scopes[scopeId] = this;
    Compiler::getAnnotation<Scope_Annotation>(annotationId)->parentScopeId = scope_id;

    //init the scope id in the commands in the scope. 
    for(ICommand* command : _scopeCode)
        command->insertAndCheckScopeId(scopeId);

    moveScopeVars(REMOVE);
}

void Scope::moveScopeVars(bool in_out) const
{
    for(ICommand* command : _scopeCode)
        command->moveScopeVars(in_out);
}


// void Scope::searchVar(std::string varName) const
// {
//     Scope_Annotation* ann = Compiler::getAnnotation<Scope_Annotation>(annotationId);
//     std::map<std::string, unsigned int> stack_frame = ann->stackframe;
//     if(stack_frame.find(varName) == stack_frame.end()) // if the var is not declered in the scope  
//     {
//         if(ann->parentScopeId == GLOBAL_SCOPE_ID) // if the scope does not have a parent scope 
//             throw std::runtime_error("var \'" + varName + "\' is undefined");
        
//         scopes[ann->parentScopeId]->searchVar(varName); 
//     }
// }

int Scope::getStackLoc(const std::string& varName) const
{
    Scope_Annotation* ann = Compiler::getAnnotation<Scope_Annotation>(annotationId);
    if(ann->stackframe.find(varName) == ann->stackframe.end()) // if the var is not declered in the scope  
    {
        return ann->stackframeSize + scopes[ann->parentScopeId]->getStackLoc(varName);
    }
    else 
        return ann->stackframe[varName];
}

}
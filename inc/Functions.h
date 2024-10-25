#pragma once
#include "AST.h"
#include "Scopes.h"
#include "Var.h"

namespace AST
{

class Function : public ICommand
{
public:
    visitRes Accept(const IVisitor* visitor)  const override { return visitor->visitCommand(this); }

public:
    Function(const Scope* code, std::vector<VarDecl*> parameters, const Type& return_type, const std::string name);
    
    void insertAndCheckScopeId(Id scope_id) const override;


    const Scope* scope;
    std::vector<VarDecl*> params; 
    const Type returnType;
    const std::string funcName; 

    /* to avoid aligning in the parameters stored in the stack, they are need to be stored in a difference order from the order 
       in the function definition. the vector hold the right order of the parameters. 
       example: foo(int 1, bool 2, int 3) -> paramsOrder = {2, 1, 3}. (and so the second param will be first in the stack, and than
       the first, and than the third) */ 
    std::vector<int> paramsOrder; 
    
    static std::string currentFunc;
    static std::map<std::string /*function name*/, const Function* /*function object*/> functions; 
};

}
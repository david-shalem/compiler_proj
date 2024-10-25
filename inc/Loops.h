#pragma once
#include "AST.h"
#include "Scopes.h"

namespace AST
{

class While : public ICommand
{
public:
    visitRes Accept(const IVisitor* visitor)  const override { return visitor->visitCommand(this); }

public: 
    While (const Exp* cond, const Scope* scope_code, bool doWhileFlag = false) : condition(cond), scope(scope_code), doFlag(doWhileFlag) {}

    void moveScopeVars(bool in_out) const override { scope->moveScopeVars(in_out); }

    void insertAndCheckScopeId(Id scope_id) const override 
        { scope->insertAndCheckScopeId(scope_id); condition->insertAndCheckScopeId(scope_id); }

    const Scope* scope;
    const Exp* condition; 
    const bool doFlag; 

};

}
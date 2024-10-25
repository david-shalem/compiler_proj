#pragma once
#include "AST.h"
#include "Scopes.h"

namespace AST
{


/*
for some reason if there was a command after if without else, the parser could not read her and would throw an error. to avoid it the if takes the 
first command after him and do the visits for her. 3th c'tor recive the command, and it is called the 'after command'
*/

class If : public ICommand
{
public:
    visitRes Accept(const IVisitor* visitor)  const override { return visitor->visitCommand(this); }

public: 

    // c'tor for: if, if + else if, if + after command, if else. in that order 
    If (const Exp* cond, const Scope* scope_code) : condition(cond), scope(scope_code) {}
    If (const Exp* cond, const Scope* scope_code, const If* else_if) : condition(cond), scope(scope_code), elif(else_if) {}
    If (const Exp* cond, const Scope* scope_code, const ICommand* after_command) : condition(cond), scope(scope_code), after(after_command) {}
    If (const Exp* cond, const Scope* scope_code, const Scope* else_scope) : condition(cond), scope(scope_code), else_scope(else_scope) {}
    
    void insertAndCheckScopeId(Id scope_id) const override;
    void moveScopeVars(bool in_out) const override { scope->moveScopeVars(in_out); }

    const Scope* scope;
    const Exp* condition; 

    const If* elif = nullptr;
    const Scope* else_scope = nullptr;
    const ICommand* after = nullptr;
};

}
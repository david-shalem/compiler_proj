#include "Conditions.h"
#include "Exp.h"

namespace AST  
{

void If::insertAndCheckScopeId(Id scope_id) const
{
    scope->insertAndCheckScopeId(scope_id); 
    condition->insertAndCheckScopeId(scope_id);

    if(elif) { elif->insertAndCheckScopeId(scope_id); }
    else if (else_scope) { else_scope->insertAndCheckScopeId(scope_id); }
    else if (after) {after->insertAndCheckScopeId(scope_id); }
}

}
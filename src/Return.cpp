#include "Return.h"
#include "Functions.h"
#include "Compiler.h"
#include "Exp.h"
namespace AST
{

    void Return::insertAndCheckScopeId(Id scope_id) const
    {
        Compiler::getAnnotation<Var_Annotation>(annotationId)->scopeId = scope_id;
        if(Function::currentFunc == "")
            throw std::runtime_error("return command cannot be used out side of functions");

        returned->insertAndCheckScopeId(scope_id);
    }

}
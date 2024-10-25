#pragma once
#include "structs.h"
#include "visitorRes.h"
#include <iostream>

/* The main visitors file */

#define ALIGN_SIZE 4

namespace AST{
class Exp;
class VarDecl;
class VarAssign;
class VarInit;
class If;
class While;
class SysFunc;
class Scope;
class Function;
class Return;
}

/// @brief abstract class for the visitors
class IVisitor
{
private:

public:
    virtual visitRes visitCommand(const AST::Exp* component) const = 0; 
    virtual visitRes visitCommand(const AST::VarDecl* component) const = 0; 
    virtual visitRes visitCommand(const AST::VarAssign* component) const = 0;
    virtual visitRes visitCommand(const AST::VarInit* component) const = 0;
    virtual visitRes visitCommand(const AST::If* component) const = 0;
    virtual visitRes visitCommand(const AST::While* component) const = 0;
    virtual visitRes visitCommand(const AST::SysFunc* component) const = 0;
    virtual visitRes visitCommand(const AST::Scope* component) const = 0;
    virtual visitRes visitCommand(const AST::Function* component) const = 0;
    virtual visitRes visitCommand(const AST::Return* component) const = 0;
 
};
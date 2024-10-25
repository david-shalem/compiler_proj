#pragma once
#include "Visitor.h"
#include "Annotations.h"

/* visitor class -> take care of all the annotation (returns nothing. check for errors and other illegal aspects in the user's code)*/

class semanticVisitor : public IVisitor
{
private:

public:
    visitRes visitCommand(const AST::Exp* component) const override ; 
    visitRes visitCommand(const AST::VarDecl* component) const override;
    visitRes visitCommand(const AST::VarAssign* component) const override;
    visitRes visitCommand(const AST::VarInit* component) const override;
    visitRes visitCommand(const AST::If* component) const override;
    visitRes visitCommand(const AST::While* component) const override;
    visitRes visitCommand(const AST::Scope* component) const override;
    visitRes visitCommand(const AST::Function* component) const override;
    visitRes visitCommand(const AST::Return* component) const override;
    visitRes visitCommand(const AST::SysFunc* component) const override;
    /*--- static variables for future use ---*/

    static TYPES checkTypeName(std::string type_name);
    /*---------------------------------------*/
};

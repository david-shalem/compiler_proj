#include "Visitor.h"

/* visitor class -> take care of all the annotation (returns nothing. collect necessary information for the compilation procces)*/

class annotationVisitor : public IVisitor
{
private:

public:
    //visits
    visitRes visitCommand(const AST::Exp* component) const override; 
    visitRes visitCommand(const AST::VarDecl* component) const override;
    visitRes visitCommand(const AST::VarAssign* component) const override;
    visitRes visitCommand(const AST::VarInit* component) const override;
    visitRes visitCommand(const AST::If* component) const override;
    visitRes visitCommand(const AST::While* component) const override;
    visitRes visitCommand(const AST::SysFunc* component) const override;
    visitRes visitCommand(const AST::Scope* component) const override;
    visitRes visitCommand(const AST::Function* component) const override;
    visitRes visitCommand(const AST::Return* component) const override;

    /*--- static variables for future use ---*/
    /* Expressions */

    /// @brief The maximum height of all Exp_Tree
    static int numMaxHeight; // for numeric expressions
    static int boolMaxHeight; // for boolean expressions
    static void setNumMax(int new_hight);

     /// @brief The current smallest str name not in use
    static unsigned int nextStrName;
    
    /*---------------------------------------*/
};

#include "Visitor.h"

/* visitor class -> take care of all the data segment mips (returns the mips of the data segment that is required for the command)*/

class dataVisitor : public IVisitor
{
private:

public:
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

    /// @brief Checks if data was already requested
    static bool gotDataNum;
    static bool gotDataStr;
    static bool gotDataBool;
    
    /*---------------------------------------*/
};
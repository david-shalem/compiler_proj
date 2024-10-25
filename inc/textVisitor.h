#pragma once
#include "Visitor.h"
#include "Annotations.h"
#include "AST.h"
/* visitor class -> take care of all the text segment mips (returns the mips code of the commands actions)*/


class textVisitor : public IVisitor
{
private:

public:
    visitRes visitCommand(const AST::Exp* component) const override; 
    visitRes visitCommand(const AST::VarDecl* component) const override { return std::string(); }
    visitRes visitCommand(const AST::VarAssign* component) const override;
    visitRes visitCommand(const AST::VarInit* component) const override;
    visitRes visitCommand(const AST::If* component) const override;
    visitRes visitCommand(const AST::While* component) const override;
    visitRes visitCommand(const AST::Scope* component) const override;
    visitRes visitCommand(const AST::Function* component) const override;
    visitRes visitCommand(const AST::Return* component) const override;
    visitRes visitCommand(const AST::SysFunc* component) const override;

    /*--- static variables for future use ---*/
    static bool expIsBool; 

    static const AST::ICommand* afterCommand;
    static bool highestIfFlag; // indicate whether the current if command is the top if command (if) or lower (else if)
    static int condLabel; // the last number of condition label (works like boolLabel in 'exp tree')
    static int condEndLabel; // the last number of condition end label (works like boolLabel in 'exp tree')

    static int loopLabel; // the last number of loop label (works like boolLabel in 'exp tree')
    static int whileLabel; // the last number of while label (works like boolLabel in 'exp tree')
    /*---------------------------------------*/
private:
    static std::string load(TYPES type);
    static std::string store(TYPES type);

    /// @brief function create the mips code of the additional part of the if command (else if, else and after command)
    /// @param component the command object 
    /// @param highest_flag top if flag (tmp of 'highestIfFlag')
    /// @return the resulte mips code
    std::string buildIfAdditional(const AST::If* component, bool highest_flag) const;
};

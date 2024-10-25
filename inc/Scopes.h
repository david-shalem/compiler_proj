#pragma once
#include "AST.h"

#define EMPTY -1

namespace AST
{

typedef std::vector<AST::ICommand*> commands;

/// @brief scopes class (functions, loop, conditions, and just scopes)
class Scope : public ICommand
{
    public:
        visitRes Accept(const IVisitor* visitor)  const override { return visitor->visitCommand(this); }
        unsigned long annotationId; 

    public:
        Scope(commands code, Id id) : _scopeCode(code), annotationId(id) { }

        /// @brief function search for var in its scope, and in its parent scope 
        /// @param varName the var to search 
        // void searchVar(std::string varName) const; 
        int getStackLoc(const std::string& varName) const;    

        void insertAndCheckScopeId(Id scope_id) const override;
        void moveScopeVars(bool in_out) const override;

        commands _scopeCode;

        static std::map<Id, const Scope*> scopes; 
        static Id scopeIds;
};


}
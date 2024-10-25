#pragma once
#include "AST.h"

/* file contains the variables and assinments classes*/

#define EXP_TYPE_INDEX 0

namespace AST
{


/// @brief Abstract class for variable commands
class Var : public ICommand
{
    public:
    
    //variable data getters
    std::string getName() const { return name; }
    Type getType() const;

    static std::map<std::string, VarData> vars;

    static int getSizeByType(const std::string& name);
    

    Id id;
    
    protected:
    Var(const Type& vtype, const std::string& vname);
    Var(const std::string& name, Id annotation_id);
    virtual ~Var() = default;

    std::string name;
    Type type;
};

/// @brief variable declare class
class VarDecl : virtual public Var
{
    public: 
        //visitor function
        visitRes Accept(const IVisitor* visitor)  const override { return visitor->visitCommand(this); }
    public:
    /// @param type the string representing the type of the var
    /// @param name the name of the var
    VarDecl(const Type &type, const std::string& name, Id annotation_id);
    virtual ~VarDecl() = default;

    /// @brief function recive the scope id of the variable action, save it, and check if the var exist or not in the scope  
    void insertAndCheckScopeId(Id scope_id) const override;
    void moveScopeVars(bool in_out) const override; 
    
};

/// @brief variable assignment class
class VarAssign : virtual public Var
{
    public:
        visitRes Accept(const IVisitor* visitor) const override  {return visitor->visitCommand(this); }
    public:
    VarAssign(const std::string& name, const Exp* val, unsigned short pointerDepth, Id annotation_id);
    virtual ~VarAssign();
    
    /// @brief function recive the scope id of the variable action, save it, and check if the var exist or not in the scope  
    void insertAndCheckScopeId(Id scope_id) const override;

    const Exp* val;
    unsigned short pointerDepthUsed; // whether the reciver is pointer that gets a value (*pointer = exp)
};

class VarInit : public VarDecl, public VarAssign
{
    public:
        visitRes Accept(const IVisitor* visitor) const override  {return visitor->visitCommand(this); }
    public:
    VarInit(const Type &type, const std::string& name, const Exp* val, Id annotation_id);
    virtual ~VarInit() = default;

    /// @brief function recive the scope id of the variable action, save it, and check if the var exist or not in the scope  
    void insertAndCheckScopeId(Id scope_id) const override;
};

}
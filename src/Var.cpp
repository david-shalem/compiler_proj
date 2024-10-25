#include "Var.h"
#include "Exp.h"
#include "Scopes.h"
#include "Compiler.h"
#include "semanticVisitor.h"

namespace AST
{

std::map<std::string, VarData> Var::vars;

Type Var::getType() const
{
    return type; 
}

/// @brief function return the size of var in the memory
int Var::getSizeByType(const std::string& name)
{
    if(!vars[name].varT.pointerDepth)
        return ADDRES_SIZE;

    switch (vars[name].varT.type)
    {
    case INT:
    case FLOAT:
        return WORD_SIZE;
    case BOOL:
        return BYTE_SIZE;
    default:
        return 0;
    }
}

Var::Var(const Type &vtype, const std::string &vname) : name(vname), type(vtype) { }

Var::Var(const std::string& name, Id annotation_id): name(name), id(annotation_id), type(vars[name].varT) { }

VarDecl::VarDecl(const Type &type, const std::string& name, Id annotation_id) : Var(type, name) { 
    Compiler::getAnnotation<Var_Annotation>(annotation_id)->varInfo = {type, false};
    id = annotation_id;

    if (vars.find(name) != vars.end())
        throw std::runtime_error("error: var: \'" + name + "\' already exists");
    else 
        vars[name] = {type, false};    
}

void VarDecl::insertAndCheckScopeId(Id scope_id) const
{
    auto annotation = Compiler::getAnnotation<Var_Annotation>(id);
    annotation->scopeId = scope_id;

    if(scope_id == GLOBAL_SCOPE_ID)
        annotation->varInfo.isGlobal = true;
    else
        Compiler::getAnnotation<AST::Scope_Annotation>(Scope::scopes[scope_id]->annotationId)->stackframe[name] = EMPTY; // insert into vars of the scope 

    vars[name] = annotation->varInfo;

}

void VarDecl::moveScopeVars(bool in_out) const
{
    if(in_out == REMOVE)
        vars.erase(name);
    else 
        vars[name] = Compiler::getAnnotation<Var_Annotation>(id)->varInfo;
}

VarAssign::VarAssign(const std::string& name, const Exp* val_exp, unsigned short pointerDepth, Id annotation_id) : 
    Var(name, annotation_id), val(val_exp), pointerDepthUsed(pointerDepth) {}

void VarAssign::insertAndCheckScopeId(Id scope_id) const
{
    Compiler::getAnnotation<Var_Annotation>(id)->scopeId = scope_id;

    if(vars.find(name) == vars.end())
        throw std::runtime_error("var \'" + name + "\' is undefined");

    val->insertAndCheckScopeId(scope_id); // insert the id to the expression 
}

VarAssign::~VarAssign()
{
    delete val;
}

VarInit::VarInit(const Type &type, const std::string& name, const Exp* val, Id annotation_id): 
    Var(type, name), VarDecl(type, name, annotation_id), VarAssign(name, val, false, annotation_id) { 
    }

void VarInit::insertAndCheckScopeId(Id scope_id) const
{
    VarDecl::insertAndCheckScopeId(scope_id);
    this->val->insertAndCheckScopeId(scope_id);
}

}
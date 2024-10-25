#pragma once
#include <map>
#include "structs.h"

namespace AST
{

/// @brief General interface for all annotations
struct Annotation_data
{
};

/// @brief Map containing all ast annotations
class anMap : public std::map<unsigned long, AST::Annotation_data*>
{
    public:
        ~anMap()
        {
            for(auto data : *this)
            {
                delete data.second;
            }
        }
};

/// @brief Annotation for scopes
class Scope_Annotation : public Annotation_data {
    public:
    unsigned int stackframeSize;
    Id parentScopeId;
    std::map<std::string /*var name*/, unsigned int /*var location in the stackframe*/> stackframe;
};

class Var_Annotation : public Annotation_data {
public:
/// @brief the id of the scope that the var is in
Id scopeId;
/// @brief the type and the globalizasion of the var
VarData varInfo;
};

/// @brief Annotation for mathematical expression
class Exp_Annotation : public Annotation_data {
    public:

    /// @param t the type of value the expression contains
    Exp_Annotation(Type t):exp_type(t) {}

    /// @brief Default c'tor - assumes type is int
    Exp_Annotation():exp_type({INT, 0}) {}

    /// @brief the type of value the expression contains
    Type exp_type;
};

class Exp_Var_Annotation : public Exp_Annotation
{
public:
/// @brief the id of the scope that the var is in
Id scopeId;
};



/// @brief Annotation for string expression
class String_Annotation : public Exp_Annotation {
    public:

    /// @param num the id assigned to the string
    String_Annotation(unsigned int num):Exp_Annotation({STR, 0}), str_num(num) {} 

    /// @brief Default c'tor - assumes id is 0
    String_Annotation():Exp_Annotation({STR, 0}), str_num(0) {}
    
    /// @brief the id assigned to the string
    unsigned int str_num;
};

/// @brief Annotation for binary operation
class Binop_Annotation : public Exp_Annotation {
    public:

    /// @param t the type of value the binop returns
    /// @param side the longer side of the branch (true = left, false = right)
    Binop_Annotation(Type t, bool side):Exp_Annotation(t), biggest_side(side) {} 

    /// @brief Default c'tor, assumes type = int and longest side = left
    Binop_Annotation():biggest_side(true) {}

    bool biggest_side;
};

}



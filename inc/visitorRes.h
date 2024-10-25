#pragma once
#include "structs.h"
#include <vector>
#include <string>
#include <variant>

typedef std::string TextRes;
typedef std::vector<data> DataRes;
typedef std::variant<DataRes, TextRes> ResVariant;


/// @brief the return value of the visitors
class visitRes : private ResVariant
{
    public:
    template<class T>
    visitRes(const T& res) : ResVariant(res) {}
    visitRes() = default;
    
    template<class T>
    T& get()
    {
        return std::get<T>(*this);
    }
};


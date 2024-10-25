#pragma once
#include <iostream>
#include <vector>

#define FIND_IN_LIST(l, n) ( std::find(l.begin(), l.end(), n) == l.end() )

/* file contains all the main data structurs of the system */


/// @brief Supported types in the compiler
enum TYPES 
{
    INT = 1,
    FLOAT = 2,
    BOOL = 3,
    STR = 4,
    VOID = -1
};


//data types of vars in the data segment 
enum dataType {
    SPACE,
    WORD,
    ASCII,
    BYTE
};

//data segment mips code and the data type of in it (as enum)
struct data {
    dataType type;
    std::string mips;
};

//type of variable 
struct Type
{
    TYPES type; 
    unsigned short pointerDepth; 
}; // example: int*** -> {type = int, pointerDepth = 3} 


struct VarData
{
    Type varT;
    bool isGlobal;
};

typedef unsigned int Id;
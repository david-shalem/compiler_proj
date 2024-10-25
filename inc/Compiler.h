#ifndef __COMPILER_H__
#define __COMPILER_H__

/*************************/
/*                       */
/* general include files */
/*                       */
/*************************/
#include <stdio.h>
#include <iostream>
#include <map>

/*************************/
/*                       */
/* project include files */
/*                       */
/*************************/
#include "AST.h"

/************/
/*          */
/* Compiler */
/*          */
/************/



/// @brief contains all compiling processes
class Compiler {
private:

    // source code filename
    const char *input;

    // machine code filename
    const char *output;

    // The commands of the input code 
    std::vector<AST::ICommand*> commands; 
    
    /// @brief The mips code generated from the ast 
    std::string mips;

    /// @brief A flag that is triggered when the compiler runs into an error
    bool error_flag = false;

    /// @brief The map that contains all the annotations for the ast
    static AST::anMap annotation_map;

private:

    /// @brief Converts the input file to an ast
    /// @return success = true, error = false
    bool /* step 1 */src_to_AST();

    /// @brief runs semantic tests on the ast
    /// @return Whether the ast is semantically valid
    bool /* step 2 */semantic_checks_on_AST();

    /// @brief Stores annations for the ast in the annotation_map
    void /* step 3 */annotate_the_AST();

    /// @brief Generates mips assembly from the ast
    void /* step 4 */AST_to_IR();

    /// @brief Converts the mips assembly to an ELF file 
    void /* step 5 */IR_to_ELF();

    /// @brief Build the data segment code 
    /// @return the data segment code
    std::string getDataMips();

    int functions_amount;

public:
    /// @param i input file name
    /// @param o output file name
    Compiler(
        const char *i,
        const char *o):
        input(i),output(o){}
     
    ~Compiler()
    {
        for(AST::ICommand* command_ptr : commands)
            delete command_ptr;
    }

public:
    /// @brief runs all compilation processes
    /// @return success = true, error = false
    bool compile();

    /// @brief gets an annotation from the annotation map
    /// @tparam T The type of the annotation
    /// @param id The id of the annotation
    /// @return The annotation from the map
    template <class T>
    static T* getAnnotation(unsigned long id)
    {
        if(annotation_map[id] == nullptr)
            annotation_map[id] = new T;
        return (T*)(annotation_map[id]);
    }
};

#endif

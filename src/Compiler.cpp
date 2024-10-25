/*************************/
/*                       */
/* general include files */
/*                       */
/*************************/
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
/*************************/
/*                       */
/* project include files */
/*                       */
/*************************/
#include "AST.h"
#include "Compiler.h"
#include "ParsingContext.h"

//visitors
#include "annotationVisitor.h"
#include "semanticVisitor.h"
#include "dataVisitor.h"
#include "textVisitor.h"

#define MEXIT "li $v0, 10\nsyscall\n"
#define MSTRCAT "cpy_str:\n#$t0 = inStr\n#$t2 = dest\n\nli $t3, 0\nloop:\n\nlb $t3, ($t0)\nbeq $t3, $zero, end_cp_str\nsb $t3, ($t2)\n\naddi $t0, $t0, 1\naddi $t2, $t2, 1\nj loop\n\nend_cp_str:\njr $ra\n\n"
#define DEF_NEWLINE "newline: .asciiz \"\\n\"\n"


AST::anMap Compiler::annotation_map; 

bool Compiler::src_to_AST()
{
    ParsingContext ctx(input);

    //the parsing. in case of errors in the c'tors of the commands, they'll throw an error that will be catched here. 
    try
    {
        this->commands = ctx.parse();
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
  
    std::vector<AST::ICommand*> functions;
    for (int i = 0; i < commands.size(); i) { // collecting all the functions in the code
        if(dynamic_cast<const AST::Function*>(commands[i])) {
            functions.push_back(commands[i]);
            commands.erase(commands.begin() + i);
        }
        else 
            i++;
    }

    functions_amount = functions.size();
    commands.insert(commands.begin(), functions.begin(), functions.end()); // puting all the functions at the start of the commands 

    return this->commands.size() != 0;
}

bool Compiler::semantic_checks_on_AST()
{
    semanticVisitor visitor;
    
    //calling the semantic checks visitor to visit each command and inserting them the scopes-ids.
    // in case of errors found, they'll be thrown, and catched here 
    try {
        for(AST::ICommand* command : commands) // inserting the scope-ids to the commands, and checking them 
            command->insertAndCheckScopeId(GLOBAL_SCOPE_ID); 

        for(AST::ICommand* command : commands)
            command->Accept(&visitor);
    }
    catch(const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        return false;
    }
    
    return true;
}

void Compiler::annotate_the_AST()
{
    annotationVisitor visitor; 

      //calling the annotations visitor to visit each command. 
    for(AST::ICommand* command : commands)
            command->Accept(&visitor);

    return void(); 
}

std::string Compiler::getDataMips()
{
    dataVisitor visitor; 

    //geting all the data lines from the commands by visit them with the data visitor
    std::vector<data> data_seq; 
    for(AST::ICommand* command : commands) {
        std::vector<data> curr = command->Accept(&visitor).get<DataRes>(); 
        data_seq.insert(
            data_seq.end(),
            std::make_move_iterator(curr.begin()),
            std::make_move_iterator(curr.end()));
    }

    //sorting the data lines by their type to avoid using '.align' and memory waste 
    std::sort(data_seq.begin(), data_seq.end(), [](data& a, data& b){ return a.type<b.type; });
    std::string data_seq_str;

    //summing all the resultes to one string
    for(data& data_c : data_seq)
        data_seq_str += data_c.mips;

    return data_seq_str;
}

void Compiler::AST_to_IR()
{
    textVisitor visitor;

    //creating the data segment
    mips = ".data\n"
            + getDataMips() +
            DEF_NEWLINE
           ".text\n"
           " main:j start\n";

    //visiting eahc command with the text visitor and collecting the mips code
    for(AST::ICommand* command : commands) {
        if(functions_amount == 0) { mips += "start:\n"; } // add start of global code label
        TextRes curr = command->Accept(&visitor).get<TextRes>();
        mips += curr;
        functions_amount--;
    }

    if(functions_amount == 0) { mips += "start:\n"; } // in case there is no global code, add start label to the unglobal starting point
    mips += "j Main\n"; // add start of unglobal code label
    mips += MEXIT MSTRCAT; //defaulte systme functions and exit
}

/***********/
/*         */
/* compile */
/*         */
/***********/
bool Compiler::compile()
{ 
    //converting the input code to vector of commands
    if (src_to_AST() == false) {
        std::cout << "\n\nFAILED ON SRC -> AST CONVERTION\n\n";
        return false; 
        }
 
    //applying semantic check on the commands
    if (semantic_checks_on_AST() == false) { 
        std::cout << "\n\nFAILED AT SEMANTIC CHECKS\n\n"; 
        return false; 
        }

    //annotating the commands
    annotate_the_AST();

    //converting the commands to mips code
    AST_to_IR();
    
    std::ofstream fOutput(output, std::ofstream::out);
    if(fOutput.is_open())
    {
        fOutput << "#!/bin/spim -file\n"
                << mips;
        fOutput.close();
        //make file executable and readable
        chmod(output, S_IWUSR | S_IRUSR | S_IXUSR | S_IXOTH | S_IROTH);
        std::cout << "SUCCESS\n";
    }
    else
    {
        std::cerr << "failed to write to file " << output << "\n";
    }
    
    
    return true;
}

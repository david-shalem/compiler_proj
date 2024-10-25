#pragma once
#include "Exp.h"

namespace AST
{
    
/// @brief a class that handles syscalls
class SysFunc : public Exp_Tree
{
    public:

    /// @param id the id of the syscall in the annotations map, currently not used
    /// @param func the string containing the name of the syscall
    /// @param params a list of the parameters passed to the syscall
    SysFunc(unsigned long id, const std::string& func, const std::vector<Exp*>& params);

    visitRes Accept(const IVisitor *visitor) const;
    
    /// @return function gets the type returned by the syscall
    Type getType() const override;

    /// @return functions gets the mips code generated for the syscall
    std::string getMips() const override;
    
    /// @brief function gets the number of words needed for evaluating the syscall
    int getHeight() const override;
    
    /// @brief function gets the registers which the arguments are passed throgh
    std::vector<std::string> getArgRegs() const;

    /// @return a refrence to the vector of arguments stored in the class
    const std::vector<Exp*>& getArgs() const;

    /// @brief function gets the required types of the arguments passed to the syscall
    const std::vector<TYPES> getArgsTypes() const; 
    
    /// @brief function gets the register which the syscall stores the result of the operation in 
    /// @return returns a string representing the register if it returns a value, else, it returns an empty string
    const std::string getReturnReg() const;
    
    /// @brief gets the code which needs to be passed to $v0 to call the appropriate syscall
    int getSyscall() const;
    
    /// @brief function checks whether or not the syscall returns a value
    bool returnsVal() const;


	void insertAndCheckScopeId(Id scope_id) const override;

    /// @brief an enum of all the syscalls supported by spim
    enum class MipsSyscall {
    PRINT_INT = 1, PRINT_FLOAT,
    PRINT_DOUBLE, PRINT_STRING,
    READ_INT, READ_FLOAT,
    READ_DOUBLE, READ_STRING,
    SBRK, EXIT, PRINT_CHAR,
    READ_CHAR, OPEN_FILE,
    READ_FILE, WRITE_FILE,
    CLOSE_FILE, EXIT2
    };

    private:
    /// @brief the code of the syscall that is to be executed
    MipsSyscall func;

    /// @brief the parameters passed to the syscall
    std::vector<Exp*> params;
    
    /// @brief a map that matches between the name of the syscall and it's code
    static const std::map<std::string, MipsSyscall> funcNames;

    /// @brief function translates the name of the syscall to the apropriate code
    /// @param name the name of the syscall
    static MipsSyscall getSyscall(const std::string& name);

    
};


} // namespace AST

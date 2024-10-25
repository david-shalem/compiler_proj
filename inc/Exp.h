#pragma once
#include "AST.h"
#include "textVisitor.h"

#define WORD_SIZE 4
#define BYTE_SIZE 1

enum Exp_Op {
    /* numeric operations */

    ADD = 1, // +
    SUB = 2, // -
    MUL = 3, // *
    DIV = 4, // /

    /* boolean operations */

    // simple 
    OR = 5, // |
    XOR = 6, // ^
    AND = 7, // &
    SMLR = 8, // <

    //complex
    BGR = 9, // >
    EQL = 10, // ==
    NOT = 11, // !
    SEQL = 12, // <=
    BEQL = 13, // >=
    NEQL = 14 // !=
};


namespace AST
{

/// @brief Abstract class for expressions
class Exp : public ICommand { 
    protected:/* prevent instantiation */Exp(unsigned long id): exp_id(id) {}

    public: 
        //visitor function
        visitRes Accept(const IVisitor* visitor)  const override { return visitor->visitCommand(this); }

    public: 
        /// @return The type of value the expression returns
        virtual Type getType() const = 0; 

		/// @return The type from the annotation of the exp
		Type getTypeFromAnnotation() const;

        /// @return The mips code of the expression
        virtual std::string getMips() const = 0;
        
        /// @brief function search for addrsess in complex pf expression 
        virtual void checkForIlligalPvar() const { }

        /// @brief the id of the expression for the annotation map
        unsigned long exp_id; 
        
};

/// @brief Abstract class for mathematical expressions
class Exp_Tree : public Exp { public:
    
    /// @param id the id of the expression
    Exp_Tree(unsigned long id):Exp(id) {}

    /// @brief Stores the height of the expression's ast branch in the annotation map 
    /// @param an_map the annotation map in which to store the height
    /// @return The height of the expression's ast branch
    virtual int getHeight() const = 0;
    static std::string setValueAsBoolMips(TYPES vtype, std::string regnum) ;

    static bool varPullBool;  // indicate if the binop need boolean values from his roots (left and right). used mainly to choose between bool/num index 
    static int boolIndex; // the current return index in the boolean buffer (Bvars)
    static int numIndex; // the current return index in the numeric buffer (Nvars)
    static int booLable; // the last number of branch label (works like the str names)

};

/// @brief integer class
class Exp_Int : public Exp_Tree {
private:
	/// @brief the numeric value of the integer
	const int value;

    /// @brief the location in the code in which the integer was found 
    const int location;

public:

    /// @param v the numeric value of the integer
    /// @param l the location of thi integer in the code
    /// @param id the id of the expression for the annotation map
    Exp_Int(const int v, int l, unsigned long id);

	/// @return The numeric value of the integer
	int getValue() const { return value; }

    /// @brief Generates mips assembly code that stores the integer in an array
    /// @param return_index the index in which to store the integer
    /// @return The mips assmbly generated
    std::string getMips() const override 
    {
        if(varPullBool)
            return "li $t1, " + std::string(value ? "0x01" : "0x00") + "\nsb $t1, " + std::to_string(boolIndex*BYTE_SIZE) + BOOL_BUFFER_REG + "\n";

        return "li $t1, " + std::to_string(value) + "\nsw $t1, " + std::to_string(numIndex*WORD_SIZE) + NUM_BUFFER_REG + "\n";
    }

    // to string
	std::string toString() const { return std::to_string(value); }

    /// @param an_map not used
    /// @return returns the height of the expression's ast branch ( = 1)
    int getHeight() const override { return EDGE; }
    
    /// @return The type stored in the expression (int)
    Type getType() const override;

    // destructor
    // nothing to do here
    // no memory was allocated ...
    virtual ~Exp_Int(){}
};

/// @brief boolean class
class Exp_Bool : public Exp_Tree {
private:
	/// @brief the numeric value of the integer
	const bool value;

    /// @brief the location in the code in which the integer was found 
    const int location;

public:

    /// @param v the numeric value of the integer
    /// @param l the location of thi integer in the code
    /// @param id the id of the expression for the annotation map
    Exp_Bool(const bool v, int l, unsigned long id);

	/// @return The numeric value of the integer
	bool getValue() const { return value; }

    /// @brief Generates mips assembly code that stores the integer in an array
    /// @param return_index the index in which to store the integer
    /// @return The mips assmbly generated
    std::string getMips() const override 
    {
        return "li $t1, " + std::string(value ? "0x01" : "0x00") + "\nsb $t1, " + std::to_string(boolIndex) + BOOL_BUFFER_REG + "\n";
    }

    /// @param an_map not used
    /// @return returns the height of the expression's ast branch ( = 1)
    int getHeight() const override { return EDGE; }
    
    /// @return The type stored in the expression (int)
    Type getType() const override;

    // destructor
    // nothing to do here
    // no memory was allocated ...
    virtual ~Exp_Bool(){}
};

/// @brief float class
class Exp_Float : public Exp_Tree {
private:

	/// @brief the numerical value of the expression
	const float value;

    /// @brief the location of the float in the code
    const int location;

public:

    /// @param v the numeric value of the float
    /// @param l the location of thi float in the code
    /// @param id the id of the expression for the annotation map
    Exp_Float(const float v, int l, unsigned long id);
    
	/// @return The numerical value of the expression
	float getValue() const { return value; }

    /// @brief Generates mips assembly code that stores the float in an array
    /// @param return_index the index in which to store the float
    /// @return The mips assmbly generated
    std::string getMips() const override 
    {
        if(varPullBool)
            return "li $t1, " + std::string(value ? "0x01" : "0x00") + "\nsb $f0, " + std::to_string(boolIndex*BYTE_SIZE) + BOOL_BUFFER_REG + "\n";
        return "li.s $f0, " + std::to_string(value) + "\ns.s $f0, " + std::to_string(numIndex*WORD_SIZE) + NUM_BUFFER_REG + "\n";
    }

    // to string
	std::string toString() const { return std::to_string(value); }

    /// @param an_map not used
    /// @return returns the height of the expression's ast branch ( = 1)
    int getHeight() const override { return EDGE; }

    /// @return The type stored in the expression (float)
    Type getType() const override;

    // destructor
    // nothing to do here
    // no memory was allocated ...
    virtual ~Exp_Float(){}
};

/// @brief variable value class
class Exp_Var : public Exp_Tree 
{
    public:
    
    Exp_Var(const std::string& name, unsigned long int id, bool addres_flag);

    std::string getMips() const override;

    int getHeight() const override { return EDGE; }
    std::string getName() const { return name; }
    void checkForIlligalPvar() const override;

    /// @brief function recive the scope id of the variable action, save it, and check if the var exist or not in the scope  
    void insertAndCheckScopeId(Id scope_id) const override;

    Type getType() const override;
    
    bool asAddresFlag; // whether the value of the var is requested (false) or his addres (true). for exmaple in '&var'
    
    protected:
    
    Type type;
    std::string name;
};

/// @brief pointers value (pointer1 = pointer2 + 3) and pointers' variable value (variable = *pointer + 8) class 
class Exp_PVar : public Exp_Var
{
    public:
    
    Exp_PVar(const std::string& name, unsigned long int id, unsigned short pointerDepthUse);

    void checkForIlligalPvar() const override;
    std::string getMips() const override;
    
    Type getType() const override;
    unsigned short _pointerDepthUsed; // the number of '*' used. 

    private:
        /// @brief function load the value/addres of var
        std::string loadVar() const;
};

/// @brief Binary operation class
class Exp_Binop : public Exp_Tree {
protected:

	/// @brief The left branch of the binop
	Exp_Tree *left;

    /// @brief The left branch of the binop
    Exp_Tree *right;

public:

	/// @param l the left branch of the binop
	/// @param r the right branch of the binop
	/// @param id the id of the binop for the annotation map
	Exp_Binop(const Exp *l, const Exp *r, unsigned int id, Exp_Op op):left((Exp_Tree*)l), right((Exp_Tree*)r), Exp_Tree(id), operation(op) {}

	/// @return The left branch of the binop
	const Exp *getLeft() const { return left; }

	/// @return The right branch of the binop
	const Exp *getRight() const { return right; }

    /// @return The type of value the binop returns
    Type getType() const override;

    /// @brief Stores the height of the expression's ast branch in the annotation map 
    /// @param an_map the annotation map in which to store the height
    /// @return The height of the expression's ast branch
    int getHeight() const override;

    /// @brief Generates mips assembly code that stores the result of the operation in an array
    /// @param return_index the index in which to store the result
    /// @return The mips assmbly generated
    std::string getMips() const override;

    /// @brief function check for illigal addres actions in the exp
    void checkForIlligalPointers();
    void checkForIlligalPvar() const override;
    void insertAndCheckScopeId(Id scope_id) const override;

public:

    // destructor
    // nothing to do here
    // no memory was allocated ...
    virtual ~Exp_Binop()
    {
        delete left;
        delete right;
    }
private:
    /// @brief Generates mips assembly code that stores the result of the operation in an array
    /// @param return_index the index in which to store the result
    /// @param side the branch which generated the assmebly first 
    /// @return The mips assmbly generated
    std::string buildMips(bool side) const;

    /// @brief Function build a mips that represent the boolean operation of the binop in mips. It is done in a seperate function since the boolean operations 
    ///     are more complex then the regular numeric operations
    /// @return the mips code 
    std::string buildBoolOpMips() const;

    /// @brief function multiply the the numeric side of the binop if the other side is an addres. 
    ///     so for example (<int addres> + 1) will become (<int addres> + 4)
    /// @param type the type of the numeric side
    /// @return the mips of the multiplation 
    std::string matchAddresType(Type type) const;

    Exp_Op operation; // the operartion of the binop
};

/// @brief Abstract class for string expressions
class Exp_String : public Exp 
{
    public:
        /// @param id the id of the expression in the annotation map
        Exp_String(unsigned long id);

        /// @brief default c'tor, assumed id is maximum long value
        Exp_String():Exp(__LONG_MAX__) {}

        /// @brief Stores the names of the strings in the expression and their ids in a given vector
        /// @param names the vector to stores the names of the strings and their ids in
        virtual void getTmpStrNames(std::vector<std::pair<unsigned int, std::string>>& names) const = 0;

        /// @brief Stores the name of the string in the annotation map
        /// @param name_num the id of the string which is used to generate the name of the string
        virtual void setTmpStrName(unsigned int name_num) const = 0;

        /// @return The type of the expression (STR) 
        Type getType() const override;


};

/// @brief String value class
class Exp_Str : public Exp_String {
private:

	/// @brief The value of the string
	const std::string value;

    /// @brief The location of the string in the code
    const int location;

public:

    /// @param v the value of the string
    /// @param l the location of the string in the code
    /// @param id the id of the string for the annotation map
    Exp_Str(const std::string &v, int l,unsigned long id):value(v), location(l), Exp_String(id) {}

	/// @return The value of the string
	const std::string &getValue() const { return value; }

    // to string
	std::string toString() const { return value; }

    /// @brief Generates mips assembly for the string
    /// @param return_index unused parameter
    /// @return The mips assembly generated
    std::string getMips() const override;

    /// @brief Stores the names of the strings in the expression and their ids in a given vector
    /// @param names the vector to stores the names of the strings and their ids in
    void getTmpStrNames(std::vector<std::pair<unsigned int, std::string>>& names) const override;

    /// @brief Stores the name of the string in the annotation map
    /// @param name_num the id of the string which is used to generate the name of the string
    void setTmpStrName(unsigned int name_num) const override;

    // destructor
    // nothing to do here
    // no memory was allocated ...
    virtual ~Exp_Str(){}
};

/// @brief String concatenation operation class
class Binop_StrCat : public Exp_String {
    public: 

    /// @param next_val the expression which to concat the string to
    /// @param val the string to concat
    Binop_StrCat(const Exp_String *next_val, const Exp_Str* val):Exp_String((val->exp_id)), _str(*val), next(next_val) {}


    /// @brief Stores the names of the strings in the expression and their ids in a given vector
    /// @param names the vector to stores the names of the strings and their ids in
    void getTmpStrNames(std::vector<std::pair<unsigned int, std::string>>& names) const override;

    /// @brief Stores the name of the string in the annotation map
    /// @param name_num the id of the string which is used to generate the name of the string
    void setTmpStrName(unsigned int name_num) const override;

    /// @brief Generates mips assembly for the concat operation
    /// @param return_index unused parameter
    /// @return The mips assembly generated
    std::string getMips() const override;

    virtual ~Binop_StrCat()
    {
        delete next;
    }
    
    private: 

        /// @brief the string to concat
        Exp_Str _str;
        
        /// @brief the expression which to concat the string to
        const Exp_String* next;
};


}

/*********************/
/* FILENAME: AST.cpp */
/*********************/

/*************************/
/* GENERAL INCLUDE FILES */
/*************************/
#include <tuple>

/*************************/
/* PROJECT INCLUDE FILES */
/*************************/
#include "AST.h"
#include "Compiler.h"
#include "Exp.h"
#include "annotationVisitor.h"
#include "Var.h"
#include "Scopes.h"

#define INT_AND_FLOAT 3
#define INT_AND_BOOL 4
#define BOOL_AND_FLOAT 5

namespace AST
{
    bool Exp_Tree::varPullBool = false; 
    int Exp_Tree::boolIndex = 0; 
    int Exp_Tree::numIndex = 0; 
    int Exp_Tree::booLable = 0; 

    std::string getMipsOp(Exp_Op op)
    {
        switch (op)
        {
        case ADD:
            return "add";
            break;
        case SUB:
            return "sub";
            break;
        case MUL:
            return "mul";
            break;
        case DIV:
            return "div";
            break;
        case OR:
            return "or";
            break;
        case AND:
            return "and";
            break;
        case XOR:
            return "xor";
            break;
        case SMLR:
            return "slt";
            break;
        default:
            break;
        }
        return "";
    }

	Type Exp::getTypeFromAnnotation() const 
	{
		return Compiler::getAnnotation<Exp_Annotation>(exp_id)->exp_type;
	}

    std::string Exp_Tree::setValueAsBoolMips(TYPES vtype, std::string regnum) 
    {
        if(vtype == FLOAT)
        {
            if(Exp_Tree::varPullBool) {//in case the resulte of the binop is needed as boolean, convert it to bool. example: -> 4.0 - 5.0 <- & true -> 4 - 5 = -1 = true
                return "li $t" + regnum + ", 0x00\nbeq $f" + regnum + ", $zero, boolLabel" + std::to_string(Exp_Tree::booLable++) + "\nli $t" + regnum + ", 0x01\nboolLabel" + std::to_string(Exp_Tree::booLable) + ":\n"
                    "sb $t" + regnum + " " + std::to_string(Exp_Tree::boolIndex*BYTE_SIZE) + BOOL_BUFFER_REG + "\n"; }
            else {
                return "s.s $f" + regnum + " " + std::to_string(Exp_Tree::numIndex*WORD_SIZE) + NUM_BUFFER_REG + "\n"; }
        }
        else
        {
            if(Exp_Tree::varPullBool) {//in case the resulte of the binop is needed as boolean, convert it to bool. example: 4 - 5 & true -> 4 - 5 = -1 = true
                std::string newnum = std::to_string(atoi(regnum.c_str()) + 1);
                return "li $t" + newnum + ", 0x00\nbeq $t" + regnum + ", $zero, boolLabel" + std::to_string(Exp_Tree::booLable++) + "\nli $t" + newnum + ", 0x01\nboolLabel" + std::to_string(Exp_Tree::booLable) + ":\n"
                    "sb $t" + newnum + " " + std::to_string(Exp_Tree::boolIndex*BYTE_SIZE) + BOOL_BUFFER_REG + "\n"; }
            else {
                return "sw $t" + regnum + " " + std::to_string(Exp_Tree::numIndex*WORD_SIZE) + NUM_BUFFER_REG + "\n"; }
        }
        return "";
    }

    void Exp_Binop::insertAndCheckScopeId(Id scope_id) const
    {
        ((ICommand*)left)->insertAndCheckScopeId(scope_id);
        ((ICommand*)right)->insertAndCheckScopeId(scope_id);
    }

    Type Exp_Binop::getType() const
    {
        Type type1 = left->getType();
        Type type2 = right->getType();
        Type res = type1;
        
        // if one of the sides are addreses, save the type and continue 
        if(type1.pointerDepth) {
            if(type2.type != INT)
                throw std::runtime_error("error: addres cannot be operated with types other than INT");
            res = type1;
        }
        else if (type2.pointerDepth){
            if(type1.type != INT)
                throw std::runtime_error("error: addres cannot be operated with types other than INT");
            res = type2;
        }
        else 
        {
            TYPES t1 = type1.type, t2 = type2.type, rest = type1.type;
            //check if type of the expression from both sides is same. (if not: check if it's a legal difference. (if not: throw exception))
            if(t1 != t2)
                if(t1 + t2 == INT_AND_FLOAT) // if the binop members are numeric 
                    rest = FLOAT;
                else if(t1 + t2 == INT_AND_BOOL || (t1 + t2 == BOOL_AND_FLOAT && t1 != STR && t2 != STR)) // if the binop members are numeric and boolean
                    rest = BOOL;
                else 
                    throw std::runtime_error("error: expression types aren't matching");

            if(rest == BOOL)
                if(operation < OR || operation > AND) // check if the sides are boolean in an numeric/complex-boolean operation. example: 4 < true, or true + 2
                    throw std::runtime_error("error: expression types and expression binop aren't matching");

            // in case the binop have numeric members but complex-bool operation (example: 3 < 4) 
            if((rest == INT || rest == FLOAT) && operation >= OR)
                rest = BOOL;

            res = {rest, 0};
        }
        

        Compiler::getAnnotation<Binop_Annotation>(exp_id)->exp_type = res;
        return res;
    }

    int Exp_Binop::getHeight() const
    {
        int d1 = left->getHeight();
        int d2 = right->getHeight();
        int biggest = (d1 > d2 ? d1 : d2) + 1;

        Binop_Annotation* annotation = Compiler::getAnnotation<Binop_Annotation>(exp_id);
        Binop_Annotation* Lannotation = Compiler::getAnnotation<Binop_Annotation>(left->exp_id);
        Binop_Annotation* Rannotation = Compiler::getAnnotation<Binop_Annotation>(right->exp_id);
        
        int roots_types = Lannotation->exp_type.type + Rannotation->exp_type.type; 

        // in case the binop is boolean but one or more of it sides are numeric, the height need to be set to the boolean side height, and the max height of 
        // the numeric tree is need to be set (if the current numeric height is bigger ofc). All that is because we seperate the vars buffers 
        if(annotation->exp_type.type == BOOL && !(annotation->exp_type.pointerDepth) && roots_types <= BOOL_AND_FLOAT) {
            if(operation > AND) { // if bouth sides are numeric but the binop is boolean
                annotationVisitor::setNumMax(biggest);
                return EDGE;
            }
            else if(Lannotation->exp_type.type == BOOL) {
                annotationVisitor::setNumMax(d2);
                return d1;
            }
            else {
                annotationVisitor::setNumMax(d1);
                return d2;
            }
        }

        //sets the dippest size of the binop in the annotation node if the binop
        annotation->biggest_side = d1 > d2;

        
        return biggest; 
    }

    std::string Exp_Binop::buildMips(bool side) const
    {
        TYPES operation_type;
        Type binop_type = Compiler::getAnnotation<Binop_Annotation>(exp_id)->exp_type;
        std::string mips;

        //the resulte of the two sides is loaded so the left side res is stored in t1/f0 and the right side res in t2/f1

        if(binop_type.pointerDepth)
            operation_type = INT; // if the binop is about addreses, the value transformation need to be made like the type is int 
        else
            operation_type = binop_type.type;

        switch (operation_type)
        {
        case FLOAT:
            // put the values of the binop sides in the registers
            mips += "l.s $f0, " + std::to_string((numIndex + (side ? 1 : 2))*WORD_SIZE) + NUM_BUFFER_REG + "\n"
              "l.s $f1, " + std::to_string((numIndex + (!side ? 1 : 2))*WORD_SIZE) + NUM_BUFFER_REG + "\n";

            // in case one of the sides is int, convert it to float 
            if(Compiler::getAnnotation<Exp_Annotation>(left->exp_id)->exp_type.type == INT)
                mips += "cvt.s.w $f0, $f0\n";
            else if(Compiler::getAnnotation<Exp_Annotation>(right->exp_id)->exp_type.type == INT)
                mips += "cvt.s.w $f1, $f1\n";

            // doing the actual action of the binop and inserting the resulte in the exp buffer
            mips += getMipsOp(operation) + ".s $f3, $f0, $f1\n" + setValueAsBoolMips(FLOAT, "3");
            break;
        
        case BOOL: 
            if(operation < SMLR) {
                mips += "lb $t1, " + std::to_string((boolIndex + (side ? 1 : 2))*BYTE_SIZE) + BOOL_BUFFER_REG + "\n"
                    "lb $t2, " + std::to_string((boolIndex + (!side ? 1 : 2))*BYTE_SIZE) + BOOL_BUFFER_REG + "\n";
            }
            else {
                mips += "lw $t1, " + std::to_string((numIndex + (side ? 1 : 2))*WORD_SIZE) + NUM_BUFFER_REG + "\n"
                    "lw $t2, " + std::to_string((numIndex + (!side ? 1 : 2))*WORD_SIZE) + NUM_BUFFER_REG + "\n";
            }

            mips += buildBoolOpMips() + "sb $t3 " + std::to_string(boolIndex*BYTE_SIZE) + BOOL_BUFFER_REG + "\n";
            break; 

        case INT:
            mips += "lw $t1, " + std::to_string((numIndex + (side ? 1 : 2))*WORD_SIZE) + NUM_BUFFER_REG + "\n"
              "lw $t2, " + std::to_string((numIndex + (!side ? 1 : 2))*WORD_SIZE) + NUM_BUFFER_REG + "\n" + matchAddresType(binop_type) 
              + getMipsOp(operation) + " $t3, $t1, $t2\n" + setValueAsBoolMips(INT, "3");
            break;
        }
        return mips;
    }

    std::string Exp_Binop::buildBoolOpMips() const
    {
        // in case the boolean operation is simple operation (like the numeric operations)
        if(operation <= SMLR) { return getMipsOp(operation) + " $t3, $t1, $t2\n"; }
        std::string mips = "li $t3, 0x00\n";

        switch (operation)
        {
        case BGR:
            return "slt $t3, $t2, $t1\n"; 
        case EQL:
            mips += "bne ";  
            break;
        case NEQL:
            mips += "beq ";
            break;
        case SEQL:
            mips += "bgt ";
            break;
        case BEQL:
            mips += "blt ";
            break;
        }

        return mips + "$t1, $t2, boolLabel" + std::to_string(booLable++) + "\nli $t3, 0x01\nboolLabel" + std::to_string(booLable) + ":\n";
    }

    std::string Exp_Binop::getMips() const
    {
        std::string mips;
        Binop_Annotation* annotation = Compiler::getAnnotation<Binop_Annotation>(exp_id);
        bool side = annotation->biggest_side;
        
        varPullBool = false;
        if(operation <= AND && operation >= OR) varPullBool = true; 
        bool temp = varPullBool; // varPullBool can change in the mips function of left and right, so we save his value now

        // increasing the right buffer index and calling the first side
        varPullBool ? boolIndex++ : numIndex++;
        mips += side ? left->getMips() : right->getMips();

        //increasing the right buffer second time and calling the second size
        varPullBool = temp;
        varPullBool ? boolIndex++ : numIndex++;
        mips += side ? right->getMips() : left->getMips();
        
        temp ? boolIndex -= 2 : numIndex -= 2; //returning the indexes for their values in the start of the function
        mips += buildMips(side);
        
        return mips;
        
    }

    std::string Exp_Binop::matchAddresType(Type type) const 
    {
        if(type.pointerDepth)
        {
            bool side = !(dynamic_cast<const Exp_PVar*>(left) && !(((const Exp_PVar*)left)->_pointerDepthUsed));

            if(type.type == INT || type.type == FLOAT)
                return "li $t3, 4\nmul $t" + std::string(side ? "1" : "2") + ", $t" + std::string(side ? "1" : "2") + ", $t3\n";
        }

        return "";
    }

    void Exp_Binop::checkForIlligalPointers()
    {
        if(dynamic_cast<const Exp_Binop*>(left)) { left->checkForIlligalPvar(); }
        if(dynamic_cast<const Exp_Binop*>(right)) { right->checkForIlligalPvar(); }

        try { // if one of the sides is an addres
            left->checkForIlligalPvar();
            right->checkForIlligalPvar();
        } catch ( std::runtime_error& r) {
            if(!(operation == ADD || operation == SUB))
                throw std::runtime_error(
                    "variable that contains an addres, cannot be operated by operations other than '+' and '-'");
        }
    }

    void Exp_Binop::checkForIlligalPvar() const
    {
        left->checkForIlligalPvar();
        right->checkForIlligalPvar();
    }

    void Binop_StrCat::setTmpStrName(unsigned int name_num) const 
    {
        _str.setTmpStrName(name_num);
        next->setTmpStrName(name_num + 1);
    }

    void Exp_Str::setTmpStrName(unsigned int name_num) const 
    {
        String_Annotation* annotation_object = Compiler::getAnnotation<String_Annotation>(exp_id);
        annotation_object->str_num = name_num;
        annotationVisitor::nextStrName++;
    }

    std::string Exp_Str::getMips() const
    {
        return "la $t0, str" + std::to_string(Compiler::getAnnotation<String_Annotation>(exp_id)->str_num) + "\njal cpy_str\n";
    }

    std::string Binop_StrCat::getMips() const
    {
        return _str.getMips() + next->getMips();
    }

    void Exp_Str::getTmpStrNames(std::vector<std::pair<unsigned int, std::string>>& names) const 
    {
        names.push_back({Compiler::getAnnotation<String_Annotation>(exp_id)->str_num, value});
    }

    void Binop_StrCat::getTmpStrNames(std::vector<std::pair<unsigned int, std::string>>& names) const 
    {
        _str.getTmpStrNames(names);
        next->getTmpStrNames(names);
    }

    Type Exp_Int::getType() const
    { 
        return {INT, 0};
    }

    Type Exp_Float::getType() const
    { 
        return {FLOAT, 0}; 
    }

    Type Exp_Bool::getType() const
    { 
        return {BOOL, 0}; 
    }

    Type Exp_String::getType() const
    {
        return {STR, 0}; 
    }

    Exp_String::Exp_String(unsigned long id): Exp(id)
    {
        Compiler::getAnnotation<Exp_Annotation>(exp_id)->exp_type = {STR, 0};
    }

    Exp_Int::Exp_Int(const int v, int l, unsigned long id): Exp_Tree(id), value(v), location(l)
    {
        Compiler::getAnnotation<Exp_Annotation>(exp_id)->exp_type = {INT, 0};
    }

    Exp_Bool::Exp_Bool(const bool v, int l, unsigned long id): Exp_Tree(id), value(v), location(l)
    {
        Compiler::getAnnotation<Exp_Annotation>(exp_id)->exp_type = {BOOL, 0};
    }

    Exp_Float::Exp_Float(const float v, int l, unsigned long id):value(v), location(l), Exp_Tree(id)
    {
        Compiler::getAnnotation<Exp_Annotation>(exp_id)->exp_type = {FLOAT, 0};
    }

    Exp_Var::Exp_Var(const std::string& name, unsigned long int id, bool addres_flag) : name(name), Exp_Tree(id), asAddresFlag(addres_flag)
    {
        if (Var::vars.find(name) == Var::vars.end())
            throw std::runtime_error("error: var: \'" + name + "\' is undefined");

        type = Var::vars[name].varT;

        if(addres_flag)
            type.pointerDepth++;
            
        Compiler::getAnnotation<Exp_Var_Annotation>(exp_id)->exp_type = type;
    }

    void Exp_Var::insertAndCheckScopeId(Id scope_id) const 
    {
        Compiler::getAnnotation<Exp_Var_Annotation>(exp_id)->scopeId = scope_id;
    }

    Exp_PVar::Exp_PVar(const std::string& name, unsigned long int id, unsigned short pointerDepthUsed) : Exp_Var(name, id, false), _pointerDepthUsed(pointerDepthUsed)
    {
        if(pointerDepthUsed > type.pointerDepth) 
            throw std::runtime_error("error: the pointer depth of the var \'" + name + "\' is " + 
                std::to_string(Var::vars[name].varT.pointerDepth) + ", and cannot be called with the depth of " + std::to_string(pointerDepthUsed));

    }

    std::string Exp_Var::getMips() const
    {
        Id scope_id = Compiler::getAnnotation<Exp_Var_Annotation>(exp_id)->scopeId;

        if(asAddresFlag) // if the addres of the var is requested 
            return !(Var::vars[name].isGlobal) ? // if var is not global
                "addi $t1, $sp, " + std::to_string(Scope::scopes[scope_id]->getStackLoc(name)) + "\nsw $t1, " + 
                    std::to_string(numIndex * WORD_SIZE) + NUM_BUFFER_REG "\n" : // copy the location in the stack of the var to $t1
                "la $t1, " + name + "\n" + Exp_Tree::setValueAsBoolMips(INT, "1");  // copy the adddres of the global vairable 

        std::string src = !(Var::vars[name].isGlobal) ? (std::to_string(Scope::scopes[scope_id]->getStackLoc(name)) + "($sp)") : name;
        std::string mips;
    
        switch(type.type)
        {
            case INT:
                mips += "lw $t1, " + src + "\n" + Exp_Tree::setValueAsBoolMips(INT, "1");
                break;
            case FLOAT:
                mips += "l.s $f1, " + src + "\n" + Exp_Tree::setValueAsBoolMips(FLOAT, "1");
                break;
            case BOOL:
                mips += "lb $t1, " + src + "\nsb $t1, " + std::to_string(boolIndex*BYTE_SIZE) + BOOL_BUFFER_REG + "\n";
                break;
            default:
                throw std::runtime_error("type " + std::to_string(type.type) + " not supported");
        }
        
        return mips; 
    }

    void Exp_Var::checkForIlligalPvar() const
    {
        if(asAddresFlag)
            throw std::runtime_error("addreses cannot be invlolved in complex expressions. (instead of 3 + addres + 4, try addres + (4 + 3))");
    }

    Type Exp_Var::getType() const
    { 
        return type; 
    }

    void Exp_PVar::checkForIlligalPvar() const
    {
        if(!_pointerDepthUsed) 
            throw std::runtime_error("addreses cannot be invlolved in complex expressions. (instead of 3 + addres + 4, try addres + (4 + 3))");
    }

    std::string Exp_PVar::loadVar() const
    {
        std::string res; 

        if(!(Var::vars[name].isGlobal))
        {
            int stack_loc = Scope::scopes[Compiler::getAnnotation<Exp_Var_Annotation>(exp_id)->scopeId]->getStackLoc(name);
            res = "lw $t1, " + std::to_string(stack_loc) + "($sp)\n";
        }
        else
            res = "lw $t1, " + name + "\n";

        for (int i = 0; i < _pointerDepthUsed - 1; i++) // getting into the pointer depth -> *(*(*(...))) 
            res += "lw $t1, ($t1)\n";
        return res;
    }

    std::string Exp_PVar::getMips() const
    {
        std::string mips = loadVar(); 
        if(!_pointerDepthUsed) // return the addres stored in the pointer
            return mips + Exp_Tree::setValueAsBoolMips(INT, "1");

        TYPES action_type = _pointerDepthUsed < type.pointerDepth ? INT : type.type; // ***(&&&float) : float | **(&&&float) : addres(int)
        switch(action_type)
        {
            case INT:
                mips += "lw $t1, ($t1)\n" + Exp_Tree::setValueAsBoolMips(INT, "1");
                break;
            case FLOAT:
                mips += "l.s $f1, ($t1)\n" + Exp_Tree::setValueAsBoolMips(FLOAT, "1");
                break;
            case BOOL:
                mips += "lb $t1, ($t1)\n" + std::to_string(boolIndex*BYTE_SIZE) + BOOL_BUFFER_REG + "\n";
                break;
            default:
                throw std::runtime_error("type " + std::to_string(type.type) + " not supported");
        }
        
        return mips; // return the value stored in the addres that stored in the pointer
    }

    Type Exp_PVar::getType() const
    {
        Type res = type;
        res.pointerDepth -= _pointerDepthUsed;
        Compiler::getAnnotation<Exp_Var_Annotation>(exp_id)->exp_type = res;
        return res;
    }
};

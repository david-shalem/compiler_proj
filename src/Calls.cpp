#include "Calls.h"
#include "Functions.h"
#include "Compiler.h"

namespace AST
{

void Call::insertAndCheckScopeId(Id scope_id) const
{    
    if(AST::Function::functions.find(callTo) == AST::Function::functions.end()) // check if function exist 
        throw std::runtime_error("there is no function called \'" + callTo + "\'");
    
    auto def_params = AST::Function::functions[callTo]->params;
    
    if(def_params.size() != params.size()) // check if there are enough params sended to the function 
        throw std::runtime_error("cannot call function \'" + callTo + "\' with " + 
            std::to_string(params.size()) + " parameter" + (params.size() > 1 ? "s" : ""));

    for(Exp* param : params)
        param->insertAndCheckScopeId(scope_id);
}

std::string Call::getMips() const
{
    // the call stack order: <$ra save <size: 4> | buffers saves <size: changing> 
    //   | the returned value <size: changing> | the params <size: changing> <= $sp 

    std::string res; 
    const AST::Function* func = AST::Function::functions[callTo];
    int return_value_size = func->returnType.pointerDepth || func->returnType.type != BOOL ? ADDRES_SIZE : BYTE_SIZE;
    int buffers_size = boolIndex + (numIndex * WORD_SIZE);
    buffers_size += align(buffers_size); 

    return_value_size += align(return_value_size); // add aligning edge 

    int param_frame_size = 0;
    for(AST::VarDecl* var : func->params) // count the size of the params
        param_frame_size += Var::getSizeByType(var->getName());
    param_frame_size += align(param_frame_size); // add aligning edge 

    int command_stack_size = param_frame_size + return_value_size + ADDRES_SIZE + buffers_size;
    // creating and stackframe and inserting values (params frame + return frame + $ra save + buffers save)
    res += saveBuffers(); // saving the exp-buffers
    res += "sw $ra, -" + std::to_string(ADDRES_SIZE) + "($sp)\n"; // saving $ra
    res += insertParamsToStack(command_stack_size); // inserting the params to the stack
    res += "sub $sp, $sp, " + std::to_string(command_stack_size) + "\n"; // open call satackframe
    res += "jal " + callTo + "\n"; // calling the function 
    res += "add $sp, $sp, " + std::to_string(command_stack_size) + "\n"; // close call stackframe 

    // load the returned value to the exp buffer
    res += "lw $t1, -" + std::to_string(return_value_size + ADDRES_SIZE + buffers_size) + "($sp)\n" + Exp_Tree::setValueAsBoolMips(INT, "1"); 
    res += "lw $ra, -" + std::to_string(ADDRES_SIZE) + "($sp)\n"; // loading back $ra
    res += loadBuffers();
    return res;
}

void Call::checkForIlligalPvar() const
{
    if(Function::functions[callTo]->returnType.pointerDepth) 
            throw std::runtime_error("addreses cannot be invlolved in complex expressions. (instead of 3 + addres + 4, try addres + (4 + 3))");
}

int Call::getHeight() const
{
    int heighest = 0;
    for(Exp* param : params){ // get the dippest exp from all the params
        int height = ((Exp_Tree*)param)->getHeight();
        if(height > heighest) heighest = height;
    }

    return heighest;
}

Type Call::getType() const
{
    auto ordered_p = getParamsByOrder();
    auto var = ordered_p.begin();

    // check if the types of params sended to the function are legal
    for(AST::VarDecl* param : AST::Function::functions[callTo]->params) 
    {
        Type exp_t = Compiler::getAnnotation<Exp_Annotation>((*(var))->exp_id)->exp_type;
        Type param_t = param->getType();

        if(!(param_t.pointerDepth * exp_t.pointerDepth)) { // if def-param or/and call-param are not pointers
            if((exp_t.pointerDepth || param_t.pointerDepth) // if def-param is pointer and call-param is not or the opposite
            || (exp_t.type != param_t.type && param_t.type != BOOL)) // if the types of the def-param and the call-param arn't matching 
               throw std::runtime_error("the definition-paramter \'" + param->getName() + 
                "\' does not have a similar type as his parallel call-parameter");
        }

        var++;    
    }

    return Function::functions[callTo]->returnType;
}

int Call::align(int dst)
{
    return dst % ALIGN_SIZE == 0 ? 0 : (ALIGN_SIZE - (dst % ALIGN_SIZE));
}

std::string Call::insertParamsToStack(int call_frame_size) const
{   
    std::string mips;
    // inserting the params into the stack
    int stack_pointer = 0;
    auto def_param = AST::Function::functions[callTo]->params.begin();
    for(Exp* param : getParamsByOrder())
    {
        Type exp_type = Compiler::getAnnotation<Exp_Annotation>(param->exp_id)->exp_type;

        TYPES mips_type = exp_type.pointerDepth ? INT : exp_type.type; // if the param is pointer, work like if he was an int type
        mips_type != BOOL ? numIndex++ : boolIndex++;
        mips += param->getMips(); // calculating the parameter 

        switch (mips_type)
        {
        case INT: // reading the exp resulte and inserting it to the stack
            mips += "lw $t1, " + std::to_string(numIndex *  4) + NUM_BUFFER_REG "\n";
            
            if((*(def_param))->getType().type != BOOL) { // load value as int (or addres)
                mips += "sw $t1, " + std::to_string(stack_pointer - call_frame_size) + "($sp)\n"; 
                stack_pointer += WORD_SIZE;
            }
            else { // load value as bool
                mips += "li $t2, 0x00\nbeq $t1, $zero, boolLabel" + std::to_string(Exp_Tree::booLable++) + "\nli $t2, 0x01\nboolLabel" + 
                    std::to_string(Exp_Tree::booLable) + ":\nsb $t2, " + std::to_string(stack_pointer - call_frame_size) + "($sp)\n"; 
                stack_pointer += BYTE_SIZE;
            }
            break;
        
        case FLOAT: // reading the exp resulte and inserting it to the stack
            mips += "l.s $f0, " + std::to_string(numIndex *  4) + NUM_BUFFER_REG "\n";

            if((*(def_param))->getType().type != BOOL) { // load value as float 
                mips += "s.s $f0, " + std::to_string(stack_pointer - call_frame_size) + "($sp)\n"; 
                stack_pointer += WORD_SIZE;
            }
            else { // load value as bool
                mips += "li $t1, 0x00\nbeq $f0, $zero, boolLabel" + std::to_string(Exp_Tree::booLable++) + "\nli $t1, 0x01\nboolLabel" + 
                    std::to_string(Exp_Tree::booLable) + ":\nsb $t1, " + std::to_string(stack_pointer - call_frame_size) + "($sp)\n"; 
                stack_pointer += BYTE_SIZE;
            }
            break;
        
        case BOOL:
            mips += "lb $t1, " + std::to_string(boolIndex) + BOOL_BUFFER_REG "\n sb $t1, " +
                std::to_string(stack_pointer - call_frame_size)+ "($sp)\n";
            stack_pointer += BYTE_SIZE;
        default:
            break;
        }

        mips_type != BOOL ? numIndex-- : boolIndex--;
        def_param++;
    }
    return mips;
}

std::vector<Exp *> Call::getParamsByOrder() const
{
    std::vector<Exp *> res;
    for(int index : Function::functions[callTo]->paramsOrder)
        res.push_back(params[index]);

    return res;
}

std::string Call::saveBuffers() const
{
    std::string res; 
    int stack_pointer = boolIndex + (numIndex * WORD_SIZE) + ADDRES_SIZE; // the sizes of the used parts in the buffers + $ra save

    // there is no need to save the last used index in the buffers, because he is for the 
    //    resulte of this command, and not keeping any value yet
    if(boolIndex > 1) 
        stack_pointer--;
    if(numIndex > 1)
        stack_pointer -= WORD_SIZE;


    for (int i = 1; i < boolIndex; i++) // save all the values of the boolean-buffer
        res += "lb $t1, " + std::to_string(i) + BOOL_BUFFER_REG "\nsb $t1, -" + std::to_string(stack_pointer--) + "($sp)\n";

    for (int i = 1; i < numIndex; i++) { // save all the values of the numeric-buffer
        res += "lw $t1, " + std::to_string(i*WORD_SIZE) + NUM_BUFFER_REG "\nsw $t1, -" + std::to_string(stack_pointer) + "($sp)\n";
        stack_pointer -= WORD_SIZE;
    }

    return res;
}

std::string Call::loadBuffers() const
{
    std::string res; 
    int stack_pointer = boolIndex + (numIndex * WORD_SIZE) + ADDRES_SIZE; // the sizes of the used parts in the buffers + $ra save
    
    // the last indexes in the buffers are not saved 
    if(boolIndex > 1) 
        stack_pointer--;
    if(numIndex > 1)
        stack_pointer -= WORD_SIZE;

    for (int i = 1; i < boolIndex; i++) // load all the values of the boolean-buffer
        res += "lb $t1, -" + std::to_string(stack_pointer--) + "($sp)\nsb $t1, " + std::to_string(i) + BOOL_BUFFER_REG "\n";

    for (int i = 1; i < numIndex; i++) { // load all the values of the numeric-buffer
        res += "lw $t1, -" + std::to_string(stack_pointer) + "($sp)\nsw $t1, " + std::to_string(i*WORD_SIZE) + NUM_BUFFER_REG "\n";
        stack_pointer -= WORD_SIZE;
    }

    return res;
}
}

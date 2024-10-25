#include "SysFunc.h"
#include "Annotations.h"
#include "Compiler.h"

namespace AST
{

typedef SysFunc::MipsSyscall funcCode;

const std::map<std::string, funcCode> SysFunc::funcNames = {
    {"printI", funcCode::PRINT_INT}, {"printF", funcCode::PRINT_FLOAT},
    {"scanI", funcCode::READ_INT}, {"scanF", funcCode::READ_FLOAT}
    };
SysFunc::SysFunc(unsigned long id, const std::string& func, const std::vector<Exp*>& params) :
 Exp_Tree(id), func(getSyscall(func)), params(params) {}

visitRes SysFunc::Accept(const IVisitor* visitor) const
{
    return visitor->visitCommand(this);
}


const std::vector<Exp*>& SysFunc::getArgs() const
{
    return params;
} 

Type SysFunc::getType() const
{
    Type result;
    result.pointerDepth = 0;
    switch (func)
    {
    case funcCode::READ_INT:
        result.type = INT;
        break;
    case funcCode::READ_FLOAT:
        result.type = FLOAT;
        break;
    default:
        result.type = VOID;
    }
    Compiler::getAnnotation<Exp_Annotation>(exp_id)->exp_type = result;
    return result;
}

std::string SysFunc::getMips() const 
{
    return textVisitor().visitCommand(this).get<TextRes>(); 
}

funcCode SysFunc::getSyscall(const std::string& name) 
{
    try
    {
        return funcCode(funcNames.at(name));
    }
    catch(const std::out_of_range& e)
    {
        throw std::runtime_error("error: syscall " + name + " not implemented");
    }
    
}

int SysFunc::getHeight() const
{
    int maxHeight = returnsVal() ? 1 : 0;
    for(const Exp* exp: params)
    {
        if(exp->getTypeFromAnnotation().type == INT
				|| exp->getTypeFromAnnotation().type == FLOAT)
        {
            int temp = ((const Exp_Tree*)exp)->getHeight();
            maxHeight = (maxHeight < temp) ? temp : maxHeight;
        }
    }
    maxHeight += params.size();
    return maxHeight;
}


std::vector<std::string> SysFunc::getArgRegs() const
{
    switch (func)
    {
    case funcCode::PRINT_INT:
    case funcCode::PRINT_STRING:
        return { "$a0" };
    case funcCode::PRINT_FLOAT:
        return { "$f12" };
    case funcCode::READ_INT:
    case funcCode::READ_FLOAT:
        return {};
    case funcCode::READ_STRING:
        return { "$a0", "$a1"};
    default:
        throw std::runtime_error("error: unsupported syscall");
    }
} // namespace AST

const std::vector<TYPES> SysFunc::getArgsTypes() const 
{
    switch (func)
    {
    case funcCode::PRINT_INT:
        return { INT };
    case funcCode::PRINT_FLOAT:
        return { FLOAT };
    case funcCode::PRINT_STRING:
        return { STR };
    case funcCode::READ_STRING:
        return {STR, INT};    
    default:
        return std::vector<TYPES>();
    }
}

const std::string SysFunc::getReturnReg() const
{
    switch (func)
    {
        case MipsSyscall::READ_INT:
            return "$v0";
        case MipsSyscall::READ_FLOAT:
            return "$f0";
        default:
            return "";
    }
}
int SysFunc::getSyscall() const
{
    return (int)func;
}
bool SysFunc::returnsVal() const
{
    switch(func)
    {
        case MipsSyscall::READ_INT:
        case MipsSyscall::READ_FLOAT:
            return true;
        default:
            return false;
    }
}
void SysFunc::insertAndCheckScopeId(Id scope_id) const
{
    for(const Exp* i : params)
    {
        i->insertAndCheckScopeId(scope_id);
    }
}
}

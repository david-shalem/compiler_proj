#pragma once
#include "Exp.h"

namespace AST
{

class Call : public Exp_Tree
{
public:
    Call(const std::string &func, std::vector<Exp*> parameters, Id annotationId) : 
        Exp_Tree(annotationId), callTo(func), params(parameters) {}

    void insertAndCheckScopeId(Id scope_id) const;

    // exp functions:

    std::string getMips() const override; 
    void checkForIlligalPvar() const override;
    int getHeight() const override;
    Type getType() const override;

    const std::string callTo;
    std::vector<Exp*> params; 

    static int align(int dst);

private:

    std::vector<Exp*> getParamsByOrder() const;
    std::string insertParamsToStack(int call_frame_size) const;
    std::string saveBuffers() const; 
    std::string loadBuffers() const; 

};

}
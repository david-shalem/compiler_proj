#pragma once
#include "AST.h"
#include "Scopes.h"

namespace AST
{

class Return : public ICommand
{
public:
    visitRes Accept(const IVisitor* visitor)  const override { return visitor->visitCommand(this); }

public: 
    Return (const Exp* return_val, Id annotation_id) : returned(return_val), annotationId(annotation_id) {}

    void insertAndCheckScopeId(Id scope_id) const override;

    const Exp* returned; 
    Id annotationId;
};

}
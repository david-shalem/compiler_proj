#ifndef __AST_H__
#define __AST_H__

/*************************/
/*                       */
/* general include files */
/*                       */
/*************************/
#include <list>
#include <string>
#include <vector>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <functional>
#include <exception>
#include <map>


#include "Annotations.h"
#include "Visitor.h"
#include "structs.h"

#define EDGE 1
#define GLOBAL_SCOPE_ID 0

#define INSERT true
#define REMOVE false

#define LEFT true
#define RIGHT false

#define NUM_BUFFER_REG "($t0)"
#define BOOL_BUFFER_REG "($t5)"

#define ADDRES_SIZE 4

namespace AST {

class Exp;
/************************/
/*                      */
/* shorthands for types */
/*                      */
/************************/
using str = std::string;
using loc = int;

/*********************/
/*                   */
/* Abstract AST Node */
/*                   */
/*********************/
class ICommand { public:
	/// @brief function call the visitor to visit it's command object 
	/// @param visitor the visitor
	/// @return resulte of the visit (see more info in 'struct.h')
	virtual visitRes Accept(const IVisitor* visitor) const = 0;
	virtual ~ICommand() = default;

	public:

	/// @brief function recive the scope id of the command, save it, and check the relevent actions and commands with it  
	virtual void insertAndCheckScopeId(Id scope_id) const { }

	/// @brief function remove/insert all the local variables in a stack from/to the 'vars' map when the scope is over/starts, 
	/// to enable using the same names in other scopes or to unable using the same names in the scope
	virtual void moveScopeVars(bool in_out) const { }
};


} // namespace AST

#endif

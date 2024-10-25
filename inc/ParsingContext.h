#ifndef __PARSING_CONTEXT_H__
#define __PARSING_CONTEXT_H__

/*************************/
/*                       */
/* general include files */
/*                       */
/*************************/
#include <string>
#include <vector>
#include <map>

/*************************/
/*                       */
/* project include files */
/*                       */
/*************************/
#include "LinkedList.h"
#include "Parser.h"
#include "AST.h"
#include "Exp.h"
#include "Loops.h"
#include "Var.h"
#include "Scopes.h"
#include "Annotations.h"

/****************************/
/* Lexer - Parser Interface */
/****************************/
class ParsingContext {
public:

	const char *code_input;

	bool error_flag = false;

    int loc(const aa::location &l) { return l.begin.line; }

	/***************************************************/
	/* Tokens locations used by the lexer --           */
	/* This location is updated during the lexing, and */
	/* mediated inside the symbol_type of each token   */
	/***************************************************/
	aa::location location;

	/*******/
	/* AST */
	/*******/
	LinkedList<AST::ICommand*>*  commands_head; 
	AST::anMap annotation_map; 

	unsigned int currentScopeId = 0; 
	unsigned int parentScopeId = 0; 
	unsigned int scopeId = 0; 
public:

	ParsingContext(const char *c):code_input(c){}

public:

    // main API function
	std::vector<AST::ICommand*> parse()
	{
		location.initialize();
		scan_begin();
		aa::parser p(*this);
		int status = p();
		scan_end();
		if (status == 0) { /* success */return LinkedList<AST::ICommand*>::move_to_vec(commands_head);    }
		else             { /* failure */return std::vector<AST::ICommand*>(); }
	}

	Id addAnnotationNode() 
	{
		curr_id++;
		annotation_map.insert(std::pair<unsigned long, AST::Annotation_data*>(curr_id, nullptr));
		return curr_id;
	}

	
private:

	// the implementation of these methods
	// is inside the lexer specification file
	void scan_begin();
	void scan_end();

	unsigned long curr_id = 0;

};

// lexing function prototype
#define YY_DECL aa::parser::symbol_type aalex(ParsingContext &ctx)

// lexing function prototype
YY_DECL;

#endif

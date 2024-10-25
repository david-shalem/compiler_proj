/*****************/
/* SKELETON FILE */
/*****************/
%skeleton "lalr1.cc"

/*************************/
/*                       */
/* minimum bison version */
/*                       */
/*************************/
%require "3.4"

/**************************/
/* PREFIX (instead of yy) */
/**************************/
%define api.prefix {aa}

/******************************************************/
/* PROVIDE TOKEN CONSTRUCTORS TO BE USED BY THE LEXER */
/******************************************************/
%define api.token.constructor

/****************************************************/
/* TOKENS and VARIABLES with a certified (C++) type */
/****************************************************/
%define api.value.type variant

/*******************/
/* PARSING CONTEXT */
/*******************/
%define parse.assert

/****************/
/* DEPENDENCIES */
/****************/
%code requires
{
    /*************************/
    /* GENERAL INCLUDE FILES */
    /*************************/
    #include <string>
    #include <list>

    /*************************/
    /* PROJECT INCLUDE FILES */
    /*************************/
    #include "AST.h"
    #include "commands_inc.h"
    #include "Annotations.h"
    #include "SysFunc.h"
    #include "LinkedList.h"
    #include "Calls.h"
    #include "semanticVisitor.h"

    /***********/
    /* CLASSES */
    /***********/
    class ParsingContext;
}

/***********************/
/* THE PARSING CONTEXT */
/***********************/
%param { ParsingContext &ctx }

/*************/
/* LOCATIONS */
/*************/
%locations

/***************************/
/* INCLUDE PARSING CONTEXT */
/***************************/
%code { #include "ParsingContext.h" }

/**********/
/* TOKENS */
/**********/
%define api.token.prefix {TOK_}

/**********/
/* TOKENS */
/**********/
%token END 0 "EOF"
%token NEWLINE "\n"

/* commands */
%token IF "if"
%token ELSE "else"
%token WHILE "while"
%token DO "do"
%token RETURN "return"

/* binops */
%token PLUS   "+"
%token MINUS  "-"
%token TIMES  "*"
%token DIVIDE "/"

/* logical operations */
%token AND   "&"
%token OR    "|"
%token XOR   "^"
%token EQL  "=="
%token NEQL "!="
%token NOT   "!"
%token SEQL "<="
%token BEQL ">="
%token BGR   ">"
%token SMLR  "<"

/* parantheses (all kinds) */
%token LPAREN "("
%token RPAREN ")"
%token LBRACE "{"
%token RBRACE "}"
%token LBRACK "["
%token RBRACK "]"

/* assignment */
%token ASSIGN "="

/* punctuations */
%token COMMA ","

/*************************************************/
/* some tokens have values associated with them: */
/*                                               */
/* - identifiers                                 */
/* - numbers (integers)                          */
/* - strings                                     */
/*                                               */
/*************************************************/
%token <std::string> NAME
%token <std::string> STR
%token <int> INT
%token <float> FLOAT
%token <bool> BOOL
%token <std::string> SYSCALL

/********************/
/*                  */
/* variables' types */
/*                  */
/********************/
/* expressions */
%type <const AST::SysFunc       *> exp_sysFunc
%type <LinkedList<AST::Exp*>    *> argList
%type <LinkedList<AST::Exp*>    *> args
%type <const AST::Exp                *> exp
%type <const AST::Exp_Int            *> exp_int
%type <const AST::Exp_Float          *> exp_float
%type <const AST::Exp_Bool           *> exp_bool
%type <const AST::Exp_Binop          *> exp_binop
%type <const AST::Exp_Str            *> exp_str
%type <const AST::Exp_Var            *> exp_var
%type <const AST::Exp_String         *> binop_strCat
%type <const AST::ICommand           *> command
%type <LinkedList<AST::ICommand*>*> commands
%type <LinkedList<AST::ICommand*>*> code
%type <Type                           > type
%type <const AST::Var                *> var_action
%type <const AST::VarDecl            *> decleration
%type <const AST::If                 *> condition
%type <const AST::While              *> loops
%type <const AST::Scope              *> scope
%type <const AST::Function           *> function_def
%type <const AST::Call               *> call
%type <const AST::Return             *> return
%type <LinkedList<AST::VarDecl*> *> decl_chain
%type <short> pointer_chain

/* precedence and associativity */
%left "|";
%left "^";
%left "&";
%left "<" "<=" ">" ">=" "==" "!=";
%left "+" "-";
%left "*" "/";

%%

/**************/
/*            */
/* start rule */
/*            */
/**************/
%start program;

/******************************/
/*                            */
/* a program is an expression */
/*                            */
/******************************/
program: 
code { ctx.commands_head = $1; };

/*---- code ----*/
code:
commands { $$ = $1; } ;

commands:
"\n" commands { $$ = $2; } | 
command { $$ = new LinkedList<AST::ICommand*>((AST::ICommand*)$1, nullptr); } | 
command "\n" { $$ = new LinkedList<AST::ICommand*>((AST::ICommand*)$1, nullptr); } | 
command "\n" commands { $$ = new LinkedList<AST::ICommand*>((AST::ICommand*)$1, $3); };

command:
function_def { $$ = $1; } |
call         { $$ = $1; } |
exp          { $$ = $1; } |
var_action   { $$ = $1; } |
loops        { $$ = $1; } |
condition    { $$ = $1; } |
return       { $$ = $1; } |
scope        { $$ = $1; } ;

argList:
exp        { $$ = new LinkedList<AST::Exp*>((AST::Exp*)$1, nullptr); } |
exp "," argList {$$ = new LinkedList<AST::Exp*>((AST::Exp*)$1, $3); };

args:
"(" argList ")" {$$ =      $2; } |
"(" ")"         {$$ = nullptr; } ;

/*---- functions ----*/
function_def:
type NAME "(" ")" scope                            { $$ = new AST::Function($5, std::vector<AST::VarDecl*>(), $1, $2); } |
type NAME "(" decl_chain ")" scope { $$ = new AST::Function($6, LinkedList<AST::VarDecl*>::move_to_vec($4), $1, $2); } ;

call:
NAME "(" ")"                       { $$ = new AST::Call($1, std::vector<AST::Exp*>() , ctx.addAnnotationNode()); } |
NAME "(" argList ")" { $$ = new AST::Call($1, LinkedList<AST::Exp*>::move_to_vec($3), ctx.addAnnotationNode()); } ;

return:
RETURN exp { $$ = new AST::Return($2, ctx.addAnnotationNode()); } ;

/*---- loops ----*/
loops:
WHILE "(" exp ")" scope          { $$ = new AST::While($3, $5); } |
DO scope "\n" WHILE "(" exp ")" { $$ = new AST::While($6, $2, true); } ;


/*---- conditions ----*/
condition:
IF "(" exp ")" scope "\n" ELSE condition { $$ = new AST::If($3, $5, $8); } | // if + else if 
IF "(" exp ")" scope "\n" ELSE scope     { $$ = new AST::If($3, $5, $8); } | // if + else
IF "(" exp ")" scope "\n" command         { $$ = new AST::If($3, $5, $7); }| // if + after command
IF "(" exp ")" scope "\n"                     { $$ = new AST::If($3, $5); }| // if
IF "(" exp ")" scope                          { $$ = new AST::If($3, $5); }; // if


/*---- variables ----*/
var_action:
decleration                                                                  { $$ = $1; } |
type NAME "=" exp          { $$ = new AST::VarInit($1, $2, $4,ctx.addAnnotationNode()); } | // type* pointer = addres exp
NAME "=" exp               { $$ = new AST::VarAssign($1, $3, 0,ctx.addAnnotationNode());} | // var '=' expression
pointer_chain NAME "=" exp { $$ = new AST::VarAssign($2, $4, $1,ctx.addAnnotationNode()); } ; // *pointer = expression (var = expression)

decleration:
type NAME  { $$ = new AST::VarDecl($1, $2,ctx.addAnnotationNode()); } ; // type* pointer

decl_chain:
decleration { $$ = new LinkedList<AST::VarDecl*>((AST::VarDecl*)$1, nullptr); } | 
decleration "," decl_chain { $$ = new LinkedList<AST::VarDecl*>((AST::VarDecl*)$1, $3); };

exp_var : 
"&" NAME { $$ = new AST::Exp_Var($2, ctx.addAnnotationNode(), true); } | // pointer = &var
pointer_chain NAME { $$ = new AST::Exp_PVar($2, ctx.addAnnotationNode(), $1); } | // *var
NAME { 
    if(AST::Var::vars[$1].varT.pointerDepth) // if var is a pointer 
        $$ = new AST::Exp_PVar($1, ctx.addAnnotationNode(), 0);
    else
        $$ = new AST::Exp_Var($1, ctx.addAnnotationNode(), false); 
} ; // var

type:
NAME { $$ = Type({ .type = semanticVisitor::checkTypeName($1), .pointerDepth = 0}); } |
NAME pointer_chain { $$ = Type({.type = semanticVisitor::checkTypeName($1), .pointerDepth = $2}); };

pointer_chain:
"*" { $$ = 1; } | 
"*" pointer_chain { $$ = $2 + 1; } ;

/*---- expressions ----*/
exp:
exp_int       { $$ = $1; } | // int
binop_strCat  { $$ = $1; } | // str '+' str
exp_bool      { $$ = $1; } | // bool
exp_binop     { $$ = $1; } | // value/var + operation value/var 
exp_float     { $$ = $1; } | // float
exp_var       { $$ = $1; } | // variable
exp_sysFunc   { $$ = $1; } | // syscalls
call          { $$ = $1; } | // function resulte 
"(" exp ")"   { $$ = $2; } ; 

exp_sysFunc:
SYSCALL args {$$ = new AST::SysFunc(ctx.addAnnotationNode(), $1, LinkedList<AST::Exp*>::move_to_vec($2)); };

binop_strCat:
exp_str { $$ = $1; } | 
exp_str "+" binop_strCat { $$ = new AST::Binop_StrCat($3, $1); } ;

exp_int: INT { $$ = new AST::Exp_Int($1,ctx.loc(@1),ctx.addAnnotationNode()); } ;
exp_float: FLOAT { $$ = new AST::Exp_Float($1,ctx.loc(@1),ctx.addAnnotationNode()); } ;
exp_str: STR { $$ = new AST::Exp_Str($1,ctx.loc(@1),ctx.addAnnotationNode()); } ;
exp_bool: BOOL { $$ = new AST::Exp_Bool($1,ctx.loc(@1),ctx.addAnnotationNode()); } ;


exp_binop:

exp "+" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), ADD); } |
exp "-" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), SUB); } |
exp "*" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), MUL); } |
exp "/" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), DIV); } |

exp "|" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), OR);   } |
exp "&" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), AND);  } |
exp "^" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), XOR);  } |
exp "==" exp { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), EQL);  } |
exp "!" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), NOT);  } |
exp "<=" exp { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), SEQL); } |
exp ">=" exp { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), BEQL); } |
exp "<" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), SMLR); } |
exp "!=" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), NEQL); } |
exp ">" exp  { $$ = new AST::Exp_Binop($1,$3,ctx.addAnnotationNode(), BGR);  } ;


/*---- scopes ----*/
scope:
op_scope commands end_scope { 
    const AST::Scope* scope = new AST::Scope( LinkedList<AST::ICommand*>::move_to_vec($2), ctx.addAnnotationNode());
    $$ =  scope;
    scope->moveScopeVars(REMOVE);
};

op_scope:
"{" |
"\n" "{" |
"{" "\n" | 
"\n" "{" "\n" ;

end_scope: 
"}" |
"\n" "}" ;

%%

/*********/
/* error */
/*********/
void aa::parser::error(const location_type &l, const std::string &m)
{
    std::cerr << l << ": " << m << '\n';
    ctx.error_flag = true;
}


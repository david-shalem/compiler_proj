%{

/*****************/
/* INCLUDE FILES */
/*****************/
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>
#include "ParsingContext.h"
#include "Parser.h"
#include <cfloat>

%}

/**********/
/* PREFIX */
/**********/
%option prefix="aa"

/*****************/
/* OTHER OPTIONS */
/*****************/
%option noyywrap nounput noinput batch

%{
    /****************/
    /* ascii -> int */
    /****************/
    aa::parser::symbol_type make_INT(
        const std::string &s,
        const aa::parser::location_type& loc);

    aa::parser::symbol_type make_FLOAT(
        const std::string &s,
        const aa::parser::location_type& loc);

    aa::parser::symbol_type make_BOOL(
        const std::string &s,
        const aa::parser::location_type& loc);

    int count_NL(const std::string &s);
%}

/***********************************/
/*                                 */
/* non trivial regular expressions */
/*                                 */
/***********************************/
IF         if 
ELSE       else
WHILE      while 
DO         do 
RETURN     return
BOOL       (true)|(false) 
NAME       _*[a-zA-Z][a-zA-Z_0-9]*
STR        \"[^\"]*\"
INT        0|[1-9][0-9]*
FLOAT      ([1-9][0-9]*|0)\.[0-9]+
blank      [ ]
newline    \n[ \n]*
comment    "//"[^\n]*\n|"/*"([^*]|[*][^/])*"*/"
SYSCALL   (printI)|(printF)|(scanI)|(scanF)

%{
    /********************/
    /* HANDLE LOCATIONS */
    /********************/
    // Code run each time a pattern is matched.
#    define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
    // A handy shortcut to the location held by the driver.
    aa::location& loc = ctx.location;
    // Code run each time aalex is called.
    loc.step ();
%}
{newline}    loc.step(); return aa::parser::make_NEWLINE( loc ); 
{blank}      loc.step();
{comment}    loc.step();
%{
/***************/
/*             */
/* parnetheses */
/*             */
/***************/
%}
"(" return aa::parser::make_LPAREN( loc );
")" return aa::parser::make_RPAREN( loc );
"{" return aa::parser::make_LBRACE( loc );
"}" return aa::parser::make_RBRACE( loc );
"[" return aa::parser::make_LBRACK( loc );
"]" return aa::parser::make_RBRACK( loc );
%{
/*****************************/
/*                           */
/* binary operators (binops) */
/*                           */
/*****************************/
%}
"+" return aa::parser::make_PLUS  ( loc );
"-" return aa::parser::make_MINUS ( loc );
"*" return aa::parser::make_TIMES ( loc );
"/" return aa::parser::make_DIVIDE( loc );
%{
/*******************************/
/*                             */
/* relation operators (relops) */
/*                             */
/*******************************/
%}
%{
/*********************/
/*                   */
/* logical operators */
/*                   */
/*********************/
%}
"|"  return aa::parser::make_OR    ( loc );
"&"  return aa::parser::make_AND   ( loc );
"^"  return aa::parser::make_XOR   ( loc );
"!=" return aa::parser::make_NEQL   ( loc );
"==" return aa::parser::make_EQL   ( loc );
"!"  return aa::parser::make_NOT   ( loc );
"<=" return aa::parser::make_SEQL  ( loc );
">=" return aa::parser::make_BEQL  ( loc );
">"  return aa::parser::make_BGR   ( loc );
"<"  return aa::parser::make_SMLR  ( loc );
%{
/*********************/
/*                   */
/* reserved keywords */
/*                   */
/*********************/
%}
%{
/**********/
/*        */
/* assign */
/*        */
/**********/
%}
"=" return aa::parser::make_ASSIGN  ( loc );
%{
/************/
/*          */
/* commands */
/*          */
/************/
%}
{IF} return aa::parser::make_IF(loc);
{ELSE} return aa::parser::make_ELSE(loc);
{WHILE} return aa::parser::make_WHILE(loc);
{DO} return aa::parser::make_DO(loc);
{RETURN} return aa::parser::make_RETURN(loc);
%{
/************/
/*          */
/* Literals */
/*          */
/************/
%}
{BOOL}     return                make_BOOL(yytext,loc);
{SYSCALL}  return aa::parser::make_SYSCALL(yytext,loc);
{INT}      return                 make_INT(yytext,loc);
{STR}      return     aa::parser::make_STR(yytext,loc);
{NAME}     return    aa::parser::make_NAME(yytext,loc);
{FLOAT}    return               make_FLOAT(yytext,loc);
%{
/****************/
/*              */
/* punctuations */
/*              */
/***************/
%}
"," return aa::parser::make_COMMA ( loc );
%{
/*********/
/*       */
/* error */
/*       */
/*********/
%}
.       {
            throw aa::parser::syntax_error(
                loc,
                "invalid character: " +
                std::string(yytext));
        }
%{
/*******/
/*     */
/* EOF */
/*     */
/*******/
%}
<<EOF>> return aa::parser::make_END (loc);

%%

aa::parser::symbol_type make_INT (const std::string &s, const aa::parser::location_type& loc)
{
  errno = 0;
  long n = strtol (s.c_str(), NULL, 10);
  if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE))
    throw aa::parser::syntax_error (loc, "integer is out of range: " + s);
  return aa::parser::make_INT ((int) n, loc);
}

aa::parser::symbol_type make_FLOAT (const std::string &s, const aa::parser::location_type& loc)
{
  errno = 0;
  double n = std::stod(s);
  if (! (FLT_MIN <= n && n <= FLT_MAX && errno != ERANGE))
    throw aa::parser::syntax_error (loc, "float is out of range: " + s);
  return aa::parser::make_FLOAT (n, loc);
}

aa::parser::symbol_type make_BOOL (const std::string &s, const aa::parser::location_type& loc)
{
  errno = 0;
  bool n = (s == "true");
  return aa::parser::make_BOOL (n, loc);
}

int count_NL(const std::string &s) {
    int nl = 0;
    for (auto ch : s) if (ch == '\n') ++nl;
    return nl;
}

// implement methods declared in ParsingContext
void ParsingContext::scan_begin()
{
    aain = fopen(code_input,"rt");
	assert(aain && "cannot open input file");
}

// implement methods declared in ParsingContext
void ParsingContext::scan_end() { (void) fclose(aain); }


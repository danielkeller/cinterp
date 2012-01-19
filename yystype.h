/*******************************************************************************
*
* FILE:		yystype.h for java2cpp compiler
*
* DESC:		EECS 337 project
*
* AUTHOR:	caseid
*
* DATE:		October 14, 2010
*
* EDIT HISTORY:	
*
*******************************************************************************/
#ifndef	YYSTYPE_H
#define	YYSTYPE_H	1

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdbool.h>
#include	<string>
#include	<vector>
#include	<stack>
#include	<map>

     # define YYLLOC_DEFAULT(Current, Rhs, N)                                \
         do                                                                  \
           if (N)                                                            \
             {                                                               \
               (Current).first_line   = YYRHSLOC(Rhs, 1).first_line;         \
               (Current).first_column = YYRHSLOC(Rhs, 1).first_column;       \
               (Current).last_line    = YYRHSLOC(Rhs, N).last_line;          \
               (Current).last_column  = YYRHSLOC(Rhs, N).last_column;        \
               (Current).filename     = YYRHSLOC(Rhs, 1).filename;           \
             }                                                               \
           else                                                              \
             {                                                               \
               (Current).first_line   = (Current).last_line   =              \
                 YYRHSLOC(Rhs, 0).last_line;                                 \
               (Current).first_column = (Current).last_column =              \
                 YYRHSLOC(Rhs, 0).last_column;                               \
               (Current).filename     = YYRHSLOC(Rhs, 0).filename;           \
             }                                                               \
         while (0)

/*
 *	define for yyparser debugging
 */
#define	YYDEBUG	1

#define YYLTYPE YYLTYPE
struct YYLTYPE
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
	char* filename;
};

struct Stmt;

template<class A, class B>
struct Pair
{
	A a; B b;
	Pair(A aa, B bb) : a(aa), b(bb) {}
	~Pair() {delete a; delete b;}
};

union YYSTYPE;
struct YYLVAL;

/*
 *	external variables and functions from scan.l
 */
extern int yylex (YYSTYPE *yylval, YYLTYPE *yyloc);
extern int check_type( void);

/*
 *	external variables and functions from gram.y
 */
extern	int	yydebug;
extern	int	yyparse( void);
extern	void yyerror(YYLTYPE* loc, int tok, char *s);
extern	void yywarn(YYLTYPE* loc, char *s);
/*
 *	external variables and functions from main.c
 */
extern	int	main( int argc, char *argv[]);

#endif


%{
#include <stdio.h>
#include "yystype.h"
#include "type.h"
#include "evaltree.h"
#include "y.tab.h"
%}

%define api.pure
%define api.push_pull "push"
%locations
%defines
%error-verbose
%parse-param {int tok}
//%parse-param {bool eval}

%union
{
	std::string * txt_v;
	long long_v;
	double dbl_v;
	std::vector<Lval> * var_v;
	Expr * expr_v;
	Stmt * stmt_v;
	Block * block_v;
	Type * type_v;
};

%token <txt_v> IDENTIFIER STRING_LITERAL
%token <long_v> CONSTANT
%token SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
%token CHAR INT FLOAT DOUBLE VOID
%token BOOL COMPLEX 32 IMAGINARY 128
%token SHORT 1 LONG 2 LONGLONG 4
%token SIGNED 8 UNSIGNED 16
%token VOLATILE CONST

%start translation_unit

%type <type_v> type_specifier type_modifier type_qualifier storage_class_specifier function_specifier declaration_specifiers pointer specifier_qualifier_list abstract_declarator direct_abstract_declarator type_name
%type <var_v> declaration declarator direct_declarator init_declarator_list init_declarator
%type <block_v> block_item_list iteration_statement
%type <stmt_v> statement compound_statement selection_statement
%type <expr_v> primary_expression postfix_expression unary_expression cast_expression multiplicative_expression additive_expression shift_expression relational_expression equality_expression and_expression exclusive_or_expression inclusive_or_expression logical_and_expression logical_or_expression conditional_expression assignment_expression constant_expression expression initializer expression_statement
%%

primary_expression
	: IDENTIFIER
	{
		$$ = new LvalExpr(*$1, @1);
		delete $1;
	}
	| CONSTANT
	{
		Rval rv;
		rv().sintv = $1;
		rv.type = signed_t + int_t;
		$$ = new ConstExpr(rv, @1);
	}
	| STRING_LITERAL
	{
		char * str = strdup($1->c_str());
		delete $1;
		Rval rv;
		rv.type = signed_t + const_t + char_t;
		rv.type.indirection = 1;
		rv().ptrv = (void*)str;
		$$ = new ConstExpr(rv, @1);
	}
	| '(' expression ')'
	{
		$$ = $2;
	}
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	{
		LvalExpr * le = dynamic_cast<LvalExpr*>($1);
		if (le)
			$$ = new LvalBinExpr<index>(le, $3, @3);
		else
		{
			yyerror(&@1, 0, "Array index operand must be lval");
			YYERROR;
		}
	}
	| postfix_expression '(' ')'
	| postfix_expression '(' argument_expression_list ')'
	| postfix_expression '.' IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP
	{
		LvalExpr * le = dynamic_cast<LvalExpr*>($1);
		if (le)
			$$ = new LvalUnExpr<post_inc>(le, @2);
		else
		{
			yyerror(&@2, 0, "Increment of non-lval");
			YYERROR;
		}
	}
	| postfix_expression DEC_OP
	{
		LvalExpr * le = dynamic_cast<LvalExpr*>($1);
		if (le)
			$$ = new LvalUnExpr<post_dec>(le, @2);
		else
		{
			yyerror(&@2, 0, "Decrement of non-lval");
			YYERROR;
		}
	}
	| '(' type_name ')' '{' initializer_list '}'
	| '(' type_name ')' '{' initializer_list ',' '}'
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	{
		LvalExpr * le = dynamic_cast<LvalExpr*>($2);
		if (le)
			$$ = new LvalUnExpr<pre_inc>(le, @1);
		else
		{
			yyerror(&@1, 0, "Increment of non-lval");
			YYERROR;
		}
	}
	| DEC_OP unary_expression
	{
		LvalExpr * le = dynamic_cast<LvalExpr*>($2);
		if (le)
			$$ = new LvalUnExpr<pre_dec>(le, @1);
		else
		{
			yyerror(&@1, 0, "Decrement of non-lval");
			YYERROR;
		}
	}
	| '&' cast_expression
	{
		LvalExpr * le = dynamic_cast<LvalExpr*>($2);
		if (le)
			$$ = new LvalUnExpr<addr_of>(le, @1);
		else
		{
			yyerror(&@1, 0, "Addressof of non-lval");
			YYERROR;
		}
	}
	| '*' cast_expression
	{
		$$ = new UnExprL<deref>($2, @1);
	}
	| '+' cast_expression
	| '-' cast_expression
	{
		$$ = new UnExpr<negate>($2, @1);
	}
	| '~' cast_expression
	| '!' cast_expression
	| SIZEOF unary_expression
	| SIZEOF '(' type_name ')'
	;

cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression
	{
		$$ = new CastExpr($4, *$2, @2);
		delete $2;
	}
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression
	{
		$$ = new BinExpr<product>($1, $3, @2);
	}
	| multiplicative_expression '/' cast_expression
	{
		$$ = new BinExpr<quotient>($1, $3, @2);
	}
	| multiplicative_expression '%' cast_expression
	{
		$$ = new BinExpr<mod>($1, $3, @2);
	}
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	{
		$$ = new BinExpr<sum>($1, $3, @2);
	}
	| additive_expression '-' multiplicative_expression
	{
		$$ = new BinExpr<difference>($1, $3, @2);
	}
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	{
		$$ = new BinExpr<less>($1, $3, @2);
	}
	| relational_expression '>' shift_expression
	{
		$$ = new BinExpr<greater>($1, $3, @2);
	}
	| relational_expression LE_OP shift_expression
	{
		$$ = new BinExpr<lesseq>($1, $3, @2);
	}
	| relational_expression GE_OP shift_expression
	{
		$$ = new BinExpr<greatereq>($1, $3, @2);
	}
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	{
		$$ = new BinExpr<equal>($1, $3, @2);
	}
	| equality_expression NE_OP relational_expression
	{
		$$ = new BinExpr<nequal>($1, $3, @2);
	}
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	{
		$$ = new BinExpr<land>($1, $3, @2);
	}
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	{
		$$ = new BinExpr<lor>($1, $3, @2);
	}
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression '=' assignment_expression
	{
		LvalExpr * le = dynamic_cast<LvalExpr*>($1);
		if (le)
			$$ = new LvalBinExpr<assign>(le, $3, @2);
		else
		{
			yyerror(&@1, 0, "Assignment to non-lval");
			YYERROR;
		}
	}
	| unary_expression MUL_ASSIGN assignment_expression
	| unary_expression DIV_ASSIGN assignment_expression
	| unary_expression MOD_ASSIGN assignment_expression
	| unary_expression ADD_ASSIGN assignment_expression
	| unary_expression SUB_ASSIGN assignment_expression
	| unary_expression LEFT_ASSIGN assignment_expression
	| unary_expression RIGHT_ASSIGN assignment_expression
	| unary_expression AND_ASSIGN assignment_expression
	| unary_expression XOR_ASSIGN assignment_expression
	| unary_expression OR_ASSIGN assignment_expression
	;

expression
	: assignment_expression
	| expression ',' assignment_expression
	;

constant_expression
	: conditional_expression
	;

declaration
	: declaration_specifiers ';'
	| declaration_specifiers init_declarator_list ';'
	{
		if (!$1->fix(&@1))
			YYERROR;
		
		for (std::vector<Lval>::iterator it = $2->begin(); it != $2->end(); ++it)
		{	
			it->type = *$1;
		}
		$$ = $2;
	}
	;

declaration_specifiers
	: storage_class_specifier {$$ = $1;}
	| storage_class_specifier declaration_specifiers {$$ = $1->add($2);}
	| type_qualifier {$$ = $1;}
	| type_qualifier declaration_specifiers {$$ = $1->add($2);}
	| function_specifier {$$ = $1;}
	| function_specifier declaration_specifiers {$$ = $1->add($2);}
	| type_modifier {$$ = $1;}
	| type_modifier declaration_specifiers {$$ = $1->add($2);}
	| type_specifier {$$ = $1;}
	;

init_declarator_list
	: init_declarator
	| init_declarator_list ',' init_declarator {$$ = $1; $$->push_back($3->front()); delete $3;}
	;

init_declarator
	: declarator {$$ = $1;}
	| declarator '=' initializer
	{
		$$ = $1;
		$$->front().init = $3;
	} 
	;

storage_class_specifier
	: TYPEDEF
	| EXTERN
	| STATIC
	| AUTO
	| REGISTER
	;

type_modifier
	: SHORT		{$$ = new Type(short_t);}
	| LONG		{$$ = new Type(long_t);}
	| SIGNED	{$$ = new Type(signed_t);}
	| UNSIGNED	{$$ = new Type(unsigned_t);}
	;

type_specifier
	: VOID		{$$ = new Type(void_t);}
	| CHAR		{$$ = new Type(char_t);}
	| INT		{$$ = new Type(int_t);}
	| FLOAT		{$$ = new Type(float_t);}
	| DOUBLE	{$$ = new Type(double_t);}
	| BOOL		{$$ = new Type(bool_t);}
	| COMPLEX	{$$ = 0;}
	| IMAGINARY	{$$ = 0;}
	| struct_or_union_specifier
	| enum_specifier
	| TYPE_NAME
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list {$$ = $1->add($2);}
	| type_specifier {$$ = $1;}
	| type_qualifier specifier_qualifier_list {$$ = $1->add($2);}
	| type_qualifier {$$ = $1;}
	| type_modifier specifier_qualifier_list {$$ = $1->add($2);}
	| type_modifier {$$ = $1;}
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expression
	| declarator ':' constant_expression
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
	| ENUM '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
	| IDENTIFIER '=' constant_expression
	;

type_qualifier
	: CONST		{$$ = new Type(const_t);}
	| RESTRICT
	| VOLATILE
	;

function_specifier
	: INLINE
	;

declarator
	: pointer direct_declarator
	{
		$2->front().type.add($1);
		$$ = $2;
	}
	| direct_declarator
	;


direct_declarator
	: IDENTIFIER
	{
		$$ = new std::vector<Lval>();
		Lval v;
		v.name = *$1;
		delete $1;
		$$->push_back(v);
	}
	| '(' declarator ')'
	| direct_declarator '[' type_qualifier_list assignment_expression ']'
	| direct_declarator '[' type_qualifier_list ']'
	| direct_declarator '[' assignment_expression ']'
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list '*' ']'
	| direct_declarator '[' '*' ']'
	| direct_declarator '[' ']'
	| direct_declarator '(' parameter_type_list ')'
	| direct_declarator '(' identifier_list ')'
	| direct_declarator '(' ')'
	;

pointer
	: '*'
	{
		$$ = new Type();
		$$->indirection++;
	}
	| '*' type_qualifier_list
	{
		$$ = new Type();
		$$->qualifiers[1] = true;
	}
	| '*' pointer
	{
		$$ = $2;
		$$->indirection++;
	}
	| '*' type_qualifier_list pointer
	{
		$$ = $3;
		$$->indirection++;
		$$->qualifiers[$$->indirection] = true;
	}
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;


parameter_type_list
	: parameter_list
	| parameter_list ',' ELLIPSIS
	;

parameter_list
	: parameter_declaration
	| parameter_list ',' parameter_declaration
	;

parameter_declaration
	: declaration_specifiers declarator
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
	: specifier_qualifier_list
	{
		if (!$$->fix(&@1))
			YYERROR;
	}
	| specifier_qualifier_list abstract_declarator
	{
		$$ = $1->add($2);
		if (!$$->fix(&@1))
			YYERROR;
	}
	;

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
	{
		$$ = $1; //->add($2);
	}
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' assignment_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' assignment_expression ']'
	| '[' '*' ']'
	| direct_abstract_declarator '[' '*' ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: assignment_expression
	| '{' initializer_list '}'
	| '{' initializer_list ',' '}'
	;

initializer_list
	: initializer
	| designation initializer
	| initializer_list ',' initializer
	| initializer_list ',' designation initializer
	;

designation
	: designator_list '='
	;

designator_list
	: designator
	| designator_list designator
	;

designator
	: '[' constant_expression ']'
	| '.' IDENTIFIER
	;

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'
	{
		$$ = new Block();
	}
	| '{' block_item_list '}'
	{
		$$ = $2;
	}
	;

block_item_list
	: declaration
	{
		$$ = new Block();
		for (std::vector<Lval>::iterator it = $1->begin(); it != $1->end(); ++it)
		{
			$$->context[it->name] = *it;
			if (it->init)
				$$->add(new LvalBinExpr<assign>(new LvalExpr(it->name, @1), it->init, @1) );
		}
		delete $1;
	}
	| block_item_list declaration
	{
		$$ = $1;
		for (std::vector<Lval>::iterator it = $2->begin(); it != $2->end(); ++it)
		{
			$$->context[it->name] = *it;
			if (it->init)
				$$->add(new LvalBinExpr<assign>(new LvalExpr(it->name, @2), it->init, @2) );
		}
		delete $2;
	}
	| statement
	{
		$$ = new Block();
		$$->add($1);
	}
	| block_item_list statement
	{
		$$ = $1;
		$$->add($2);
	}
	;

expression_statement
	: ';'
	{
		Rval v;
		v().ulongv = 0;
		$$ = new ConstExpr(v, @1);
	}
	| expression ';'
	;

selection_statement
	: IF '(' expression ')' statement
	{
		$$ = new CondStmt($3, $5, new Stmt(), @1);
	}
	| IF '(' expression ')' statement ELSE statement
	{
		$$ = new CondStmt($3, $5, $7, @1);
	}
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
	{
		$$ = new Block();
		$$->add(new LoopStmt($3, $5, @1));
	}
	| DO statement WHILE '(' expression ')' ';'
	{
		$$ = new Block();
		$$->add($2);
		$$->add(new LoopStmt($5, $2, @3));
	}
	| FOR '(' expression_statement expression_statement ')' statement
	{
		$$ = new Block();
		$$->add($3);
		$$->add(new LoopStmt($4, $6, @1));
	}
	| FOR '(' expression_statement expression_statement expression ')' statement
	{
		$$ = new Block();
		Block * innerb = new Block();
		innerb->add($7);
		innerb->add($5);
		$$->add($3);
		$$->add(new LoopStmt($4, innerb, @1));
	}
	| FOR '(' declaration expression_statement ')' statement
	{
		$$ = new Block();
		for (std::vector<Lval>::iterator it = $3->begin(); it != $3->end(); ++it)
		{
			$$->context[it->name] = *it;
			if (it->init)
				$$->add(new LvalBinExpr<assign>(new LvalExpr(it->name, @1), it->init, @1) );
		}
		delete $3;
		$$->add(new LoopStmt($4, $6, @1));
	}
	| FOR '(' declaration expression_statement expression ')' statement
	{
		$$ = new Block();
		for (std::vector<Lval>::iterator it = $3->begin(); it != $3->end(); ++it)
		{
			$$->context[it->name] = *it;
			if (it->init)
				$$->add(new LvalBinExpr<assign>(new LvalExpr(it->name, @1), it->init, @1) );
		}
		delete $3;
		Block * innerb = new Block();
		innerb->add($7);
		innerb->add($5);
		$$->add(new LoopStmt($4, innerb, @1));
	}
	;

jump_statement
	: GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN expression ';'
	;

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement
	{
		if (!$4->check())
			YYERROR;
		$4->eval(); //whheeeeeee
		delete $4;
	}
	| declaration_specifiers declarator compound_statement
	{
		if (!$3->check())
			YYERROR;
		//yyerror(&@2, 0, "sadf");
		$3->eval(); //whheeeeeee
		delete $3;
	}
	;

declaration_list
	: declaration
	| declaration_list declaration
	;


%%

void yyerror(YYLTYPE* loc, int tok, char *s)
{
	fflush(stdout);
	printf("Error in %s at line %d: %s\n", loc->filename, loc->first_line, s);
}

void yywarn(YYLTYPE* loc, char *s)
{
	fflush(stdout);
	printf("Warning in %s at line %d: %s\n", loc->filename, loc->first_line, s);
}

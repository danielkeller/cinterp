#include "type.h"
#include "variable.h"
#include <string>
#include <vector>
struct Stmt;
struct Expr;
struct Block;
#include "yystype.h"
#include "y.tab.h"
#include <string.h>
#include <ostream>

Type char_t(CHAR), int_t(INT), void_t(VOID), float_t(FLOAT), double_t(DOUBLE), bool_t(BOOL);
Type short_t(SHORT), long_t(LONG), llong_t(LONGLONG), signed_t(SIGNED), unsigned_t(UNSIGNED), const_t(CONST);

Type::Type()
{
	memset(this, 0, sizeof(Type));
}

Type::Type(int init)
{
	memset(this, 0, sizeof(Type));
	operator=(init);
}

std::string Type::toString()
{
	std::string out;
	if (qualifiers[0])
		out += "const ";
	
	if (modifiers & UNSIGNED)
		out += "unsigned ";
	if (modifiers & SIGNED)
		out += "signed ";
	
	if (modifiers & SHORT)
		out += "short ";
	if (modifiers & LONG)
		out += "long ";
	if (modifiers & LONGLONG)
		out += "longlong ";
	
	switch (type)
	{
		case CHAR:	out += "char "; break;
		case INT:	out += "int "; break;
		case FLOAT:	out += "float "; break;
		case DOUBLE:out += "double "; break;
		case BOOL:	out += "bool "; break;
		case VOID:	out += "void "; break;
		default: break;
	}
	
	for (int i=1; i<=indirection; i++)
	{
		out += "* ";
		if (qualifiers[i])
			out += "const ";
	}
	return out;
}

std::ostream& operator<< (std::ostream& os, Type& type)
{
	os << type.toString();
	return os;
}

int Type::size()
{
	if (indirection > 0)
		return sizeof(void*);
	if (type == CHAR)
		return sizeof(char);
	if (modifiers & SHORT)
		return sizeof(short);
	if (type == INT && modifiers & LONG)
		return sizeof(long);
	if (modifiers & LONGLONG)
		return sizeof(long long);
	if (type == INT)
		return sizeof(int);
}

Type Type::operator= (int it)
{
	if (it == CHAR || it == INT || it == FLOAT || it == DOUBLE || it == VOID || it == BOOL)
		type = it;
	else if (it == SHORT || it == LONG || it == LONGLONG || it == SIGNED || it == UNSIGNED)
		modifiers |= it;
	else if (it == EXTERN || it == STATIC)
		specifiers |= it;
	else if (it == CONST)
		qualifiers[0] = true;
	else 
		std::cout << "Invalid type index: " << it << std::endl;
}

Type Type::operator= (Type t)
{
	add(&t);
}

Type Type::operator+ (Type t)
{
	Type ret = *this;
	for (int i = 0; i < sizeof(Type)/sizeof(int); i++)
		((int*)&ret)[i] |= ((int*)&t)[i];
	
	return ret;
}

Type Type::operator- (Type t)
{
	Type ret = *this;
	for (int i = 0; i < sizeof(Type)/sizeof(int); i++)
		((int*)&ret)[i] &= ~((int*)&t)[i];
	
	return ret;
}

Type* Type::add(Type *t)
{
	for (int i = 0; i < sizeof(Type)/sizeof(int); i++)
		((int*)this)[i] |= ((int*)t)[i];
	return this;
}

bool Type::operator== (Type t)
{
	for (int i = 0; i < sizeof(Type)/sizeof(int); i++)
		if(((int*)this)[i] != ((int*)&t)[i])
			return false;
	return true;
}

bool Type::operator& (Type t)
{
	if (type != 0 && type == t.type)
		return true;
	
	for (int i = 1; i < sizeof(Type)/sizeof(int); i++)
	{
		if(((int*)this)[i] & ((int*)&t)[i])
			return true;
	}
	
	return false;
}

bool Type::fix (YYLTYPE *loc) //TODO: do this right
{
	if (*this & unsigned_t)
	{
		if (*this & signed_t)
		{
			yyerror(loc, 0, "Invalid type, cannot be signed and unsigned");
			return false;
		}
	}
	else
		*this = signed_t;
		
	if ((*this & short_t || *this & long_t || *this & llong_t) && (*this & char_t || *this & float_t))
	{
		yyerror(loc, 0, "Invalid type, char and float cannot be short/long");
		return false;
	}
	if (*this & double_t && (*this & short_t || *this & llong_t))
	{
		yyerror(loc, 0, "Invalid type, double cannot be short/longlong");
		return false;
	}
	
	if (*this & short_t || *this & long_t || *this & llong_t)
		*this = int_t;
	return true;
}

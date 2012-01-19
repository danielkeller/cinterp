#ifndef _TYPE_H_
#define _TYPE_H_

#include <string>
#include <ostream>

struct YYLTYPE;

struct Type
{
	int type;
	int modifiers;
	int specifiers;
	bool qualifiers [8];
	int dims [8];
	int indirection;
	
	Type operator= (int it);
	Type operator= (Type t);
	Type operator+ (Type t);
	Type operator- (Type t);
	bool operator== (Type t);
	bool operator& (Type t);
	Type * add(Type *t);
	std::string toString();
	int size();
	bool fix(YYLTYPE *loc);
	Type();
	Type(int init);
};

extern Type char_t, int_t, void_t, float_t, double_t, bool_t;
extern Type short_t, long_t, llong_t, signed_t, unsigned_t, const_t;

std::ostream& operator<< (std::ostream& os, Type& type);

#endif

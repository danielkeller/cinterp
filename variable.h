#include <map>
#include <stack>
#include <string>
#include <iostream>
#include "type.h"

struct Expr;
	
union Value {
	unsigned char  ucharv;
	signed char    scharv;
	unsigned short int ushortv;
	signed short int   sshortv;
	unsigned int uintv;
	signed int   sintv;
	unsigned long int ulongv;
	signed long int   slongv;
	unsigned long long int ullongv;
	signed long long int sllongv;
	float floatv;
	double doublev;
	long double ldoublev;
	bool boolv;
	void * ptrv;
	Value() {ullongv = 0;}
	//std::string toString(Type t);
};
	
struct Rval
{
	Type type;
	Value val;
	Value & operator()() {return val;}
	Rval () {}
};

struct Lval
{
	Lval() {pval = new Value(); init = 0;}
	Lval(Value * v) {pval = v; init = 0;}
	Type type;
	std::string name;
	Expr * init;
	Value * pval;
	operator Rval();
	Value & operator()() {return *pval;}
};

typedef Rval (*binOp)(Rval, Rval);
typedef Rval (*unOp)(Rval);
typedef Lval (*unOpL)(Rval);
typedef Lval (*lvalBinOp)(Lval, Rval);
typedef Rval (*lvalUnOp)(Lval);

Rval sum(Rval l, Rval r);
Rval difference(Rval l, Rval r);
Rval product(Rval l, Rval r);
Rval quotient(Rval l, Rval r);
Rval mod(Rval l, Rval r);

Rval equal(Rval l, Rval r);
Rval nequal (Rval l, Rval r);
Rval less(Rval l, Rval r);
Rval greater (Rval l, Rval r);
Rval lesseq (Rval l, Rval r);
Rval greatereq (Rval l, Rval r);
Rval land (Rval l, Rval r);
Rval lor (Rval l, Rval r);

Rval negate (Rval l);

Lval deref(Rval l);

Rval pre_inc(Lval l);
Rval pre_dec(Lval l);
Rval post_inc(Lval l);
Rval post_dec(Lval l);
Rval addr_of(Lval l);

Lval assign(Lval l, Rval r);
Lval index(Lval l, Rval r);

void clean(Value & val, Type type);
void cast(Value & val, Type from, Type to);
Type auto_type(Type ltype, Type rtype);
Type do_types(Rval & l, Rval & r);
Type size(Type type);

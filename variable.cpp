#include "yystype.h"
#include "variable.h"
#include "string.h"
#include <vector>

struct Stmt;
struct Block;

#include "y.tab.h"
/*
std::string Value::toString(Type t)
{
	char[256] buf;
	
}*/

//-----------------------------------Rval binary
Rval sum(Rval l, Rval r)
{
	Rval ret;
	
	ret.type = do_types(l, r);
	ret().ullongv = l().ullongv + r().ullongv;
	return ret;
}

Rval difference(Rval l, Rval r)
{
	Rval ret;
	ret.type = do_types(l, r);
	ret().ullongv = l().ullongv - r().ullongv;
	return ret;
}

Rval product(Rval l, Rval r)
{
	Rval ret;
	ret.type = do_types(l, r);
	ret().ullongv = l().ullongv * r().ullongv;
	return ret;
}

Rval quotient(Rval l, Rval r)
{
	Rval ret;
	ret.type = do_types(l, r);
	ret().ullongv = l().ullongv / r().ullongv;
	return ret;
}

Rval mod(Rval l, Rval r)
{
	Rval ret;
	ret.type = do_types(l, r);
	ret().ullongv = l().ullongv % r().ullongv;
	return ret;
}

//-----------------------------------Logical

Rval equal(Rval l, Rval r)
{
	Rval ret;
	ret.type = bool_t;
	do_types(l, r);
	ret().boolv = (l().ullongv == r().ullongv);
	return ret;
}

Rval nequal (Rval l, Rval r)
{
	Rval ret;
	ret.type = bool_t;
	do_types(l, r);
	ret().boolv = (l().ullongv != r().ullongv);
	return ret;
}

Rval less(Rval l, Rval r)
{
	Rval ret;
	ret.type = bool_t;
	do_types(l, r);
	ret().boolv = (l().ullongv < r().ullongv);
	return ret;
}

Rval greater (Rval l, Rval r)
{
	Rval ret;
	ret.type = bool_t;
	do_types(l, r);
	ret().boolv = (l().ullongv > r().ullongv);
	return ret;
}

Rval lesseq (Rval l, Rval r)
{
	Rval ret;
	ret.type = bool_t;
	do_types(l, r);
	ret().boolv = (l().ullongv <= r().ullongv);
	return ret;
}

Rval greatereq (Rval l, Rval r)
{
	Rval ret;
	ret.type = bool_t;
	do_types(l, r);
	ret().boolv = (l().ullongv >= r().ullongv);
	return ret;
}

Rval land (Rval l, Rval r)
{
	Rval ret;
	ret.type = bool_t;
	ret().boolv = (l().boolv && r().boolv);
	return ret;
}

Rval lor (Rval l, Rval r)
{
	Rval ret;
	ret.type = bool_t;
	ret().boolv = (l().boolv || r().boolv);
	return ret;
}

//-----------------------------------Rval unary
Rval negate (Rval l)
{
	Rval ret;
	ret.type = l.type;
	ret().ullongv = -l().ullongv;
	return ret;
}

Lval deref(Rval l)
{
	if (l.type.indirection < 1)
	{
		std::cout << "Dereference of non-pointer value\n" << std::endl;
		exit(-1);
	}
	Lval ret((Value*)l().ptrv);
	ret.type.indirection = l.type.indirection - 1;
	ret.type = l.type;
	return ret;
}

//-----------------------------------Lval unary
Rval pre_inc(Lval l)
{
	l().ullongv++;
	return l;
}

Rval pre_dec(Lval l)
{
	l().ullongv--;
	return l;
}

Rval post_inc(Lval l)
{
	Rval ret = l;
	l().ullongv++;
	return ret;
}

Rval post_dec(Lval l)
{
	Rval ret = l;
	l().ullongv--;
	return ret;
}

Rval addr_of(Lval l)
{
	Rval ret;
	ret.type = l.type;
	ret.type.indirection = l.type.indirection + 1;
	ret().ptrv = &l().ullongv;
	return ret;
}

//-----------------------------------Lval binary
Lval assign(Lval l, Rval r)
{
	std::cout << l.name << " (" << l.type.toString() << ") (" << &l().ullongv << ") = " << r().ullongv << " (" << r.type.toString() << ")" << std::endl;
	
	cast(r(), r.type, l.type);
	l().ullongv = r().ullongv;
	
	return l;
}

Lval index(Lval l, Rval r)
{
	if (!l.type.indirection)
		std::cout << "Operand must be of array type" << std::endl;
	Type rt = l.type;
	rt.indirection = l.type.indirection - 1;
	cast(r(), r.type, signed_t + int_t);
//	std::cout << l().ptrv << " " << r().sintv << " " << l.type.size() << std::endl;
	Lval ret ((Value*)(l().ptrv + r().sintv * rt.size()));
	ret.type = rt;
	return ret;
}

Lval::operator Rval()
{
	Rval ret;
	ret.type = type;
	ret.val = *pval;
	clean(ret.val, ret.type);
	return ret;
}

//-----------------------------------Type conversions
Type do_types(Rval & l, Rval & r)
{
	Type type = auto_type(l.type, r.type);
	cast(l(), l.type, type);
	cast(r(), r.type, type);

	return type;
}

inline Type greater_rank(Type ltype, Type rtype)
{
	return ltype.size() > rtype.size() ? ltype : rtype;
}

Type auto_type(Type ltype, Type rtype)
{
	if (ltype == rtype)
		return ltype;
	if (ltype & signed_t && rtype & signed_t || ltype & unsigned_t && rtype & unsigned_t)
		return greater_rank(ltype, rtype);
	if (ltype & unsigned_t)
		if(ltype == greater_rank(ltype, rtype))
			return ltype;
		else
			return rtype;
	if (rtype & unsigned_t)
		if(rtype == greater_rank(ltype, rtype))
			return rtype;
		else
			return ltype;
	if (ltype & signed_t)
		return ltype - signed_t + unsigned_t;
	else
		return rtype - signed_t + unsigned_t;
}

void clean(Value & val, Type t)
{
	memset(((void*)&val)+t.size(), 0, sizeof(Value)-t.size());
	//TODO: warn about data loss if neccesary
}

void cast(Value & val, Type from, Type to)
{
	if (from.indirection)
		from = unsigned_t + int_t;
	if (to.indirection)
		to = unsigned_t + int_t;

	if (from == to)
		return;
	if ((from & char_t || from & int_t) && (to & char_t || to & int_t))
	{
		if (from.size() > to.size())
		{
			clean(val, to); //easy!
			return;
		}
		else if (from & unsigned_t) //sign ext. is only done if it starts out signed
			return; //easier!
		else
		{
			if (from & char_t)
			{
				if (to & short_t) val.sshortv = val.scharv;
				else if (to & long_t) val.slongv = val.scharv;
				else if (to & llong_t) val.slongv = val.scharv;
				else val.sintv = val.scharv;
			}
			else if (from & short_t)
			{
				if (to & long_t) val.slongv = val.scharv;
				else if (to & llong_t) val.slongv = val.scharv;
				else val.sintv = val.scharv;
			}
			else if (from & long_t) val.slongv = val.scharv;
			else
			{
				if (to & long_t) val.slongv = val.scharv;
				else val.slongv = val.scharv;
			}
		}
	} //TODO: Add float-float and float-int
}

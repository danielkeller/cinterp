#include "yystype.h"
#include "variable.h"
#include <iostream>

//TODO: only exprs check, check is done when reducing expr, for loop with decl is separate reduction
// ie push stack when reducing for (), pop stack when reducing for () stmt;
// also check returns type

typedef std::map<std::string, Lval> ctx;

extern ctx global;
extern std::vector<ctx> ctstack;

struct Stmt
{
	YYLTYPE loc;
	virtual void eval() {};
	virtual bool check() {};
	virtual ~Stmt() {};
};

struct Block : public Stmt
{
	std::vector<Stmt*> stmts;
	ctx context;
	void eval();
	~Block();
	
	bool check();
	
	inline void add(Stmt* s) {stmts.push_back(s);}
};

struct CondStmt : public Stmt
{
	Expr * cond;
	Stmt * iftrue;
	Stmt * iffalse;
	bool check();
	CondStmt(Expr * c, Stmt * t, Stmt * f, YYLTYPE l) : cond(c), iftrue(t), iffalse(f) {loc = l;}
	void eval();
};

struct LoopStmt : public Stmt
{
	Expr * cond;
	Stmt * body;
	bool check();
	LoopStmt(Expr * c, Stmt * b, YYLTYPE l) : cond(c), body(b) {loc = l;}
	void eval();
};

struct Expr : public Stmt
{
	virtual Rval eeval() = 0;
	void eval() {eeval();}
};

struct ConstExpr : public Expr
{
	Rval val;
	bool check() {return true;}
	ConstExpr(Rval v, YYLTYPE l) : val(v) {loc = l;}
	Rval eeval() {return val;}
};

template<unOp Op>
struct UnExpr : public Expr
{
	Expr * rv;
	bool check() {return rv->check();}
	UnExpr(Expr * r, YYLTYPE l) : rv (r) {loc = l;}
	~UnExpr() {delete rv;}
	Rval eeval() {return Op(rv->eeval());}
};

template<binOp Op>
struct BinExpr : public Expr
{
	Expr * left, * right;
	bool check() {return left->check() && right->check();}
	BinExpr(Expr * l, Expr * r, YYLTYPE lo) : left(l), right (r) {loc = lo;}
	~BinExpr() {delete left; delete right;}
	Rval eeval() {return Op(left->eeval(), right->eeval());}
};

struct CastExpr : public Expr
{
	Expr * e;
	Type t;
	bool check() {return true;}
	CastExpr(Expr * ex, Type ty, YYLTYPE lo) : e(ex), t(ty) {loc = lo;}
	~CastExpr() {delete e;}
	Rval eeval();
};

struct LvalExpr : public Expr
{
	std::string name;
	bool check();
	LvalExpr(std::string & n, YYLTYPE l) : name(n) {loc = l;}
	Rval eeval() {return (Rval)leeval();}
	virtual Lval leeval();
protected:
	LvalExpr() {}
};

template<unOpL Op>
struct UnExprL : public LvalExpr
{
	Expr * rv;
	bool check() {return rv->check();}
	UnExprL(Expr * r, YYLTYPE l) : rv (r) {loc = l;}
	~UnExprL() {delete rv;}
	Lval leeval() {return Op(rv->eeval());}
};

template<lvalUnOp Op>
struct LvalUnExpr : public Expr
{
	LvalExpr * lv;
	bool check() {return lv->check();}
	LvalUnExpr(LvalExpr * l, YYLTYPE lo) : lv(l) {loc = lo;}
	~LvalUnExpr() {delete lv;}
	Rval eeval() {return Op(lv->leeval());}
};

template<lvalBinOp Op>
struct LvalBinExpr : public LvalExpr
{
	LvalExpr * left;
	Expr * right;
	bool check() {return left->check() && right->check();}
	LvalBinExpr(LvalExpr * l, Expr * r, YYLTYPE lo) : left(l), right (r) {loc = lo;}
	~LvalBinExpr() {delete left; delete right;}
	Lval leeval() {return Op(left->leeval(), right->eeval());}
};

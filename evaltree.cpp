#include "evaltree.h"

ctx global;
std::vector<ctx> ctstack;

void Block::eval()
{
	ctstack.push_back(context);
	for(std::vector<Stmt*>::iterator it = stmts.begin(); it != stmts.end(); it++)
	{
		//std::cout << "Eval: " << (*it)->loc.first_line << std::endl;
		(*it)->eval();
	}	
	ctstack.pop_back();
	
	for(ctx::iterator it = context.begin(); it != context.end(); ++it)
		printf("%s, %d\n", (*it).first.c_str(), (*it).second().uintv);
}

Block::~Block()
{
	for(std::vector<Stmt*>::iterator it = stmts.begin(); it != stmts.end(); it++)
		delete *it;
}

Rval CastExpr::eeval()
{
	Rval ret = e->eeval();
	cast(ret(), ret.type, t);
	ret.type = t;
	return ret;
}

void CondStmt::eval()
{
	if (cond->eeval()().boolv)
		iftrue->eval();
	else
		iffalse->eval();
}

void LoopStmt::eval()
{
	while (cond->eeval()().boolv)
		body->eval();
}

Lval LvalExpr::leeval()
{
	for(std::vector<ctx>::reverse_iterator it = ctstack.rbegin(); it != ctstack.rend(); it++)
	{
		ctx::iterator varit = it->find(name);
		if (varit != it->end())
			return varit->second;
	}
	return 0;
}

bool Block::check()
{
	bool succ = true;
	ctstack.push_back(context);
	for(std::vector<Stmt*>::iterator it = stmts.begin(); it != stmts.end(); it++)
	{
		succ = (*it)->check() && succ;
	}	
	ctstack.pop_back();
	
	return succ;
}

bool CondStmt::check()
{
	return cond->check() && iftrue->check() && iffalse->check();
}

bool LoopStmt::check()
{
	return cond->check() && body->check();
}

bool LvalExpr::check()
{
	for(std::vector<ctx>::reverse_iterator it = ctstack.rbegin(); it != ctstack.rend(); it++)
	{
		ctx::iterator varit = it->find(name);
		if (varit != it->end())
			return true;
	}
	yyerror(&loc, 0, (char*)("Variable not defined: " + name).c_str());
	exit(-1);
	return false;
}

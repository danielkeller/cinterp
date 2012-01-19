#include "yystype.h"
#include "evaltree.h"
#include "y.tab.h"
#include "type.h"

int main(int argc, char*argv[])
{
	if(argc > 1)
		if (strncmp(argv[1], "-d", 2)==0)
			yydebug = 1;

	int status;
	
	YYSTYPE yylval;
	YYLTYPE yyloc = {0};
	int tok;
	
	yypstate *ps = yypstate_new ();
	do {
		tok = yylex (&yylval, &yyloc);
		status = yypush_parse (ps, tok, &yylval, &yyloc, tok);
	} while (status == YYPUSH_MORE);
	yypstate_delete (ps);

	return 0;
}

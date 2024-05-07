#include <pip.h>

int main(int argc, char **args)
{
	struct dict *dict = NULL;
	dict = dict_define(dict, new_word("eval"), Op(OP_EVAL));
	
	dict = dict_define(dict, new_word("print"), Op(OP_PRINT));

	dict = dict_define(dict, new_word("="), Op(OP_BIND));
	dict = dict_define(dict, new_word("fn"), Op(OP_FN));

	dict = dict_define(dict, new_word("+"), Op(OP_ADD));
	dict = dict_define(dict, new_word("-"), Op(OP_SUBTRACT));
	dict = dict_define(dict, new_word("*"), Op(OP_MULTIPLY));
	dict = dict_define(dict, new_word("/"), Op(OP_DIVIDE));

	repl(dict);

	return 0;
}

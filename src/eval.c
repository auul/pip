#include <pip.h>

void op_print_f(FILE *f, enum op_code op)
{
	assert(f);

	switch (op) {
	case OP_EVAL:
		fprintf(f, "eval");
		break;
	case OP_PRINT:
		fprintf(f, "print");
		break;
	case OP_BIND:
		fprintf(f, "=");
		break;
	case OP_ADD:
		fprintf(f, "+");
		break;
	case OP_SUBTRACT:
		fprintf(f, "-");
		break;
	case OP_MULTIPLY:
		fprintf(f, "*");
		break;
	case OP_DIVIDE:
		fprintf(f, "/");
		break;
	default:
		fprintf(f, "op(%u)", op);
		break;
	}
}

void op_print(enum op_code op)
{
	op_print_f(stdout, op);
}

int op_compare(enum op_code a, enum op_code b)
{
	if (a > b) {
		return -1;
	} else if (a < b) {
		return 1;
	}

	return 0;
}

struct data eval_op(struct dict **dict_p, struct cons **ctrl_p, enum op_code op)
{
	struct data a, b;

	switch (op) {
	case OP_EVAL:
		if (!*ctrl_p) {
			error_print("Usage: eval EXPR");
			return Nil();
		}
		a = eval(dict_p, ctrl_p);
		if (a.type != List) {
			*ctrl_p = new_cons(a, *ctrl_p);
			return eval(dict_p, ctrl_p);
		}
		*ctrl_p = list_join(a.list, *ctrl_p);
		return eval(dict_p, ctrl_p);

	case OP_PRINT:
		if (!*ctrl_p) {
			error_print("Usage: print EXPR");
			return Nil();
		} else if (*ctrl_p && (*ctrl_p)->car.type == Word && (*ctrl_p)->car.word->str[0] == '{') {
			a = list_pop(ctrl_p);
			if (!parse_check_quote(a.word->str)) {
				error_print("Invalid quote");
				data_deref(a);
				return Nil();
			}
			printf("%.*s", (int)(strlen(a.word->str) - 2), a.word->str + 1);
			trigger_newline();
			data_deref(a);
			return Nil();
		}

		a = eval(dict_p, ctrl_p);
		data_print(a);
		trigger_newline();
		data_deref(a);
		return Nil();

	case OP_BIND:
		a = list_pop(ctrl_p);
		if (a.type != Word || !*ctrl_p) {
			error_print("Usage: WORD = THING");
			data_deref(a);
			return Nil();
		}
		b = eval(dict_p, ctrl_p);
		*dict_p = dict_define(*dict_p, a.word, data_ref(b));
		return b;
	case OP_FN:
		if (!*ctrl_p || !(*ctrl_p)->cdr) {
			error_print("Usage: fn {ARGS} {BODY}");
			return Nil();
		}

		a = eval(dict_p, ctrl_p);
		if (a.type == Nil) {
			a = List(NULL);
		} else if (a.type != List) {
			a = List(new_cons(a, NULL));
		}

		b = eval(dict_p, ctrl_p);
		if (b.type == Nil) {
			b = List(NULL);
		} else if (b.type != List) {
			b = List(new_cons(b, NULL));
		}

		return Fn(new_fn(a.list, parse_infix(b.list)));

	case OP_ADD:
		a = eval(dict_p, ctrl_p);
		b = eval(dict_p, ctrl_p);
		if (a.type != Num || b.type != Num) {
			error_print("Usage: NUM + NUM");
			data_deref(a);
			data_deref(b);
			return Nil();
		}
		return Num(a.num + b.num);
	case OP_SUBTRACT:
		a = eval(dict_p, ctrl_p);
		b = eval(dict_p, ctrl_p);
		if (a.type != Num || b.type != Num) {
			error_print("Usage: NUM - NUM");
			data_deref(a);
			data_deref(b);
			return Nil();
		}
		return Num(a.num - b.num);
	case OP_MULTIPLY:
		a = eval(dict_p, ctrl_p);
		b = eval(dict_p, ctrl_p);
		if (a.type != Num || b.type != Num) {
			error_print("Usage: NUM * NUM");
			data_deref(a);
			data_deref(b);
			return Nil();
		}
		return Num(a.num * b.num);
	case OP_DIVIDE:
		a = eval(dict_p, ctrl_p);
		b = eval(dict_p, ctrl_p);
		if (a.type != Num || b.type != Num) {
			error_print("Usage: NUM / NUM");
			data_deref(a);
			data_deref(b);
			return Nil();
		}
		return Num(a.num / b.num);
	default:
		return Nil();
	}
}

struct data eval_thing(struct dict **dict_p, struct cons **ctrl_p)
{
	assert(dict_p);
	assert(ctrl_p);

	struct data car = list_pop(ctrl_p);
	if (car.type != Word) {
		return car;
	} else if (car.word->str[0] == '{') {
		if (!parse_check_quote(car.word->str)) {
			error_print("Invalid quote encountered");
			word_deref(car.word);
			return Nil();
		}

		struct cons *list = parse_quote(car.word->str);
		word_deref(car.word);

		if (!list) {
			return Nil();
		} else if (!list->cdr) {
			return list_pop(&list);
		}

		return List(list);
	} else if (word_is_num(car.word)) {
		return word_to_num(car.word);
	}

	return dict_lookup(*dict_p, car.word);
}

struct data eval(struct dict **dict_p, struct cons **ctrl_p)
{
	assert(dict_p);
	assert(ctrl_p);

	struct cons *args, *body;
	struct dict *dict;

	struct data car = eval_thing(dict_p, ctrl_p);
	switch (car.type) {
	case Fn:
		dict = dict_ref(*dict_p);
		args = list_ref(car.fn->args);
		body = list_ref(car.fn->body);
		fn_deref(car.fn);

		while (args && *ctrl_p) {
			struct word *var = list_pop(&args).word;
			dict = dict_define(dict, var, eval(dict_p, ctrl_p));
		}

		if (args) {
			error_print("Insufficient arguments to function");
			dict_deref(dict);
			list_deref(args);
			list_deref(body);
			return Nil();
		}

		car = eval(&dict, &body);
		while (body) {
			data_deref(car);
			car = eval(&dict, &body);
		}

		dict_deref(dict);

		return car;
	case Op:
		return eval_op(dict_p, ctrl_p, car.op);
	default:
		return car;
	}
}

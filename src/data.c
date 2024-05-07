#include <pip.h>

void data_print_f(FILE *f, const struct data data)
{
	assert(f);

	switch (data.type) {
	case Nil:
		fprintf(f, "nil");
		break;
	case Bool:
		bool_print_f(f, data.b);
		break;
	case Num:
		num_print_f(f, data.num);
		break;
	case Word:
		word_print_f(f, data.word);
		break;
	case List:
		list_print_f(f, data.list);
		break;
	case Dict:
		dict_print_f(f, data.dict);
		break;
	case Fn:
		fn_print_f(f, data.fn);
		break;
	case Op:
		op_print_f(f, data.op);
		break;
	default:
		fprintf(f, "%p", data.ptr);
		break;
	}
}

void data_print(const struct data data)
{
	data_print_f(stdout, data);
}

struct data data_ref(struct data data)
{
	switch (data.type) {
	case Word:
		word_ref(data.word);
		break;
	case List:
		list_ref(data.list);
		break;
	case Dict:
		dict_ref(data.dict);
		break;
	case Fn:
		fn_ref(data.fn);
		break;
	default:
		break;
	}

	return data;
}

void data_deref(struct data data)
{
	switch (data.type) {
	case Word:
		word_deref(data.word);
		break;
	case List:
		list_deref(data.list);
		break;
	case Dict:
		dict_deref(data.dict);
		break;
	case Fn:
		fn_deref(data.fn);
		break;
	default:
		break;
	}
}

bool data_equal(const struct data a, const struct data b)
{
	if (a.type != b.type) {
		return false;
	}

	switch (a.type) {
	case Nil:
		return true;
	case Bool:
		return bool_equal(a.b, b.b);
	case Num:
		return num_equal(a.num, b.num);
	case Word:
		return word_equal(a.word, b.word);
	case List:
		return list_equal(a.list, b.list);
	case Dict:
		return dict_equal(a.dict, b.dict);
	case Fn:
		return fn_equal(a.fn, b.fn);
	case Op:
		return a.op == b.op;
	default:
		return a.ptr == b.ptr;
	}
}

int data_compare(const struct data a, const struct data b)
{
	if (a.type > b.type) {
		return -1;
	} else if (a.type < b.type) {
		return 1;
	}

	switch (a.type) {
	case Nil:
		return 0;
	case Bool:
		return bool_compare(a.b, b.b);
	case Num:
		return num_compare(a.num, b.num);
	case Word:
		return word_compare(a.word, b.word);
	case List:
		return list_compare(a.list, b.list);
	case Dict:
		return dict_compare(a.dict, b.dict);
	case Fn:
		return fn_compare(a.fn, b.fn);
	case Op:
		return op_compare(a.op, b.op);
	default:
		if (a.ptr > b.ptr) {
			return -1;
		} else if (a.ptr < b.ptr) {
			return 1;
		}

		return 0;
	}
}

size_t data_len(const struct data data)
{
	switch (data.type) {
	case List:
		return list_len(data.list);
	case Dict:
		return dict_len(data.dict);
	case Fn:
		return fn_len(data.fn);
	default:
		return 1;
	}
}

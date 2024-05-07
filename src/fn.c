#include <pip.h>

struct fn *new_fn(struct cons *args, struct cons *body)
{
	struct fn *fn = malloc(sizeof(struct fn));
	if (!fn) {
		error_print(strerror(errno));
		exit(1);
	}

	fn->ref = 1;
	fn->args = args;
	fn->body = body;

	return fn;
}

void fn_print_f(FILE *f, const struct fn *fn)
{
	assert(f);
	assert(fn);

	fprintf(f, "(");
	for (struct cons *args = fn->args; args; args = args->cdr) {
		fprintf(f, "λ");
		word_print_f(f, args->car.word);
		fprintf(f, ". ");
	}
	list_print_raw_f(f, fn->body);
	fprintf(f, ")");
}

void fn_print(const struct fn *fn)
{
	fn_print_f(stdout, fn);
}

struct fn *fn_ref(struct fn *fn)
{
	assert(fn);
	fn->ref++;
	return fn;
}

void fn_deref(struct fn *fn)
{
	assert(fn);

	fn->ref--;
	if (fn->ref) {
		return;
	}

	list_deref(fn->args);
	list_deref(fn->body);
	free(fn);
}

bool fn_equal(const struct fn *a, const struct fn *b)
{
	assert(a);
	assert(b);

	if (a == b) {
		return true;
	}

	return list_equal(a->args, b->args) && list_equal(a->body, b->body);
}

int fn_compare(const struct fn *a, const struct fn *b)
{
	assert(a);
	assert(b);

	if (a == b) {
		return 0;
	}

	int cmp = list_compare(a->args, b->args);
	if (cmp) {
		return cmp;
	}

	return list_compare(a->body, b->body);
}

size_t fn_len(const struct fn *fn)
{
	assert(fn);

	return list_len(fn->args) + list_len(fn->body);
}

struct fn *fn_edit(struct fn *fn)
{
	assert(fn);

	if (fn->ref == 1) {
		return fn;
	}

	return new_fn(list_ref(fn->args), list_ref(fn->body));
}

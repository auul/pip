#include <pip.h>

char END_SENTINEL;

struct cons *new_cons(struct data car, struct cons *cdr)
{
	struct cons *cons = malloc(sizeof(struct cons));
	if (!cons) {
		error_print(strerror(errno));
		exit(1);
	}

	cons->ref = 1;
	cons->car = car;
	cons->cdr = cdr;

	return cons;
}

struct cons *new_list_v(struct data car, va_list args)
{
	if (is_end(car)) {
		return NULL;
	}

	struct cons *list = new_cons(car, NULL);
	struct cons *node = list;
	car = va_arg(args, struct data);

	while (!is_end(car)) {
		node->cdr = new_cons(car, NULL);
		node = node->cdr;
		car = va_arg(args, struct data);
	}

	return list;
}

struct cons *new_list(struct data car, ...)
{
	va_list args;
	va_start(args, car);
	struct cons *list = new_list_v(car, args);
	va_end(args);

	return list;
}

void list_print_raw_f(FILE *f, const struct cons *list)
{
	assert(f);

	while (list) {
		data_print_f(f, list->car);
		list = list->cdr;
		if (list) {
			fprintf(f, " ");
		}
	}
}

void list_print_f(FILE *f, const struct cons *list)
{
	assert(f);

	fprintf(f, "[");
	list_print_raw_f(f, list);
	fprintf(f, "]");
}

void list_print(const struct cons *list)
{
	list_print_f(stdout, list);
}

struct cons *list_ref(struct cons *list)
{
	if (list) {
		list->ref++;
	}

	return list;
}

void list_deref(struct cons *list)
{
	while (list) {
		list->ref--;
		if (list->ref) {
			return;
		}

		data_deref(list->car);
		struct cons *cdr = list->cdr;
		free(list);

		list = cdr;
	}
}

bool list_equal(const struct cons *a, const struct cons *b)
{
	while (a) {
		if (a == b) {
			return true;
		} else if (!b || !data_equal(a->car, b->car)) {
			return false;
		}

		a = a->cdr;
		b = b->cdr;
	}

	return !b;
}

int list_compare(const struct cons *a, const struct cons *b)
{
	while (a) {
		if (a == b) {
			return 0;
		} else if (!b) {
			return -1;
		}

		int cmp = data_compare(a->car, b->car);
		if (cmp) {
			return cmp;
		}

		a = a->cdr;
		b = b->cdr;
	}

	return b ? 1 : 0;
}

size_t list_len(const struct cons *list)
{
	size_t len = 0;

	while (list) {
		len++;
		list = list->cdr;
	}

	return len;
}

struct cons *list_edit(struct cons **list_p, struct cons *node)
{
	assert(list_p);

	struct cons *list = *list_p;
	if (!list) {
		return NULL;
	} else if (list->ref > 1) {
		list_deref(list);
		list = new_cons(data_ref(list->car), list_ref(list->cdr));
		if (*list_p == node) {
			*list_p = list;
			return list;
		}
		*list_p = list;
	} else if (list == node) {
		return list;
	}

	while (list->cdr != node) {
		if (list->cdr->ref > 1) {
			list_deref(list->cdr);
			list->cdr = new_cons(data_ref(list->cdr->car), list_ref(list->cdr->cdr));
		}
		list = list->cdr;
	}

	if (node) {
		if (node->ref > 1) {
			list_deref(node);
			list->cdr = new_cons(data_ref(node->car), list_ref(node->cdr));

			return list->cdr;
		}

		return node;
	}

	return list;
}

struct data list_pop(struct cons **list_p)
{
	assert(list_p);

	struct cons *list = *list_p;
	if (!list) {
		return Nil();
	}

	struct data value = data_ref(list->car);
	*list_p = list_ref(list->cdr);
	list_deref(list);

	return value;
}

struct cons *list_reverse(struct cons *src)
{
	struct cons *dest = NULL;

	while (src) {
		dest = new_cons(list_pop(&src), dest);
	}

	return dest;
}

struct cons *list_join(struct cons *a, struct cons *b)
{
	if (!a) {
		if (!b) {
			return NULL;
		}
		return b;
	} else if (!b) {
		return a;
	}

	list_edit(&a, NULL)->cdr = b;
	return a;
}

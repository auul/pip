#include <pip.h>

struct dict *new_dict(struct word *key, struct data value, struct dict *left, struct dict *right)
{
	assert(key);

	struct dict *dict = malloc(sizeof(struct dict));
	if (!dict) {
		error_print(strerror(errno));
		exit(1);
	}

	dict->ref = 1;
	dict->key = key;
	dict->value = value;
	dict->left = left;
	dict->right = right;

	return dict;
}

void dict_print_f(FILE *f, const struct dict *dict)
{
	assert(f);

	while (dict) {
		if (dict->left) {
			dict_print_f(f, dict->left);
			fprintf(f, " ");
		}

		word_print_f(f, dict->key);
		fprintf(f, ":");
		data_print_f(f, dict->value);

		dict = dict->right;
		if (dict) {
			fprintf(f, " ");
		}
	}
}

void dict_print(const struct dict *dict)
{
	dict_print_f(stdout, dict);
}

struct dict *dict_ref(struct dict *dict)
{
	if (dict) {
		dict->ref++;
	}

	return dict;
}

void dict_deref(struct dict *dict)
{
	while (dict) {
		dict->ref--;
		if (dict->ref) {
			return;
		}

		word_deref(dict->key);
		data_deref(dict->value);
		dict_deref(dict->left);

		struct dict *right = dict->right;
		free(dict);
		dict = right;
	}
}

const struct dict *dict_indexed_const(const struct dict *dict, size_t index)
{
	while (dict) {
		size_t left = dict_len(dict->left);
		if (left > index) {
			dict = dict->left;
		} else if (left == index) {
			return dict;
		} else {
			index -= left;
			dict = dict->right;
		}
	}

	return NULL;
}

bool dict_equal(const struct dict *a, const struct dict *b)
{
	if (a == b) {
		return true;
	}

	size_t len = dict_len(a);
	if (len != dict_len(b)) {
		return false;
	}

	for (size_t i = 0; i < len; i++) {
		const struct dict *a_node = dict_indexed_const(a, i);
		const struct dict *b_node = dict_indexed_const(b, i);

		if (a_node != b_node) {
			if (!word_equal(a->key, b->key) || !data_equal(a->value, b->value)) {
				return false;
			}
		}
	}

	return true;
}

int dict_compare(const struct dict *a, const struct dict *b)
{
	if (a == b) {
		return 0;
	}

	size_t len = dict_len(a);

	for (size_t i = 0; i < len; i++) {
		const struct dict *a_node = dict_indexed_const(a, i);
		const struct dict *b_node = dict_indexed_const(b, i);

		if (!b_node) {
			return -1;
		} else if (a_node != b_node) {
			int cmp = word_compare(a_node->key, b_node->key);
			if (cmp) {
				return cmp;
			}

			cmp = data_compare(a_node->value, b_node->value);
			if (cmp) {
				return cmp;
			}
		}
	}

	if (dict_len(b) > len) {
		return 1;
	}

	return 0;
}

size_t dict_len(const struct dict *dict)
{
	size_t len = 0;

	while (dict) {
		len += dict_len(dict->left) + 1;
		dict = dict->right;
	}

	return len;
}

struct dict *dict_define(struct dict *dict, struct word *key, struct data value)
{
	if (!dict) {
		return new_dict(key, value, NULL, NULL);
	} else if (dict->ref > 1) {
		dict_deref(dict);
		dict = new_dict(word_ref(dict->key), data_ref(dict->value), dict_ref(dict->left), dict_ref(dict->right));
	}

	struct dict *node = dict;

	while (true) {
		int cmp = word_compare(node->key, key);
		if (cmp < 0) {
			if (!node->left) {
				node->left = new_dict(key, value, NULL, NULL);
				return dict;
			} else if (node->left->ref > 1) {
				dict_deref(node->left);
				node->left = new_dict(word_ref(node->left->key), data_ref(node->left->value), dict_ref(node->left->left), dict_ref(node->left->right));
			}
			node = node->left;
		} else if (cmp > 0) {
			if (!node->right) {
				node->right = new_dict(key, value, NULL, NULL);
				return dict;
			} else if (node->right->ref > 1) {
				dict_deref(node->right);
				node->right = new_dict(word_ref(node->right->key), data_ref(node->right->value), dict_ref(node->right->left), dict_ref(node->right->right));
			}
			node = node->right;
		} else {
			word_deref(key);
			data_deref(node->value);
			node->value = value;
			return dict;
		}
	}
}

struct data dict_lookup_simple(struct dict *dict, const struct word *key)
{
	while (dict) {
		int cmp = word_compare(dict->key, key);
		if (cmp < 0) {
			dict = dict->left;
		} else if (cmp > 0) {
			dict = dict->right;
		} else {
			return dict->value;
		}
	}

	return Nil();
}

bool is_ref_cycle(struct dict *dict, struct cons *prev, struct word *key)
{
	for (struct cons *trace = prev; trace; trace = trace->cdr) {
		if (word_equal(trace->car.word, key)) {
			return true;
		}
	}

	struct data value = dict_lookup_simple(dict, key);
	if (value.type != Word) {
		return false;
	}

	struct cons at = {
		.car = Word(key),
		.cdr = prev,
	};

	return is_ref_cycle(dict, &at, value.word);
}

struct data dict_lookup(struct dict *dict, struct word *key)
{
	struct data value = dict_lookup_simple(dict, key);
	if (value.type == Nil) {
		return Word(key);
	} else if (value.type != Word) {
		word_deref(key);
		return data_ref(value);
	} else if (is_ref_cycle(dict, NULL, value.word)) {
		return Word(key);
	}

	return dict_lookup(dict, word_ref(value.word));
}

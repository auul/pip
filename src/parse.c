#include <pip.h>

const char *delim_token_list[] = {
	"(", ")", "{", "}",
	NULL
};

bool is_same_token(const char *token, const char *src)
{
	for (size_t i = 0; token[i]; i++) {
		if (token[i] != src[i]) {
			return false;
		}
	}

	return true;
}

const char *get_delim_token(const char *src)
{
	for (size_t i = 0; delim_token_list[i]; i++) {
		if (is_same_token(delim_token_list[i], src)) {
			return delim_token_list[i];
		}
	}

	return NULL;
}

bool is_delim(const char *src)
{
	return !*src || isspace((int)*src) || get_delim_token(src);
}

size_t get_quote_len(const char *src)
{
	if (src[0] != '{') {
		return 0;
	}

	size_t len = 1;

	while (src[len] != '}') {
		if (src[len] == '{') {
			len += get_quote_len(src + len);
		} else {
			len++;
		}
	}

	return len + 1;
}

size_t get_chunk_len(const char *src)
{
	if (src[0] == '{') {
		return get_quote_len(src);
	}

	const char *delim_token = get_delim_token(src);
	if (delim_token) {
		return strlen(delim_token);
	}

	size_t len = 0;
	while (!is_delim(src + len)) {
		len++;
	}

	return len;
}

const char *skip_space(const char *src)
{
	while (*src && isspace((int)*src)) {
		src++;
	}

	return src;
}

struct word *get_chunk(const char **src_p)
{
	const char *src = *src_p;
	size_t chunk_len = get_chunk_len(src);
	if (!chunk_len) {
		return NULL;
	}

	*src_p = skip_space(src + chunk_len);
	return new_word_n(src, chunk_len);
}

struct cons *parse_n(const char *src, size_t len)
{
	if (!src || !len) {
		return NULL;
	}

	const char *end = src + len;
	src = skip_space(src);

	struct word *chunk = get_chunk(&src);
	if (!chunk) {
		return NULL;
	}

	struct cons *list = new_cons(Word(chunk), NULL);
	struct cons *node = list;
	chunk = get_chunk(&src);

	while (chunk && src <= end) {
		node->cdr = new_cons(Word(chunk), NULL);
		node = node->cdr;
		chunk = get_chunk(&src);
	}

	if (chunk) {
		word_deref(chunk);
	}

	return list;
}

struct cons *parse(const char *src)
{
	if (!src) {
		return NULL;
	}

	return parse_n(src, strlen(src));
}

struct cons *parse_quote(const char *src)
{
	return parse_n(src + 1, get_quote_len(src) - 2);
}

const char *check_bracket(char close, const char *src)
{
	while (*src) {
		if (*src == close) {
			return src + 1;
		}

		switch (*src) {
		case '(':
			src = check_bracket(')', src + 1);
			if (!src) {
				return NULL;
			}
			break;
		case '{':
			src = check_bracket('}', src + 1);
			if (!src) {
				return NULL;
			}
			break;
		case ')':
		case '}':
			return NULL;
		default:
			src++;
			break;
		}
	}

	return NULL;
}

bool parse_check_n(const char *src, size_t len)
{
	const char *end = src + len;

	while (src < end) {
		switch (*src) {
		case '(':
			src = check_bracket(')', src + 1);
			if (!src || src > end) {
				return false;
			}
			break;
		case '{':
			src = check_bracket('}', src + 1);
			if (!src || src > end) {
				return false;
			}
			break;
		case ')':
		case '}':
			return false;
		default:
			src++;
			break;
		}
	}

	return true;
}

bool parse_check(const char *src)
{
	return parse_check_n(src, strlen(src));
}

bool parse_check_quote(const char *src)
{
	return check_bracket('}', src + 1);
}

struct {
	const char *token;
	unsigned precedence;
	bool right_associative;
} infix_op_list[] = {
	{"=",    2, true},
	{"or",   3, false},
	{"and",  3, false},
	{"==",   4, true},
	{"<",    4, true},
	{">",    4, true},
	{"<=",   4, true},
	{">=",   4, true},
	{"!=",   4, true},
	{"+",    5, false},
	{"-",    5, false},
	{"*",    6, false},
	{"/",    7, true},
	{"else", 9, false},

	{NULL,  0, false}
};

unsigned get_precedence(const struct word *word)
{
	for (size_t i = 0; infix_op_list[i].token; i++) {
		if (!strcmp(infix_op_list[i].token, word->str)) {
			return infix_op_list[i].precedence;
		}
	}

	return 0;
}

bool is_right_associative(const struct word *word)
{
	for (size_t i = 0; infix_op_list[i].token; i++) {
		if (!strcmp(infix_op_list[i].token, word->str)) {
			return infix_op_list[i].right_associative;
		}
	}

	return true;
}

struct cons *parse_infix(struct cons *in)
{
	struct cons *hold = NULL;
	struct cons *out = NULL;

	while (in) {
		struct data car = list_pop(&in);
		if (car.type == Word && !strcmp("-", car.word->str)) {
			if (!in) {
				data_deref(car);
				car = Word(new_word("~"));
			} else if (in->car.type == Word && in->car.word->str[0] == '(') {
				data_deref(car);
				car = Word(new_word("~"));
			} else if (in->car.type == Word && get_precedence(in->car.word)) {
				data_deref(car);
				car = Word(new_word("~"));
			}
		}

		if (false) {
		} else if (car.type != Word) {
			out = new_cons(car, out);
		} else if (!strcmp(")", car.word->str)) {
			hold = new_cons(car, hold);
		} else if (!strcmp("(", car.word->str)) {
			data_deref(car);
			while (hold && strcmp(")", hold->car.word->str)) {
				out = new_cons(list_pop(&hold), out);
			}
			data_deref(list_pop(&hold));
		} else if (!strcmp(",", car.word->str)) {
			data_deref(car);
			while (hold && strcmp(")", hold->car.word->str)) {
				out = new_cons(list_pop(&hold), out);
			}
		} else {
			unsigned precedence = get_precedence(car.word);
			if (!precedence) {
				if (hold && !get_precedence(hold->car.word)) {
					while (hold && strcmp(")", hold->car.word->str)) {
						out = new_cons(list_pop(&hold), out);
					}
				}
				hold = new_cons(car, hold);
			} else {
				if (hold && !get_precedence(hold->car.word)) {
					out = new_cons(list_pop(&hold), out);
				}

				while (hold && (precedence < get_precedence(hold->car.word) || (precedence == get_precedence(hold->car.word) && is_right_associative(hold->car.word)))) {
					out = new_cons(list_pop(&hold), out);
				}

				hold = new_cons(car, hold);
			}
		}
	}

	while (hold) {
		out = new_cons(list_pop(&hold), out);
	}

	return out;
}

#include <pip.h>

struct word *new_word_n(const char *src, size_t len)
{
	assert(src);
	assert(len);

	struct word *word = malloc(sizeof(struct word) + len + 1);
	if (!word) {
		error_print(strerror(errno));
		exit(1);
	}

	word->ref = 1;
	memcpy(word->str, src, len);
	word->str[len] = 0;

	return word;
}

struct word *new_word(const char *src)
{
	assert(src);
	return new_word_n(src, strlen(src));
}

struct word *word_ref(struct word *word)
{
	assert(word);
	word->ref++;
	return word;
}

void word_deref(struct word *word)
{
	assert(word);

	word->ref--;
	if (word->ref) {
		return;
	}

	free(word);
}

void word_print_f(FILE *f, const struct word *word)
{
	assert(f);
	assert(word);

	fprintf(f, "%s", word->str);
}

void word_print(const struct word *word)
{
	word_print_f(stdout, word);
}

bool word_equal(const struct word *a, const struct word *b)
{
	assert(a);
	assert(b);

	if (a == b) {
		return true;
	}

	return !strcmp(a->str, b->str);
}

int word_compare(const struct word *a, const struct word *b)
{
	assert(a);
	assert(b);

	if (a == b) {
		return 0;
	}

	return -strcmp(a->str, b->str);
}

size_t word_len(const struct word *word)
{
	assert(word);
	return strlen(word->str);
}

struct word *word_quote(struct word *word)
{
	assert(word);

	size_t len = word_len(word);
	if (word->ref == 1) {
		word = realloc(word, sizeof(struct word) + len + 3);
		if (!word) {
			error_print(strerror(errno));
			exit(1);
		}
		memmove(word->str + 1, word->str, len);
		word->str[0] = '{';
		word->str[len + 1] = '}';
		word->str[len + 2] = 0;
		return word;
	}

	word_deref(word);
	struct word *dest = malloc(sizeof(struct word) + len + 3);
	if (!word) {
		error_print(strerror(errno));
		exit(1);
	}

	dest->ref = 1;
	dest->str[0] = '{';
	memcpy(dest->str + 1, word->str, len);
	dest->str[len + 1] = '}';
	dest->str[len + 2] = 0;

	return dest;
}

bool word_is_num(const struct word *word)
{
	assert(word);

	bool dot = false;
	bool digit = false;

	for (size_t i = 0; word->str[i]; i++) {
		if (word->str[i] == '.') {
			if (dot) {
				return false;
			}
			dot = true;
		} else if (isdigit((int)word->str[i])) {
			digit = true;
		} else {
			return false;
		}
	}

	return digit;
}

struct data word_to_num(struct word *word)
{
	assert(word);

	double left = 0.0;
	double right = 0.0;
	unsigned place = 0;
	bool dot = false;

	for (size_t i = 0; word->str[i]; i++) {
		if (word->str[i] == '.') {
			dot = true;
		} else if (dot) {
			right = (10.0 * right) + (double)(word->str[i] - '0');
			place++;
		} else {
			left = (10.0 * left) + (double)(word->str[i] - '0');
		}
	}

	while (place) {
		right /= 10.0;
		place--;
	}

	return Num(left + right);
}

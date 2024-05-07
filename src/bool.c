#include <pip.h>

void bool_print_f(FILE *f, bool b)
{
	assert(f);

	if (b) {
		fprintf(f, "true");
	} else {
		fprintf(f, "false");
	}
}

void bool_print(bool b)
{
	bool_print_f(stdout, b);
}

bool bool_equal(bool a, bool b)
{
	return a == b;
}

int bool_compare(bool a, bool b)
{
	if (a) {
		if (b) {
			return 0;
		}

		return -1;
	} else if (b) {
		return 1;
	}

	return 0;
}

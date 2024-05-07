#include <pip.h>

void num_print_f(FILE *f, double num)
{
	assert(f);

	if (num_equal(num, floor(num))) {
		fprintf(f, "%i", (int)floor(num));
	} else {
		fprintf(f, "%f", num);
	}
}

void num_print(double num)
{
	num_print_f(stdout, num);
}

bool num_equal(double a, double b)
{
	return fabs(a - b) < NUM_EPSILON;
}

int num_compare(double a, double b)
{
	if (num_equal(a, b)) {
		return 0;
	} else if (a > b) {
		return -1;
	}

	return 1;
}

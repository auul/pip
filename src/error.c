#include <pip.h>

int (*error_print_fn)(FILE *, const char *, va_list) = vfprintf;
FILE *error_print_file;

void error_print(const char *fmt, ...)
{
	if (!error_print_file) {
		error_print_file = stderr;
	}

	va_list args;
	va_start(args, fmt);
	error_print_fn(error_print_file, "Error: ", args);
	error_print_fn(error_print_file, fmt, args);
	error_print_fn(error_print_file, "\n", args);
	va_end(args);
}

#include <pip.h>

bool newline_needed;

void trigger_newline(void)
{
	newline_needed = true;
}

void repl(struct dict *dict)
{
	for (bool running = true; running; ) {
		char *line = readline("pip:: ");
		if (!line) {
		} else if (!strcmp("exit", line)) {
			running = false;
		} else {
			add_history(line);

			struct cons *ctrl = parse_infix(list_reverse(parse(line)));
			struct data value = eval(&dict, &ctrl);

			while (ctrl) {
				data_deref(value);
				value = eval(&dict, &ctrl);
			}

			if (newline_needed) {
				printf("\n");
				newline_needed = false;
			}

			if (value.type != Nil) {
				data_print(value);
				printf("\n");
				data_deref(value);
			}
		}

		if (line) {
			free(line);
		}
	}
}

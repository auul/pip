#ifndef PIP_H
#define PIP_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

// Error Handling (error.c)

void error_print(const char *fmt, ...);

// Data Primitives (data.c)

enum data_type {
	Nil,
	Bool,
	Num,
	Word,
	List,
	Dict,
	Fn,
	Op,
};

struct data {
	enum data_type type;
	union {
		void *ptr;
		bool b;
		double num;
		struct word *word;
		struct cons *list;
		struct dict *dict;
		struct fn *fn;
		unsigned op;
	};
};

#define Nil() \
	((struct data) {.type = Nil, .ptr = NULL})
#define Bool(value) \
	((struct data) {.type = Bool, .b = value})
#define Num(value) \
	((struct data) {.type = Num, .num = value})
#define Word(ptr) \
	((struct data) {.type = Word, .word = ptr})
#define List(ptr) \
	((struct data) {.type = List, .list = ptr})
#define Dict(ptr) \
	((struct data) {.type = Dict, .dict = ptr})
#define Fn(ptr) \
	((struct data) {.type = Fn, .fn = ptr})
#define Op(value) \
	((struct data) {.type = Op, .op = value})

void data_print_f(FILE *f, const struct data data);
void data_print(const struct data data);
struct data data_ref(struct data data);
void data_deref(struct data data);
bool data_equal(const struct data a, const struct data b);
int data_compare(const struct data a, const struct data b);
size_t data_len(const struct data data);

// Boolean Type (bool.c)

void bool_print_f(FILE *f, bool b);
void bool_print(bool b);
bool bool_equal(bool a, bool b);
int bool_compare(bool a, bool b);

// Number Type (num.c)

#define NUM_EPSILON 0.0000000000001

void num_print_f(FILE *f, double num);
void num_print(double num);
bool num_equal(double a, double b);
int num_compare(double a, double b);

// Word Type (word.c)

struct word {
	uint32_t ref;
	char str[];
};

struct word *new_word_n(const char *src, size_t len);
struct word *new_word(const char *src);
void word_print_f(FILE *f, const struct word *word);
void word_print(const struct word *word);
struct word *word_ref(struct word *word);
void word_deref(struct word *word);
bool word_equal(const struct word *a, const struct word *b);
int word_compare(const struct word *a, const struct word *b);
size_t word_len(const struct word *word);
struct word *word_quote(struct word *word);
bool word_is_num(const struct word *word);
struct data word_to_num(struct word *word);

// List Type (list.c)

struct cons {
	uint32_t ref;
	struct data car;
	struct cons *cdr;
};

extern char END_SENTINEL;
#define END \
	((struct data) {.type = Nil, .ptr = &END_SENTINEL})
#define is_end(car) \
	((car).type == Nil && (car).ptr == &END_SENTINEL)

struct cons *new_cons(struct data car, struct cons *cdr);
struct cons *new_list_v(struct data car, va_list args);
struct cons *new_list(struct data car, ...);
void list_print_raw_f(FILE *f, const struct cons *list);
void list_print_f(FILE *f, const struct cons *list);
void list_print(const struct cons *list);
struct cons *list_ref(struct cons *list);
void list_deref(struct cons *list);
bool list_equal(const struct cons *a, const struct cons *b);
int list_compare(const struct cons *a, const struct cons *b);
size_t list_len(const struct cons *list);
struct cons *list_edit(struct cons **list_p, struct cons *node);
struct data list_pop(struct cons **list_p);
struct cons *list_reverse(struct cons *src);
struct cons *list_join(struct cons *a, struct cons *b);

// Dictionary Type (dict.c)

struct dict {
	uint32_t ref;
	struct word *key;
	struct data value;
	struct dict *left;
	struct dict *right;
};

struct dict *new_dict(struct word *key, struct data value, struct dict *left, struct dict *right);
void dict_print_f(FILE *f, const struct dict *dict);
void dict_print(const struct dict *dict);
struct dict *dict_ref(struct dict *dict);
void dict_deref(struct dict *dict);
bool dict_equal(const struct dict *a, const struct dict *b);
int dict_compare(const struct dict *a, const struct dict *b);
size_t dict_len(const struct dict *dict);
struct dict *dict_define(struct dict *dict, struct word *key, struct data value);
struct data dict_lookup(struct dict *dict, struct word *key);

// Function Type (fn.c)

struct fn {
	uint32_t ref;
	struct cons *args;
	struct cons *body;
};

struct fn *new_fn(struct cons *args, struct cons *body);
void fn_print_f(FILE *f, const struct fn *fn);
void fn_print(const struct fn *fn);
struct fn *fn_ref(struct fn *fn);
void fn_deref(struct fn *fn);
bool fn_equal(const struct fn *a, const struct fn *b);
int fn_compare(const struct fn *a, const struct fn *b);
size_t fn_len(const struct fn *fn);
struct fn *fn_edit(struct fn *fn);

// Interpreter (eval.c)

enum op_code {
	OP_EVAL,

	OP_PRINT,

	OP_BIND,
	OP_FN,

	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
};

void op_print_f(FILE *f, enum op_code op);
void op_print(enum op_code op);
int op_compare(enum op_code a, enum op_code b);

struct data eval_thing(struct dict **dict_p, struct cons **ctrl_p);
struct data eval(struct dict **dict_p, struct cons **ctrl_p);

// Parser (parse.c)

struct cons *parse_n(const char *src, size_t len);
struct cons *parse(const char *src);
struct cons *parse_quote(const char *src);
bool parse_check_n(const char *src, size_t len);
bool parse_check(const char *src);
bool parse_check_quote(const char *src);
struct cons *parse_infix(struct cons *in);

// REPL (repl.c)

void trigger_newline(void);
void repl(struct dict *dict);

#endif

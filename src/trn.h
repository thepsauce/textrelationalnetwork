#ifndef INCLUDED_TRN_H
#define INCLUDED_TRN_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define ARRLEN(a) (sizeof(a)/sizeof*(a))
#define ASSERT(expr, msg) do { \
	if (expr) \
		break; \
	fprintf(stderr, "%s:%s:%d - %s\n", __FILE__, __FUNCTION__, __LINE__, (msg)); \
	exit(1); \
} while(0)

#define MAX(a, b) ({ \
	__auto_type _a = (a); \
	__auto_type _b = (b); \
	_a > _b ? _a : _b; \
})

#define MIN(a, b) ({ \
	__auto_type _a = (a); \
	__auto_type _b = (b); \
	_a < _b ? _a : _b; \
})

#define TCHECK(t, b) ((t)[(uint8_t)(b)>>4]&(1<<((uint8_t)(b)&0xf)))
#define TSET(t, b) ((t)[(uint8_t)(b)>>4]|=(1<<((uint8_t)(b)&0xf)))
#define TTOGGLE(t, b) ((t)[(uint8_t)(b)>>4]^=(1<<((uint8_t)(b)&0xf)))

#define NODE_START_OF_MATCH 0x1

typedef struct node {
	uint32_t flags;
	uint16_t tests[16];
} Node;

typedef struct set {
	Node **elements;
	size_t numElements;
	size_t refCount;
} Set;

Node *set_addnew(Set *set, const Node *node);
void set_uninit(Set *set);

typedef struct pair {
	Node *node1;
	Node *node2;
} Pair;

typedef struct relation {
	Set *set;
	Pair *pairs;
	size_t numPairs;
} Relation;

void relation_init(Relation *relation, Set *set);
int relation_add(Relation *relation, Pair *pair);
Node *relation_getnext(Relation *relation, Node *node);
Node *relation_getprevious(Relation *relation, Node *node);
void relation_uninit(Relation *relation);

int relation_parse(Relation *relation, const char *regex);
void relation_removeduplicates(Relation *relation);

typedef struct matcher {
	int unused;
} Matcher;

void print_char(FILE *fp, char ch);
void print_node(FILE *fp, const Node *node);

#endif

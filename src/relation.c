#include "trn.h"

Node *set_addnew(Set *set, const Node *node)
{
	Node **newElements;
	Node *newNode;

	newElements = realloc(set->elements, sizeof(*set->elements) *
			(set->numElements + 1));
	if (newElements == NULL)
		return NULL;
	set->elements = newElements;

	newNode = malloc(sizeof(*newNode));
	if (newNode == NULL)
		return NULL;
	*newNode = *node;
	newElements[set->numElements++] = newNode;
	return newNode;
}

void set_uninit(Set *set)
{
	for (size_t i = 0; i < set->numElements; i++)
		free(set->elements[i]);
	free(set->elements);
}

void relation_init(Relation *relation, Set *set)
{
	memset(relation, 0, sizeof(*relation));
	relation->set = set;
}

int relation_add(Relation *relation, Pair *pair)
{
	Pair *newPairs;

	newPairs = realloc(relation->pairs, sizeof(*relation->pairs) *
			(relation->numPairs + 1));
	if (newPairs == NULL)
		return -1;
	relation->pairs = newPairs;
	relation->pairs[relation->numPairs++] = *pair;
	return 0;
}

Node *relation_getnext(Relation *relation, Node *node)
{
	for (size_t i = 0; i < relation->numPairs; i++)
		if (relation->pairs[i].node1 == node)
			return relation->pairs[i].node2;
	return NULL;
}

Node *relation_getprevious(Relation *relation, Node *node)
{
	for (size_t i = 0; i < relation->numPairs; i++)
		if (relation->pairs[i].node2 == node)
			return relation->pairs[i].node1;
	return NULL;
}

void relation_uninit(Relation *relation)
{
	if (--relation->set->refCount == 0)
		set_uninit(relation->set);
	free(relation->pairs);
}

static inline int parse_group(Node *node, const char *regex, const char **pRegex)
{
	int pt = -1, t;
	bool invert;

	memset(node, 0, sizeof(*node));
	invert = *(++regex) == '^';
	if (invert && *(++regex) == ']') {
		TSET(node->tests, '^');
		goto end;
	}

	while (*regex != ']') {
		if (*regex == EOF) {
			fprintf(stderr, "'[' expects a closing bracket ']'\n");
			return -1;
		}
		if (*regex == '\\') {
			switch (*(++regex)) {
			case 'a': t = '\a'; break;
			case 'b': t = '\b'; break;
			case 'f': t = '\f'; break;
			case 'n': t = '\n'; break;
			case 'r': t = '\r'; break;
			case 't': t = '\t'; break;
			case 'v': t = '\v'; break;
			default:
				t = *regex;
			}
		} else if (*regex == '-') {
			if (pt == -1 || *(++regex) == ']') {
				t = '-';
			} else {
				t = *regex;
				for (; pt < t; pt++)
					TSET(node->tests, pt);
			}
		} else {
			t = *regex;
			regex++;
		}
		pt = t;
		TSET(node->tests, t);
	}

	if (invert)
		for (int i = 0; i < (int) ARRLEN(node->tests); i++)
			node->tests[i] ^= 0xffffffff;
end:
	*pRegex = regex + 1;
	return 0;
}

int relation_parse(Relation *relation, const char *regex)
{
	Node node, *pNode;
	Pair pair;
	size_t numTransitive = 1;
	size_t start;
	struct frame {
		size_t numTransitive;
		size_t start;
	} stack[32];
	uint32_t depth = 0;

	start = relation->set->numElements;
	pair.node1 = NULL;
	node.flags = NODE_START_OF_MATCH;
	if ((pair.node2 = set_addnew(relation->set, &node)) == NULL)
		return -1;
	if (relation_add(relation, &pair) < 0)
		return -1;
	while (*regex != '\0') {
		switch (*regex) {
		case '(':
			if (depth == ARRLEN(stack)) {
				fprintf(stderr, "too many '(' without a ')' (max depth=%zu)\n",
						ARRLEN(stack));
				return -1;
			}
			stack[depth].start = relation->set->numElements;
			stack[depth].numTransitive = numTransitive;
			depth++;
			regex++;
			continue;
		case ')':
			if (depth == 0) {
				fprintf(stderr, "')' has no preceding '('\n");
				return -1;
			}
			depth--;
			regex++;
			if (stack[depth].start == relation->set->numElements) {
				fprintf(stderr, "'(' empty bracket content ')'\n");
				return -1;
			}
			switch (*regex) {
			case '+':
				pair.node1 = relation->pairs[relation->numPairs - 1].node2;
				pair.node2 = relation->set->elements[stack[depth].start];
				if (relation_add(relation, &pair) < 0)
					return -1;
				regex++;
				break;
			case '*':
				pair.node1 = relation->pairs[relation->numPairs - 1].node2;
				pair.node2 = relation->set->elements[stack[depth].start];
				if (relation_add(relation, &pair) < 0)
					return -1;
				/* fall through */
			case '?':
				regex++;
				/* if we have a question mark at the end, but
				 * all nodes inside the brackets have a
				 * question mark as well, we can ignore it
				 */
				/*if (relation->numPairs - stack[depth].start ==
						numTransitive - stack[depth].numTransitive)
					break;
				start = stack[depth].start - 1;*/
				break;
			}
			continue;

		case '[':
			if (parse_group(&node, regex, &regex) < 0)
				return -1;
			pNode = set_addnew(relation->set, &node);
			if (pNode == NULL)
				return -1;
			if (depth > 0 && stack[depth - 1].start == 0)
				stack[depth - 1].start =
					relation->set->numElements - 1;
			pair.node2 = pNode;
			for (size_t i = 0; i < numTransitive; i++) {
				pair.node1 = relation->set->elements[start + i];
				if (relation_add(relation, &pair) < 0)
					return -1;
			}
			break;

		default:
			fprintf(stderr, "invalid character: ");
			print_char(stderr, *regex);
			fprintf(stderr, "\n");
			return -1;
		}

		switch (*regex) {
		case '+':
			pair.node1 = pair.node2 = pNode;
			relation_add(relation, &pair);
			regex++;
			/* fall through */
		default:
			numTransitive = 1;
			start = relation->set->numElements - 1;
			break;
		case '*':
			pair.node2 = pair.node1;
			relation_add(relation, &pair);
			/* fall through */
		case '?':
			numTransitive++;
			regex++;
			break;
		}
	}
	if (depth > 0) {
		fprintf(stderr, "'(' has no matching ')'\n");
		return -1;
	}
	return 0;
}

static inline void relation_removepair(Relation *relation, size_t index)
{
	memmove(&relation->pairs[index], &relation->pairs[index + 1],
			sizeof(*relation->pairs) *
			(relation->numPairs - index - 1));
	relation->numPairs--;
}

void relation_removeduplicates(Relation *relation)
{
	for (size_t i = 0; i < relation->numPairs; i++)
		for (size_t j = i + 1; j < relation->numPairs; j++) {
			const Pair pair1 = relation->pairs[i],
				pair2 = relation->pairs[j];
			if (pair1.node1 == pair2.node1 &&
					pair1.node2 == pair2.node2) {
				relation_removepair(relation, j);
				j--;
			}
		}
}

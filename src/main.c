#include "trn.h"

int main(void)
{
	Set set;
	Relation relation;

	memset(&set, 0, sizeof(set));
	relation_init(&relation, &set);

	if (relation_parse(&relation, "[a-b][c-d]?[e-f]+([a-l]?[b-z]?)+[0-9]") < 0) {
		fprintf(stderr, "> ERROR <\n");
		relation_uninit(&relation);
		return 1;
	}
	printf("Got %zu element(s):\n", set.numElements);
	for (size_t i = 0; i < set.numElements; i++) {
		print_node(stdout, set.elements[i]);
		printf("\n");
	}
	printf("\n");
	relation_removeduplicates(&relation);
	printf("Got %zu pair(s):\n", relation.numPairs);
	for (size_t i = 0; i < relation.numPairs; i++) {
		if (relation.pairs[i].node1 != NULL) {
			print_node(stdout, relation.pairs[i].node1);
			printf(" -> ");
		}
		print_node(stdout, relation.pairs[i].node2);
		printf("\n");
	}

	relation_uninit(&relation);
	return 0;
}

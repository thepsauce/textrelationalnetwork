#include "test.h"

void relation_print(const Relation *relation)
{
	printf("Got %zu pair(s):\n", relation->numPairs);
	for (size_t i = 0; i < relation->numPairs; i++) {
		if (relation->pairs[i].node1 != NULL) {
			print_node(stdout, relation->pairs[i].node1);
			printf(" -> ");
		}
		print_node(stdout, relation->pairs[i].node2);
		printf("\n");
	}
}

void get_union(const Relation *a, const Relation *b, Relation *ion)
{
	for (size_t i = 0; i < a->numPairs; i++)
		relation_add(ion, &a->pairs[i]);
	for (size_t i = 0; i < b->numPairs; i++)
		relation_add(ion, &b->pairs[i]);
	relation_removeduplicates(ion);
}

void get_intersection(const Relation *a, const Relation *b, Relation *sect)
{
	for (size_t i = 0; i < a->numPairs; i++) {
		Pair *const paira = &a->pairs[i];
		for (size_t j = 0; j < b->numPairs; j++) {
			Pair *const pairb = &b->pairs[i];
			if (paira->node1 == pairb->node1 &&
					paira->node2 == pairb->node2)
				relation_add(sect, paira);
		}
	}
}

void get_without(const Relation *a, const Relation *b, Relation *out)
{
	for (size_t i = 0; i < a->numPairs; i++) {
		Pair *const paira = &a->pairs[i];
		for (size_t j = 0; j < b->numPairs; j++) {
			Pair *const pairb = &b->pairs[i];
			if (paira->node1 == pairb->node1 &&
					paira->node2 == pairb->node2)
				continue;
			relation_add(out, paira);
		}
	}
}

void get_reflexive_closure(const Relation *relation, Relation *closure)
{
	relation_init(closure, relation->set);
	for (size_t i = 0; i < relation->numPairs; i++) {
		Pair *const pair = &relation->pairs[i];
		if (pair->node1 == pair->node2)
			relation_add(closure, pair);
	}
}

void get_symmetric_closure(const Relation *relation, Relation *closure)
{
	relation_init(closure, relation->set);
	for (size_t i = 0; i < relation->numPairs; i++) {
		Pair *const pair1 = &relation->pairs[i];
		for (size_t j = i + 1; j < relation->numPairs; j++) {
			Pair *const pair2 = &relation->pairs[j];
			if (pair1->node1 == pair2->node1 &&
					pair1->node2 == pair2->node2) {
				relation_add(closure, pair1);
				relation_add(closure, pair2);
				break;
			}
		}
	}
}

void get_transitive_closure(const Relation *relation, Relation *closure)
{
	Pair pair;

	relation_init(closure, relation->set);
	for (size_t i = 0; i < relation->numPairs; i++) {
		Pair *const pair1 = &relation->pairs[i];
		for (size_t j = 0; j < relation->numPairs; j++) {
			Pair *const pair2 = &relation->pairs[j];
			if (pair1->node2 == pair2->node1) {
				pair.node1 = pair1->node1;
				pair.node2 = pair2->node2;
				relation_add(closure, &pair);
			}
		}
	}
}

int main(void)
{
	Set set;
	Relation relation;
	Relation now;

	memset(&set, 0, sizeof(set));
	relation_init(&relation, &set);
	if (relation_parse(&relation, "[a-b][c-d]?[e-f]+([a-l]?[b-z]?)+[0-9]") < 0) {
		fprintf(stderr, "> ERROR <\n");
		relation_uninit(&relation);
		return 0;
	}
	get_reflexive_closure(&relation, &now);
	relation_print(&now);
	relation_uninit(&now);

	get_symmetric_closure(&relation, &now);
	relation_print(&now);
	relation_uninit(&now);

	get_transitive_closure(&relation, &now);
	relation_print(&now);
	relation_uninit(&now);

	relation_uninit(&relation);
	return 0;
}

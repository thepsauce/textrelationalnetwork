#include "trn.h"

void print_char(FILE *fp, char ch)
{
	if (ch < 0) {
		fprintf(fp, "\x1b[32m\\%x\x1b[0m", (uint8_t) ch);
	} else if (ch >= 0 && ch < 32) {
		fprintf(fp, "\x1b[34m^%c\x1b[0m", ch + '@');
	} else {
		fprintf(fp, "%c", ch);
	}
}

void print_node(FILE *fp, const Node *node)
{
	int succ = 0;

	if (node->flags & NODE_START_OF_MATCH) {
		fprintf(fp, "<start>");
		return;
	}
	const int uint16_bits = sizeof(uint16_t) * CHAR_BIT;
	for (int i = 0; i < (int) ARRLEN(node->tests); i++) {
		uint16_t t;

		t = node->tests[i];
		for (int j = 0, p; j < uint16_bits; j++) {
			p = i * uint16_bits + j;
			if (t & 1)
				succ++;
			if (!(t & 1) || (p == 255 && ++p)) {
				if (succ > 3) {
					print_char(fp, p - succ);
					fprintf(fp, "\x1b[31m-\x1b[0m");
					print_char(fp, p - 1);
					succ = 0;
				} else {
					for (; succ > 0; succ--)
						print_char(fp, p - succ);
				}
			}
			t >>= 1;
		}
	}
	fprintf(fp, " (%p)", node);
}

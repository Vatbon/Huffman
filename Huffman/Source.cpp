/* Êîäèðîâùèê àëãîðèòìîì Õàôôìàíà */

#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>
#define POS_COMMAND 1 
#define POS_INPUTFILE 2 
#define POS_OUTPUTFILE 3 
#define POS_MANUAL 1
#define BUFFSIZE 32767 /* Äîëæåí áûòü áîëüøå 10240 */
#define WAY_SIZE 2048
#define ARG_ENCODE "-e"
#define ARG_DECODE "-d"
#define ARG_MANUAL "man"
#define MANUAL "\n------------------------------------------------------\n Huffman.exe [-e/-d] [pathname1] [pathname2]\n\n	1. -e - This switch will pack [pathname1] into [pathname2].\n\n	2. -d - This switch will unpack [pathname1] into [pathname2].\n\n	3. [pathname1] - This path is to a file ,which will be packed or unpacked.\n\n	4. [pathname2] - This path is to a file ,in which [pathname1] will be packed or unpacked.\n------------------------------------------------------\n"

struct huff_node {
	unsigned char data;
	unsigned long int freq;
	struct huff_node *left, *right, *parent;
}; typedef struct huff_node huff_node;

huff_node* huff_node_create(unsigned char data, unsigned long int freq, huff_node * left, huff_node * right, huff_node * parent)
{
	huff_node *node = (huff_node*)malloc(sizeof(huff_node));
	if (node) {
		node->data = data;
		node->freq = freq;
		node->left = left;
		node->right = right;
		node->parent = parent;
	}
	return node;
}

int comp(const void *x1, const void *x2) {
	huff_node* h1 = *((huff_node**)x1);
	huff_node* h2 = *((huff_node**)x2);
	return (h1->freq - h2->freq);
}

huff_node* huff_tree_build(huff_node **node, short int count) {
	short int t_count = count;
	int i;
	while (t_count > 1) {
		qsort(node, t_count, sizeof(node), comp);

		huff_node *top = huff_node_create(NULL,
			node[0]->freq + node[1]->freq, node[1], node[0], NULL);

		node[0] = top;

		for (i = 1; i < t_count - 1; i++)
		{
			node[i] = node[i + 1];
		}

		t_count--;
	}

	if (t_count == 1)
		return node[0];
	else
		return NULL;
}

void huff_bits(char bits[256][256], huff_node * node, char *path, int heigth) {
	if ((node->left == NULL) && (node->right == NULL)) {
		for (int i = 0; i < heigth; i++)
			bits[node->data][i] = path[i];
		bits[node->data][heigth] = -1;
	}
	else
	{
		path[heigth] = 1;
		huff_bits(bits, node->left, path, heigth + 1);
		path[heigth] = 0;
		huff_bits(bits, node->right, path, heigth + 1);

	}
}

short int count_freq(unsigned long int * size, unsigned long int * freq, FILE * InputFile) {
	short int count = 0;
	unsigned char inputchar[BUFFSIZE];
	size_t temp;
	size_t i;

	*size = 0;

	for (i = 0; i < 256; i++)
		freq[i] = 0;
	do {
		temp = fread(inputchar, sizeof(char), BUFFSIZE, InputFile);
		for (i = 0; i < temp; i++)
			freq[inputchar[i]] += 1;
		*size += temp;
	} while (temp == BUFFSIZE);

	for (i = 0; i < 256; i++) {
		if (freq[i])
			count++;
	}

	return count;
}

void write_tree(huff_node *node, unsigned char alph[256], char *way, int *length, short int *amount) {
	/* Lenght, Data, Path */
	if ((node->left == NULL) && (node->right == NULL)) {
		alph[*amount] = node->data;
		*amount = *amount + 1;
	}
	if (node->left != NULL) {
		way[*length] = 1;
		*length = *length + 1;
		write_tree(node->left, alph, way, length, amount);
		way[*length] = 0;
		*length = *length + 1;
	}
	if (node->right != NULL) {
		way[*length] = 1;
		*length = *length + 1;
		write_tree(node->right, alph, way, length, amount);
		way[*length] = 0;
		*length = *length + 1;
	}
}

int Huffman_encode(FILE* InputFile, FILE* OutputFile) {
	unsigned long int freq[256];
	unsigned long int size;
	short int count = count_freq(&size, freq, InputFile);
	huff_node *node[256];
	size_t i,
		j,
		q,
		p;

	j = 0;
	for (i = 0; i < 256; i++) {
		if (freq[i]) {
			node[j] = huff_node_create((unsigned char)i, freq[i], NULL, NULL, NULL);
			j++;
		}
	}

	huff_node *tree;
	tree = huff_tree_build(node, count);

	char bits[256][256];/* bits[data][path] */
	char path[256];


	for (i = 0; i < 256; i++) {
		for (j = 0; j < 256; j++) {
			bits[i][j] = -1;
		}
	}

	if (tree)
		huff_bits(bits, tree, path, 0);

	/*------------------------------------------------------------*/

	unsigned char outputchar[BUFFSIZE];
	size_t temp;
	unsigned char alph[256];

	char way[WAY_SIZE];

	for (i = 0; i < WAY_SIZE; i++)
		way[i] = -1;

	short int tcount = 0;
	int	tlength = 0;

	if (tree)
		write_tree(tree, alph, way, &tlength, &tcount);

	for (i = 0; i < BUFFSIZE; i++) {
		outputchar[i] = 0;
	}

	q = 0;
	p = 0;
	i = 0;

	while ((way[i] != -1) && (i < WAY_SIZE)) {
		outputchar[q] |= way[i] << (7 - p);
		p++;
		i++;
		if (p == 8) {
			p = 0;
			q++;
		}
	}

	if (p)
	{
		q++;
	}
	/* Çàïèñü ñëóæåáíîé èíôîðìàöèè */
	if (size) {
		fwrite(&size, sizeof(unsigned long int), 1, OutputFile);
		fwrite(&tcount, sizeof(short int), 1, OutputFile);
		fwrite(alph, sizeof(char), tcount, OutputFile);
		fwrite(&q, sizeof(int), 1, OutputFile);
		fwrite(outputchar, sizeof(char), q, OutputFile);
	}

	/*------------------------------------------------------------*/
	/* Çàïèñü â Ôàéë */

	fseek(InputFile, 0, SEEK_SET);
	unsigned char inputchar[BUFFSIZE];
	for (i = 0; i < BUFFSIZE; i++) {
		outputchar[i] = 0;
	}
	p = 0;
	q = 0;
	do {
		temp = fread(inputchar, sizeof(char), BUFFSIZE, InputFile);
		for (i = 0; i < temp; i++) {
			j = 0;
			while (bits[inputchar[i]][j] != -1) {
				outputchar[q] |= bits[inputchar[i]][j] << (7 - p);
				p++;
				j++;
				if (p == 8) {
					p = 0;
					q++;
					if (q > BUFFSIZE - 1) {
						fwrite(outputchar, sizeof(char), q, OutputFile);
						for (int l = 0; l < BUFFSIZE; l++) {
							outputchar[l] = 0;
						}
						q = 0;
					}
				}

			}
		}
	} while (temp == BUFFSIZE);

	if (q) {
		fwrite(outputchar, sizeof(char), q, OutputFile);
		outputchar[0] = outputchar[q];
		q = 0;
	}

	if (p) {
		fwrite(outputchar, sizeof(char), 1, OutputFile);
	}

	/* êîíåö çàïèñè â ôàéë*/
	/*------------------------------------------------------------*/
	return 0;
}

huff_node * decode_tree(unsigned char *alph, short int count, int amount, unsigned char *inputchar) {
	unsigned char way[WAY_SIZE];
	size_t i, q;
	unsigned char posb;
	q = 0;
	posb = 128;
	for (i = 0; i<(size_t)(amount * 8); i++) {

		way[i] = ((inputchar[q] & posb) > 0);
		posb = posb >> 1;

		if (posb == 0) {
			posb = 128;
			q++;
		}

	}

	huff_node *tree = huff_node_create(0, 0, NULL, NULL, NULL);
	huff_node *now = tree;

	q = 0; i = 0;
	while (q != count) {
		if (way[i] == 1)
		{
			if (now->left == NULL) {
				now->left = huff_node_create(0, 0, NULL, NULL, now);
				now = now->left;
			}
			else
				if (now->right == NULL) {
					now->right = huff_node_create(0, 0, NULL, NULL, now);
					now = now->right;
				}
		}
		if (way[i] == 0) {
			if ((now->right == NULL) && (now->left == NULL))
			{
				now->data = alph[q];
				q++;
			}
			now = now->parent;
		}
		i++;
	}
	return tree;
}

int Huffman_decode(FILE* InputFile, FILE* OutputFile) {
	short int count;
	unsigned long int size = 0,
		f_size = 0;

	unsigned char inputchar[BUFFSIZE],
		outputchar[BUFFSIZE],
		alph[256];

	size_t temp, i;
	int way_size;

	fseek(InputFile, 0, SEEK_SET);
	fread(&size, sizeof(unsigned long int), 1, InputFile);

	/* Åñëè ôàéë ïóñòîé */
	if (size == 0)
	{
		return 0;
	}

	fread(&count, sizeof(short int), 1, InputFile);
	fread(alph, sizeof(char), count, InputFile);
	fread(&way_size, sizeof(int), 1, InputFile);

	/* Åñëè â àëôàâèòå òîëüêî îäèí ñèìâîë */
	if (count == 1) {
		for (unsigned long int w = 0; w < size; w++)
			fputc(alph[0], OutputFile);
		return 0;
	}

	temp = fread(inputchar, sizeof(char), way_size, InputFile);

	huff_node *tree;
	if (way_size == 0) {
		tree = huff_node_create(alph[0], 1, NULL, NULL, NULL);
	}
	else {
		tree = decode_tree(alph, count, way_size, inputchar);
	}

	unsigned char posb = 128;
	int q, p;
	huff_node* now = tree;
	char trig = 1;

	posb = 128;
	q = 0; p = 0;
	/*------------------------------------------------------------*/
	do {
		temp = fread(inputchar, sizeof(char), BUFFSIZE, InputFile);
		for (i = 0; i < temp; i++) {
			trig = 1;
			while (trig) {
				if (((inputchar[i] & posb)>0) == 1) {

					now = now->left;
					posb = posb >> 1;

					if (posb == 0) {
						posb = 128;
						trig = 0;
					}

				}
				else
					if (((inputchar[i] & posb)>0) == 0) {

						now = now->right;
						posb = posb >> 1;

						if (posb == 0) {
							posb = 128;
							trig = 0;
						}

					}
				if ((now->right == NULL) && (now->left == NULL)) {

					if (f_size < size) {

						outputchar[q] = now->data;
						q++;
						f_size++;

					}
					now = tree;
				}
				if (q > BUFFSIZE - 1) {
					fwrite(outputchar, sizeof(char), q, OutputFile);
					q = 0;
				}
			}

		}
	} while (temp == BUFFSIZE);

	if (q)
		fwrite(outputchar, sizeof(char), q, OutputFile);
	/*------------------------------------------------------------*/
	return 0;
}

int main(int argc, char* argv[]) {

	if (!strcmp(argv[POS_MANUAL], ARG_MANUAL)) {
		printf(MANUAL);
		return 0;
	}
	else
		if ((argc != 4)) {
			printf("\nExpected 3 arguments, received %d arguments.\n", argc - 1);
			printf(MANUAL);
			return EXIT_FAILURE;
		}

	char* InputFileName = argv[POS_INPUTFILE];
	FILE* InputFile;
	fopen_s(&InputFile, InputFileName, "rb");
	if (!InputFile) {
		printf("\nAn error has occurred around [%s] : ", InputFileName);
		perror("");
		return EXIT_FAILURE;
	}
	char* OutputFileName = argv[POS_OUTPUTFILE];
	FILE* OutputFile;
	fopen_s(&OutputFile, OutputFileName, "wb");
	if (!OutputFile) {
		printf("An error has occurred around [%s] : ", OutputFileName);
		perror("");
		return EXIT_FAILURE;
	}

	if (!strcmp(argv[POS_COMMAND], ARG_ENCODE)) {
		if (Huffman_encode(InputFile, OutputFile) == 0) {
			printf("\nPacking is completed.\n");
		}
	}
	else
		if (!strcmp(argv[POS_COMMAND], ARG_DECODE)) {
			if (Huffman_decode(InputFile, OutputFile) == 0) {
				printf("\nUnpacking is completed.\n");
			}
		}
		else
		{
			printf("\nAn error has occurred around first argument.\n");
			printf(MANUAL);
			fclose(InputFile);
			fclose(OutputFile);
			return EXIT_FAILURE;
		}

	fclose(InputFile);
	fclose(OutputFile);
	return 0;
}

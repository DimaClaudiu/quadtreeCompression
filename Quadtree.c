#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct QuadtreeNode
{
	unsigned char blue, green, red;
	unsigned int area;
	int top_left, top_right;
	int bottom_left, bottom_right;
} __attribute__((packed)) QuadtreeNode;

typedef struct color
{
	unsigned char r;
	unsigned char g;
	unsigned char b;

} color;

typedef struct node
{
	color c;
	int length;
	struct node *NW;
	struct node *NE;
	struct node *SW;
	struct node *SE;

} node;

void printColor(color c)
{
	printf("r: %d, g: %d, b: %d\n", c.r, c.g, c.b);
}

void printQuadtree(QuadtreeNode v)
{
	printf("%d %d %d\n", v.red, v.green, v.blue);
	printf("%d\n", v.area);
	printf("%d %d %d %d\n", v.top_left, v.top_right, v.bottom_right, v.bottom_left);
}

// Allocates memory for a new node
node *createNode(color c, int l)
{
	node *newNode = (node *)malloc(sizeof(node));

	newNode->c = c;
	newNode->length = l;

	newNode->NW = NULL;
	newNode->NE = NULL;
	newNode->SW = NULL;
	newNode->SE = NULL;

	return newNode;
}

// Adds node to the first free position in tree
void addNode(node **root, color c, int length)
{
	node *q = createNode(c, length);

	if (*root == NULL)
	{
		*root = q;
		return;
	}

	if ((*root)->NW == NULL)
	{
		(*root)->NW = q;
		return;
	}
	if ((*root)->NE == NULL)
	{
		(*root)->NE = q;
		return;
	}
	if ((*root)->SW == NULL)
	{
		(*root)->SW = q;
		return;
	}
	if ((*root)->SE == NULL)
	{
		(*root)->SE = q;
		return;
	}
}

// Returns the average color of a block inside a matrix, starting from [x,y], going l blocks
color averageColor(int x, int y, int l, int width, color a[width][width])
{
	color c;
	long long red = 0;
	long long green = 0;
	long long blue = 0;

	int i, j;
	for (i = x; i < x + l; i++)
		for (j = y; j < y + l; j++)
		{
			red += a[i][j].r;
			green += a[i][j].g;
			blue += a[i][j].b;
		}

	c.r = red / (l * l);
	c.g = green / (l * l);
	c.b = blue / (l * l);

	return c;
}

// Compares the mean calculated by the given formula to a given tolerance
int checkUniformity(int x, int y, int l, int width, color a[width][width], int p)
{
	color c = averageColor(x, y, l, width, a);
	float mean = 0;

	int i, j;
	for (i = x; i < x + l; i++)
		for (j = y; j < y + l; j++)
		{
			mean += (c.r - a[i][j].r) * (c.r - a[i][j].r);
			mean += (c.g - a[i][j].g) * (c.g - a[i][j].g);
			mean += (c.b - a[i][j].b) * (c.b - a[i][j].b);
		}

	mean /= (3 * l * l);

	return (mean <= p);
}

// Recursively dives the image into quad block until no division is necessary
void compress(node **root, int x, int y, int l, int width, color a[width][width], int p, int *nn)
{
	if (l <= 1) // can't divide 1 pixel
		return;

	addNode(root, averageColor(x, y, l, width, a), l); //storing the average color, wheter it's flat or not
	(*nn)++;

	if (!checkUniformity(x, y, l, width, a, p)) //if it isn't flat(enough), we'll divide
	{
		compress(&(*root)->NW, x, y, l / 2, width, a, p, nn);
		compress(&(*root)->NE, x, y + l / 2, l / 2, width, a, p, nn);
		compress(&(*root)->SE, x + l / 2, y + l / 2, l / 2, width, a, p, nn);
		compress(&(*root)->SW, x + l / 2, y, l / 2, width, a, p, nn);
	}
	// if we're here it means the square is flat and it's been already added to the tree, so we can move on(return)
}

int isLeaf(node *root)
{
	return (root->NW == NULL && root->NE == NULL && root->SE == NULL && root->SW == NULL);
}

// Place the tree inside a vector, also gets the number of leaves
void createVector(node *root, QuadtreeNode q[], int pi, int *fi, int *nc)
{
	if (root == NULL)
		return;
	q[pi].red = (root->c).r;
	q[pi].green = (root->c).g;
	q[pi].blue = (root->c).b;

	q[pi].area = root->length * root->length;

	if (isLeaf(root)) // if it's a leaf
	{
		q[pi].top_left = -1;
		q[pi].top_right = -1;
		q[pi].bottom_right = -1;
		q[pi].bottom_left = -1;
		(*nc)++; // number of useful color blocks(leaf)
	}
	else
	{
		q[pi].top_left = *fi;
		q[pi].top_right = *fi + 1;
		q[pi].bottom_right = *fi + 2;
		q[pi].bottom_left = *fi + 3;

		int i = *fi;
		(*fi) += 4;
		createVector(root->NW, q, i, fi, nc);
		createVector(root->NE, q, i + 1, fi, nc);
		createVector(root->SE, q, i + 2, fi, nc);
		createVector(root->SW, q, i + 3, fi, nc);
	}
}

void writeVector(QuadtreeNode q[], int nn, int nc, char *fileName)
{
	FILE *f = fopen(fileName, "wb");

	fwrite(&nc, sizeof(int), 1, f);
	fwrite(&nn, sizeof(int), 1, f);

	fwrite(q, sizeof(QuadtreeNode), nn, f);

	fclose(f);
}

// Fills an uniform block inside a matrix
void fillSquare(int x, int y, color c, int l, int width, color a[width][width])
{
	int i, j;
	for (i = x; i < x + l; i++)
		for (j = y; j < y + l; j++)
		{
			a[i][j].r = c.r;
			a[i][j].g = c.g;
			a[i][j].b = c.b;
		}
}

// Converting the tree to a 2D array
void decompress(node **root, int x, int y, int l, int width, color a[width][width])
{
	if (*root == NULL)
		return;
	if (isLeaf(*root))
	{
		fillSquare(x, y, (*root)->c, l, width, a);
	}

	decompress(&(*root)->NW, x, y, l / 2, width, a);
	decompress(&(*root)->NE, x, y + l / 2, l / 2, width, a);
	decompress(&(*root)->SE, x + l / 2, y + l / 2, l / 2, width, a);
	decompress(&(*root)->SW, x + l / 2, y, l / 2, width, a);
}

void swapNodes(node **n1, node **n2)
{
	node *temp = *n1;
	*n1 = *n2;
	*n2 = temp;
}

void mirror(node **root, char orientation)
{

	if ((*root)->NW != NULL)
	{
		if (orientation == 'h') // horizontally
		{
			swapNodes(&(*root)->NW, &(*root)->NE);
			swapNodes(&(*root)->SW, &(*root)->SE);
		}
		else // vertically
		{
			swapNodes(&(*root)->NW, &(*root)->SW);
			swapNodes(&(*root)->NE, &(*root)->SE);
		}
	}
	else
		return;


	mirror(&(*root)->NW, orientation);
	mirror(&(*root)->NE, orientation);
	mirror(&(*root)->SE, orientation);
	mirror(&(*root)->SW, orientation);

}

void printImage(char *fileName, int width, color a[width][width])
{
	FILE *f = fopen(fileName, "wb");

	fprintf(f, "P6\n");
	fprintf(f, "%d %d\n", width, width);
	fprintf(f, "255\n");

	int i;
	for (i = 0; i < width; i++)
		fwrite(a[i], sizeof(color), width, f);

	fclose(f);
}

void freeTree(node **root)
{
    if(*root == NULL)
        return;
    
    freeTree(&(*root)->NE);
    freeTree(&(*root)->NW);
    freeTree(&(*root)->SE);
    freeTree(&(*root)->SW);

    free(*root);
}

void readImage(int p, char *inFile, char *outFile, int mode, char orientation)
{
	FILE *f = fopen(inFile, "rb");

	char u[3]; // placehoder
	int width, height, max_value;
	fscanf(f, "%s%d%d%d%c", u, &width, &height, &max_value, &u[0]);

	color colors[width][height];

	int i;
	for (i = 0; i < width; i++)
		fread(&colors[i], sizeof(color), width, f);
	fclose(f);

	node *root = NULL;
	int nc = 0, nn = 0;
	compress(&root, 0, 0, width, width, colors, p, &nn);

	if (mode)
	{
		color a[width][width];
		mirror(&root, orientation);
		decompress(&root, 0, 0, width, width, a);
		printImage(outFile, width, a);

        freeTree(&root);

		return;
	}

	QuadtreeNode q[nn];
	int fi = 1;
	createVector(root, q, 0, &fi, &nc);
	writeVector(q, nn, nc, outFile);

    freeTree(&root);
}

void createTree(node **root, QuadtreeNode q[], int i, int nn)
{
	color c;
	c.r = q[i].red;
	c.g = q[i].green;
	c.b = q[i].blue;
	addNode(root, c, sqrt(q[i].area));

	if (q[i].top_left != -1) // if it isn't a leaf
	{
		createTree(&(*root)->NW, q, q[i].top_left, nn);
		createTree(&(*root)->NE, q, q[i].top_right, nn);
		createTree(&(*root)->SE, q, q[i].bottom_right, nn);
		createTree(&(*root)->SW, q, q[i].bottom_left, nn);
	}
}


void readCompress(char *inFile, char *outFile)
{

	FILE *f = fopen(inFile, "rb");
	if (!f)
	{
		printf("Could not read the file!\n");
		return;
	}

	int nc, nn;
	fread(&nc, sizeof(int), 1, f);
	fread(&nn, sizeof(int), 1, f);

	QuadtreeNode q[nn];

	fread(&q, sizeof(QuadtreeNode), nn, f);
	fclose(f);

	node *root = NULL;
	int width = sqrt(q[0].area);
	color a[width][width];
	createTree(&root, q, 0, nn);
	decompress(&root, 0, 0, width, width, a);
	printImage(outFile, width, a);

    freeTree(&root);
}


int main(int argc, char **argv)
{
	int tolerance = 0;
	char *inFile;
	char *outFile;
	if (argc > 1)
	{
		// compress the image
		if (strstr(argv[1], "-c"))
		{
			tolerance = atoi(argv[2]);
			inFile = argv[3];
			outFile = argv[4];

			readImage(tolerance, inFile, outFile, 0, ' ');
		}
		// decompress the image
		else if (strstr(argv[1], "-d"))
		{
			inFile = argv[2];
			outFile = argv[3];
			readCompress(inFile, outFile);
		}
		// mirror the image (vertically or horizontally)
        else if (strstr(argv[1], "-m"))
        {
            char orientation = argv[2][0];
            tolerance = atoi(argv[3]);
            inFile = argv[4];
            outFile = argv[5];

            readImage(tolerance, inFile, outFile, 1, orientation);

        }
	}
	return 0;
}

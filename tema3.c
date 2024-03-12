///Balteanu Vlad
///311 CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

///structura matricei de pixeli color
typedef struct{
	int R, G, B;
} RGB;

///structra in care retin intreaga poza
typedef struct {
	char magic_word[3];
	int max_val;
	int rows, colls;
	int **pxl_gr;
	RGB **pxl_clr;
} PIC;

///structura cu care retin colturile selectiei
typedef struct {
	int x, y;
} CRN;

///functia care ma ajuta ca valorile
///pixelilor sa nu depaseasca valorile
///0 si 225
int clamp(int x)
{
	if (x > 255)
		return 255;
	if (x < 0)
		return 0;
	return x;
}

///aceasta functie o folosesc pentru
///a stabili cu ce tip de poza am de a face
///in timpul operatiilor
int what_image_type(char *s)
{
	///aici verific daca poza esti in tonuri de gri
	if (strcmp(s, "P2") == 0 || strcmp(s, "P5") == 0)
		return 1;
	///returnez 0 daca este color
	return 0;
}

///aceasta functie verifica selectia pentru rotire
int verify_selection(int a, int b, int c, int d, int rows, int colls)
{
	///daca este patratica
	if (c - a == d - b)
		return 0;
	///sau daca este intreaga poza
	if (c == rows && a == 0 && d == colls && b == 0)
		return 0;
	///retunerez 1 daca selectia este invalida
	return 1;
}

///aici verific daca unghiul de rotire apratine
///multimii date in cerinta
int verify_rotation_angle(int x)
{
	if (x == 0)
		return 0;
	if (x == 90)
		return 0;
	if (x == 180)
		return 0;
	if (x == 270)
		return 0;
	if (x == 360)
		return 0;
	if (x == -90)
		return 0;
	if (x == -180)
		return 0;
	if (x == -270)
		return 0;
	if (x == -360)
		return 0;
	return 1;
}

///aceasta functie o folosesc pentru a stabili care dintre operatii
///se aplica
int choose_querry(char *s)
{
	if (strcmp(s, "LOAD") == 0)
		return 0;
	if (strcmp(s, "SELECT") == 0)
		return 1;
	if (strcmp(s, "SELECT ALL") == 0)
		return 2;
	if (strcmp(s, "HISTOGRAM") == 0)
		return 3;
	if (strcmp(s, "EQUALIZE") == 0)
		return 4;
	if (strcmp(s, "CROP") == 0)
		return 5;
	if (strcmp(s, "APPLY") == 0)
		return 6;
	if (strcmp(s, "SAVE") == 0)
		return 7;
	if (strcmp(s, "ROTATE") == 0)
		return 8;
	if (strcmp(s, "EXIT") == 0)
		return 9;
	return -1;
}

///pentru operatia de APPLY stabilesc care e efectul
int choose_effect(char *s)
{
	if (strcmp(s, "EDGE") == 0)
		return 1;
	if (strcmp(s, "SHARPEN") == 0)
		return 2;
	if (strcmp(s, "BLUR") == 0)
		return 3;
	if (strcmp(s, "GAUSSIAN_BLUR") == 0)
		return 4;
	return 0;
}

///aici verific daca coordonatele unei selectii sunt valide
int verif_coor(int x1, int x2, int y1, int y2, int x, int y)
{
	if (x1 < 0)
		return 1;
	if (x2 > y)
		return 1;
	if (y1 < 0)
		return 1;
	if (y2 > x)
		return 1;
	if (x1 == x2)
		return 1;
	if (y1 == y2)
		return 1;
	return 0;
}

///functie de interschimbat int-uri
void swap(int *x, int *y)
{
	int aux = *x;
	*x = *y;
	*y = aux;
}

///aceasta functie imi elibereaza memoria
///pentru ultima poza incarcata
void free_memory(PIC *pic)
{
	if (what_image_type((*pic).magic_word)) {
		for (int i = 0; i < (*pic).rows; i++)
			free((*pic).pxl_gr[i]);
		free((*pic).pxl_gr);
	} else {
		for (int i = 0; i < (*pic).rows; i++)
			free((*pic).pxl_clr[i]);
		free((*pic).pxl_clr);
	}
}

///nestiind lungimea numelor de fisiere, folosesc
///aceasta functie pentru a determina numele fisierelor
void get_file_name(char **file_name)
{
	int sizemax = 5;
	///incep prin a-mi aloca 5 elemente
	(*file_name) = malloc(sizemax * sizeof(char));
	int size = 0;
	char c;
	scanf("%c", &c);
	///apoi, pana la enter, citesc caracter cu caracter
	///numele fisierului. Cand dimensiunea trece peste
	///cat am alocat, realoc mai multa memorie.
	while (c != '\n') {
		if (size == sizemax - 1) {
			sizemax = sizemax + 5;
			(*file_name) = realloc((*file_name), sizemax * sizeof(char));
			(*file_name)[size] = c;
			size++;
		} else {
			(*file_name)[size] = c;
			size++;
		}
		scanf("%c", &c);
	}
	///la final pun caracterul '\0' pentru ca numele sa fie
	///considerat sir de caractere, si il realoc la size
	///pentru a nu folosi mai multa memorie decat am nevoie
	(*file_name)[size] = '\0';
	(*file_name) = realloc((*file_name), (size + 1) * sizeof(char));
}

///urmatoarele 4 functii sunt foarte asemanatoare, singura diferenta
///find ca fiecare incarca un alt tip de poza
void load_pic_grey_txt(PIC *pic, char **file_name, int *ok1, int *ok2)
{
	///incep prin a deschide fisierul
	FILE *f = fopen((*file_name), "r");
	int ok = 1;
	char txt[3], c, ch;
	///citesc magic word-ul, dar pe care nu-l retin, tinand cont
	///ca l-am citit deja
	fscanf(f, "%s\n", txt);
	///urmatorele randuri sunt folosite pentru a sari peste liniile
	///comentate
	fscanf(f, "%c", &c);
	while (c == '#') {
		while (ch != '\n')
			fscanf(f, "%c", &ch);
		fscanf(f, "%c", &c);
	}
	///pentru a continua citirea ma mut inapoi cu o pozitie in fisier
	fseek(f, -1, SEEK_CUR);
	///citesc numarul de coloane si de randuri din poza
	fscanf(f, "%d%d", &((*pic).colls), &((*pic).rows));
	fscanf(f, "\n%c", &c);
	while (c == '#') {
		while (ch != '\n')
			fscanf(f, "%c", &ch);
		fscanf(f, "%c", &c);
	}
	fseek(f, -1, SEEK_CUR);
	///citesc valoarea maxima pe care o poate lua un pixel
	fscanf(f, "%d", &((*pic).max_val));
	///imi aloc matricea de pixeli, incepand cu vectorul de linii
	(*pic).pxl_gr = (int **)malloc((*pic).rows * sizeof(int *));
	if (!(*pic).pxl_gr) {
		*ok1 = 0;
		printf("Failed to load %s\n", (*file_name));
	} else {
		for (int i = 0; i < (*pic).rows; i++) {
			///apoi pentru fiecare linie aloc vectorul de coloane
			(*pic).pxl_gr[i] = malloc((*pic).colls * sizeof(int));
			if (!(*pic).pxl_gr[i]) {
				*ok1 = 0;
				ok = 0;
				printf("Failed to load %s\n", (*file_name));
				for (int j = 0; j < i; j++)
					free((*pic).pxl_gr[j]);
				free((*pic).pxl_gr);
			}
		}
		///daca alocarea de memorie a reusit, continui cu citirea
		///valorilor pixelilor.
		if (ok == 1) {
			fscanf(f, "\n%c", &c);
			while (c == '#') {
				while (ch != '\n')
					fscanf(f, "%c", &ch);
				fscanf(f, "%c", &c);
			}
			fseek(f, -1, SEEK_CUR);
			for (int i = 0; i < (*pic).rows; i++) {
				for (int j = 0; j < (*pic).colls; j++)
					fscanf(f, "%d", &((*pic).pxl_gr[i][j]));
			}
			printf("Loaded %s\n", (*file_name));
			*ok1 = 1;
			*ok2 = 1;
		}
	}
	///intrucat nu mai am nevoie de numele fisierului,
	///eliberez zona de memorie alocata acestuia si
	///inchid fisierul
	free((*file_name));
	fclose(f);
}

void load_pic_grey_bin(PIC *pic, char **file_name, int *ok1, int *ok2)
{
	FILE *f = fopen((*file_name), "rb");
	int ok = 1;
	char txt[3], c, ch;
	fscanf(f, "%s\n", txt);
	fscanf(f, "%c", &c);
	while (c == '#') {
		while (ch != '\n')
			fscanf(f, "%c", &ch);
		fscanf(f, "%c", &c);
	}
	fseek(f, -1, SEEK_CUR);
	fscanf(f, "%d%d", &((*pic).colls), &((*pic).rows));
	fscanf(f, "\n%c", &c);
	while (c == '#') {
		while (ch != '\n')
			fscanf(f, "%c", &ch);
		fscanf(f, "%c", &c);
	}
	fseek(f, -1, SEEK_CUR);
	fscanf(f, "%d", &((*pic).max_val));
	(*pic).pxl_gr = (int **)malloc((*pic).rows * sizeof(int *));
	if (!(*pic).pxl_gr) {
		*ok1 = 0;
		printf("Failed to load %s\n", (*file_name));
	} else {
		for (int i = 0; i < (*pic).rows; i++) {
			(*pic).pxl_gr[i] = (int *)calloc((*pic).colls, sizeof(int));
			if (!(*pic).pxl_gr[i]) {
				*ok1 = 0;
				ok = 0;
				printf("Failed to load %s\n", (*file_name));
				for (int j = 0; j < i; j++)
					free((*pic).pxl_gr[j]);
				free((*pic).pxl_gr);
			}
		}
		if (ok == 1) {
			fscanf(f, "\n%c", &c);
			while (c == '#') {
				while (ch != '\n')
					fscanf(f, "%c", &ch);
				fscanf(f, "%c", &c);
			}
			///singura diferenta fata de mai sus este ca citesc
			///folosind fread, avand in vedere ca valorile pixelilor
			///sunt retinute binar.
			fseek(f, -1, SEEK_CUR);
			for (int i = 0; i < (*pic).rows; i++) {
				for (int j = 0; j < (*pic).colls; j++)
					fread(&((*pic).pxl_gr[i][j]), 1, 1, f);
			}
			printf("Loaded %s\n", (*file_name));
			*ok1 = 1;
			*ok2 = 1;
		}
	}
	free((*file_name));
	fclose(f);
}

void load_pic_color_txt(PIC *pic, char **file_name, int *ok1, int *ok2)
{
	FILE *f = fopen((*file_name), "r");
	int ok = 1;
	char txt[3], c, ch;
	fscanf(f, "%s\n", txt);
	fscanf(f, "%c", &c);
	while (c == '#') {
		while (ch != '\n')
			fscanf(f, "%c", &ch);
		fscanf(f, "%c", &c);
	}
	fseek(f, -1, SEEK_CUR);
	fscanf(f, "%d%d", &((*pic).colls), &((*pic).rows));
	fscanf(f, "\n%c", &c);
	while (c == '#') {
		while (ch != '\n')
			fscanf(f, "%c", &ch);
		fscanf(f, "%c", &c);
	}
	fseek(f, -1, SEEK_CUR);
	fscanf(f, "%d", &((*pic).max_val));
	///aloc matricea de pixeli color, asa ca elementele nu mai
	///au dimensiunea sizeof(int) ci dimensiunea structurii RGB
	///in care retin cate 3 int-uri pentru fiecare element
	(*pic).pxl_clr = (RGB **)malloc((*pic).rows * sizeof(RGB *));
	if (!(*pic).pxl_clr) {
		*ok1 = 0;
		printf("Failed to load%s\n", (*file_name));
	} else {
		for (int i = 0; i < (*pic).rows; i++) {
			(*pic).pxl_clr[i] = malloc((*pic).colls * sizeof(RGB));
			if (!(*pic).pxl_clr[i]) {
				*ok1 = 0;
				ok = 0;
				printf("Failed to load %s\n", (*file_name));
				for (int j = 0; j < i; j++)
					free((*pic).pxl_clr[j]);
				free((*pic).pxl_clr);
			}
		}
		if (ok == 1) {
			fscanf(f, "\n%c", &c);
			while (c == '#') {
				while (ch != '\n')
					fscanf(f, "%c", &ch);
				fscanf(f, "%c", &c);
			}
			fseek(f, -1, SEEK_CUR);
			///aici diferenta esta ca incarc o poza color,
			///asa ca citesc cele 3 valori de rosu, verde si,
			///albastru
			for (int i = 0; i < (*pic).rows; i++) {
				for (int j = 0; j < (*pic).colls; j++) {
					fscanf(f, "%d", &((*pic).pxl_clr[i][j].R));
					fscanf(f, "%d", &((*pic).pxl_clr[i][j].G));
					fscanf(f, "%d", &((*pic).pxl_clr[i][j].B));
				}
			}
			printf("Loaded %s\n", (*file_name));
			*ok1 = 1;
			*ok2 = 1;
		}
	}
	free((*file_name));
	fclose(f);
}

void load_pic_color_bin(PIC *pic, char **file_name, int *ok1, int *ok2)
{
	FILE *f = fopen((*file_name), "rb");
	int ok = 1;
	char txt[3], c, ch;
	fscanf(f, "%s\n", txt);
	fscanf(f, "%c", &c);
	while (c == '#') {
		while (ch != '\n')
			fscanf(f, "%c", &ch);
		fscanf(f, "%c", &c);
	}
	fseek(f, -1, SEEK_CUR);
	fscanf(f, "%d%d", &((*pic).colls), &((*pic).rows));
	fscanf(f, "\n%c", &c);
	while (c == '#') {
		while (ch != '\n')
			fscanf(f, "%c", &ch);
		fscanf(f, "%c", &c);
	}
	fseek(f, -1, SEEK_CUR);
	fscanf(f, "%d", &((*pic).max_val));
	(*pic).pxl_clr = (RGB **)malloc((*pic).rows * sizeof(RGB *));
	if (!(*pic).pxl_clr) {
		printf("Failed to load%s\n", (*file_name));
		*ok1 = 0;
	} else {
		for (int i = 0; i < (*pic).rows; i++) {
			(*pic).pxl_clr[i] = (RGB *)calloc((*pic).colls, sizeof(RGB));
			if (!(*pic).pxl_clr[i]) {
				*ok1 = 0;
				ok = 0;
				printf("Failed to load%s\n", (*file_name));
				for (int j = 0; j < i; j++)
					free((*pic).pxl_clr[j]);
				free((*pic).pxl_clr);
			}
		}
		if (ok == 1) {
			fscanf(f, "\n%c", &c);
			while (c == '#') {
				while (ch != '\n')
					fscanf(f, "%c", &ch);
				fscanf(f, "%c", &c);
			}
			fseek(f, -1, SEEK_CUR);
			///la fel ca mai sus, doar ca citesc cu fread, fisierul
			///fiind unul binar
			for (int i = 0; i < (*pic).rows; i++) {
				for (int j = 0; j < (*pic).colls; j++) {
					fread(&((*pic).pxl_clr[i][j].R), 1, 1, f);
					fread(&((*pic).pxl_clr[i][j].G), 1, 1, f);
					fread(&((*pic).pxl_clr[i][j].B), 1, 1, f);
				}
			}
			printf("Loaded %s\n", (*file_name));
			*ok1 = 1;
			*ok2 = 1;
		}
	}
	free((*file_name));
	fclose(f);
}

///aceasta este functia principala de load
void load_pic(PIC *pic, int *ok, CRN *c1, CRN *c2, int *ok1)
{
	///aici eliberez memoria pozei anterior incarcate,
	///daca este cazul
	if ((*ok1) == 1 && (*ok) == 1)
		free_memory(pic);
	char *file_name, c;
	scanf("%c", &c);
	if (c == '\n') {
		printf("Invalid command\n");
		return;
	}
	///citesc numele fisierului pe care-l deschid
	get_file_name(&file_name);
	FILE *f = fopen(file_name, "rb");
	if (!f) {
		*ok = 0;
		printf("Failed to load %s\n", file_name);
		free(file_name);
	} else {
		///citesc magic word-ul din fisier, apoi il inchid
		fread((*pic).magic_word, 1, 2, f);
		(*pic).magic_word[2] = '\0';
		fclose(f);
		///in functie de magic word, se executa una din functii
		if (strcmp((*pic).magic_word, "P2") == 0)
			load_pic_grey_txt(pic, &file_name, ok, ok1);
		if (strcmp((*pic).magic_word, "P5") == 0)
			load_pic_grey_bin(pic, &file_name, ok, ok1);
		if (strcmp((*pic).magic_word, "P3") == 0)
			load_pic_color_txt(pic, &file_name, ok, ok1);
		if (strcmp((*pic).magic_word, "P6") == 0)
			load_pic_color_bin(pic, &file_name, ok, ok1);
		///la incarcarea unei poze, selectia o initializez ca
		///fiind toata poza, coltul stanga sus are coordonatele
		///0-0 iar cel din dreapta jos are coordonatele
		///(*pic).rows - (*pic).colls
		(*c1).x = 0;
		(*c1).y = 0;
		(*c2).x = (*pic).rows;
		(*c2).y = (*pic).colls;
	}
}

///aceasta functie face functia de select all
///retinand ca si coordonatele selectiei coltul
///din stanga sus al pozei si din dreapta jos al pozei.
void select_all(CRN *c1, CRN *c2, int ok, PIC pic)
{
	if (ok == 0) {
		printf("No image loaded\n");
	} else {
		(*c1).x = 0;
		(*c1).y = 0;
		(*c2).x = pic.rows;
		(*c2).y = pic.colls;
		printf("Selected ALL\n");
	}
}

///aceasta functie face o selectie
void sel_coor(CRN *c1, CRN *c2, int ok, PIC pic)
{
	int x1, x2, y1, y2, k = 0, *v, size_max = 4;
	///imi aloc un vector pentru a ma asigura ca sunt
	///citite 4 valori, toate numere.
	v = malloc(4 * sizeof(int));
	char *s, c;
	scanf("%c", &c);
	if (c == '\n') {
		printf("Invalid command\n");
		free(v);
		return;
	}
	get_file_name(&s);
	if (strstr(s, "ALL")) {
		select_all(c1, c2, ok, pic);
	} else {
		char *p = strtok(s, " ");
		while (p) {
			int n = strlen(p);
			for (int i = 0; i < n; i++) {
				if (isalpha(p[i])) {
					printf("Invalid command\n");
					free(s);
					free(v);
					return;
				}
			}
			v[k] = atoi(p);
			k++;
			if (k == size_max - 1) {
				size_max += 4;
				v = realloc(v, size_max * sizeof(int));
			}
			p = strtok(NULL, " ");
		}
		v = realloc(v, k * sizeof(int));
		///aici ajung doar daca citesc 4 numere.
		if (k == 4) {
			x1 = v[0];
			y1 = v[1];
			x2 = v[2];
			y2 = v[3];
			if (x1 > x2)
				swap(&x1, &x2);
			if (y1 > y2)
				swap(&y1, &y2);
			if (ok == 0) {
				printf("No image loaded\n");
			} else {
				///verific daca coordonatele sunt valide
				if (verif_coor(x1, x2, y1, y2, pic.rows, pic.colls)) {
					printf("Invalid set of coordinates\n");
				} else {
					///daca da, le retin in variabilele c1 si c2.
					(*c1).x = y1;
					(*c1).y = x1;
					(*c2).y = x2;
					(*c2).x = y2;
					printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
				}
			}
		} else {
			printf("Invalid command\n");
		}
	}
	free(v);
	free(s);
}

///aceasta functie afiseaza histograma unei poze in tonuri de gri,
///cu y binuri si x stelute
void make_histogram(PIC pic, int x, int y)
{
	int interval = 256 / y, frecv_max = -1;
	int *w;
	w = calloc(y, sizeof(int));
	for (int i = 0; i < pic.rows; i++) {
		for (int j = 0; j < pic.colls; j++) {
			int t = pic.pxl_gr[i][j] / interval;
			w[t]++;
			if (w[t] > frecv_max)
				frecv_max = w[t];
		}
	}
	for (int i = 0; i < y; i++) {
		int star_number = (w[i] * x) / frecv_max;
		printf("%d\t|\t", star_number);
		for (int j = 0; j < star_number; j++)
			printf("*");
		printf("\n");
	}
	free(w);
}

///in aceasta functie fac verificarea daca citesc 2 inturi
///sau daca am o poza incarcata in memorie si daca acea poza
///este in tonuri de gri
void histogram(PIC pic, int ok)
{
	int x, y, *v, k = 0, size_max = 2;
	char *s;
	v = malloc(2 * sizeof(int));
	if (ok == 0) {
		printf("No image loaded\n");
		char c;
		scanf("%c", &c);
		while (c != '\n')
			scanf("%c", &c);
	} else {
		char c;
		scanf("%c", &c);
		if (c == '\n') {
			printf("Invalid command\n");
			free(v);
			return;
		}
		get_file_name(&s);
		char *p = strtok(s, " ");
		while (p) {
			int n = strlen(p);
			for (int i = 0; i < n; i++) {
				if (isalpha(p[i])) {
					printf("Invalid command\n");
					free(v);
					free(s);
					return;
				}
			}
			v[k] = atoi(p);
			k++;
			if (k == size_max - 1) {
				size_max += 2;
				v = realloc(v, size_max * sizeof(int));
			}
			p = strtok(NULL, " ");
		}
		free(s);
		v = realloc(v, k * sizeof(int));
		if (k == 2) {
			if (!what_image_type(pic.magic_word)) {
				printf("Black and white image needed\n");
			} else {
				///aici ajung doar daca pot face histograma
				x = v[0];
				y = v[1];
				make_histogram(pic, x, y);
			}
		} else {
			printf("Invalid command\n");
		}
	}
	free(v);
}

///aceasta functie face egalizarea unei poze, folosind
///acelasi principiu ca la histograma
void equalize(PIC *pic, int ok)
{
	if (ok == 0) {
		printf("No image loaded\n");
	} else {
		if (!what_image_type((*pic).magic_word)) {
			printf("Black and white image needed\n");
		} else {
			///acesta este vectorul de frecventa al tuturor
			///valorilor posibile
			int *v;
			v = calloc(256, sizeof(int));
			for (int i = 0; i < (*pic).rows; i++) {
				for (int j = 0; j < (*pic).colls; j++)
					v[(*pic).pxl_gr[i][j]]++;
			}
			double x;
			double area = (*pic).rows * (*pic).colls;
			///aceste valori le folosesc pentru a calcula,
			///dupa cerinta, valorile pixelilor dupa egalizare
			for (int i = 0; i < (*pic).rows; i++) {
				for (int j = 0; j < (*pic).colls; j++) {
					int s = 0;
					for (int k = 0; k <= (*pic).pxl_gr[i][j]; k++)
						s = s + v[k];
					x = (255 / area) * s;
					(*pic).pxl_gr[i][j] = round(x);
					(*pic).pxl_gr[i][j] = clamp((*pic).pxl_gr[i][j]);
				}
			}
			printf("Equalize done\n");
			free(v);
		}
	}
}

///aceasta functie roteste toata poza
void rotate_whole_pic(PIC *pic, CRN *c2)
{
	///aici stabilesc cu care tip de poza am de a face
	if (what_image_type((*pic).magic_word)) {
		///imi aloc o matrice auxiliara
		int **temp_pic;
		temp_pic = (int **)malloc((*pic).colls * sizeof(int *));
		if (!temp_pic) {
			fprintf(stderr, "Can`t allocate memory\n");
			free_memory(pic); exit(-1);
		}
		for (int i = 0; i < (*pic).colls; i++) {
			temp_pic[i] = malloc((*pic).rows * sizeof(int));
			if (!temp_pic[i]) {
				fprintf(stderr, "Can`t allocate memory\n");
				for (int j = 0; j < i; j++)
					free(temp_pic[i]);
				free(temp_pic); free_memory(pic); exit(-1);
			}
		}
		///copiez valorile in ordinea dorita
		for (int i = 0; i < (*pic).colls; i++) {
			for (int j = 0; j < (*pic).rows; j++)
				temp_pic[i][j] = (*pic).pxl_gr[(*pic).rows - j - 1][i];
		}
		///eliberez matricea de pixeli
		free_memory(pic);
		///si o realloc cu numarul de randuri si coloane interschimbat
		(*pic).pxl_gr = (int **)malloc((*pic).colls * sizeof(int *));
		for (int i = 0; i < (*pic).colls; i++)
			(*pic).pxl_gr[i] = malloc((*pic).rows * sizeof(int));
		for (int i = 0; i < (*pic).colls; i++) {
			for (int j = 0; j < (*pic).rows; j++)
				(*pic).pxl_gr[i][j] = temp_pic[i][j];
		}
		///interschimb numarul de linii si de coloane
		swap(&(*pic).rows, &(*pic).colls);
		///interschimb si coordonatele coltului dreapta jos al selectiei curente
		swap(&(*c2).x, &(*c2).y);
		///eliberez matricea auxiliara
		for (int i = 0; i < (*pic).rows; i++)
			free(temp_pic[i]);
		free(temp_pic);
	} else {
		///acelasi principiu este aplicat si aici doar ca matricea
		///de pixeli este RGB
		RGB **temp_pic;
		temp_pic = (RGB **)malloc((*pic).colls * sizeof(RGB *));
		if (!temp_pic) {
			fprintf(stderr, "Can`t allocate memory\n");
			free_memory(pic);
			exit(-1);
		}
		for (int i = 0; i < (*pic).colls; i++) {
			temp_pic[i] = malloc((*pic).rows * sizeof(RGB));
			if (!temp_pic[i]) {
				fprintf(stderr, "Can`t allocate memory\n");
				for (int j = 0; j < i; j++)
					free(temp_pic[i]);
				free(temp_pic); free_memory(pic); exit(-1);
			}
		}
		for (int i = 0; i < (*pic).colls; i++) {
			for (int j = 0; j < (*pic).rows; j++)
				temp_pic[i][j] = (*pic).pxl_clr[(*pic).rows - j - 1][i];
		}
		free_memory(pic);
		int y = (*pic).colls;
		(*pic).pxl_clr = (RGB **)malloc(y * sizeof(RGB));
		for (int i = 0; i < (*pic).colls; i++)
			(*pic).pxl_clr[i] = malloc((*pic).rows * sizeof(RGB));
		for (int i = 0; i < (*pic).colls; i++) {
			for (int j = 0; j < (*pic).rows; j++)
				(*pic).pxl_clr[i][j] = temp_pic[i][j];
		}
		swap(&(*pic).rows, &(*pic).colls);
		swap(&(*c2).x, &(*c2).y);
		for (int i = 0; i < (*pic).rows; i++)
			free(temp_pic[i]);
		free(temp_pic);
	}
}

///aceasta este functia de rotate pe o anumita selectie
void rotate_pic(PIC *pic, CRN c1, CRN *c2, int angle)
{
	///dau o noua valoare unghiului pentru a face o singura operatie
	///de rotire la dreapta, insa aplicata de mai multe ori
	angle = angle / 90;
	if (angle < 0)
		angle += 4;
	///daca selectia este facuta pe toata matricea, apelez functia de
	///rotate a intregii poze
	if (c1.x == 0 && c1.y == 0 &&
		(*c2).x == (*pic).rows && (*c2).y == (*pic).colls) {
		for (int k = 0; k < angle; k++)
			rotate_whole_pic(pic, c2);
	} else {
		if (what_image_type((*pic).magic_word)) {
			for (int k = 0; k < angle; k++) {
				///imi aloc o matrice auxiliara in care copiez valorile
				///corespunzatoare
				int **temp_pic =
				(int **)malloc(((*c2).x - c1.x) * sizeof(int *));
				for (int i = 0; i < (*c2).x - c1.x; i++)
					temp_pic[i] = malloc(((*c2).y - c1.y) * sizeof(int));
				for (int i = c1.x; i < (*c2).x; i++) {
					for (int j = c1.y; j < (*c2).y; j++)
						temp_pic[i - c1.x][j - c1.y] =
					(*pic).pxl_gr[(*c2).x - (j - c1.y) - 1][c1.y + (i - c1.x)];
				}
				for (int i = c1.x; i < (*c2).x; i++) {
					for (int j = c1.y; j < (*c2).y; j++)
						(*pic).pxl_gr[i][j] = temp_pic[i - c1.x][j - c1.y];
				}
				///dupa ce termin operatie elibere memoria folosita auxiliar
				for (int i = 0; i < (*c2).x - c1.x; i++)
					free(temp_pic[i]);
				free(temp_pic);
			}
		} else {
			for (int k = 0; k < angle; k++) {
				RGB **temp_pic =
				(RGB **)malloc(((*c2).x - c1.x) * sizeof(RGB *));
				for (int i = 0; i < (*c2).x - c1.x; i++)
					temp_pic[i] = malloc(((*c2).y - c1.y) * sizeof(RGB));
				for (int i = c1.x; i < (*c2).x; i++) {
					for (int j = c1.y; j < (*c2).y; j++)
						temp_pic[i - c1.x][j - c1.y] =
					(*pic).pxl_clr[(*c2).x - (j - c1.y) - 1][c1.y + i - c1.x];
				}
				for (int i = c1.x; i < (*c2).x; i++) {
					for (int j = c1.y; j < (*c2).y; j++)
						(*pic).pxl_clr[i][j] = temp_pic[i - c1.x][j - c1.y];
				}
				for (int i = 0; i < (*c2).x - c1.x; i++)
					free(temp_pic[i]);
				free(temp_pic);
			}
		}
	}
}

///aceasta este functia principala de rotate in care verific daca
///unghiul de rotatie apartine multimii date, daca am o matrice
///incarcata in memorie sau daca selectia data este valida
void rotate(PIC *pic, int ok, CRN c1, CRN *c2)
{
	///c1 si c2 reprezinta corner1 (stanga sus), respectiv corner2 (dreapta jos)
	int x = c1.x, y = c1.y, x1 = (*c2).x, y1 = (*c2).y;
	int angle; char c;
	scanf("%c", &c);
	if (c == '\n') {
		printf("Invalid command\n");
		return;
	}
	scanf("%d", &angle);
	if (ok == 0) {
		printf("No image loaded\n");
	} else {
		if (verify_selection(x, y, x1, y1, (*pic).rows, (*pic).colls)) {
			printf("The selection must be square\n");
		} else {
			if (verify_rotation_angle(angle)) {
				printf("Unsupported rotation angle\n");
			} else {
				rotate_pic(pic, c1, c2, angle);
				printf("Rotated %d\n", angle);
			}
		}
	}
}

///aceasta este functie care da crop unei poze gri
void crop_grey(PIC *pic, int x, int y, CRN *c1, CRN *c2)
{
	///incep prin a-mi aloca o matrice auxiliara
	int **temp_pic; ///temp -> temporary
	temp_pic = (int **)malloc(x * sizeof(int *));
	if (!temp_pic) {
		free_memory(pic);
		fprintf(stderr, "Couldn`t allocate memory");
		exit(-1);
	}
	for (int i = 0; i < x; i++) {
		temp_pic[i] = malloc(y * sizeof(int));
		if (!temp_pic[i]) {
			for (int j = 0; j < i; j++)
				free(temp_pic[j]);
			free(temp_pic);
			free_memory(pic);
			fprintf(stderr, "Couldn't allocate memory");
			exit(-1);
		}
	}
	///aici imi copiez valorile de care am nevoie din selectie
	for (int i = (*c1).x; i < (*c2).x; i++) {
		for (int j = (*c1).y; j < (*c2).y; j++)
			temp_pic[i - (*c1).x][j - (*c1).y] = (*pic).pxl_gr[i][j];
	}
	free_memory(pic);
	///iar aici imi realoc memoria pentru a le recopia inapoi in poza
	(*pic).pxl_gr = (int **)malloc(x * sizeof(int *));
	for (int i = 0; i < x; i++)
		(*pic).pxl_gr[i] = malloc(y * sizeof(int));
	(*pic).rows = x;
	(*pic).colls = y;
	(*c1).x = 0;
	(*c1).y = 0;
	(*c2).x = (*pic).rows;
	(*c2).y = (*pic).colls;
	///le copiez inapoi
	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++)
			(*pic).pxl_gr[i][j] = temp_pic[i][j];
	}
	///si eliberez zona de memorie folosita auxilar
	for (int i = 0; i < x; i++)
		free(temp_pic[i]);
	free(temp_pic);
}

///aceasta functie imi da crop la o poza color si functioneaza
///pe acelasi principiu ca cea de mai sus
void crop_clr(PIC *pic, int x, int y, CRN *c1, CRN *c2)
{
	RGB **temp_pic; ///temp -> temporary
	temp_pic = (RGB **)malloc(x * sizeof(RGB *));
	if (!temp_pic) {
		free_memory(pic);
		fprintf(stderr, "Couldn`t allocate memory");
		exit(-1);
	}
	for (int i = 0; i < x; i++) {
		temp_pic[i] = malloc(y * sizeof(RGB));
		if (!temp_pic[i]) {
			for (int j = 0; j < i; j++)
				free(temp_pic[j]);
			free(temp_pic);
			free_memory(pic);
			fprintf(stderr, "Couldn't allocate memory");
			exit(-1);
		}
	}
	for (int i = (*c1).x; i < (*c2).x; i++) {
		for (int j = (*c1).y; j < (*c2).y; j++)
			temp_pic[i - (*c1).x][j - (*c1).y] = (*pic).pxl_clr[i][j];
	}
	free_memory(pic);
	(*pic).pxl_clr = (RGB **)malloc(x * sizeof(RGB *));
	for (int i = 0; i < x; i++)
		(*pic).pxl_clr[i] = malloc(y * sizeof(RGB));
	(*pic).rows = x;
	(*pic).colls = y;
	(*c1).x = 0;
	(*c1).y = 0;
	(*c2).x = (*pic).rows;
	(*c2).y = (*pic).colls;
	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++)
			(*pic).pxl_clr[i][j] = temp_pic[i][j];
	}
	for (int i = 0; i < x; i++)
		free(temp_pic[i]);
	free(temp_pic);
}

///aceasta este functia principala de crop in care verific
///daca am o poza incarcata in memorie
void crop(PIC *pic, CRN *c1, CRN *c2, int ok)
{
	if (ok == 0) {
		printf("No image loaded\n");
	} else {
		int x = (*c2).x - (*c1).x;
		int y = (*c2).y - (*c1).y;
		if (what_image_type((*pic).magic_word))
			crop_grey(pic, x, y, c1, c2);
		else
			crop_clr(pic, x, y, c1, c2);
		printf("Image cropped\n");
	}
}

///aceata este functia care imi aplica un efect dat de un anumit kernel
///pe un anumit pixel
RGB effect(int kernel[3][3], int d, PIC *pic, int i, int j)
{
	///new_pixel este folosit pentru a calcula valorile
	///de care am nevoie
	RGB new_pixel;
	double sum = 0;
	///in valoarea sum retin suma elementelor dupa ce aplic
	///kernelul pe un pixel
	sum = sum + kernel[1][1] * (*pic).pxl_clr[i][j].R;
	sum = sum + kernel[0][0] * (*pic).pxl_clr[i - 1][j - 1].R;
	sum = sum + kernel[0][1] * (*pic).pxl_clr[i - 1][j].R;
	sum = sum + kernel[0][2] * (*pic).pxl_clr[i - 1][j + 1].R;
	sum = sum + kernel[1][0] * (*pic).pxl_clr[i][j - 1].R;
	sum = sum + kernel[1][2] * (*pic).pxl_clr[i][j + 1].R;
	sum = sum + kernel[2][0] * (*pic).pxl_clr[i + 1][j - 1].R;
	sum = sum + kernel[2][1] * (*pic).pxl_clr[i + 1][j].R;
	sum = sum + kernel[2][2] * (*pic).pxl_clr[i + 1][j + 1].R;
	sum = sum / d;
	new_pixel.R = 0;
	new_pixel.R = clamp(round(sum));
	sum = 0;
	sum = sum + kernel[1][1] * (*pic).pxl_clr[i][j].G;
	sum = sum + kernel[0][0] * (*pic).pxl_clr[i - 1][j - 1].G;
	sum = sum + kernel[0][1] * (*pic).pxl_clr[i - 1][j].G;
	sum = sum + kernel[0][2] * (*pic).pxl_clr[i - 1][j + 1].G;
	sum = sum + kernel[1][0] * (*pic).pxl_clr[i][j - 1].G;
	sum = sum + kernel[1][2] * (*pic).pxl_clr[i][j + 1].G;
	sum = sum + kernel[2][0] * (*pic).pxl_clr[i + 1][j - 1].G;
	sum = sum + kernel[2][1] * (*pic).pxl_clr[i + 1][j].G;
	sum = sum + kernel[2][2] * (*pic).pxl_clr[i + 1][j + 1].G;
	sum = sum / d;
	new_pixel.G = 0;
	new_pixel.G = clamp(round(sum));
	sum = 0;
	sum = sum + kernel[1][1] * (*pic).pxl_clr[i][j].B;
	sum = sum + kernel[0][0] * (*pic).pxl_clr[i - 1][j - 1].B;
	sum = sum + kernel[0][1] * (*pic).pxl_clr[i - 1][j].B;
	sum = sum + kernel[0][2] * (*pic).pxl_clr[i - 1][j + 1].B;
	sum = sum + kernel[1][0] * (*pic).pxl_clr[i][j - 1].B;
	sum = sum + kernel[1][2] * (*pic).pxl_clr[i][j + 1].B;
	sum = sum + kernel[2][0] * (*pic).pxl_clr[i + 1][j - 1].B;
	sum = sum + kernel[2][1] * (*pic).pxl_clr[i + 1][j].B;
	sum = sum + kernel[2][2] * (*pic).pxl_clr[i + 1][j + 1].B;
	sum = sum / d;
	new_pixel.B = 0;
	new_pixel.B = clamp(round(sum));
	return new_pixel;
}

///urmatoarele 4 functii ruleaza pe acelasi principiu, doar ca
///difera initializarea kernelului, asa ca voi explica functionarea
///lor doar aici
void apply_edge(PIC *pic, CRN c1, CRN c2)
{
	///imi aloc o matrice auxiliara in care pot aplica operatia
	///fara sa schimba valorile pixelilor din poza
	RGB **temp_pic;
	int kernel[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
	int x1 = c1.x, x2 = c2.x, y1 = c1.y, y2 = c2.y;
	///ca sa aplic un efect am nevoie de o zona de minim 3x3
	///asa ca marginile pozei nu le voi schimba
	if (c1.x == 0)
		x1 = 1;
	if (c1.y == 0)
		y1 = 1;
	if (c2.x == (*pic).rows)
		x2 = x2 - 1;
	if (c2.y == (*pic).colls)
		y2 = y2 - 1;
	///aloc matricea auxiliara
	temp_pic = (RGB **)malloc((c2.x - c1.x) * sizeof(RGB *));
	if (!temp_pic) {
		fprintf(stderr, "Can`t allocate memory\n");
		free_memory(pic);
		exit(-1);
	}
	for (int i = 0; i < c2.x - c1.x; i++) {
		temp_pic[i] = malloc((c2.y - c1.y) * sizeof(RGB));
		if (!temp_pic[i]) {
			for (int j = 0; j < i; j++)
				free(temp_pic[j]);
			free(temp_pic);
			free_memory(pic);
			fprintf(stderr, "Can`t allocate memory\n");
			exit(-1);
		}
	}
	///aici aplic efectiv efectul in matricea auxiliara
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++)
			temp_pic[i - x1][j - y1] = effect(kernel, 1, pic, i, j);
	}
	///apoi copiez noile valori ale pixelilor
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++)
			(*pic).pxl_clr[i][j] = temp_pic[i - x1][j - y1];
	}
	///eliberez matricea auxiliara
	for (int i = 0; i < c2.x - c1.x; i++)
		free(temp_pic[i]);
	free(temp_pic);
}

void apply_sharpen(PIC *pic, CRN c1, CRN c2)
{
	RGB **temp_pic;
	int kernel[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};
	int x1 = c1.x, x2 = c2.x, y1 = c1.y, y2 = c2.y;
	if (c1.x == 0)
		x1 = 1;
	if (c1.y == 0)
		y1 = 1;
	if (c2.x == (*pic).rows)
		x2 = x2 - 1;
	if (c2.y == (*pic).colls)
		y2 = y2 - 1;
	temp_pic = (RGB **)malloc((c2.x - c1.x) * sizeof(RGB *));
	if (!temp_pic) {
		fprintf(stderr, "Can`t allocate memory\n");
		free_memory(pic);
		exit(-1);
	}
	for (int i = 0; i < c2.x - c1.x; i++) {
		temp_pic[i] = malloc((c2.y - c1.y) * sizeof(RGB));
		if (!temp_pic[i]) {
			for (int j = 0; j < i; j++)
				free(temp_pic[j]);
			free(temp_pic);
			free_memory(pic);
			fprintf(stderr, "Can`t allocate memory\n");
			exit(-1);
		}
	}
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++)
			temp_pic[i - x1][j - y1] = effect(kernel, 1, pic, i, j);
	}
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++)
			(*pic).pxl_clr[i][j] = temp_pic[i - x1][j - y1];
	}
	for (int i = 0; i < c2.x - c1.x; i++)
		free(temp_pic[i]);
	free(temp_pic);
}

void apply_blur(PIC *pic, CRN c1, CRN c2)
{
	RGB **temp_pic;
	int kernel[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
	int x1 = c1.x, x2 = c2.x, y1 = c1.y, y2 = c2.y;
	if (c1.x == 0)
		x1 = 1;
	if (c1.y == 0)
		y1 = 1;
	if (c2.x == (*pic).rows)
		x2 = x2 - 1;
	if (c2.y == (*pic).colls)
		y2 = y2 - 1;
	temp_pic = (RGB **)malloc((c2.x - c1.x) * sizeof(RGB *));
	if (!temp_pic) {
		fprintf(stderr, "Can`t allocate memory\n");
		free_memory(pic);
		exit(-1);
	}
	for (int i = 0; i < c2.x - c1.x; i++) {
		temp_pic[i] = malloc((c2.y - c1.y) * sizeof(RGB));
		if (!temp_pic[i]) {
			for (int j = 0; j < i; j++)
				free(temp_pic[j]);
			free(temp_pic);
			free_memory(pic);
			fprintf(stderr, "Can`t allocate memory\n");
			exit(-1);
		}
	}
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++)
			temp_pic[i - x1][j - y1] = effect(kernel, 9, pic, i, j);
	}
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++)
			(*pic).pxl_clr[i][j] = temp_pic[i - x1][j - y1];
	}
	for (int i = 0; i < c2.x - c1.x; i++)
		free(temp_pic[i]);
	free(temp_pic);
}

void apply_gaussian_blur(PIC *pic, CRN c1, CRN c2)
{
	RGB **temp_pic;
	int kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
	int x1 = c1.x, x2 = c2.x, y1 = c1.y, y2 = c2.y;
	if (c1.x == 0)
		x1 = 1;
	if (c1.y == 0)
		y1 = 1;
	if (c2.x == (*pic).rows)
		x2 = x2 - 1;
	if (c2.y == (*pic).colls)
		y2 = y2 - 1;
	temp_pic = (RGB **)malloc((c2.x - c1.x) * sizeof(RGB *));
	if (!temp_pic) {
		fprintf(stderr, "Can`t allocate memory\n");
		free_memory(pic);
		exit(-1);
	}
	for (int i = 0; i < c2.x - c1.x; i++) {
		temp_pic[i] = malloc((c2.y - c1.y) * sizeof(RGB));
		if (!temp_pic[i]) {
			for (int j = 0; j < i; j++)
				free(temp_pic[j]);
			free(temp_pic);
			free_memory(pic);
			fprintf(stderr, "Can`t allocate memory\n");
			exit(-1);
		}
	}
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++)
			temp_pic[i - x1][j - y1] = effect(kernel, 16, pic, i, j);
	}
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++)
			(*pic).pxl_clr[i][j] = temp_pic[i - x1][j - y1];
	}
	for (int i = 0; i < c2.x - c1.x; i++)
		free(temp_pic[i]);
	free(temp_pic);
}

///aceasta este functia principala de apply in care verific
///daca am o mattrice incarcata in memorie sau daca poza pe care
///aplic efectul este color
void apply(PIC *pic, CRN c1, CRN c2, int ok)
{
	char c, effect[13];
	scanf("%c", &c);
	if (ok == 0) {
		printf("No image loaded\n");
		while (c != '\n')
			scanf("%c", &c);
	} else {
		if (c == '\n') {
			printf("Invalid command\n");
		} else {
			scanf("%s", effect);
			if (what_image_type((*pic).magic_word)) {
				printf("Easy, Charlie Chaplin\n");
			} else {
				///aici iau fiecare caz de efect pentru a
				///apela functa adecvata
				switch (choose_effect(effect)) {
				case 1:
						apply_edge(pic, c1, c2);
						printf("APPLY %s done\n", effect);
					break;
				case 2:
						apply_sharpen(pic, c1, c2);
						printf("APPLY %s done\n", effect);
					break;
				case 3:
						apply_blur(pic, c1, c2);
						printf("APPLY %s done\n", effect);
					break;
				case 4:
						apply_gaussian_blur(pic, c1, c2);
						printf("APPLY %s done\n", effect);
					break;
				default:
					printf("APPLY parameter invalid\n");
					break;
				}
			}
		}
	}
}

///aceasta este functia in care dau save
void save_pic(PIC pic, int is_file)
{
	///mai intai citesc numele sub forma caruia vreau sa salvez poza
	char *file_name, c;
	if (is_file == 0) {
		printf("No image loaded\n");
		scanf("%c", &c);
		while (c != '\n')
			scanf("%c", &c);
	} else {
		scanf("%c", &c);
		if (c == '\n') {
			printf("Invalid command\n");
			return;
		}
		get_file_name(&file_name);
		if (strchr(file_name, ' ')) {
			int q = strchr(file_name, ' ') - file_name;
			file_name[q] = '\0';
		}
		///in acest if verific daca salvez poza sub forma text sau binar
		if (strstr(file_name, "ascii")) {
			int p = strstr(file_name, "ascii") - file_name;
			file_name[p - 1] = '\0';
			FILE *f = fopen(file_name, "w");
			//daca o salvez text, afisez magic word adecvat in functie de poza
			if (what_image_type(pic.magic_word))
				fprintf(f, "P2\n");
			else
				fprintf(f, "P3\n");
			//apoi afisez celelalte caracteristici ale pozei
			fprintf(f, "%d %d\n", pic.colls, pic.rows);
			fprintf(f, "%d\n", pic.max_val);
			///dupa, in functie de ce poza salvez, afisez matricea de pixeli
			if (what_image_type(pic.magic_word)) {
				for (int i = 0; i < pic.rows; i++) {
					for (int j = 0; j < pic.colls; j++)
						fprintf(f, "%d ", pic.pxl_gr[i][j]);
					fprintf(f, "\n");
				}
			} else {
				for (int i = 0; i < pic.rows; i++) {
					for (int j = 0; j < pic.colls; j++) {
						fprintf(f, "%d ", pic.pxl_clr[i][j].R);
						fprintf(f, "%d ", pic.pxl_clr[i][j].G);
						fprintf(f, "%d ", pic.pxl_clr[i][j].B);
					}
					fprintf(f, "\n");
				}
			}
			printf("Saved %s\n", file_name);
			fclose(f); free(file_name);
			///apoi inchid fisierul si eliberez numele fisierului
		} else {
			///cazul unei poze binare merge la fel,
			///doar ca matricea de pixeli o afisez eficient sub forma binara
			FILE *f = fopen(file_name, "wb");
			if (what_image_type(pic.magic_word))
				fprintf(f, "P5\n");
			else
				fprintf(f, "P6\n");
			fprintf(f, "%d %d\n", pic.colls, pic.rows);
			fprintf(f, "%d\n", pic.max_val);
			if (what_image_type(pic.magic_word)) {
				for (int i = 0; i < pic.rows; i++) {
					for (int j = 0; j < pic.colls; j++)
						fwrite(&pic.pxl_gr[i][j], 1, 1, f);
				}
			} else {
				for (int i = 0; i < pic.rows; i++) {
					for (int j = 0; j < pic.colls; j++) {
						fwrite(&pic.pxl_clr[i][j].R, 1, 1, f);
						fwrite(&pic.pxl_clr[i][j].G, 1, 1, f);
						fwrite(&pic.pxl_clr[i][j].B, 1, 1, f);
					}
				}
			}
			printf("Saved %s\n", file_name);
			fclose(f); free(file_name);
		}
	}
}

int main(void)
{
	char command[10];
	int is_file = 0, was_file = 0;
	PIC picture;
	CRN lft_crn, rgt_crn;
	scanf("%s", command);
	///citesc comanda pe care o aplic si in functie de choose query,
	///iau cazul bun
	while (strcmp(command, "EXIT") != 0 || was_file == 0) {
		switch (choose_querry(command)) {
		case 0:
				load_pic(&picture, &is_file, &lft_crn, &rgt_crn, &was_file);
			break;
		case 1:
				sel_coor(&lft_crn, &rgt_crn, is_file, picture);
			break;
		case 2:
			break;
		case 3:
				histogram(picture, is_file);
			break;
		case 4:
				equalize(&picture, is_file);
			break;
		case 5:
				crop(&picture, &lft_crn, &rgt_crn, is_file);
			break;
		case 6:
				apply(&picture, lft_crn, rgt_crn, is_file);
			break;
		case 7:
				save_pic(picture, is_file);
			break;
		case 8:
				rotate(&picture, is_file, lft_crn, &rgt_crn);
			break;
		case 9:
				printf("No image loaded\n");
			break;
		case -1:
			printf("Invalid command\n");
			char c;
			scanf("%c", &c);
			while (c != '\n')
				scanf("%c", &c);
			break;
		default:
			break;
		}
		scanf("%s", command);
	}
	if (is_file == 0)
		printf("No image loaded\n");
	///la final, daca am ce elibera, dau free la ultima zona de memorie alocata.
	if (was_file == 1 && is_file == 1)
		free_memory(&picture);
	return 0;
}
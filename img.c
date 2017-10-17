#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define P 255

#pragma pack(1)

struct bmp_fileheader{
	unsigned char  fileMarker1; 
	unsigned char  fileMarker2; 
	unsigned int   bfSize; 
	unsigned short unused1; 
	unsigned short unused2; 
	unsigned int   imageDataOffset;
};

struct bmp_infoheader{
	unsigned int   biSize; 
	signed int     width;
	signed int     height;
	unsigned short planes;
	unsigned short bitPix; 
	unsigned int   biCompression; 
	unsigned int   biSizeImage;
	int            biXPelsPerMeter;
	int            biYPelsPerMeter;
	unsigned int   biClrUsed;
	unsigned int   biClrImportant;
};

#pragma pack(0)

typedef struct pix{
	unsigned char r, g ,b;
}pixel;

pixel **alocareMatrice(int h, int w){
	int i, j;
	pixel **mat;
	mat = malloc(h * sizeof(pixel*));
	if(!mat){
		printf("eroare in alocareMatrice\n");
		exit(1);
	}
	for( i = 0 ; i < h ; i ++){
		mat[i] = calloc(w, sizeof(pixel));
		if(!mat[i]){
			for( j = 0 ; j < i ; j ++){
				free(mat[j]);
			}
			printf("eroare in alocareMatrice\n");
			exit(1);
		}
	}
	return mat;
}

void dezalocareMatrice(pixel **mat, int h){
	int i;
	if(mat){
		for(i = 0 ; i < h ; i ++){
			if(mat[i]){
				free(mat[i]);
			}
		}
		free(mat);
	}
}

void printareMatreice(pixel **mat, int h, int w){
	int i, j;
	for(i = 0 ; i < h ; i ++){
		for( j = 0 ; j < w ; j ++){
			printf("%d %d %d  ", mat[i][j].r, mat[i][j].g, mat[i][j].b);
		}
		printf("\n");
	}
}

void creareImagine(pixel **mat, int h, int w){
	struct bmp_fileheader fh;
	struct bmp_infoheader ih;
	FILE *template, *imagine;
	template = fopen("template.dat", "rb");
	if(!template){
		printf("eroare in creareImagine\n");
		exit(1);
	}
	fread(&fh, sizeof(struct bmp_fileheader), 1, template);
	fread(&ih, sizeof(struct bmp_infoheader), 1, template);
	fclose(template);
	ih.width = w;
	ih.height = h;

	imagine = fopen("imagine.bmp", "wb");
	if(!imagine){
		printf("eroare in creareImagine\n");
		exit(1);
	}
	fwrite(&fh, sizeof(struct bmp_fileheader), 1, imagine);
	fwrite(&ih, sizeof(struct bmp_infoheader), 1, imagine);
	fseek(imagine, fh.imageDataOffset , SEEK_SET);
	int i, j, wp;
	unsigned char o = 0;
	if( w % 4 != 0)
		wp = ( w / 4 + 1 ) * 4;
	else
		wp = w;
	for( i = h - 1  ; i >= 0 ; i--){
		for( j = 0 ; j < wp ; j++){
			if( j < w){
				fwrite(&mat[i][j].b, sizeof(unsigned char), 1, imagine);
				fwrite(&mat[i][j].g, sizeof(unsigned char), 1, imagine);
				fwrite(&mat[i][j].r, sizeof(unsigned char), 1, imagine);
			}
			else
				fwrite(&o, sizeof(unsigned char), 1, imagine);
		}
	}
	fclose(imagine);
}

//Returneaza minimul
int min(int a, int b){
	if(a <= b)
		return a;
	return b;
}
//Returneaza maximul
int max(int a, int b){
	if(a >= b)
		return a;
	return b;
}
//Returneaza distanta euclidiana
double distanta(double x1, double y1, double x2, double y2){
	return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) );
}

int cerc(double x, double y, double r){
	if( x * x + y * y <= r * r)
		return 1;
	else
		return 0;
}

int elipsa(double x, double y, double a, double b){
	if( pow(x, 2) / pow(a, 2) + pow(y, 2) / pow(b, 2) -1 <= 0)
		return 1;
	return 0;
}

void grafGeneral(pixel **mat, int h, int w, int x1, int y1, int x2 ,int y2, int cod, pixel c){
	int i, j, r, *que;
	double x, y, a, b;
	switch(cod){
		case 1: //cerc
			//x1 si y1 sunt coord centrului
			//x2 e raza
			r = x2;
			for( i = max(y1 - r, 0) ; i < min(y1 + r, h) ; i ++){
				for( j = max(x1 - r, 0) ; j < min(x1 + r, w) ; j ++){
					if(cerc( j - x1 , i - y1 , r) == 1){
						mat[i][j] = c;
					}
				}
			}
			break;
		case 2://linie
			if( x1 == x2 && y1 == y2){
				mat[y1][x1] = c;
				break;
			}
			i = x1;
			j = x2;
			x1 = min(i, j);
			x2 = max(i, j);
			i = y1;
			j = y2;
			y1 = min(i, j);
			y2 = max(i, j);
			for( i = y1 ; i < y2 ; i++){
				for( j = x1 ; j < x2 ; j++){
					if( (i - y1) * (x2 - x1) == (j - x1) * (y2 - y1) ){
						mat[i][j] = c;
					}
				}
			}
			break;
		case 3://elipsa
			// centrul elipsei
			x = (x1 + x2) / 2;
			y = (y1 + y2) / 2;
			//dimensiunile elipsei
			a = distanta(x, 0, x1, 0);
			b = distanta(y, 0, y1, 0);
			for( i = max(x - a, 0) ; i < min(x + a, h) ; i ++){
				for( j = max(y - b, 0) ; j < min(y + b, w) ; j ++){
					if( elipsa(x - i, y - j, a, b) == 1)
					{
						mat[i][j] = c;
					}
				}
			}
	}
}

//Roteste matricea in sens ceasornic cu 90 de grade
pixel **rotatie(pixel **mat, int *h, int *w){
	int i, j, aux;
	pixel **mataux = alocareMatrice(*w, *h);
	for( i = 0 ; i < *w ; i ++){
		for ( j = 0 ; j < *h ; j ++){
			mataux[i][j] = mat[*h - j - 1][i];
		}
	}
	dezalocareMatrice(mat, *h);
	aux = *h;
	*h = *w;
	*w = aux;
	return mataux;
}

//Face ca jumateatea din dreapta sa fie in oglinda celei din stanga
void oglinda(pixel **mat, int h, int w){
	int i, j;
	for( i = 0 ; i < h ; i ++){
		for( j = 0 ; j < w / 2 ; j ++){
			mat[i][w - 1 - j] = mat[i][j];
		}
	}
}

//Mareste imaginea dupa factorii dati
//facW si facH sunt factorii de scalare a latimii respectiv a inaltimii
pixel **scaleaza(pixel **mat, int *h, int *w){
	int i, j, k, facH, facW;
	//printf("facH & facW:");
	scanf("%d %d", &facH, &facW);
	pixel **mataux = alocareMatrice(*h * facH, *w);
	for( i = 0 ; i < *h ; i ++){
		for( j = 0 ; j < *w ; j ++){
			for( k = 0 ; k < facH ; k ++){
				mataux[i * facH + k][j] = mat[i][j];
			}
		}
	}
	dezalocareMatrice(mat, *h);
	*h = *h * facH;
	mat = alocareMatrice(*h, *w * facW);
	for( i = 0 ; i < *h ; i ++){
		for( j = 0 ; j < *w ; j ++){
			for( k = 0 ; k < facW ; k ++){
				mat[i][j * facW + k] = mataux[i][j];
			}
		}
	}
	dezalocareMatrice(mataux, *h);
	*w = *w * facW;
	return mat;
}

//Fill iterativ
void fill(pixel **mat, int h, int w){
	int i, j, k = 0, n = 0, **lista;
	pixel c, a;
	//printf("Coordonatele punctului si culoarea:");
	//scanf("%d %d %d %d %d", &i, &j, &a.r, &a.g, &a.b);
	lista = malloc( 2 * sizeof(int*));
	if(!lista){
		printf("eroare in fill\n");
		exit(1);
	}
	for( i = 1 ; i <= 2 ; i++){
		lista[i] = malloc( h * w * sizeof(int));
		if(!lista[i]) {
			printf("eroare in fill\n");
			exit(1);
		}
	}
	c = mat[i][j];
	mat[i][j] = a;
	lista[1][k] = i;
	lista[2][k] = j;
	do{
		if( i > 0 && mat[i - 1][j].r == c.r 
				&& mat[i - 1][j].g == c.g && mat[i - 1][j].b == c.b){
			i--;
			n++;
			mat[i][j] = a;
			lista[1][n] = i;
			lista[2][n] = j;
		}
		if( i < h - 1 && mat[i + 1][j].r == c.r 
				&& mat[i + 1][j].g == c.g && mat[i + 1][j].b == c.b){
			i++;
			n++;
			mat[i][j] = a;
			lista[1][n] = i;
			lista[2][n] = j;
		}
		if( j > 0 && mat[i][j - 1].r == c.r 
				&& mat[i][j - 1].g == c.g && mat[i][j - 1].b == c.b){
			j--;
			n++;
			mat[i][j] = a;
			lista[1][n] = i;
			lista[2][n] = j;
		}
		if( j < w - 1 && mat[i][j + 1].r == c.r 
				&& mat[i][j + 1].g == c.g && mat[i][j + 1].b == c.b){
			j++;
			n++;
			mat[i][j] = a;
			lista[1][n] = i;
			lista[2][n] = j;
		}
		k++;
		i = lista[1][k];
		j = lista[2][k];
	}while(k <= n);
}

/*
	Date de intrare: 
 	mat: matricea de pixeli
 	o: culoarea cu care o sa se coloreze
 	h, w: dimensiunile matricii
 	x, y: coordonatele de unde incepe sa se coloreze
 */
void gradient(pixel **mat, int h, int w){
	int x, y;
	// se citesc coordonatele punctului central
	scanf("%d %d", &x, &y);
	pixel o;
	// se citeste culoarea
	scanf("%d %d %d", &o.r, &o.g, &o.b);
	int i, j, grd, diam = distanta(0, 0, h, w);
	for(i = 0 ; i < h ; i ++){
		for(j = 0 ; j < w ; j ++){
			grd = distanta(x, y, i, j);
			mat[i][j].r += (o.r * grd) / diam;
			mat[i][j].g += (o.g * grd) / diam;
			mat[i][j].b += (o.b * grd) / diam;
		}
	}
}

void functii(pixel ***mat, int *h, int *w, int cod){
	switch(cod){
		case 3:
			*mat = rotatie(*mat, h, w);
		case 2:
			*mat = rotatie(*mat, h, w);
		case 1:
			*mat = rotatie(*mat, h, w);
			break;
		case 4:
			oglinda(*mat, *h, *w);
			break;
		case 5:
			*mat = scaleaza(*mat, h, w);
			break;
		case 6:
			fill(*mat, *h, *w);
			break;
		case 7:
			gradient(*mat,*h ,*w);
			break;
	}
}

int main(void){
	int h, w, cod = 0;
	scanf("%d %d", &h, &w);
	pixel **mat = alocareMatrice(h, w);
	
	do {
		scanf("%d", &cod);
		functii(&mat, &h, &w, cod);
	} while (cod != 0);

	creareImagine(mat, h, w);
	dezalocareMatrice(mat, h);
	return 0;
}
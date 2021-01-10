/*
 ============================================================================
 Name        : genetic.c
 Author      : Pep Puerto
 Version     : 0.1
 Copyright   : GPL
 Description : GAC - Contenidor d'algorismes genetics
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Tamany màxim en bytes de l'estructura utilitzable en genetic
#define _DADES 		256
#define _NPOB		500

#define _VIUU	0x0001
#define _MORT	0x0002
#define _MUTA	0x0006
#define _SOLU	0x0008
#define _IMPR	0x0016

#define _NFUNC	5
#define _FSOLU	0
#define _FAVAL	1
#define _FMUTA	2
#define _FIMPR	3
#define _FGADN	4


//Estructura general de genetic
typedef struct _node {
   char dades[_DADES];
   struct _node *seg;
   struct _node *ant;
} t_node;

typedef t_node *t_pnode;
typedef t_node *t_lista;

//Estructures pròpies del problema particular
//Cal definir l'entorn i l'ADN de cada element
typedef struct _ENTORN{
	int maxpid;

	double objectiu;
	double epsilo;
} t_ENTORN;

typedef struct _ADN{
	int pid;
	int ppid;
	int generacio;

	double num[9];
	char ope[8];
} t_ADN;

int _maxpid;
/*
 ============================================================================
 _funcions generals: Declaració
 ============================================================================
 */
/* Insereix un node al principi de la llista*/
void _inserir (t_lista *lista, char *dades, int size);
/* Elimina el node apuntat*/
void _eliminar (t_pnode *node);
/*Processador general de llista. */
int _processa_lista(t_lista *lista, char *entorn, int size_entorn, int size_adn, int (*function[])(char *adn, int size_adn, char *ent, int size_ent));
/* Genera població inicial*/
void _genera_poblacio (t_lista *lista, int n_pob, int size, int (*function[])(char *adn, int size_adn, char *ent, int size_ent));


/*
 ============================================================================
 _funcions particulars: Declaració
 ============================================================================
 */
//Processos propis del problema particular
int _favaluac (char *adn, int size_adn, char *ent, int size_ent);
int _fsolucio (char *adn, int size_adn, char *ent, int size_ent);
int _fimpresi (char *adn, int size_adn, char *ent, int size_ent);
int _fmutacio (char *adn, int size_adn, char *ent, int size_ent);
int _fgeneADN (char *adn, int size_adn, char *ent, int size_ent);

int _iniEntor (t_ENTORN *ent);

int main() {
int (*function[_NFUNC])(char *adn, int size_adn, char *ent, int size_ent);
int estat = 0;
t_lista lista = NULL;
t_ENTORN entorn;


_iniEntor(&entorn);
function[_FSOLU] = &_fsolucio;
function[_FAVAL] = &_favaluac;
function[_FMUTA] = &_fmutacio;
function[_FIMPR] = &_fimpresi;
function[_FGADN] = &_fgeneADN;

_maxpid = 0;
_genera_poblacio (&lista, _NPOB, sizeof(t_ADN), function);
while (!(_SOLU & estat )) estat = _processa_lista(&lista, (char*)&entorn, sizeof(t_ENTORN), sizeof(t_ADN), function);

return 0;
}


/*
 ============================================================================
 _funcions generals: implementació
 ============================================================================
 */
void _inserir(t_lista *lista, char *dades,int size) {
int i=0;
t_pnode nou;

	/* Crear un node nou */
	nou = (t_pnode)malloc(sizeof(t_node));

	while (i<size) nou->dades[i++]=*dades++;

	/* Ubiquem nou_node en la primera posició de la llista_població */
	nou->ant = NULL;
	nou->seg = *lista;
	*lista = nou;
}

void _eliminar(t_pnode *node){
t_pnode node_a_eliminar = *node;
	//Elimina el node apuntat per *node

	//Si es el primer element de la llista
	if (!(node_a_eliminar->ant)) *node = node_a_eliminar->seg;
	//Si no es el primer element
	if (node_a_eliminar->ant)
		node_a_eliminar->ant->seg = node_a_eliminar->seg;
	//Si no es l'ultim element
	if (node_a_eliminar->seg)
	   node_a_eliminar->seg->ant = node_a_eliminar->ant;
	free (node_a_eliminar);
}

void _genera_poblacio (t_lista *lista, int n_pob, int size, int (*function[])(char *adn, int size_adn, char *ent, int size_ent)){
char *adn;
int i;
	adn = malloc(size);
	srandom(time(NULL));
	for(i=0; i<n_pob;i++){
		(*function[_FGADN])(adn, size, NULL, 0);
		_inserir(lista, adn, sizeof(t_node));
	}
}


int _processa_lista(t_lista *lista, char *entorn, int size_entorn, int size_adn, int (*function[])(char *adn, int size_adn, char *ent, int size_ent)){
t_pnode *node = lista;
int estat;
char adn_mut[_DADES];

	while (*node){
//getchar();
		//Si es el cas s'imprimeix. De moment sense filtre
		(*function[_FIMPR])((*node)->dades, size_adn, entorn, size_entorn);
		//Analitza si el node es solucio
		if (_SOLU & (*function[_FSOLU])((*node)->dades, size_adn, entorn, size_entorn)){
			printf("\nENCONTRADA SOLUCIO\n");
			(*function[_FIMPR])((*node)->dades, size_adn, entorn, size_entorn);
			return(_SOLU);
		}
		//Avalua el node
		//Si queda viu replica i muta (segons problema pot replicar o mutar en un % guardat en ADN)
		if (_VIUU & (estat = (*function[_FAVAL])((*node)->dades, size_adn, entorn, size_entorn))){
			//Segons ADN %muta %replica. Ara un 100%. Parametrizar replicar 1 o més repliques.
			(*function[_FMUTA])((*node)->dades, size_adn, adn_mut, size_adn);
			_inserir(lista, adn_mut ,sizeof(t_node));
			node = &(*node)->seg;
		}
		//Si esta mort cal eliminar-lo
		if (_MORT & estat) _eliminar(node);
	}
	return(0);
}


/*
 ============================================================================
 Funcions particulars.
 ============================================================================
 */

int _iniEntor (t_ENTORN *ent){
	ent->objectiu = 90;
	ent->epsilo = 5;
	return(-1);
}

int _fsolucio(char *adn, int size_adn, char *ent, int size_ent){
	t_ADN *a;
	t_ENTORN *e;
	double num,res;
	char ope;
	int i;
	e = (t_ENTORN*)ent;
	a=(t_ADN*)adn;
	res=a->num[0];
	for (i=1;i<9;i++){
	    num = a->num[i];
		ope = a->ope[i-1];
		switch (ope){
				case '+': res = res + num;break;
				case '-': res = res - num;break;
				case '*': res = res * num;break;
				case '/': res = res / num;break;
				default: break;
		}
	}
	if (e->objectiu == res) return (_SOLU);
	return(0);
}

int _favaluac(char *adn, int size_adn, char *ent, int size_ent){
	t_ADN *a;
	t_ENTORN *e;
	double num,res;
	char ope;
	int i;
	e = (t_ENTORN*)ent;
	a=(t_ADN*)adn;
	res=a->num[0];
	for (i=1;i<9;i++){
	    num = a->num[i];
		ope = a->ope[i-1];
		switch (ope){
				case '+': res = res + num;break;
				case '-': res = res - num;break;
				case '*': res = res * num;break;
				case '/': res = res / num;break;
				default: break;
		}
	}
	if (abs(e->objectiu - res) > e->epsilo) return(_MORT);
	return(_VIUU);
}

int _fimpresi(char *adn, int size_adn, char *ent, int size_ent){
	t_ADN *a;
	t_ENTORN *e;
	double num,res;
	char ope;
	int i;
	e = (t_ENTORN*)ent;
	a = (t_ADN*)adn;
	printf("\n%3.2f ",a->num[0]);
	res=a->num[0];
	for (i=1;i<9;i++){
	        num = a->num[i];
			ope = a->ope[i-1];
			switch (ope){
				case '+': res = res + num;break;
				case '-': res = res - num;break;
				case '*': res = res * num;break;
				case '/': res = res / num;break;
				default: break;
			}
			printf("%c %3.2f ",ope,num);
	}
	printf("\tRES %5.2f  ",res);
printf("\tGEN %d",a->generacio);
printf("\tPID %d",a->pid);
printf("\tPPID %d",a->ppid);

//	printf("\tENTORN: %f  %f ", e->objectiu, e->epsilo );
	if (abs(e->objectiu - res) > e->epsilo) printf("X");
	return(-1);
}

int _fmutacio(char *adn, int size_adn, char *adn_mut, int size_adn_mut){
	//Versió 0.1
	t_ADN *_adn;
	t_ADN *_adn_mut;
	int i,inc;
	int signe1, signe2;
	char signe;

	//Copia estructura adn a adn_mutat
	i = 0;
	while ( i < size_adn ) adn_mut[i] = adn[i++];

	_adn = (t_ADN*)adn;
	_adn_mut = (t_ADN*)adn_mut;

_maxpid = _maxpid + 1;
_adn_mut->generacio = _adn->generacio + 1;
_adn_mut->ppid = _adn->pid;
_adn_mut->pid = _maxpid;


	//Canvia un numero (+/- 10)
	i = random() % 9;
	inc = random() % 20;
	inc = (10 - inc);
	_adn_mut->num[i] = 1 + (int)(_adn_mut->num[i] + inc) % 256;

	//Dos posicions de signe a l'atzar i les intercanvio
	signe1 = random() % 8;
	signe2 = random() % 8;
	signe = _adn_mut->ope[signe1];
	_adn_mut->ope[signe1] = _adn_mut->ope[signe2];
	_adn_mut->ope[signe2]=signe;
	return(-1);
}

int _fgeneADN(char *ADN, int size_adn, char *ent, int size_ent){
	int i;
	t_ADN *adn;

		adn = (t_ADN*)ADN;

_maxpid = _maxpid + 1;
adn->pid = _maxpid;
adn->ppid = -1;
adn->generacio = 0;

		//Els numeros a l'atzar
		adn->num[0] = 1 + random() % 256;
		adn->num[1] = 1 + random() % 256;
		adn->num[2] = 1 + random() % 256;
		adn->num[3] = 1 + random() % 256;
		adn->num[4] = 1 + random() % 256;
		adn->num[5] = 1 + random() % 256;
		adn->num[6] = 1 + random() % 256;
		adn->num[7] = 1 + random() % 256;
		adn->num[8] = 1 + random() % 256;

		for(i=0;i<8;i++)adn->ope[i] = '0';
		//Els operadors a l'atzar
		adn->ope[random()%8]='+';
		while (adn->ope[i=random()%8] != '0');
		adn->ope[i]='+';
		while (adn->ope[i=random()%8] != '0');

		adn->ope[i]='-';
		while (adn->ope[i=random()%8] != '0');
		adn->ope[i]='-';
		while (adn->ope[i=random()%8] != '0');
		adn->ope[i]='*';
		while (adn->ope[i=random()%8] != '0');
		adn->ope[i]='*';
		while (adn->ope[i=random()%8] != '0');
		adn->ope[i]='/';
		while (adn->ope[i=random()%8] != '0');
		adn->ope[i]='/';
		return(-1);
}


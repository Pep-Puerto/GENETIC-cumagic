/*
 ============================================================================
 Name        : genetic-cumagic.c
 Author      : Pep Puerto
 Version     : 0.1
 Copyright   : GPL
 Description : GAC - Contenidor d'algorismes genetics
 Problem 	 : quadrats 3x3 (nxn))
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

//Tamany màxim en bytes de l'estructura utilitzable en genetic
#define _DADES 	128
#define _NPOB		10
#define _NPOB_MAX	3000000
//Frequència de mutacio reproducció sobre 100
#define _FRQMR	50

#define _VIUU	0x0001
#define _MORT	0x0002
#define _MUTA	0x0006
#define _SOLU	0x0008
#define _IMPR	0x0016

#define _NFUNC	6
#define _FSOLU	0
#define _FAVAL	1
#define _FMUTA	2
#define _FIMPR	3
#define _FGADN	4
#define _FREPR 5

//Define propis del problema
#define NUM_TEST 500

//Estructura general de genetic
typedef struct _historic{
	long n_vius;
	long n_morts;
} t_historic;
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
	long maxpid;
	
	long set_num_test[NUM_TEST];
	int num_quadrats;
	double mut_rep;
} t_ENTORN;

typedef struct _ADN{
	long pid;
	long ppid;
	long generacio;
	
	int num_quadrats;
	
	//base[0] es a1, base[1] es a2, base[2] es b2 
	long base[3];
	long a3; //a3 = 3b2-a1-a2
	long b1; //b1 = 3b2-a1-c1
	long b3; //b3 = 3b2-b2-b1
	long c1; //c1 = 3b2-b2-a3
	long c2; //c2 = 3b2-b2-a2
	long c3; //c3 = 3b2-b2-a1
	
} t_ADN;

long _maxpid;
t_historic historic;
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
int _processa_lista(t_lista *lista, char *entorn, int size_entorn, int size_adn, int (*function[])(char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *ent, int size_ent));
/* Genera població inicial*/
void _genera_poblacio (t_lista *lista, int n_pob, int size, char *ent, int size_ent, int (*function[])(char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *ent, int size_ent));


/*
 ============================================================================
 _funcions particulars: Declaració
 ============================================================================
 */
//Processos propis del problema particular
int _favaluac (char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *ent, int size_ent);
int _fsolucio (char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *ent, int size_ent);
int _fimpresi (char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *ent, int size_ent);
int _fmutacio (char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *ent, int size_ent);
int _frepAB_R (char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *adn_r, int size_adn_r);
int _fgeneADN (char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *ent, int size_ent);

int _iniEntor (t_ENTORN *ent);

/*
 ============================================================================
 _funcions auxiliars: Declaració
 ============================================================================
 */

int q(int n);

int main() {
int (*function[_NFUNC])(char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *ent, int size_ent);
int estat = 0;
t_lista lista = NULL;
t_ENTORN entorn;


_iniEntor(&entorn);
function[_FSOLU] = &_fsolucio;
function[_FAVAL] = &_favaluac;
function[_FMUTA] = &_fmutacio;
function[_FIMPR] = &_fimpresi;
function[_FGADN] = &_fgeneADN;
function[_FREPR] = &_frepAB_R;

_maxpid = 0;
_genera_poblacio (&lista, _NPOB, sizeof(t_ADN), (char*)&entorn, sizeof(t_ENTORN), function);
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

	/* Registrar un naixement */
	historic.n_vius++;
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
	/* Registrar una mort */
	historic.n_vius--;
	historic.n_morts++;
}

void _genera_poblacio (t_lista *lista, int n_pob, int size, char *ent, int size_ent, int (*function[])(char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *ent, int size_ent)){
char *adn;
int i;
	adn = malloc(size);
	srandom(time(NULL));
	for(i=0; i<n_pob;i++){
		(*function[_FGADN])(adn, size, NULL, 0, ent, size_ent);
		_inserir(lista, adn, sizeof(t_node));
	}
}

int _processa_lista(t_lista *lista, char *entorn, int size_entorn, int size_adn, int (*function[])(char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *ent, int size_ent)){
t_pnode *node = lista;
int estat;
int frqmr;
char adn_mut[_DADES];
char adn_rep[_DADES];
	
	frqmr = 0;
	while (*node){
//getchar();
		frqmr = frqmr++%100;
		//Si es el cas s'imprimeix. De moment sense filtre
		(*function[_FIMPR])((*node)->dades, size_adn, NULL, 0, entorn, size_entorn);
		//Analitza si el node es solucio
		if (_SOLU & (*function[_FSOLU])((*node)->dades, size_adn, NULL, 0, entorn, size_entorn)){
			printf("\nENCONTRADA SOLUCIO\n");
			(*function[_FIMPR])((*node)->dades, size_adn, NULL, 0, entorn, size_entorn);
			return(_SOLU);
		}
		//Avalua el node
		//Si queda viu replica i muta (segons problema pot replicar o mutar en un % guardat en ADN)
		if (_VIUU & (estat = (*function[_FAVAL])((*node)->dades, size_adn, NULL, 0, entorn, size_entorn))){
			//Segons ADN %muta %replica. Ara un 100%. Parametrizar replicar 1 o més repliques.
			if (frqmr <_FRQMR){
				(*function[_FMUTA])((*node)->dades, size_adn, adn_mut, size_adn, entorn, size_entorn);
				_inserir(lista, adn_mut ,sizeof(t_node));
			}
			else{
				if ((*node)->ant)
					(*function[_FREPR])((*node)->dades, size_adn, (*node)->ant->dades, size_adn, adn_rep, size_adn);
				_inserir(lista, adn_rep ,sizeof(t_node));
			}
			node = &(*node)->seg;
		}
		//Si esta mort cal eliminar-lo
		if (_MORT & estat) _eliminar(node);
//		if (*node == NULL) return(-1);
	}
	return(estat);
}


/*
 ============================================================================
 Funcions particulars.
 ============================================================================
 */

int _iniEntor (t_ENTORN *ent){
	int i,k;
	ent->num_quadrats = 0;
	//Genera una llista de quadrats de nombres imparells -podrien ser parells també?- de NUM_TEST
	k = 1;
	for (i=0; i<NUM_TEST; i++)	{ent->set_num_test[i] = k*k; k+=2;}
	srand ( time(NULL) );
	return(-1);
}

int _fsolucio(char *adn, int size_adn, char *adn_b, int size_adn_b, char *ent, int size_ent){
	t_ADN *a;
	t_ENTORN *e;
	int i,j;
	int suma;
	e = (t_ENTORN*)ent;
	a=(t_ADN*)adn;

//printf("fsolucio\n");

	if  ( ( a->num_quadrats == 6 ) &&
		   ( q(a->a3) && q(a->b1) && q(a->b3) && q(a->c1) && q(a->c2) && q(a->c3) ) ) 
	return(_SOLU);
	return(0);
}

int _favaluac(char *adn, int size_adn, char *adn_b, int size_adn_b, char *ent, int size_ent){
	t_ADN *a;
	t_ENTORN *e;
	int i,j, n, num_quadrats;
	e = (t_ENTORN*)ent;
	a=(t_ADN*)adn;

	//Si algun negatiu l'eliminem	
	if ((a->a3<0) || (a->b1<0) || (a->b3<0) || (a->c1<0) || (a->c2<0) || (a->a3<0) ) return(_MORT);
	//Si tenim algun repetit a la base l'eliminem. Això no hauria de passar ...
	if ( (a->base[0]==a->base[1]) || (a->base[0]==a->base[2]) || (a->base[1]==a->base[2]) ) return(_MORT);
	//Si Repetits amb algun de la base eliminar
	for (i=0; i<3; i++)
			if ( ( a->base[i]==a->a3 ) || ( a->base[i]==a->b1 ) || ( a->base[i]==a->b3 ) ||
		     	  ( a->base[i]==a->c1 ) || ( a->base[i]==a->c2 ) || ( a->base[i]==a->c3 ) )  return(_MORT);
	//La favaluació busca maximizar ( fins a 6 ) el nombre de valors derivats quadrats perfectes
	num_quadrats = 0;
	if ( q(a->a3) ) num_quadrats++;
	if ( q(a->b1) ) num_quadrats++;
	if ( q(a->b3) ) num_quadrats++;
	if ( q(a->c1) ) num_quadrats++;
	if ( q(a->c2) ) num_quadrats++;
	if ( q(a->c3) ) num_quadrats++;
	a->num_quadrats = num_quadrats;
printf("\nVius %d Morts %d \n", historic.n_vius, historic.n_morts);
//Control població
//	if ( historic.n_vius > _NPOB_MAX ) exit(1);
//Control de número de sumes iguals -> VIU o MORT
//	if (historic.n_vius > 100000)  n = 1; else n = 2;
	switch (e->num_quadrats){
		case 0: n = 0; break;
		case 1: n = 0; break;
		case 2: n = 0; break;
		case 3: n = 1; break;
		case 4: n = 2; break;
		case 5: n = 3; break;
		default: break;
	}
	if ( a->num_quadrats > e->num_quadrats ) e->num_quadrats = a->num_quadrats - n;
	if (e->num_quadrats < 0) e->num_quadrats = 0;
printf("favaluacio2 num_quadrats:%d e->num_quadrats:%d \n",num_quadrats, e->num_quadrats);
	if ( a->num_quadrats < e->num_quadrats ) return(_MORT);
	return(_VIUU);
}

int _fimpresi(char *adn, int size_adn, char *adn_b, int size_adn_b, char *ent, int size_ent){
	t_ADN *a;
	t_ENTORN *e;
	int i,j,x1,y1,x2,y2,temp;
	e = (t_ENTORN*)ent;
	a = (t_ADN*)adn;
	
//system("clear");
	printf("\nGeneracio %d PID %d PPID %d\n", a->generacio, a->pid, a->ppid);

if ( a->num_quadrats == e->num_quadrats ){
   printf("\t%d\t",a->base[0]);
	printf("\t%d\t",a->base[1]);
	printf("\t%d\t",a->a3);
	printf("\n");
	printf("\t%d\t",a->b1);
	printf("\t%d\t",a->base[2]);
	printf("\t%d\t",a->b3);
	printf("\n");
	printf("\t%d\t",a->c1);
	printf("\t%d\t",a->c2);
	printf("\t%d\t",a->c3);
	printf("\n");
	printf("\t%d\t",a->num_quadrats);
	printf("\n");
}
	return(-1);
}

int _fmutacio(char *adn, int size_adn, char *adn_mut, int size_adn_mut, char *ent, int size_ent){
	t_ADN *_adn;
	t_ADN *_adn_mut;
	t_ENTORN *e;

	int i,k,a1,a2,b2;

	//Copia estructura adn a adn_mutat
	i = 0;
	while ( i < size_adn ) adn_mut[i] = adn[i++];
	_adn = (t_ADN*)adn;
	_adn_mut = (t_ADN*)adn_mut;
	e = (t_ENTORN*)ent;
	_maxpid = _maxpid + 1;
	_adn_mut->generacio = _adn->generacio + 1;
	_adn_mut->ppid = _adn->pid;
	_adn_mut->pid = _maxpid;

	//Canvio dos ... Però he d'agafar un diferent als que ja tinc de la base!
	k = random()%3;
	i = random()%NUM_TEST;
	_adn_mut->base[k] = e->set_num_test[i];
	k = random()%3;
	i = random()%NUM_TEST;
	_adn_mut->base[k] = e->set_num_test[i];
	/*
	while ( (_adn_mut->base[k] == _adn_mut->base[(k+1)%3]) || 
			  (_adn_mut->base[k] == _adn_mut->base[(k+2)%3]) ) 
			_adn_mut->base[k] = e->set_num_test[random() % NUM_TEST];
	*/
//printf("fmutacio: k:%d base[0]:%d base[1]:%d base[2]:%d", k, _adn_mut->base[0], _adn_mut->base[1],_adn_mut->base[2] ); getchar();

	//Calculo la resta a3, b1, b3, c1, c2, c3
	a1 = _adn_mut->base[0]; 
	a2 = _adn_mut->base[1]; 
	b2 = _adn_mut->base[2];
	_adn_mut->c3 = 3*b2-b2-a1;
	_adn_mut->a3 = 3*b2-a1-a2;
	_adn_mut->c2 = 3*b2-b2-a2;
	_adn_mut->c1 = 3*b2-b2-_adn_mut->a3;
	_adn_mut->b1 = 3*b2-a1-_adn_mut->c1;
	_adn_mut->b3 = 3*b2-b2-_adn_mut->b1;
	
	_adn_mut->num_quadrats=0;
	return(-1);
}

int _frepAB_R (char *adn_a, int size_adn_a, char *adn_b, int size_adn_b, char *adn_r, int size_adn_r){
	long tmp, a1,a2,b2;	
	
	t_ADN *_adn_a;
	t_ADN *_adn_b;
	t_ADN *_adn_r;

	_adn_a = (t_ADN*)adn_a;
	_adn_b = (t_ADN*)adn_b;
	_adn_r = (t_ADN*)adn_r;
	
	//Barrejo a i b i creo r. Una forma de fer-ho, podem provar d'altres
	_adn_r->base[0] = _adn_a->base[0];
	_adn_r->base[1] = _adn_a->base[2];
	_adn_r->base[2] = _adn_b->base[1];

	a1 = _adn_r->base[0]; 
	a2 = _adn_r->base[1]; 
	b2 = _adn_r->base[2];	
	_adn_r->c3 = 3*b2-b2-a1;
	_adn_r->a3 = 3*b2-a1-a2;
	_adn_r->c2 = 3*b2-b2-a2;
	_adn_r->c1 = 3*b2-b2-_adn_r->a3;
	_adn_r->b1 = 3*b2-a1-_adn_r->c1;
	_adn_r->b3 = 3*b2-b2-_adn_r->b1;

}

int _fgeneADN(char *ADN, int size_adn, char *adn_b, int size_adn_b, char *ent, int size_ent){
	long i, a1,a2,b2;
	t_ADN *adn;
	t_ENTORN *e;
	
	adn = (t_ADN*)ADN;
	e = (t_ENTORN*)ent;

	_maxpid = _maxpid + 1;
	adn->pid = _maxpid;
	adn->ppid = -1;
	adn->generacio = 0;

	//per a cada element de base agafo un a l'atzar diferent del conjunt set_num_test
	//base[0] es a1, base[1] es a2, base[2] es b2 
	for (i=0; i<3; i++) {
		adn->base[i] = e->set_num_test[random() % NUM_TEST]; 
		while ( (adn->base[i]==adn->base[(i+1)%3]) || (adn->base[i]==adn->base[(i+2)%3]) ) adn->base[i] = e->set_num_test[random() % NUM_TEST];
	}
	//Calculo la resta a3, b1, b3, c1, c2, c3
	a1 = adn->base[0]; 
	a2 = adn->base[1]; 
	b2 = adn->base[2];
	adn->c3 = 3*b2-b2-a1;
	adn->a3 = 3*b2-a1-a2;
	adn->c2 = 3*b2-b2-a2;
	adn->c1 = 3*b2-b2-adn->a3;
	adn->b1 = 3*b2-a1-adn->c1;
	adn->b3 = 3*b2-b2-adn->b1;
	
	adn->num_quadrats=0;
	return(-1);
}

//Retorna TRUE si n te arral quadrada entera
int q(int n){ 
	double n_sqr;
	n_sqr = pow(n,0.5);
   return (trunc(n_sqr)==n_sqr);
}

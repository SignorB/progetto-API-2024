#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM_CHAR 256
#define DIM_FRASE 120
#define MAX_INGREDIENTI 29
#define TABELLLA_INGREDIENTI_SIZE 1000


#define CMD_AGGIUNGI_RICETTA "aggiungi_ricetta"
#define CMD_RIMUOVI_RICETTA "rimuovi_ricetta"
#define CMD_RIFORNIMENTO "rifornimento"
#define CMD_ORDINE "ordine"

unsigned long hash(unsigned char *str) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % TABELLLA_INGREDIENTI_SIZE;
}
typedef struct coda_s {
    int ora;
    char nome[DIM_CHAR];
    int quantita;
    struct coda_s *next;
} ilist_t;

typedef struct ingrediente_s {
    char nome[DIM_CHAR];
    int quantita;
    struct ingrediente_s *next;
} ingrediente;

typedef struct {
    char nome[DIM_CHAR];
    ingrediente *ingredienti;
} ricetta;

typedef struct {
    ricetta **el;
} tabella_ricette;

tabella_ricette *crea_tabella_ricette() {
    int a = TABELLLA_INGREDIENTI_SIZE;
    tabella_ricette* tab = malloc(sizeof(tabella_ricette));
    tab->el = calloc(a, sizeof(ricetta *)); // Allocate for pointers to ricetta
    return tab;
}

ingrediente * aggiungi_ingrediente(ingrediente * h, char * nome, int quantita) {
    ingrediente *new;
    if ((new = (ingrediente*) malloc(sizeof(ingrediente)))) {
        new->quantita = quantita;
        strcpy(new->nome, nome);
        new->next = h;
        h = new;
    } else {
        printf("push: errore allocazione memoria");
    }
    return h;
}

void inserisci_ricetta(tabella_ricette * tab, char * key, ingrediente * ingredienti) {
    int index = hash(key);
    ricetta* el = tab->el[index];
    int i = 1;
    while(el != NULL) {
        index = (index + i) % TABELLLA_INGREDIENTI_SIZE;
        el = tab->el[index+i];
        i++;
    }
    
    el = malloc(sizeof(ricetta));
    strcpy(el->nome, key);
    el->ingredienti = ingredienti;
    tab->el[index] = el;
}

ricetta * cerca_ricetta(tabella_ricette * tab, char * key) {
    int index = hash(key);
    ricetta * el = tab->el[index];
    int i = 1;
    while(el != NULL) {
        if(strcmp(el->nome, key)== 0) {
            return el;
        }
        index = (index + i) % TABELLLA_INGREDIENTI_SIZE;
        el = tab->el[index];
        i++;
    }
    return NULL;
}

void cancella_ricetta(tabella_ricette * tab, char * key) {
    int index = hash(key);
    ricetta * el = tab->el[index];
    ingrediente *head, *tmp;
    int i = 1;
    while(el != NULL) {
        if(strcmp(el->nome, key)== 0) {
            head = el->ingredienti;
            while (head != NULL) {
                tmp = head;
                head = head->next;
                free(tmp);
            }
            strcpy(el->nome, "|DELETED|");
            return;
        }
        index = (index + i) % TABELLLA_INGREDIENTI_SIZE;
        el = tab->el[index];
        i++;
    }
}


// metti in coda 
ilist_t * append(ilist_t * h, int ora, int quantita, char nome[]) {
	ilist_t * new, * el;
	if((new = (ilist_t *)malloc(sizeof(ilist_t)))) {
		new->ora = ora;
        new->quantita= quantita;
        strcpy(new->nome, nome);
		new->next = NULL;
		
		if(!h) 
			h = new;
		else {
			el = h;
			while(el->next) 
				el = el->next;
			el->next = new;
		} 
        return h;
	} else printf("append: errore allocazione memoria");
    return NULL;
}

// rimuovi un elemento dalla coda
ilist_t * pop(ilist_t * h, int ora) {
    ilist_t *curr = h;
    ilist_t *prev = NULL;

    while (curr != NULL) {
        if (curr->ora == ora) {
            if (prev == NULL) 
                h = curr->next;
            else 
                prev->next = curr->next;
            
            free(curr);
            return h; 
        }
        prev = curr;
        curr = curr->next;
    }
    printf("ERROR remove: ora %d non trovata\n", ora);
    return h;
}

// aggiungi un elemneto nella testa della coda
ilist_t * push(ilist_t * h, int ora, int quantita, char nome[]) {
    ilist_t *new;
    if ((new = (ilist_t*) malloc(sizeof(ilist_t)))) {
        new->ora = ora;
        new->quantita = quantita;
        strcpy(new->nome, nome);
        new->next = h;
        h = new;
    } else {
        printf("push: errore allocazione memoria");
    }
    return h;
}

// cerca un elemento in una lista
int find(ilist_t * h, char nome[]) {
    ilist_t *tmp;
    tmp = h;
    while(tmp != NULL) {
        if (strcmp(tmp->nome, nome) == 0) return 1;
        tmp = tmp->next;
    }
    return 0;
}

ilist_t * head_cucina = NULL;
ilist_t * head_corriere = NULL;
tabella_ricette * tab_ricette = NULL;

int t_corriere, c_corriere, time = 0;


int isCommand(char * cmd) {
    if (strcmp(cmd, CMD_AGGIUNGI_RICETTA) == 0) return 1;
    if (strcmp(cmd, CMD_RIMUOVI_RICETTA) == 0) return 2;
    if (strcmp(cmd, CMD_RIFORNIMENTO) == 0) return 3;
    if (strcmp(cmd, CMD_ORDINE) == 0) return 4;
    return 0;
}

void aggiungi_ricetta() {
    /* 
        Resistuisce 
            AGGIUNTA
            IGNORATO (se c'è già nella lista delle ricette)
    */    
    char v[DIM_CHAR];
    char nome[DIM_CHAR];
    int c;
    int quantita;
    ingrediente * h = NULL;
    if (scanf("%s", v) == 0) printf("ERROR");
    ricetta * el = cerca_ricetta(tab_ricette, v);

    if(scanf("%s %d", nome, &quantita) == 0) printf("ERROR");
    if (el == NULL) {
        h = aggiungi_ingrediente(NULL, nome, quantita);
    }
    while(1) {
        c = getchar();  
        if (c == '\n' || c == EOF) break;
        ungetc(c, stdin);
        if(scanf("%s %d", nome, &quantita) == 0) printf("ERROR");
        if (el == NULL) {
            h = aggiungi_ingrediente(NULL, nome, quantita);
        }
    }
    if (el== NULL) {
        inserisci_ricetta(tab_ricette, v, h);
        printf("aggiunta\n");
    }
    else 
        printf("ignorato %s\n", v);
}

void rimuovi_ricetta(char * nome_ricetta){
    /* 
        Resistuisce 
            RIMOSSA
            ORDINI IN SOSPESO
            NON PRESENTE
    */
    ricetta *el = cerca_ricetta(tab_ricette, nome_ricetta);
    if (el == NULL) {
        printf("non presente\n");
        return;
    }
    int trovato = find(head_cucina, nome_ricetta);
    if(trovato == 1) {
        printf("ordini in sospeso");
        return;
    }

    cancella_ricetta(tab_ricette, nome_ricetta);
    printf("rimossa\n");
    return;
}

void ordine(char * dolce, int quantita) {
    // prende gli ingredienti dal magazzino in ordine di scadenza
    // se non ci sono gli ingredienti viene messo in una coda di attesa
    /* 
        Resistuisce 
            ACCETTATO
            RIFIUTATO (se non c'è nella lista delle ricette)
    */
    // printf("ordine\n");
}

void rifornimento() {
    // aggiorna il magazzino
    // verifica se è possibile prepare qualcosa dalla lista di attesa alla preparazione
    /* 
        Resistuisce 
            RIFORNITO
    */
    // printf("rifornimento\n");
}

void corriere() {
    /* 
        Resistuisce 
            (istante_ordine) (nome ricetta) (numero elementi ordinati)
            CAMIONCINO VUOTO se non c'è nulla in coda
    */
    // printf("=> CORRIERE <=\n ");
}


int main(int argc, char *argv[]) {
    char v[DIM_CHAR];
    int util;
    int read = 1;
    int action;
    
    tab_ricette = crea_tabella_ricette();
    if(scanf("%d %d", &t_corriere, &c_corriere) == 0) {
        printf("ERROR: scanf - 1");
    };
    
    read = scanf("%s", v);
    while(read && read != EOF) {
        if (time % t_corriere == 0) corriere();
        action = isCommand(v);
        if (action == 1) {
            aggiungi_ricetta();
        }
        else if (action == 2) {
            if(scanf("%s", v) == 0) printf("ERROR: scanf - 3");
            rimuovi_ricetta(v);
        }
        else if (action == 3) {
            rifornimento();
        } else if (action == 4) {
            if (scanf("%s %d", v, &util) == 0) printf("ERROR: scanf - 2") ;
            ordine(v, util);
        }

        time++;
        read = scanf("%s", v);
    }

    return 0;
};

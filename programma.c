#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM_CHAR 256
#define DIM_FRASE 120
#define MAX_INGREDIENTI 29
#define TABELLLA_INGREDIENTI_SIZE 1000
#define TABELLLA_MAGAZZINO_SIZE 1000

#define CMD_AGGIUNGI_RICETTA "aggiungi_ricetta"
#define CMD_RIMUOVI_RICETTA "rimuovi_ricetta"
#define CMD_RIFORNIMENTO "rifornimento"
#define CMD_ORDINE "ordine"

int t_corriere, c_corriere, time = 0;

unsigned int hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % TABELLLA_INGREDIENTI_SIZE;
}

typedef struct coda_s {
    int ora;
    char nome[DIM_CHAR];
    int quantita;
    int peso;
    struct coda_s *next;
} ilist_t;

// Struttura per Ricettario
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

// Struttura dati magazzino
typedef struct lotto_s {
    int quantita;
    int scadenza;
    struct lotto_s *next;
    struct lotto_s *prec;
} lotto;
typedef struct scorta_s {
    char nome[DIM_CHAR];
    lotto *lotto;
} scorta;
typedef struct {
    scorta **el;
} inventario;

inventario *crea_inventario() {
    int a = TABELLLA_MAGAZZINO_SIZE;
    inventario* tab = malloc(sizeof(inventario));
    tab->el = calloc(a, sizeof(scorta*));
    return tab;
}

lotto * clean_lotto(lotto * h) {
    if (h == NULL) return h;
    
    while(h->next != NULL && h->scadenza <= time) { // TODO: CHECK TIME =
        h = h->next;
        free(h);
    }

    if (h->next == NULL && h->scadenza <= time) { // TODO: ^ here too
        free(h);
        return NULL;
    }
    return h;
}

scorta * inserisci_inventario(inventario * tab, char * key){
    int index = hash(key);
    scorta* el= tab->el[index];
    int i = 1;
    while(el != NULL) {
        index = (index + i) % TABELLLA_MAGAZZINO_SIZE;
        el = tab->el[index+i];
        i++;
    }
    el = malloc(sizeof(scorta));
    strcpy(el->nome, key);
    el->lotto = NULL;
    tab->el[index] = el;
    return el;
}

scorta * cerca_scorta(inventario * tab, char * key){
    int index= hash(key);
    scorta * el= tab->el[index];
    int i = 1;
    while(el != NULL) {
        if(strcmp(el->nome, key) == 0) {
            return el;
        }
        index = (index + i) % TABELLLA_INGREDIENTI_SIZE;
        el = tab->el[index];
        i++;
    }
    return NULL;
}

lotto * aggiungi_lotto(inventario * inv, char *key, int quantita, int scadenza) {
    scorta *el = cerca_scorta(inv, key);
    if (el == NULL) {
        el = inserisci_inventario(inv, key);
    }
    lotto *h = el->lotto;
    lotto *new, *curr;
    if ((new = (lotto *) malloc(sizeof(lotto))) != NULL)  {
        new->quantita = quantita;
        new->scadenza = scadenza;
        new->prec = NULL;
        new->next = NULL;
        if (h == NULL) {
            h = new;
        } else {
            curr = h;
            while(curr->next != NULL && curr->scadenza < scadenza) {
                curr = curr->next;
            }
            if (curr->scadenza == scadenza) {
                curr->quantita = curr->quantita + quantita;
                free(new);
            } else if (curr->scadenza > scadenza) {
                new->next = curr;
                new->prec = curr->prec;
                curr->prec->next = new;
                curr->prec = new;
            } else {
                curr->next = new;
                new->prec = curr;
            }
        }
    } else {
        printf("push: errore allocazione memoria");
    }
    return h;
}

tabella_ricette *crea_tabella_ricette() {
    int a = TABELLLA_INGREDIENTI_SIZE;
    tabella_ricette* tab = malloc(sizeof(tabella_ricette));
    tab->el = calloc(a, sizeof(ricetta *));
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
ilist_t * append(ilist_t * h, int ora, int quantita, char nome[], int peso) {
	ilist_t * new, * el;
	if((new = (ilist_t *)malloc(sizeof(ilist_t)))) {
		new->ora = ora;
        new->quantita= quantita;
        strcpy(new->nome, nome);
		new->next = NULL;
        new->peso = peso;
		
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
ilist_t * push(ilist_t * h, int ora, int quantita, char nome[], int peso) {
    ilist_t *new;
    if ((new = (ilist_t*) malloc(sizeof(ilist_t)))) {
        new->ora = ora;
        new->quantita = quantita;
        strcpy(new->nome, nome);
        new->next = h;
        new->peso = peso;
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
inventario * tab_inventario = NULL;




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
        printf("ignorato\n");
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

int verifica_magazzino(inventario * inv, tabella_ricette* tab, char * key, int num) {
    ingrediente * tmp;
    scorta * el;
    lotto * info;
    ricetta * ricetta = cerca_ricetta(tab, key);
    int count;
    
    if (ricetta == NULL) return 0; // Non c'è la ricetta

    tmp = ricetta->ingredienti;
    while(tmp != NULL) {
        el = cerca_scorta(inv, tmp->nome);
        if (el == NULL || el->lotto == NULL) return 1; // Non c'è l'ingrediente, metti in coda
        clean_lotto(el->lotto);
        count = 0;
        info = el->lotto;
        while(info != NULL && count < (tmp->quantita * num)) {
            count += info->quantita;
            info = info->next;
        }

        if (count < (tmp->quantita * num)) return 1;
        tmp = tmp->next;
    }

    return 2; // ci sono abbastanza ingredienti
}

void processa_ordine(inventario * inv, tabella_ricette* tab, char * key, int num) {
    
}

void ordine(char * dolce, int quantita) {
    // prende gli ingredienti dal magazzino in ordine di scadenza
    // se non ci sono gli ingredienti viene messo in una coda di attesa
    /* 
        Resistuisce 
            ACCETTATO
            RIFIUTATO (se non c'è nella lista delle ricette)
    */
    int result = verifica_magazzino(tab_inventario, tab_ricette, dolce, quantita);
    if (result == 0) printf("rifiutato\n");
    else if (result == 1) {

    } else if (result == 2) {

    } else {
        printf("ERROR - ordine");
    }
    
    printf("accettato\n");
}

void rifornimento() {
    /* 
        Resistuisce 
            RIFORNITO
    */
    char v[DIM_CHAR];
    int scadenza;
    int quantita;
    int c;

    while(1) {
        c = getchar();  
        if (c == '\n' || c == EOF) break;
        ungetc(c, stdin);
        if(scanf("%s %d %d", v, &quantita, &scadenza) == 0) printf("ERROR");
        aggiungi_lotto(tab_inventario, v, quantita, scadenza);
    }

    printf("rifornito\n");
    // TODO: verifica se è possibile prepare qualcosa dalla lista di attesa alla preparazione
}

void corriere(ilist_t * coda) {
    int count = 0;
    int tmp = 0;
    /* 
        Resistuisce 
            (istante_ordine) (nome ricetta) (numero elementi ordinati)
            CAMIONCINO VUOTO se non c'è nulla in coda
    */
    // TODO: rifare la restituzione di ordine di peso
    while(count < c_corriere && coda != NULL) {
        if ((count + coda->peso) < c_corriere){
            tmp = (coda->peso) * (coda->quantita);
            printf("%d %s %d\n", coda->ora, coda->nome, coda->quantita);
            count += tmp;
            coda = pop(coda, coda->ora);
        } else 
            break;
    }
    if (count == 0) printf("camioncino vuoto\n");
}


int main(int argc, char *argv[]) {
    char v[DIM_CHAR];
    int util;
    int read = 1;
    int action;
    
    tab_ricette = crea_tabella_ricette();
    tab_inventario = crea_inventario();
    
    if(scanf("%d %d", &t_corriere, &c_corriere) == 0) {
        printf("ERROR: scanf - 1");
    };
    
    read = scanf("%s", v);
    while(read && read != EOF) {
        if (time != 0 && time % t_corriere == 0) corriere(head_corriere);
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

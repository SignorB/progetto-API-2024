#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM_CHAR 256
#define HASH_TABLE 65536
#define MIN_HEAP 100
#define MAX_HEAP 100
#define GROW_FACTOR 1.5

#define CMD_AGGIUNGI_RICETTA "aggiungi_ricetta"
#define CMD_RIMUOVI_RICETTA "rimuovi_ricetta"
#define CMD_RIFORNIMENTO "rifornimento"
#define CMD_ORDINE "ordine"


typedef struct ingrediente_s {
    char nome[DIM_CHAR];
    int quantita;
    struct ingrediente_s *next;
} ingrediente;

typedef struct ricetta_s {
    char nome[DIM_CHAR];
    int peso;
    ingrediente *ingredienti;
    struct ricetta_s *prec;
    struct ricetta_s *next;
} ricetta;

typedef struct {
    int quantita;
    int scadenza;
} lotto;

typedef struct scorta_s {
    char nome[DIM_CHAR];
    int count;
    int disponibile;
    int dim;
    lotto *lotto;
    struct scorta_s *prec;
    struct scorta_s *next;
} scorta;

typedef struct coda_s {
    char nome[DIM_CHAR];
    int quantita;
    int peso;
    int ora;
    struct coda_s * next;
    struct coda_s * prec;
} coda;

typedef struct {
    int quantita;
    char nome[DIM_CHAR];
    int ora;
    int peso;
} preparato;

typedef struct {
    int count;
    int dim;
    preparato *preparato;
} coda_corriere;

int isCommand(char *);
int cancella_ricetta(char * );
int verifica_magazzino(ricetta * , int );
int verifica_scorta(char * , int );
int findin_list(coda *, char *);
unsigned int hash(const char *);
void clear_ingredienti(ingrediente *);
void inserisci_ricetta(char *, int , ingrediente * );
void scambia_lotti(lotto * , lotto * ); 
void scambia_corriere(preparato * , preparato * ); 
void inserisci_lotto(char * , int , int );
void consuma_scorta(char * nome, int totale);
void processa_ordine(ricetta* ricetta, int quantita_ordine);
void refresh();
void heapify_corriere(coda_corriere * , int );
void heapify_corriere_out(coda_corriere * , int );
void remove_min_corriere(coda_corriere * );
void remove_min_corriere_out(coda_corriere * );
void inserisci_corriere(char * , int , int , int );
void inserisci_corriere_out(char * , int , int , int );
void crea_coda_corriere();
void print_corr(coda_corriere *); 
ingrediente * push_ingrediente(ingrediente * , char * , int );
ricetta * cerca_ricetta(char *);
scorta * cerca_scorta(char *);
scorta * clean_scaduti(scorta *);
coda * push_coda(coda * , char *, int , int , int );


// VARIABILI GLOBALI
int t_corriere, c_corriere, time = 0;
ricetta * ricettario[HASH_TABLE];
scorta * magazzino[HASH_TABLE];
coda * coda_head = NULL;
coda * coda_bott = NULL;
coda_corriere * coda_corr;
coda_corriere * coda_out;

/* 
    Resistuisce 
        AGGIUNTA
        IGNORATO (se c'è già nella lista delle ricette)
*/ 
void aggiungi_ricetta() {   
    char v[DIM_CHAR];
    char nome[DIM_CHAR];
    char c;
    int quantita;
    int peso = 0;
    ingrediente * ing = NULL;
    
    
    if (scanf("%s", v) == 0) printf("ERROR");
    ricetta * ricetta = cerca_ricetta(v);

    while(1) {
        if(scanf("%s %d%c", nome, &quantita, &c) == 0) printf("ERROR");
        if (ricetta == NULL) {
            peso += quantita;
            ing = push_ingrediente(ing, nome, quantita);
        }
        if (c == '\n' || c == EOF) break;
    }
    if (ricetta == NULL) {
        inserisci_ricetta(v, peso, ing);
        printf("aggiunta\n");
    } else{
        printf("ignorato\n");
    }
}

/* 
    Resistuisce 
        RIMOSSA
        ORDINI IN SOSPESO
        NON PRESENTE
*/ 
void rimuovi_ricetta(char * nome_ricetta){
    int result = cancella_ricetta(nome_ricetta);
    if (result == -1) printf("non presente\n");
    else if(result == 0) printf("ordini in sospeso\n");
    else printf("rimossa\n");
}

/* 
    Resistuisce 
        ACCETTATO
        RIFIUTATO (se non c'è nella lista delle ricette)
*/
void ordine(char * nome, int quantita_ordine) {
    ricetta * ricetta = cerca_ricetta(nome);
    if (ricetta == NULL) {
        printf("rifiutato\n");
    } else {
        int result = verifica_magazzino(ricetta, quantita_ordine);

        if (result == 0) {
            coda_head = push_coda(coda_head, nome, quantita_ordine, (quantita_ordine * ricetta->peso), time);
        } else {
            processa_ordine(ricetta, quantita_ordine);
            inserisci_corriere(nome, quantita_ordine, time, (ricetta->peso * quantita_ordine));
        }
        printf("accettato\n");
    }
}

/* 
    Resistuisce 
        RIFORNITO
*/
void rifornimento() {
    char v[DIM_CHAR];
    char c;
    int scadenza;
    int quantita;

    while(1) {
        if(scanf("%s %i %i%c", v, &quantita, &scadenza, &c) == 0) printf("ERROR");
        inserisci_lotto(v, quantita, scadenza);
        if (c == '\n' || c == EOF) break;
    }
    printf("rifornito\n");
    refresh();
}

/* 
    Resistuisce 
        (istante_ordine) (nome ricetta) (numero elementi ordinati)
        CAMIONCINO VUOTO se non c'è nulla in coda
*/
void corriere() {
    int count = c_corriere;

    while(coda_corr->count >0 && coda_corr->preparato[0].peso < count) {
        inserisci_corriere_out(coda_corr->preparato[0].nome, coda_corr->preparato[0].quantita, coda_corr->preparato[0].ora, coda_corr->preparato[0].peso);
        count -= coda_corr->preparato[0].peso;
        remove_min_corriere(coda_corr);
    }

    if (count == c_corriere) printf("camioncino vuoto\n");
    else {
        while(coda_out->count > 0) {
            printf("%i %s %i\n", coda_out->preparato[0].ora,coda_out->preparato[0].nome, coda_out->preparato[0].quantita);
            remove_min_corriere_out(coda_out);
        }
    }
}

int main(int argc, char *argv[]) {
    char v[DIM_CHAR];
    int util;
    int read = 1;
    int action;

    crea_coda_corriere();
    
    if(scanf("%i %i", &t_corriere, &c_corriere) == 0) printf("ERROR: scanf - 1");
    
    read = scanf("%s", v);
    while(read && read != EOF) {
        action = isCommand(v);
        if (action == 1) {
            aggiungi_ricetta();
        }
        else if (action == 2) {
            if(scanf("%s", v) == 0) printf("ERROR: scanf - 2");
            rimuovi_ricetta(v);
        }
        else if (action == 3) {
            rifornimento();
        } else if (action == 4) {
            if (scanf("%s %i", v, &util) == 0) printf("ERROR: scanf - 3") ;
            ordine(v, util);
        }

        time++;
        if (time % t_corriere == 0 && time != 0) corriere();
        read = scanf("%s", v);
    }
    return 0;
};

int isCommand(char * cmd) {
    if (strcmp(cmd, CMD_AGGIUNGI_RICETTA) == 0) return 1;
    if (strcmp(cmd, CMD_RIMUOVI_RICETTA) == 0) return 2;
    if (strcmp(cmd, CMD_RIFORNIMENTO) == 0) return 3;
    if (strcmp(cmd, CMD_ORDINE) == 0) return 4;
    return 0;
}

unsigned int hash(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % HASH_TABLE;
}

ricetta * cerca_ricetta(char * key) {
    int index = hash(key);
    ricetta * tmp;

    tmp = ricettario[index];
    if (tmp == NULL) return NULL;

    while(tmp != NULL) {
        if (strcmp(key, tmp->nome) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}
scorta * cerca_scorta(char * key) {
    int index = hash(key);
    scorta * tmp;

    tmp = magazzino[index];
    if (tmp == NULL) return NULL;

    while(tmp != NULL) {
        if (strcmp(key, tmp->nome) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

int cancella_ricetta(char * key) {
    int index = hash(key);
    ricetta * tmp = ricettario[index];
    ricetta * prev = NULL;

    while (tmp != NULL && strcmp(tmp->nome, key) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }

    if (tmp == NULL) return -1;

    if (findin_list(coda_head, key)) return 0;

    int i= 0;
    while (i < coda_corr->count) {
        if (strcmp(coda_corr->preparato[i].nome, key) == 0 ) return 0;
        i++;
    }

    clear_ingredienti(tmp->ingredienti);

    if (prev == NULL) {
        ricettario[index] = tmp->next;
        if (tmp->next != NULL) {
            tmp->next->prec = NULL;
        }
    } else {
        prev->next = tmp->next;
        if (tmp->next != NULL) {
            tmp->next->prec = prev;
        }
    }

    free(tmp);
    return 1;
}

void inserisci_ricetta(char *key, int peso, ingrediente * ing) {
    int index = hash(key);
    ricetta * new = (ricetta *)malloc(sizeof(ricetta));
    
    strcpy(new->nome, key);
    new->peso = peso;
    new->ingredienti = ing;
    new->next = NULL;
    new->prec = NULL;
    
    if (ricettario[index] == NULL) {
        ricettario[index] = new;
    } else {
        ricetta *tmp;
        tmp = ricettario[index];
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = new;
        new->prec = tmp;
    }
}

scorta * inserisci_scorta(char *key) {
    int index = hash(key);
    scorta * new = (scorta *)malloc(sizeof(scorta));
    
    strcpy(new->nome, key);
    new->count = 0;
    new->dim = MIN_HEAP;
    new->lotto = (lotto *) malloc(sizeof(lotto) * MIN_HEAP);
    new->disponibile = 0;
    new->next = NULL;
    new->prec = NULL;
    
    if (magazzino[index] == NULL) {
        magazzino[index] = new;
    } else {
        scorta *tmp;
        tmp = magazzino[index];
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = new;
        new->prec = tmp;
    }
    return new;
}

void inserisci_lotto(char * key, int quantita, int scadenza) {
    scorta * scorta = cerca_scorta(key);
    
    if (scorta == NULL) scorta = inserisci_scorta(key);

    if(scorta->dim == scorta->count) {
        int dim = GROW_FACTOR * (scorta->dim);
        lotto * new = realloc(scorta->lotto, sizeof(lotto) * dim);
        scorta->lotto = new;
        scorta->dim = dim;
    }

    int i = scorta->count;
    scorta->lotto[i].quantita = quantita;
    scorta->lotto[i].scadenza = scadenza;

    while(i != 0 && scorta->lotto[(i-1)/2].scadenza > scorta->lotto[i].scadenza) {
        scambia_lotti(&scorta->lotto[i], &scorta->lotto[(i-1)/2]);
        i = (i-1)/2;
    }
    scorta->disponibile += quantita;
    scorta->count++;
}

void inserisci_corriere(char * key, int quantita, int ora, int peso) {
    if(coda_corr->dim == coda_corr->count) {
        int dim = GROW_FACTOR * (coda_corr->dim);
        preparato * new = realloc(coda_corr->preparato, sizeof(preparato) * dim);
        coda_corr->preparato = new;
        coda_corr->dim = dim;
    }

    int i = coda_corr->count;
    coda_corr->preparato[i].ora = ora;
    coda_corr->preparato[i].peso = peso;
    coda_corr->preparato[i].quantita = quantita;
    strcpy(coda_corr->preparato[i].nome, key);

    while(i != 0 && coda_corr->preparato[(i-1)/2].ora > coda_corr->preparato[i].ora) {
        scambia_corriere(&coda_corr->preparato[i], &coda_corr->preparato[(i-1)/2]);
        i = (i-1)/2;
    }

    coda_corr->count++;
}

void inserisci_corriere_out(char * key, int quantita, int ora, int peso) {
    if(coda_out->dim == coda_out->count) {
        int dim = GROW_FACTOR * (coda_out->dim);
        preparato * new = realloc(coda_out->preparato, sizeof(preparato) * dim);
        coda_out->preparato = new;
        coda_out->dim = dim;
    }

    int i = coda_out->count;
    coda_out->preparato[i].ora = ora;
    coda_out->preparato[i].peso = peso;
    coda_out->preparato[i].quantita = quantita;
    strcpy(coda_out->preparato[i].nome, key);

    while(i != 0 && (coda_out->preparato[(i-1)/2].peso < coda_out->preparato[i].peso || (coda_out->preparato[(i-1)/2].peso == coda_out->preparato[i].peso && coda_out->preparato[(i-1)/2].ora > coda_out->preparato[i].ora))) {
        scambia_corriere(&coda_out->preparato[i], &coda_out->preparato[(i-1)/2]);
        i = (i-1)/2;
    }

    coda_out->count++;
}

void heapify_lotto(scorta * s, int i) {
    int min = i;
    int sx = 2*i + 1;
    int dx = 2*i + 2;

    if (sx < s->count && s->lotto[sx].scadenza < s->lotto[min].scadenza) min = sx;

    if(dx < s->count && s->lotto[dx].scadenza < s->lotto[min].scadenza) min = dx;
    if (min != i) {
        scambia_lotti(&s->lotto[i], &s->lotto[min]);
        heapify_lotto(s, min);
    }
}

void heapify_corriere(coda_corriere * s, int i) {
    int min = i;
    int sx = 2*i + 1;
    int dx = 2*i + 2;
    
    if (sx < s->count && s->preparato[sx].ora < s->preparato[min].ora) min = sx;

    if(dx < s->count && s->preparato[dx].ora < s->preparato[min].ora) min = dx;
    if (min != i) {
        scambia_corriere(&s->preparato[i], &s->preparato[min]);
        heapify_corriere(s, min);
    }
}

void heapify_corriere_out(coda_corriere * s, int i) {
    int max = i;
    int sx = 2*i + 1;
    int dx = 2*i + 2;
    
    if (sx < s->count && (s->preparato[sx].peso > s->preparato[max].peso || (s->preparato[sx].peso == s->preparato[max].peso && s->preparato[sx].ora < s->preparato[max].ora))) max = sx;

    if(dx < s->count && (s->preparato[dx].peso > s->preparato[max].peso || (s->preparato[dx].peso == s->preparato[max].peso && s->preparato[dx].ora < s->preparato[max].ora))) max = dx;
    if (max != i) {
        scambia_corriere(&s->preparato[i], &s->preparato[max]);
        heapify_corriere_out(s, max);
    }
}

void remove_min_lotto(scorta * s) {
    if(s->count <= 0) {
        s->disponibile = 0;   
        return;
    }

    if(s->count == 1) {
        s->disponibile = 0;
        s->count--;
        return;
    }
    
    s->disponibile -= s->lotto[0].quantita;
    s->lotto[0] = s->lotto[s->count - 1];
    s->count--;
    heapify_lotto(s, 0);
}
void remove_min_corriere(coda_corriere * s) {
    if(s->count <= 0) return;

    if(s->count == 1) {
        s->count--;
        return;
    }
    
    s->preparato[0] = s->preparato[s->count - 1];
    s->count--;
    heapify_corriere(s, 0);
}

void remove_min_corriere_out(coda_corriere * s) {
    if(s->count <= 0) return;

    if(s->count == 1) {
        s->count--;
        return;
    }
    
    s->preparato[0] = s->preparato[s->count - 1];
    s->count--;
    heapify_corriere_out(s, 0);
}


void scambia_lotti(lotto * a, lotto * b) {
    lotto tmp = *a;
    *a = *b;
    *b = tmp;
}

void scambia_corriere(preparato * a, preparato * b) {
    preparato tmp = *a;
    *a = *b;
    *b = tmp;
}

ingrediente * push_ingrediente(ingrediente * h, char * nome, int quantita) {
    ingrediente * new = NULL;

    if ((new = (ingrediente *) malloc(sizeof(ingrediente))) != NULL) {
        strcpy(new->nome, nome);
        new->quantita = quantita;
        new->next = h;        
    }
    return new; 
}

coda * push_coda(coda * h, char *nome, int quantita, int peso, int t) {
    coda * new = NULL;

    if ((new = (coda *) malloc(sizeof(coda))) != NULL) {
        strcpy(new->nome, nome);
        new->quantita = quantita;
        new->peso = peso;
        new->next = h;
        new->ora = t;
        new->prec = NULL;

        if (h == NULL) coda_bott = new;
        if (h != NULL) h->prec = new;
    }
    return new; 
}

void clear_ingredienti(ingrediente *h) {
    ingrediente * tmp = NULL;

    while(h != NULL) {
        tmp = h;
        h = h->next;
        free(tmp);
    }
}

scorta * clean_scaduti(scorta *s) {
    if (s == NULL) printf("ERRORE CLEAN SCADUTI");
    while(s->count > 0 && s->lotto[0].scadenza <= time) {
        remove_min_lotto(s);
    }
    return s;
}

int verifica_scorta(char * nome, int totale) {
    scorta * scorta = cerca_scorta(nome);
    if (scorta == NULL) return 0;

    scorta = clean_scaduti(scorta);
    int count = scorta->disponibile;
    // int count = 0;
    // for(int i = 0; count < totale && i < scorta->count ; i++) {
    //     count += scorta->lotto[i].quantita;
    // }
    if (count >= totale) return 1;
    return 0;
}

void consuma_scorta(char * nome, int totale) {
    scorta * scorta = cerca_scorta(nome);
    if (scorta == NULL) printf("ERROR RICETTA CANCELLATA");

    int count = totale;
    while (count > 0) {
        if (scorta->lotto[0].quantita <= count) {
            count -= scorta->lotto[0].quantita;
            remove_min_lotto(scorta);
        } else {
            scorta->lotto[0].quantita -= count;
            scorta->disponibile -= count;
            count = 0;
        }
    }
    if (count != 0) printf("ERROR CONUSMA SCORTA");
}

int verifica_magazzino(ricetta * ricetta, int quantita_ordine) {
    ingrediente * tmp;
    int result = 1;

    tmp = ricetta->ingredienti;
    while(tmp != NULL && result) {
        result = verifica_scorta(tmp->nome, (quantita_ordine * tmp->quantita));
        tmp = tmp->next;
    }
    return result;
}

void processa_ordine(ricetta* ricetta, int quantita_ordine) {
    ingrediente * tmp;
    
    tmp = ricetta->ingredienti;
    while(tmp != NULL) {
        consuma_scorta(tmp->nome, (quantita_ordine * tmp->quantita));
        tmp = tmp->next;
    }   
}

void refresh() {
    coda * list = coda_bott, *tmp;
    while(list != NULL) {
        ricetta * ricetta = cerca_ricetta(list->nome);
        int result = verifica_magazzino(ricetta, list->quantita);
        if (result != 0) {
            processa_ordine(ricetta, list->quantita);
            inserisci_corriere(list->nome, list->quantita, list->ora, list->peso);

            tmp = list;

            if (list->prec != NULL) {
                list->prec->next = list->next;
            } else {
                coda_head = list->next;
            }

            if (list->next != NULL) {
                list->next->prec = list->prec;
            } else {
                coda_bott = list->prec;
            }
            list = list->prec;
            free(tmp);
        }
        else {
            list = list->prec;
        }
    } 
}  

int findin_list(coda * h, char * nome) {
    coda * tmp = h;
    while(tmp != NULL) {
        if (strcmp(tmp->nome, nome) == 0) return 1;
        tmp = tmp->next;
    }
    return 0;
}

void crea_coda_corriere() {
    coda_corr = (coda_corriere *) malloc(sizeof(coda_corriere));
    coda_corr->count = 0;
    coda_corr->dim = MIN_HEAP;
    coda_corr->preparato = (preparato *) malloc(sizeof(preparato) * MIN_HEAP);

    coda_out = (coda_corriere *) malloc(sizeof(coda_corriere));
    coda_out->count = 0;
    coda_out->dim = MAX_HEAP;
    coda_out->preparato = (preparato *) malloc(sizeof(preparato) * MAX_HEAP);
}


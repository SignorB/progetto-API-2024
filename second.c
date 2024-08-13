#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM_CHAR 256
#define HASH_RICETTE 1024
#define HASH_MAGAZZINO 1024

#define CMD_AGGIUNGI_RICETTA "aggiungi_ricetta"
#define CMD_RIMUOVI_RICETTA "rimuovi_ricetta"
#define CMD_RIFORNIMENTO "rifornimento"
#define CMD_ORDINE "ordine"

typedef struct lotto_s {
    int quantita;
    int scadenza;
    struct lotto_s * next;
} lotto;

typedef struct scorta_s {
    char nome[DIM_CHAR];
    int disponibile;
    lotto *lotto;
    struct scorta_s * next;
} scorta;

typedef struct ingrediente_s {
    scorta * scorta;
    int quantita;
    struct ingrediente_s *next;
} ingrediente;

typedef struct ricetta_s {
    char nome[DIM_CHAR];
    int peso;
    ingrediente *ingredienti;
    struct ricetta_s * next;
} ricetta;

typedef struct coda_s {
    int quantita;
    int peso;
    int ora;
    ricetta *ricetta;
    struct coda_s * next;
    struct coda_s * prec;
} coda;

int isCommand(char *);
int cancella_ricetta(char * );
int verifica_magazzino(ingrediente * , int );
int verifica_scorta(scorta * , int );
int findin_list(coda *, char *);
unsigned int hash(const char *, int);
void clear_ingredienti(ingrediente *);
void inserisci_ricetta(char *, int , ingrediente * );
void inserisci_lotto(char * , int , int );
void consuma_scorta(scorta * , int );
void processa_ordine(ingrediente * , int );
void refresh();
void inserisci_corriere(coda *,ricetta *, int , int , int );
void inserisci_corriere_out(coda * );
ingrediente * push_ingrediente(ingrediente * , scorta *, int );
ricetta * cerca_ricetta(char *);
scorta * cerca_scorta(char *);
scorta * clean_scaduti(scorta *);
scorta * inserisci_scorta(char *);
coda * push_coda(coda * , ricetta *, int , int , int );

// VARIABILI GLOBALI
int t_corriere, c_corriere, time = 0;
ricetta * ricettario[HASH_RICETTE];
scorta * magazzino[HASH_MAGAZZINO];
coda * coda_head = NULL;
coda * coda_bott = NULL;
coda * coda_corriere = NULL;
coda * coda_sorting = NULL;

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
    scorta * tmp ;
    
    if (scanf("%s", v) == 0) printf("ERROR");
    ricetta * ricetta = cerca_ricetta(v);

    while(1) {
        if(scanf("%s %d%c", nome, &quantita, &c) == 0) printf("ERROR");
        if (ricetta == NULL) {
            peso += quantita;
            tmp = cerca_scorta(nome);
            if (tmp == NULL) tmp = inserisci_scorta(nome);
            ing = push_ingrediente(ing, tmp, quantita);
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
        int result = verifica_magazzino(ricetta->ingredienti, quantita_ordine);

        if (result == 0) {
            coda_head = push_coda(coda_head, ricetta, quantita_ordine, (quantita_ordine * ricetta->peso), time);
        } else {
            processa_ordine(ricetta->ingredienti, quantita_ordine);
            inserisci_corriere(NULL, ricetta, quantita_ordine, time, (ricetta->peso * quantita_ordine));
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
    coda * tmp;

    while(coda_corriere != NULL && coda_corriere->peso < count) {
        count -= coda_corriere->peso;
        tmp = coda_corriere->next;
        inserisci_corriere_out(coda_corriere);
        coda_corriere = tmp;
    }

    if (count == c_corriere) printf("camioncino vuoto\n");
    else {
        while(coda_sorting != NULL) {
            tmp = coda_sorting;
            printf("%i %s %i\n", coda_sorting->ora,coda_sorting->ricetta->nome, coda_sorting->quantita);
            coda_sorting = coda_sorting->next;
            free(tmp);
        }
    }
}

int main(int argc, char *argv[]) {
    char v[DIM_CHAR];
    int util;
    int read = 1;
    int action;
    
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

unsigned int hash(const char* str, int h) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % h;
}

ricetta * cerca_ricetta(char * key) {
    int index = hash(key, HASH_RICETTE);
    if (ricettario[index] == NULL) return NULL;
    
    ricetta * tmp;
    tmp = ricettario[index];
    while(tmp != NULL) {
        if (strcmp(key, tmp->nome) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}
scorta * cerca_scorta(char * key) {
    int index = hash(key, HASH_MAGAZZINO);
    if (magazzino[index] == NULL) return NULL;

    scorta * tmp;
    tmp = magazzino[index];
    while(tmp != NULL) {
        if (strcmp(key, tmp->nome) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

int cancella_ricetta(char * key) {
    ricetta * ricetta = cerca_ricetta(key), *point;
    coda * tmp;

    if (ricetta == NULL) return -1;

    if (findin_list(coda_head, ricetta->nome)) return 0;

    tmp = coda_corriere;
    while (tmp != NULL) {
        if (strcmp(tmp->ricetta->nome, key) == 0 ) return 0;
        tmp = tmp->next;
    }

    clear_ingredienti(ricetta->ingredienti);
    
    int index = hash(key, HASH_MAGAZZINO);
    if (ricetta->next == NULL && ricettario[index] == ricetta) {
        ricettario[index] = NULL;
        free(ricetta);
    } else if (ricettario[index] == ricetta){
        ricettario[index] = ricetta->next;
        free(ricetta);
    } else  {
        point = ricettario[index];
        while(point->next != ricetta) {
            point = point->next;
        }
        point->next = ricetta->next;
        free(ricetta);
    }

    return 1;
}

void inserisci_ricetta(char *key, int peso, ingrediente * ing) {
    int index = hash(key, HASH_RICETTE);
    ricetta * new = (ricetta *)malloc(sizeof(ricetta));
    
    strcpy(new->nome, key);
    new->peso = peso;
    new->ingredienti = ing;

    if (ricettario[index] == NULL) {
        new->next = NULL;
        ricettario[index] = new;
    } else {
        new->next = ricettario[index];
        ricettario[index] = new;
    }
}

scorta * inserisci_scorta(char *key) {
    int index = hash(key, HASH_MAGAZZINO);
    scorta * new = (scorta *)malloc(sizeof(scorta));
    
    strcpy(new->nome, key);
    new->disponibile = 0;
    
    if (magazzino[index] == NULL) {
        new->next = NULL;
        magazzino[index] = new;
    } else {
        new->next = magazzino[index];
        magazzino[index] = new;        
    }
    return new;
}

void inserisci_lotto(char * key, int quantita, int scadenza) {
    scorta * scorta = cerca_scorta(key);
    lotto * new, *tmp;
    
    if (scorta == NULL) scorta = inserisci_scorta(key);

    if ((new = (lotto *) malloc(sizeof(lotto))) != NULL) {
        new->next = NULL;
        new->quantita = quantita;
        new->scadenza = scadenza;
        scorta->disponibile += quantita;
        if(scorta->lotto == NULL) {
            scorta->lotto = new;
        } else if (scadenza < scorta->lotto->scadenza) {
            new->next = scorta->lotto;
            scorta->lotto = new;
        } else {
            tmp = scorta->lotto;
            while(tmp->next != NULL && tmp->next->scadenza <= scadenza) {
                tmp = tmp->next;
            }
            new->next = tmp->next;
            tmp->next = new;
        }
    }
}

void inserisci_corriere(coda * ex, ricetta * ric, int quantita, int ora, int peso) {
    coda * new = ex, *tmp;
    if(new == NULL) {
        if ((new = (coda *) malloc(sizeof(coda))) != NULL) {
            new->quantita = quantita;
            new->peso = peso;
            new->ora = ora;
            new->ricetta = ric;
        }
    }
    new->next = NULL;
    new->prec= NULL;

    if (coda_corriere == NULL) coda_corriere = new;
    else if (coda_corriere->ora > ora) {
        new->next = coda_corriere;
        coda_corriere->prec = new;
        coda_corriere = new;
    } else {
        tmp = coda_corriere;
        while(tmp->next != NULL && tmp->next->ora < ora) {
            tmp = tmp->next;
        }
        new->next = tmp->next;
        new->prec = tmp;
        if (tmp->next != NULL) {
            tmp->next->prec = new;
        }
        tmp->next = new;
    }
    
}

void inserisci_corriere_out(coda * ex) {
    coda * new = ex, *tmp;

    new->next = NULL;
    new->prec= NULL;

    if (coda_sorting == NULL) coda_sorting = new;
    else if (coda_sorting->peso < new->peso || (coda_sorting->peso == new->peso && coda_sorting->ora > new->ora)) {
        new->next = coda_sorting;
        coda_sorting->prec = new;
        coda_sorting = new;
    } else {
        tmp = coda_sorting;
        while(tmp->next != NULL && (tmp->next->peso > new->peso || (tmp->next->peso == new->peso && tmp->next->ora < new->ora))) {
            tmp = tmp->next;
        }
        new->next = tmp->next;
        new->prec = tmp;
        if (tmp->next != NULL) {
            tmp->next->prec = new;
        }
        tmp->next = new;
    }
    
}


ingrediente * push_ingrediente(ingrediente * h, scorta * sco, int quantita) {
    ingrediente * new = NULL;

    if ((new = (ingrediente *) malloc(sizeof(ingrediente))) != NULL) {
        new->scorta = sco;
        new->quantita = quantita;
        new->next = h;        
    }
    return new; 
}

coda * push_coda(coda * h, ricetta * ric, int quantita, int peso, int t) {
    coda * new = NULL;

    if ((new = (coda *) malloc(sizeof(coda))) != NULL) {
        new->quantita = quantita;
        new->peso = peso;
        new->next = h;
        new->ora = t;
        new->ricetta = ric;
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
    lotto * tmp;
    while(s->lotto != NULL && s->lotto->scadenza <= time) {
        s->disponibile -= s->lotto->quantita;
        tmp = s->lotto;
        s->lotto = s->lotto->next;
        free(tmp);
    }
    return s;
}

int verifica_scorta(scorta * scorta, int totale) {
    scorta = clean_scaduti(scorta);
    int count = scorta->disponibile;
    if (count >= totale) return 1;
    return 0;
}

void consuma_scorta(scorta * scorta, int totale) {
    lotto * tmp;

    int count = totale;
    while(count > 0 && scorta->lotto != NULL) {
        if (scorta->lotto->quantita <= count) {
            count -= scorta->lotto->quantita;
            scorta->disponibile -= scorta->lotto->quantita;
            tmp = scorta->lotto;
            scorta->lotto = scorta->lotto->next;
            free(tmp);
        } else {
            scorta->lotto->quantita -= count;
            scorta->disponibile -= count;
            count = 0;
        }
    }
    if (count != 0) printf("ERROR CONUSMA SCORTA");
}

int verifica_magazzino(ingrediente * ing, int quantita_ordine) {
    int result = 1;
    while(ing != NULL && result) {
        result = verifica_scorta(ing->scorta, (quantita_ordine * ing->quantita));
        ing = ing->next;
    }
    return result;
}

void processa_ordine(ingrediente * ing, int quantita_ordine) {
    while(ing != NULL) {
        consuma_scorta(ing->scorta, (quantita_ordine * ing->quantita));
        ing = ing->next;
    }   
}

void refresh() {
    coda * list = coda_bott, *tmp;
    while(list != NULL) {
        int result = verifica_magazzino(list->ricetta->ingredienti, list->quantita);
        if (result != 0) {
            processa_ordine(list->ricetta->ingredienti, list->quantita);

            tmp = list->prec;

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
            inserisci_corriere(list, list->ricetta, list->quantita, list->ora, list->peso);
            list = tmp;
        }
        else {
            list = list->prec;
        }
    } 
}  

int findin_list(coda * h, char * nome) {
    coda * tmp = h;
    while(tmp != NULL) {
        if (strcmp(tmp->ricetta->nome, nome) == 0) return 1;
        tmp = tmp->next;
    }
    return 0;
}
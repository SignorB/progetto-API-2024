#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM_CHAR 256
#define HASH_RICETTE 1024
#define HASH_MAGAZZINO 256
#define QSORT_ARRAY 2600

typedef struct lotto {
    struct lotto * next;
    int expire;
    int q;
} lotto;
typedef struct scorta {
    struct scorta * next;
    lotto *lotto;
    char *nome;
    int disponibile;
} scorta;
typedef struct ingrediente {
    struct ingrediente *next;
    scorta * scorta;
    int q;
} ingrediente;
typedef struct ricetta {
    ingrediente *ingredienti;
    struct ricetta * next;
    char *nome;
    int peso;
    int sospeso;
} ricetta;
typedef struct attesa {
    ricetta *ricetta;
    struct attesa * next;
    struct attesa * prec;
    int q;
    int peso;
    int time;
} attesa;

attesa * sorting[QSORT_ARRAY];
ricetta * ricettario[HASH_RICETTE];
scorta * magazzino[HASH_MAGAZZINO];
attesa * testa_coda = NULL, * coda_coda = NULL, * coda_corriere = NULL;
int capacita_corriere, intervallo_corriere, time = 0, trash;

unsigned int hash(const char* nome, int h) {
    unsigned long a = 5387;
    int c;
    while ((c = *nome++)) a = c + ((a << 5) + a);
    return a % h;
}

int strcompare(char *a, char *b){
    int i= 0;
    while(a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return 1;
        i++;
    }
    if (a[i] == '\0' && b[i] == '\0') return 0;
    return 1;
}

void inserisci_ricetta(char *key, int peso, ingrediente * ing) {
    int i = hash(key, HASH_RICETTE);
    ricetta * new = (ricetta *)calloc(sizeof(ricetta), 1);
    int len = strlen(key) +1;
    
    new->nome = (char *) calloc(sizeof(char), len);
    new->sospeso = 0;
    new->peso = peso;
    new->ingredienti = ing;

    strcpy(new->nome, key);
    if (ricettario[i] == NULL) {
        new->next = NULL;
        ricettario[i] = new;
    } else {
        new->next = ricettario[i];
        ricettario[i] = new;
    }
}

int verifica_scorta(scorta * s, int totale) {
    if (s->disponibile >= totale) return 1;
    return 0;
}

ricetta * cerca_ricetta(char * nome) {
    int i = hash(nome, HASH_RICETTE);
    if (ricettario[i] == NULL) return NULL;
    
    ricetta * tmp_ricetta;
    tmp_ricetta = ricettario[i];
    while(tmp_ricetta != NULL) {
        if (strcompare(nome, tmp_ricetta->nome) == 0) return tmp_ricetta;
        tmp_ricetta = tmp_ricetta->next;
    }
    return NULL;
}

scorta * cerca_scorta(char * nome) {
    scorta * tmp_scorta;
    int i = hash(nome, HASH_MAGAZZINO);
    if (magazzino[i] == NULL) return NULL;

    tmp_scorta = magazzino[i];
    while(tmp_scorta != NULL) {
        if (strcompare(nome, tmp_scorta->nome) == 0) return tmp_scorta;
        tmp_scorta = tmp_scorta->next;
    }
    return NULL;
}

int cancella_ricetta(char * nome) {
    ricetta * ricetta = cerca_ricetta(nome), *point;

    if (ricetta == NULL) return -1;
    if (ricetta->sospeso > 0) return 0;
    
    int i = hash(nome, HASH_RICETTE);
    if (ricetta->next == NULL && ricettario[i] == ricetta) {
        ricettario[i] = NULL;
    } else if (ricettario[i] == ricetta){
        ricettario[i] = ricetta->next;
    } else  {
        point = ricettario[i];
        while(point->next != ricetta) {
            point = point->next;
        }
        point->next = ricetta->next;
    }

    return 1;
}

void rimuovi_ricetta(char * nome){
    int result = cancella_ricetta(nome);
    if (result == -1) printf("non presente\n");
    else if(result == 0) printf("ordini in sospeso\n");
    else printf("rimossa\n");
}

scorta * inserisci_scorta(char *nome) {
    int i = hash(nome, HASH_MAGAZZINO);
    scorta * new = (scorta *)calloc(sizeof(scorta), 1);
    int len = strlen(nome) + 1;

    new->nome = (char *)calloc(sizeof(char), len);
    strcpy(new->nome, nome);
    new->disponibile = 0;
    
    if (magazzino[i] == NULL) {
        new->next = NULL;
        magazzino[i] = new;
    } else {
        new->next = magazzino[i];
        magazzino[i] = new;        
    }
    return new;
}

void inserisci_lotto(char * nome, int quantita, int scadenza) {
    scorta * scorta = cerca_scorta(nome);
    lotto * new, *tmp;
    
    if (scorta == NULL) scorta = inserisci_scorta(nome);

    new = (lotto *) calloc(sizeof(lotto), 1);
    new->next = NULL;
    new->q = quantita;
    new->expire = scadenza;
    scorta->disponibile += quantita;
    if(scorta->lotto == NULL) scorta->lotto = new;
    else if (scadenza < scorta->lotto->expire) {
        new->next = scorta->lotto;
        scorta->lotto = new;
    } else {
        tmp = scorta->lotto;
        while(tmp->next != NULL && tmp->next->expire < scadenza) tmp = tmp->next;
        new->next = tmp->next;
        tmp->next = new;
    }
}

void inserisci_corriere(attesa * ex, ricetta * ric, int quantita, int ora, int peso) {
    attesa * new = ex, * tmp_coda;
    if(new == NULL) {
        new = (attesa *) calloc(sizeof(attesa), 1);
        new->q = quantita;
        new->peso = peso;
        new->time = ora;
        new->ricetta = ric;
    }
    new->next = NULL;
    new->prec= NULL;

    if (coda_corriere == NULL) coda_corriere = new;
    else if (coda_corriere->time > ora) {
        new->next = coda_corriere;
        coda_corriere->prec = new;
        coda_corriere = new;
    } else {
        tmp_coda = coda_corriere;
        while(tmp_coda->next != NULL && tmp_coda->next->time < ora) {
            tmp_coda = tmp_coda->next;
        }
        new->next = tmp_coda->next;
        new->prec = tmp_coda;
        if (tmp_coda->next != NULL) {
            tmp_coda->next->prec = new;
        }
        tmp_coda->next = new;
    }
}

ingrediente * push_ingrediente(ingrediente * h, scorta * sco, int quantita) {
    ingrediente * new = NULL;

    new = (ingrediente *) calloc(sizeof(ingrediente), 1);
    new->scorta = sco;
    new->q = quantita;
    new->next = h;        
    return new; 
}

attesa * push_coda(attesa * h, ricetta * ric, int quantita, int peso, int t) {
    attesa * new = NULL;

    new = (attesa *) calloc(sizeof(attesa), 1);
    new->q = quantita;
    new->peso = peso;
    new->next = h;
    new->time = t;
    new->ricetta = ric;
    new->prec = NULL;

    if (h == NULL) coda_coda = new;
    if (h != NULL) h->prec = new;
    return new; 
}

void consuma_scorta(scorta * scorta, int totale) {
    lotto * tmp_lotto;
    int count = totale;
    while(count > 0 && scorta->lotto != NULL) {
        if (scorta->lotto->q <= count) {
            count -= scorta->lotto->q;
            scorta->disponibile -= scorta->lotto->q;
            tmp_lotto = scorta->lotto;
            scorta->lotto = scorta->lotto->next;
            free(tmp_lotto);
        } else {
            scorta->lotto->q -= count;
            scorta->disponibile -= count;
            count = 0;
        }
    }
}

int verifica_magazzino(ingrediente * ing, int quantita_ordine) {
    int result = 1;
    while(ing != NULL && result) {
        result = verifica_scorta(ing->scorta, quantita_ordine * ing->q);
        ing = ing->next;
    }
    return result;
}

void processa_ordine(ingrediente * ing, int quantita_ordine) {
    while(ing != NULL) {
        consuma_scorta(ing->scorta, (quantita_ordine * ing->q));
        ing = ing->next;
    }   
}

void aggiungi_ricetta() {   
    char v[DIM_CHAR];
    char nome[DIM_CHAR];
    char c;
    int quantita;
    int peso = 0;
    ingrediente * ing = NULL;
    scorta * tmp_scorta;
    
    trash = scanf("%s", v);
    ricetta * ricetta = cerca_ricetta(v);

    while(1) {
        trash = scanf("%s %d%c", nome, &quantita, &c);
        if (ricetta == NULL) {
            peso += quantita;
            tmp_scorta = cerca_scorta(nome);
            if (tmp_scorta == NULL) tmp_scorta = inserisci_scorta(nome);
            ing = push_ingrediente(ing, tmp_scorta, quantita);
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

void ordine(char * nome, int quantita_ordine) {
    ricetta * ricetta = cerca_ricetta(nome);
    if (ricetta == NULL) {
        printf("rifiutato\n");
    } else {
        ricetta->sospeso++;
        int result = verifica_magazzino(ricetta->ingredienti, quantita_ordine);
        int peso_totale = quantita_ordine * ricetta->peso;

        if (result == 0) {
            testa_coda = push_coda(testa_coda, ricetta, quantita_ordine, peso_totale, time);
        } else {
            processa_ordine(ricetta->ingredienti, quantita_ordine);
            inserisci_corriere(NULL, ricetta, quantita_ordine, time, peso_totale);
        }
        printf("accettato\n");
    }
}

int compare(const void *a, const void *b) {
    attesa * primo = *(attesa **)a;
    attesa * secondo = *(attesa **)b;
    if (primo->peso > secondo->peso ) return -10;
    else if (primo->peso == secondo->peso && primo->time < secondo->time) return -10;
    else return 10;
}

void corriere() {
    attesa * tmp_coda;
    int count = capacita_corriere;
    int i = 0;
    while(coda_corriere != NULL && coda_corriere->peso < count) {
        count -= coda_corriere->peso;
        tmp_coda = coda_corriere->next;
        coda_corriere->ricetta->sospeso--;
        sorting[i] = coda_corriere;
        i++;    
        coda_corriere = tmp_coda;
    }

    if (count == capacita_corriere) printf("camioncino vuoto\n");
    else {
        qsort(sorting, i, sizeof(attesa *), compare);
        for (int k = 0; k<i; k++) {
            printf("%i %s %i\n", sorting[k]->time,sorting[k]->ricetta->nome, sorting[k]->q);
        }
    }
}

void refresh() {
    attesa * list = coda_coda, *tmp;
    while(list != NULL) {
        int result = verifica_magazzino(list->ricetta->ingredienti, list->q);
        if (result != 0) {
            processa_ordine(list->ricetta->ingredienti, list->q);

            tmp = list->prec;

            if (list->prec != NULL) {
                list->prec->next = list->next;
            } else {
                testa_coda = list->next;
            }

            if (list->next != NULL) {
                list->next->prec = list->prec;
            } else {
                coda_coda = list->prec;
            }
            inserisci_corriere(list, list->ricetta, list->q, list->time, list->peso);
            list = tmp;
        }
        else {
            list = list->prec;
        }
    } 
} 

void rifornimento() {
    char nome[DIM_CHAR];
    int scadenza;
    int quantita;
    char stop;

    do {
        trash = scanf("%s %i %i%c", nome, &quantita, &scadenza, &stop);
        if (scadenza > time) inserisci_lotto(nome, quantita, scadenza);
    } while(stop != '\n' && stop != EOF);
    printf("rifornito\n");
    refresh();
}

int main(int argc, char *argv[]) {
    char nome[DIM_CHAR];
    int read = 1,quantita;
    
    trash = scanf("%i %i", &intervallo_corriere, &capacita_corriere);
    
    read = scanf("%s", nome);
    while(read && read != EOF) {
        if (nome[0] == 'a') aggiungi_ricetta();
        else if (nome[2]== 'm') {
            trash = scanf("%s", nome);
            rimuovi_ricetta(nome);
        }
        else if (nome[2]== 'f') rifornimento();
        else if (nome[0]== 'o') {
            trash = scanf("%s %i", nome, &quantita);
            ordine(nome, quantita);
        }

        time++;
        lotto * tmp_lotto;
        scorta * point_scorta;
        for (int i = 0; i < HASH_MAGAZZINO; i++) {
            if (magazzino[i] != NULL) {
                point_scorta = magazzino[i];
                while (point_scorta != NULL) {
                    if(point_scorta->lotto != NULL && point_scorta->lotto->expire <= time) {
                        point_scorta->disponibile -= point_scorta->lotto->q;
                        tmp_lotto = point_scorta->lotto;
                        point_scorta->lotto = point_scorta->lotto->next;
                        free(tmp_lotto);
                    }
                    point_scorta = point_scorta->next;
                }
            }
        }
        if (time % intervallo_corriere == 0 && time != 0) corriere();
        read = scanf("%s", nome);
    }
    return 0;
};
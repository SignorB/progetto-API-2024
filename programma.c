#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM_CHAR 256
#define HASH_MAGAZZINO 256
#define HASH_RICETTE 4096
#define QSORT_ARRAY 2200

typedef struct lotto {
    struct lotto * succ;
    int expire;
    int q;
} lotto;
typedef struct scorta {
    struct scorta * succ;
    lotto *lotto;
    char *nome;
    int disponibile;
} scorta;
typedef struct ingrediente {
    struct ingrediente *succ;
    scorta * scorta;
    int q;
} ingrediente;
typedef struct ricetta {
    ingrediente *ingredienti;
    struct ricetta * succ;
    char *nome;
    int peso;
    int sospeso;
} ricetta;
typedef struct attesa {
    ricetta *ricetta;
    struct attesa * succ;
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

int hash(char *nome, int size) {
	int h = 0, a;
	while ((a = *nome++)) h += a;
	return h % size;
}

ricetta * cerca_ricetta(char * nome) {
    int i = hash(nome, HASH_RICETTE);
    if (ricettario[i] == NULL) return NULL;
    
    ricetta * tmp_ricetta;
    tmp_ricetta = ricettario[i];
    while(tmp_ricetta != NULL) {
        if (strcmp(nome, tmp_ricetta->nome) == 0) return tmp_ricetta;
        tmp_ricetta = tmp_ricetta->succ;
    }
    return NULL;
}

scorta * cerca_scorta(char * nome) {
    scorta * tmp_scorta;
    int i = hash(nome, HASH_MAGAZZINO);
    if (magazzino[i] == NULL) return NULL;

    tmp_scorta = magazzino[i];
    while(tmp_scorta != NULL) {
        if (strcmp(nome, tmp_scorta->nome) == 0) return tmp_scorta;
        tmp_scorta = tmp_scorta->succ;
    }
    return NULL;
}

void rimuovi_ricetta(char * nome){
    ricetta * ricetta = cerca_ricetta(nome), *point;

    if (ricetta == NULL) printf("non presente\n");
    else if (ricetta->sospeso > 0) printf("ordini in sospeso\n");
    else {
        int i = hash(nome, HASH_RICETTE);
        if (ricetta->succ == NULL && ricettario[i] == ricetta) ricettario[i] = NULL;
        else if (ricettario[i] == ricetta) ricettario[i] = ricetta->succ;
        else  {
            point = ricettario[i];
            while(point->succ != ricetta) point = point->succ;
            point->succ = ricetta->succ;
        }
        
        printf("rimossa\n");
        free(ricetta);
    }
}

scorta * inserisci_scorta(char *nome) {
    int i = hash(nome, HASH_MAGAZZINO);
    scorta * new = (scorta *)calloc(sizeof(scorta), 1);
    int len = strlen(nome) + 1;

    new->nome = (char *)calloc(sizeof(char), len);
    strcpy(new->nome, nome);
    new->disponibile = 0;
    
    if (magazzino[i] == NULL) {
        new->succ = NULL;
        magazzino[i] = new;
    } else {
        new->succ = magazzino[i];
        magazzino[i] = new;        
    }
    return new;
}

void inserisci_corriere(attesa * ex, ricetta * ric, int quantita, int ora, int peso) {
    attesa * new = ex, * tmp_coda;
    if (new == NULL) {
        new = (attesa *)calloc(sizeof(attesa), 1);
        new->ricetta = ric;
        new->peso = peso;
        new->q = quantita;
        new->time = ora;
    }
    new->succ = NULL;
    new->prec= NULL;

    if (coda_corriere == NULL) coda_corriere = new;
    else if (coda_corriere->time > ora) {
        new->succ = coda_corriere;
        coda_corriere->prec = new;
        coda_corriere = new;
    } else {
        tmp_coda = coda_corriere;
        while(tmp_coda->succ != NULL && tmp_coda->succ->time < ora) {
            tmp_coda = tmp_coda->succ;
        }
        new->succ = tmp_coda->succ;
        new->prec = tmp_coda;
        if (tmp_coda->succ != NULL) {
            tmp_coda->succ->prec = new;
        }
        tmp_coda->succ = new;
    }
}

int verifica_magazzino(ingrediente * ing, int quantita_ordine) {
    while(ing != NULL) {
        if (ing->scorta->disponibile < quantita_ordine * ing->q) return 0;
        ing = ing->succ;
    }
    return 1;
}

void processa_ordine(ingrediente * ing, int quantita_ordine) {
    lotto * tmp_lotto;
    int count;
    while(ing != NULL) {
        count = quantita_ordine * ing->q;
        while(count > 0 && ing->scorta->lotto != NULL) {
            if (ing->scorta->lotto->q <= count) {
                count -= ing->scorta->lotto->q;
                ing->scorta->disponibile -= ing->scorta->lotto->q;
                tmp_lotto = ing->scorta->lotto;
                ing->scorta->lotto = ing->scorta->lotto->succ;
                free(tmp_lotto);
            } else {
                ing->scorta->lotto->q -= count;
                ing->scorta->disponibile -= count;
                count = 0;
            }
        }
        ing = ing->succ;
    }   
}

void aggiungi_ricetta() {   
    char temp[DIM_CHAR], nome[DIM_CHAR], stop;
    int peso = 0, quantita, i;
    ingrediente * ing = NULL, *tmp_ing;
    scorta * tmp_scorta;
    
    trash = scanf("%s", temp);
    ricetta * ric = cerca_ricetta(temp);

    do {
        trash = scanf("%s %d%c", nome, &quantita, &stop);
        if (ric == NULL) {
            peso += quantita;
            tmp_scorta = cerca_scorta(nome);
            if (tmp_scorta == NULL) tmp_scorta = inserisci_scorta(nome);
            
            tmp_ing = (ingrediente *) calloc(sizeof(ingrediente), 1);
            tmp_ing->scorta = tmp_scorta;
            tmp_ing->q = quantita;
            tmp_ing->succ = ing;
            ing = tmp_ing;        
        }
    } while(stop != '\n' && stop != EOF);

    if (ric == NULL) {
        i = hash(temp, HASH_RICETTE);
        ric = (ricetta *)calloc(sizeof(ricetta), 1);
        quantita = strlen(temp) +1;
        
        ric->sospeso = 0;
        ric->peso = peso;
        ric->ingredienti = ing;
        ric->nome = (char *) calloc(sizeof(char), quantita);
        strcpy(ric->nome, temp);
        ric->succ = ricettario[i];
        if (ricettario[i] == NULL) ricettario[i] = ric;
        else  ricettario[i] = ric;
        printf("aggiunta\n");
    } else printf("ignorato\n");
}

void ordine(char * nome, int quantita_ordine) {
    ricetta * ricetta = cerca_ricetta(nome);
    if (ricetta == NULL) {
        printf("rifiutato\n");
    } else {
        attesa * new;
        int peso_totale = quantita_ordine * ricetta->peso;
        int result = verifica_magazzino(ricetta->ingredienti, quantita_ordine);
        ricetta->sospeso++;

        if (result == 0) {
            new = (attesa *) calloc(sizeof(attesa), 1);
            new->q = quantita_ordine;
            new->peso = peso_totale;
            new->succ = testa_coda;
            new->time = time;
            new->ricetta = ricetta;
            new->prec = NULL;

            if (testa_coda == NULL) coda_coda = new;
            if (testa_coda != NULL) testa_coda->prec = new;
            testa_coda = new;
        }
        else {
            processa_ordine(ricetta->ingredienti, quantita_ordine);
            inserisci_corriere(NULL, ricetta, quantita_ordine, time, peso_totale);
        }
        printf("accettato\n");
    }
}

int confronta(const void *a, const void *b) {
    attesa * primo = *(attesa **)a, * secondo = *(attesa **)b;
    if (primo->peso > secondo->peso ) return -10;
    else if (primo->peso == secondo->peso && primo->time < secondo->time) return -10;
    else return 10;
}

void corriere() {
    attesa * tmp_coda;
    int count = capacita_corriere, i = 0;

    while(coda_corriere != NULL && coda_corriere->peso < count) {
        count -= coda_corriere->peso;
        tmp_coda = coda_corriere->succ;
        coda_corriere->ricetta->sospeso--;
        sorting[i] = coda_corriere;
        i++;    
        coda_corriere = tmp_coda;
    }

    if (count == capacita_corriere) printf("camioncino vuoto\n");
    else {
        qsort(sorting, i, sizeof(attesa *), confronta);
        for (int k = 0; k<i; k++) printf("%i %s %i\n", sorting[k]->time,sorting[k]->ricetta->nome, sorting[k]->q);
    }
}

void rifornimento() {
    char nome[DIM_CHAR], stop;
    int scadenza, quantita;
    attesa * list = coda_coda, *tmp;

    do {
        trash = scanf("%s %i %i%c", nome, &quantita, &scadenza, &stop);
        if (scadenza > time) {
            scorta * scorta = cerca_scorta(nome);
            lotto * new, *tmp;
            
            if (scorta == NULL) scorta = inserisci_scorta(nome);

            new = (lotto *) calloc(sizeof(lotto), 1);
            new->succ = NULL;
            new->q = quantita;
            new->expire = scadenza;
            scorta->disponibile += quantita;
            if(scorta->lotto == NULL) scorta->lotto = new;
            else if (scadenza < scorta->lotto->expire) {
                new->succ = scorta->lotto;
                scorta->lotto = new;
            } else {
                tmp = scorta->lotto;
                while(tmp->succ != NULL && tmp->succ->expire < scadenza) tmp = tmp->succ;
                new->succ = tmp->succ;
                tmp->succ = new;
            }
        }
    } while(stop != '\n' && stop != EOF);
    printf("rifornito\n");
    
    while(list != NULL) {
        if (verifica_magazzino(list->ricetta->ingredienti, list->q)) {
            processa_ordine(list->ricetta->ingredienti, list->q);
            tmp = list->prec;

            if (tmp != NULL) tmp->succ = list->succ;
            else testa_coda = list->succ;

            if (list->succ != NULL) list->succ->prec = list->prec;
            else coda_coda = list->prec;
            
            inserisci_corriere(list, list->ricetta, list->q, list->time, list->peso);
            list = tmp;
        }
        else list = list->prec;
    } 
}

int main(int argc, char *argv[]) {
    char nome[DIM_CHAR];
    int read = 1,quantita;
    lotto * tmp_lotto;
    scorta * point_scorta;
    
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

        for (int i = 0; i < HASH_MAGAZZINO; i++) {
            if (magazzino[i] != NULL) {
                point_scorta = magazzino[i];
                while (point_scorta != NULL) {
                    if(point_scorta->lotto != NULL && point_scorta->lotto->expire <= time) {
                        point_scorta->disponibile -= point_scorta->lotto->q;
                        tmp_lotto = point_scorta->lotto;
                        point_scorta->lotto = point_scorta->lotto->succ;
                        free(tmp_lotto);
                    }
                    point_scorta = point_scorta->succ;
                }
            }
        }

        if (time % intervallo_corriere == 0 && time != 0) corriere();
        read = scanf("%s", nome);
    }
    return 0;
};
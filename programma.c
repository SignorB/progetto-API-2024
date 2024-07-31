#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM_CHAR 256
#define HASH_TABLE 1000

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
    return hash % HASH_TABLE;
}

int isCommand(char * cmd) {
    if (strcmp(cmd, CMD_AGGIUNGI_RICETTA) == 0) return 1;
    if (strcmp(cmd, CMD_RIMUOVI_RICETTA) == 0) return 2;
    if (strcmp(cmd, CMD_RIFORNIMENTO) == 0) return 3;
    if (strcmp(cmd, CMD_ORDINE) == 0) return 4;
    return 0;
}
typedef struct coda_s {
    int ora;
    char nome[DIM_CHAR];
    int quantita;
    int peso;
    struct coda_s *next;
} list;

// Struttura per Ricettario
typedef struct ingrediente_s {
    char nome[DIM_CHAR];
    int quantita;
    struct ingrediente_s *next;
} ingrediente;
typedef struct {
    char nome[DIM_CHAR];
    int peso;
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

list * head_cucina = NULL;
list * head_corriere = NULL;
tabella_ricette * tab_ricette = NULL;
inventario * tab_inventario = NULL;


// void print_ingrediente(ingrediente *ing) {
//     int i = 1;
//     while (ing != NULL) {
//         printf("  Ingrediente %d: %s, Quantita: %d\n", i++, ing->nome, ing->quantita);
//         ing = ing->next;
//     }
// }

// void print_ricetta(ricetta *r) {
//     if (r != NULL) {
//         printf("Ricetta: %s, Peso: %d\n", r->nome, r->peso);
//         printf("Ingredienti:\n");
//         if (r->ingredienti != NULL) {
//             print_ingrediente(r->ingredienti);
//         } else {
//             printf("  No ingredienti\n");
//         }
//     }
// }

// void print_tabella_ricette(tabella_ricette *tab) {
//     for (int i = 0; i < HASH_TABLE; i++) {
//         if (tab->el[i] != NULL) {
//             printf("Ricetta %d:\n", i + 1);
//             print_ricetta(tab->el[i]);
//         }
//     }
// }

// void print_lotto(lotto *l) {
//     int i = 1;
//     while (l != NULL) {
//         printf("  Lotto %d: Quantita: %d, Scadenza: %d\n", i++, l->quantita, l->scadenza);
//         l = l->next;
//     }
// }

// void print_scorta(scorta *s) {
//     if (s != NULL) {
//         printf("Scorta: %s\n", s->nome);
//         if (s->lotto != NULL) {
//             print_lotto(s->lotto);
//         } else {
//             printf("  No lotto\n");
//         }
//     }
// }

// void print_inventario(inventario *inv) {
//     for (int i = 0; i < HASH_TABLE; i++) {
//         if (inv->el[i] != NULL) {
//             printf("Scorta %d:\n", i + 1);
//             print_scorta(inv->el[i]);
//         }
//     }
// }


// funzione per inizializzare la tabella hash
inventario *crea_tabella_inventario() {
    inventario* tab = malloc(sizeof(inventario));
    tab->el = calloc(HASH_TABLE, sizeof(scorta*));
    return tab;
}

// inserisce una nuova entry nella tabella hash
scorta * inserisci_inventario(char * key){
    int index = hash(key);
    scorta* el= tab_inventario->el[index];
    int i = 1;
    while(el != NULL) {
        index = (index + i) % HASH_TABLE;
        el = tab_inventario->el[index+i];
        i++;
    }
    el = malloc(sizeof(scorta));
    strcpy(el->nome, key);
    el->lotto = NULL;
    tab_inventario->el[index] = el;
    return el;
}

// cerca una specifica chiave nella tabella hash
scorta * cerca_scorta(char * key){
    int index= hash(key);
    scorta * el= tab_inventario->el[index];
    int i = 1;
    while(el != NULL) {
        if(strcmp(el->nome, key) == 0) {
            return el;
        }
        index = (index + i) % HASH_TABLE;
        el = tab_inventario->el[index];
        i++;
    }
    return el;
}

// inserisce un rifornimento in ordine di scadenza
lotto * aggiungi_lotto(char *key, int quantita, int scadenza) {

    scorta *el = cerca_scorta(key);
    if (el == NULL) {
        el = inserisci_inventario(key);
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
                if (curr->prec != NULL) {
                    curr->prec->next = new;
                } else {
                    h = new;
                }
                curr->prec = new;
            } else {
                curr->next = new;
                new->prec = curr;
            }
        }
        el->lotto = h;
    } else {
        printf("push: errore allocazione memoria");
    }
    return h;
}
// funzione per inizializzare la tabella hash
tabella_ricette *crea_tabella_ricette() {
    tabella_ricette* tab = malloc(sizeof(tabella_ricette));
    tab->el = calloc(HASH_TABLE, sizeof(ricetta *));
    return tab;
}
// inserisce una nuova entry nella tabella hash
void inserisci_ricetta(char * key, ingrediente * ingredienti, int peso) {
    int index = hash(key);
    ricetta* el = tab_ricette->el[index];
    int i = 1;
    while(el != NULL) {
        index = (index + i) % HASH_TABLE;
        el = tab_ricette->el[index+i];
        i++;
    }
    
    el = malloc(sizeof(ricetta));
    strcpy(el->nome, key);
    el->ingredienti = ingredienti;
    el->peso = peso;
    tab_ricette->el[index] = el;
}

// aggiungi ingrediente ad una lista
ingrediente * aggiungi_ingrediente(ingrediente * h, char * nome, int quantita) {
    ingrediente *new;
    if ((new = (ingrediente*) malloc(sizeof(ingrediente))) != NULL) {
        new->quantita = quantita;
        strcpy(new->nome, nome);
        new->next = h;
        h = new;
    } else printf("push: errore allocazione memoria");
    return h;
}

// Cerca una specifica chiave nella tabella hash
ricetta * cerca_ricetta(char * key) {
    int index = hash(key);
    ricetta * el = tab_ricette->el[index];
    int i = 1;
    while(el != NULL) {
        if(strcmp(el->nome, key)== 0) {
            return el;
        }
        index = (index + i) % HASH_TABLE;
        el = tab_ricette->el[index];
        i++;
    }
    return el;
}

list * append(list *h, int ora, int quantita, char * nome, int peso) {
	list * new, * el;
	if((new = (list *) malloc(sizeof(list))) != NULL) {
		new->ora = ora;
        new->quantita= quantita;
        strcpy(new->nome, nome);
		new->next = NULL;
        new->peso = peso;
		
		if (h == NULL) h = new;
		else {
			el = h;
			while(el->next) 
				el = el->next;
			el->next = new;
		} 
        return h;
	} 
    printf("append: errore allocazione memoria - 2 \n");
    return NULL;
}

// rimuovi un elemento dalla coda
list * pop(list * h) {
    list *tmp = h;
    h = h->next;
    free(tmp);
    
    return h;
}

list * find(list * h, char nome[]) {
    list *tmp;
    tmp = h;
    while(tmp != NULL) {
        if (strcmp(tmp->nome, nome) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

// lotto * clean_lotto(lotto * h) {
//     lotto * tmp;
//     if (h == NULL) return h;
    
//     while(h->next != NULL && h->scadenza <= time) { 
//         tmp = h;
//         h = h->next;
//         free(tmp);
//     }

//     if (h->next == NULL && h->scadenza <= time) {
//         free(h);
//         return NULL;
//     }
//     return h;
// }

int verifica_magazzino(char * key, int num) {
    ingrediente * tmp;
    scorta * el;
    lotto * info;
    ricetta * ricetta = cerca_ricetta(key);
    int count;
    
    if (ricetta == NULL) return -1; // Non c'è la ricetta

    tmp = ricetta->ingredienti;
    while(tmp != NULL) {
        el = cerca_scorta(tmp->nome);
        if (el == NULL || el->lotto == NULL) return 0; // Non c'è l'ingrediente, metti in coda
        count = 0;
        info = el->lotto;
        while(info != NULL && count < (tmp->quantita * num)) {
            count += info->quantita;
            info = info->next;
        }

        if (count < (tmp->quantita * num)) return 1;
        tmp = tmp->next;
    }

    return ricetta->peso; // ci sono abbastanza ingredienti
}

void processa_ordine(char * key, int num) {
    ingrediente * tmp;
    scorta * el;
    lotto * info;
    ricetta * ricetta = cerca_ricetta(key);
    int count, richiesti;
    
    if (ricetta == NULL) {
        printf("THERE WAS AN ERROR");
        return;
    }

    tmp = ricetta->ingredienti;
    while(tmp != NULL) {
        el = cerca_scorta(tmp->nome);
        count = 0;
        info = el->lotto;
        while(info != NULL && count < (tmp->quantita * num)) {
            richiesti = (tmp->quantita * num) - count;
            if (info->quantita <=  richiesti) {
                count += info->quantita;
                el->lotto = info->next;
                free(info);
                info = el->lotto;
            } else {
                info->quantita = (info->quantita * num) - count;
            }
        }
        tmp = tmp->next;
    }
}

// Rimuovi una ricetta dalla tabella hash => per evitare collisioni viene sovrascritta e liberato spazio
void cancella_ricetta(ricetta * el) {
    if (el != NULL) {
        ingrediente *head, *tmp;
        head = el->ingredienti;
        while (head != NULL) {
            tmp = head;
            head = head->next;
            free(tmp);
        }
        strcpy(el->nome, "|DELETED|");
        el->ingredienti = head;
    }
}

// aggiunge una ricetta alla hash table, gestendo input e liste dinamiche con ingredienti
void aggiungi_ricetta() {
    /* 
        Resistuisce 
            AGGIUNTA
            IGNORATO (se c'è già nella lista delle ricette)
    */    
    char v[DIM_CHAR];
    char nome[DIM_CHAR];
    char c;
    int quantita;
    int peso = 0;
    
    ingrediente * h = NULL;
    if (scanf("%s", v) == 0) printf("ERROR");
    ricetta * el = cerca_ricetta(v);

    if(scanf("%s %d%c", nome, &quantita, &c) == 0) printf("ERROR");
    peso += quantita;
    if (el == NULL) {
        h = aggiungi_ingrediente(NULL, nome, quantita);
    }
    while(1) {
        if (c == '\n' || c == EOF) break;
        if(scanf("%s %d%c", nome, &quantita, &c) == 0) printf("ERROR");
        peso += quantita;
        // Se non si tratta di una ricetta già esistente, di cui ingoreremmo la registrazione, aggiungi alla lista
        if (el == NULL) {
            h = aggiungi_ingrediente(h, nome, quantita);
        }
    }
    if (el == NULL) {
        inserisci_ricetta(v, h, peso);
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
    ricetta *el = cerca_ricetta(nome_ricetta);
    if (el == NULL) {
        printf("non presente\n");
        return;
    }
    int trovato = 0; // TODO ordini in sospeso
    if (trovato == 1) {
        printf("ordini in sospeso\n");
        return;
    }

    cancella_ricetta(el);
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
    // int result;
    verifica_magazzino(dolce, quantita);
    // if (result == -1) {
    //     printf("rifiutato\n");
    //     return;
    // }
    // if (result == 0) {
    //     head_cucina = append(head_corriere, time, quantita, dolce, 0);
    // } else {
    //     processa_ordine(dolce, quantita);
    //     head_corriere = append(head_corriere, time, quantita, dolce, result);
    // }
    printf("accettato\n");
}

void rifornimento() {
    /* 
        Resistuisce 
            RIFORNITO
    */
    char v[DIM_CHAR];
    char c;
    int scadenza;
    int quantita;

    while(1) {
        // c = getchar();  
        // ungetc(c, stdin);
        if(scanf("%s %i %i%c", v, &quantita, &scadenza, &c) == 0) printf("ERROR");
        aggiungi_lotto(v, quantita, scadenza);
        if (c == '\n' || c == EOF) break;
    }

    printf("rifornito\n");
    // TODO: Aggiungi rifornimento
}

void corriere() {
    /* 
        Resistuisce 
            (istante_ordine) (nome ricetta) (numero elementi ordinati)
            CAMIONCINO VUOTO se non c'è nulla in coda
    */
    // TODO: rifare la restituzione di ordine di peso

}


int main(int argc, char *argv[]) {
    char v[DIM_CHAR];
    int util;
    int read = 1;
    int action;
    
    tab_ricette = crea_tabella_ricette();
    tab_inventario = crea_tabella_inventario();
    
    if(scanf("%i %i", &t_corriere, &c_corriere) == 0) {
        printf("ERROR: scanf - 1");
    };
    
    read = scanf("%s", v);
    while(read && read != EOF) {
        if (time % t_corriere == 0 && time != 0) corriere();
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
            if (scanf("%s %i", v, &util) == 0) printf("ERROR: scanf - 2") ;
            ordine(v, util);
        }

        time++;
        read = scanf("%s", v);
    }


    return 0;
};

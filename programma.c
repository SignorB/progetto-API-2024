#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DIM_CHAR 256
#define DIM_FRASE 100
#define MAX_INGREDIENTI 20

#define CMD_AGGIUNGI_RICETTA "aggiungi_ricetta"
#define CMD_RIMUOVI_RICETTA "rimuovi_ricetta"
#define CMD_RIFORNIMENTO "rifornimento"
#define CMD_ORDINE "ordine"

int t_corriere, c_corriere, time = 0;

int isCommand(char * cmd) {
    if (strcmp(cmd, CMD_AGGIUNGI_RICETTA) == 0) return 1;
    if (strcmp(cmd, CMD_RIMUOVI_RICETTA) == 0) return 2;
    if (strcmp(cmd, CMD_RIFORNIMENTO) == 0) return 3;
    if (strcmp(cmd, CMD_ORDINE )== 0) return 4;
    return 0;
}

void aggiungi_ricetta(char *nome_ricetta, char nome_ingredienti[][MAX_INGREDIENTI], int *quantita_ingredienti, int n) {
    printf("Nome Ricetta: %s\n", nome_ricetta);
    for (int i = 0; i < n; i++) {
        printf("\t%s\t%d\n", nome_ingredienti[i], quantita_ingredienti[i]);
    }
}

void rimuovi_ricetta(char * nome_ricetta){
    printf("rimuovi ricetta\n");
}

void ordine(char * dolce, int quantita) {
    printf("ordine\n");
}

void rifornimento(char nome_ingredienti[][MAX_INGREDIENTI], int *quantita_ingredienti,int *scadenza_ingredienti, int n) {
    printf("rifornimento\n");
}

void corriere() {
    printf("=> CORRIERE <=\n ");
}

int main(int argc, char *argv[]) {
    char input [DIM_CHAR*DIM_FRASE];
    char * word;
    
    fgets(input, DIM_CHAR*DIM_FRASE, stdin);
    t_corriere = atoi(strtok(input, " "));
    c_corriere = atoi(strtok(NULL, " "));
    

    while(fgets(input, DIM_CHAR*DIM_FRASE, stdin) != NULL) {
        
        if (time % t_corriere == 0 && t_corriere != 0) {
            corriere();
        }

        word = strtok(input, " ");
        int action = isCommand(word);

        if  (action == 1) {
            char nome_ricetta[DIM_CHAR];
            char nome_ingredienti[DIM_CHAR][MAX_INGREDIENTI];
            int quantita_ingredienti[MAX_INGREDIENTI];
            int i = 0;

            word = strtok(NULL, " ");
            strcpy(nome_ricetta, word);

            word = strtok(NULL, " ");
            strcpy(nome_ingredienti[i], word);
            quantita_ingredienti[i] = atoi(strtok(NULL, " "));
                        
            word = strtok(NULL, " ");
            while(word != NULL) {
                i++;
                strcpy(nome_ingredienti[i], word);
                quantita_ingredienti[i] = atoi(strtok(NULL, " "));
                word = strtok(NULL, " ");
            }
            aggiungi_ricetta(nome_ricetta, nome_ingredienti, quantita_ingredienti, i);

        } else if (action == 2) {
            word = strtok(NULL, " ");

            rimuovi_ricetta(word);
            
        } else if (action == 3) {
            char nome_ingredienti[DIM_CHAR][MAX_INGREDIENTI];
            int quantita_ingredienti[MAX_INGREDIENTI];
            int scadenza_ingredienti[MAX_INGREDIENTI];
            int i = 0;
        
            word = strtok(NULL, " ");
            strcpy(nome_ingredienti[i], word);
            quantita_ingredienti[i] = atoi(strtok(NULL, " "));
            scadenza_ingredienti[i] = atoi(strtok(NULL, " "));
                        
            word = strtok(NULL, " ");
            while(word != NULL) {
                i++;
                strcpy(nome_ingredienti[i], word);
                quantita_ingredienti[i] = atoi(strtok(NULL, " "));
                scadenza_ingredienti[i] = atoi(strtok(NULL, " "));
                word = strtok(NULL, " ");
            }
            rifornimento(nome_ingredienti, quantita_ingredienti, scadenza_ingredienti, i);

        } else if (action == 4) {
            word = strtok(NULL, " ");
            int quantita = atoi(strtok(NULL, " "));

            ordine(word, quantita);
        } else {
            
            printf("there has been an error!\n");
        }
        time++;
    }
    printf("%d\n", time+1);

    return 0;
};

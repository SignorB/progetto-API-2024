# Gestione Ordini e Magazzino – Progetto Finale  
**Valutazione: 30/30 con Lode**

## Descrizione  
Questo progetto simula un sistema avanzato di gestione di **magazzino**, **ricette** e **ordini**, con supporto al **rifornimento automatico**, **scadenza lotti**, e **spedizione tramite corriere**.  
L'obiettivo è la progettazione di un sistema efficiente e robusto per l'evasione degli ordini nel rispetto dei vincoli di capacità e disponibilità.

## Funzionalità principali  
-  **Gestione dinamica del magazzino** con supporto a più lotti per ingrediente, ciascuno con quantità e data di scadenza.  
-  **Registrazione e rimozione ricette**, ognuna composta da più ingredienti, con controllo automatico sulla disponibilità.  
-  **Gestione ordini**: verifica disponibilità, accodamento in caso di indisponibilità e successiva evasione.  
-  **Rifornimento intelligente**: inserimento ordinato dei lotti per data di scadenza.  
-  **Corriere periodico**: spedizione ordini secondo priorità (peso e tempo), entro capacità massima.  
-  **Gestione temporale automatica**: controllo scadenze e processi periodici sincronizzati col tempo.

## Struttura del codice  
-  **Tabelle hash** per accesso rapido a ricette e scorte.  
-  **Liste collegate** per gestire dinamicamente ingredienti, ordini e lotti.  
-  **Algoritmo di ordinamento `qsort`** per ottimizzare l'evasione degli ordini.  
-  **Controllo puntuale della memoria** con `malloc` e `free`.


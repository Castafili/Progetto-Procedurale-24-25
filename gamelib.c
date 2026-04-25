#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gamelib.h"

#define MAX_GIOCATORI 3

Nemico nemico_attuale = nessun_nemico;

//sezione vincitori
#define MAX_VINCITORI 3
char ultimi_vincitori[MAX_VINCITORI][20] = {"", "", ""};

void salva_vincitore(const char* nome_vincitore) {
    //Per spostare i vincitori esistenti in basso
    for (int i = MAX_VINCITORI - 1; i > 0; i--) {
        char temp[20];
        snprintf(temp, sizeof(temp), "%s", ultimi_vincitori[i - 1]);
        snprintf(ultimi_vincitori[i], sizeof(ultimi_vincitori[i]), "%s", temp);
    }

    //Copiare il nuovo vincitore alla posizione principale
    snprintf(ultimi_vincitori[0], sizeof(ultimi_vincitori[0]), "%s", nome_vincitore);
}

//Puntatore globale prima stanza
struct Stanza* pFirst = NULL;

//Variabili globali
struct Giocatore* giocatori[3] = {NULL, NULL, NULL};
int n_giocatori = 0;

//Funzioni per imposta_gioco
static void ins_stanza();
static void canc_stanza();
static void stampa_stanze();
static void genera_random();
static void chiudi_mappa();
static int conta_stanze();


//Funzioni per gioco
void avanza();
void combatti (struct Giocatore* giocatore, Nemico);
void stampa_giocatore();
void stampa_zona();
void prendi_tesoro();
void cerca_porta_segreta();


void passa() {
    printf("Turno passato.\n");
}

void scappa(struct Giocatore* giocatore) {
    if (!giocatore || giocatore->p_vita <= 0) {
        printf("Errore: il giocatore non puo scappare.\n");
        return;
    }

    //controllo numero fuge
    if ((giocatore->classe_giocatore == principe && giocatore->numero_scappate >= 1) ||
    (giocatore->classe_giocatore == doppleganger && giocatore->numero_scappate >= 2)) {
        printf("%s non puo piu scappare!\n", giocatore->nome_giocatore);
        return;
    }

    printf("%s Scappa!\n", giocatore->nome_giocatore);

    //fuga
    if (!giocatore->posizione->stanza_precedente) {
        printf("Non e possibile scappare, questa e la prima stanza\n");
        return;
    }

    giocatore->posizione = giocatore->posizione->stanza_precedente;
    giocatore->numero_scappate++;

    printf("%s e scappato ed e tornato alla stanza precedente.\n", giocatore->nome_giocatore);
    if (giocatore->posizione->trabocchetto_attivato) {
        printf("Il trabocchetto in questa stanza e gia stato attivato");
    }
}


void avanza (struct Giocatore* giocatore) {

    if (!giocatore || !giocatore->posizione) {
        printf("Errore: il giocatore o la posizione non e valida.\n");
        return;
    }

    struct Stanza* stanza_corrente = giocatore->posizione;
    struct Stanza* prossima_stanza = NULL;
    char direzione[10];

    //determinazzione direzione
    if (stanza_corrente->stanza_destra) {
        prossima_stanza = stanza_corrente->stanza_destra;
        strcpy(direzione, "destra");
    }
    else if (stanza_corrente->stanza_sinistra) {
        prossima_stanza = stanza_corrente->stanza_sinistra;
        strcpy(direzione, "sinistra");
    }
    else if (stanza_corrente->stanza_sopra) {
        prossima_stanza = stanza_corrente->stanza_sopra;
        strcpy(direzione, "sopra");
    }
    else if (stanza_corrente->stanza_sotto) {
        prossima_stanza = stanza_corrente->stanza_sotto;
        strcpy(direzione, "sotto");
    }
    else {
        printf("Non ci sono direzioni disponibili.\n");
        return;
    }

    printf("%s avanza verso %s.\n", giocatore->nome_giocatore, direzione);

    //Agg. pos. giocatore
    prossima_stanza->stanza_precedente = giocatore->posizione;
    giocatore->posizione = prossima_stanza;

    //attivo trabocchetto
    if (!prossima_stanza->trabocchetto_attivato) {
        prossima_stanza->trabocchetto_attivato = 1; //trab attivato

        //verifica abilita principe
        if (giocatore->classe_giocatore == principe && !giocatore->trabocchetto_ignorato) {
            giocatore->trabocchetto_ignorato = 1; //Segna uso abilita
            printf("Il principe evita il trabocchetto!\n");
            return; 
        }

        //effetti trab.
        switch (prossima_stanza->trabocchetto) {
            case tegola:
                printf("%s viene colpito da una tegola e perde 1 punto vita.\n", giocatore->nome_giocatore);
                giocatore->p_vita -= 1;
                break;

            case lame:
                printf("%s viene colpito da lame e perde 2 punto vita.\n", giocatore->nome_giocatore);
                giocatore->p_vita -= 2;
                break;

            case caduta: {
                int danno = (rand() % 2) + 1; //1 o 2 punti vita
                printf("%s cade e perde %d punto vita.\n", giocatore->nome_giocatore, danno);
                giocatore->p_vita -= danno;
                break;
            }

            case burrone: {
                int danno = (rand() % 2) + 1; //1 o 2 punti vita come caduta
                printf("%s cade in burrone, perde il turno e perde %d punto vita.\n", giocatore->nome_giocatore, danno);
                giocatore->p_vita -= danno;
                passa();
                break;
            }

            case nessuno:
                default:
                    printf("Nessun trabocchetto presente.\n");
                    break;
        }

        if (giocatore->p_vita <= 0) {
            printf("%s e morto a causa di un trabocchetto.\n", giocatore->nome_giocatore);
            return;
        }

        //apparizione nemico 25%
        int probabilita = rand() % 100;
        if (probabilita < 25) {
            int tipo_nemico = rand() % 100;
            if (tipo_nemico < 60) {
                printf("Appare uno scheletro!\n");
                nemico_attuale = scheletro;
            }
            else {
                printf("Appare una guardia!\n");
                nemico_attuale = guardia;
                
            }
        }
        
        
        if (giocatore->p_vita <= 0) {
            printf("Un nemico ha ucciso %s,\n", giocatore->nome_giocatore);
        }
    }
    else {
                printf("Nessun nemico presente.\n");
            }
}

void stampa_giocatore(struct Giocatore* giocatore) {
    if (giocatore == NULL) {
        printf("Errore: giocatore non valido.\n");
        return;
    }

    printf("\nInformazioni Giocatore:\n");
    printf("Nome: %s\n", giocatore->nome_giocatore);
    printf("Classe: %s\n", giocatore->classe_giocatore == principe ? "Principe" : "Doppleganger");
    printf("Punti vita: %u\n", giocatore->p_vita);
    printf("Punti vita massimi: %u\n", giocatore->p_vita_max);
    printf("Dadi Attacco: %u\n", giocatore->dadi_attacco);
    printf("Dadi Difesa: %u\n", giocatore->dadi_difesa);
}

void stampa_zona(struct Stanza* zona_corrente) {
    if (zona_corrente == NULL) {
        printf("Errore: zona non valida.\n");
        return;
    }

    printf("\nInformazioni sulla Zona:\n");
    printf(" - Tipo Stanza: %d\n", zona_corrente->stanza);
    printf(" - Trabocchetto: %d\n", zona_corrente->trabocchetto);

    //verifica tesoro
    if (zona_corrente->tesoro !=0) {
        printf(" - Presenza di un Tesoro: Si\n");
    }
    else {
        printf(" - Presenza di un Tesoro: No\n");
    }
}

void imposta_gioco() {
    int scelta = 0;

    while (1) {
        printf("\nMenu Imposta Gioco:\n");
        printf("1) Imposta giocatori\n");
        printf("2) Inserisci stanza\n");
        printf("3) Cancella ultima stanza\n");
        printf("4) Stampa stanze\n");
        printf("5) Genera mappa casuale\n");
        printf("6) Chiudi mappa\n");
        printf("Scelta: ");
        scanf("%d", &scelta);

        switch (scelta) {
            case 1: { //Imposta giocatori 
            
                printf("Inserisci il numero di giocatori (1-3): ");
                int numero_giocatori;
                scanf("%d", &numero_giocatori);
                
                if (numero_giocatori < 1 || numero_giocatori > 3) {
                    printf("Errore: Numero non valido\n");
                    break;
                }

                n_giocatori = numero_giocatori;
                int principe_presente = 0;

                for (int i = 0; i < n_giocatori; i++) {
                     giocatori[i] = malloc(sizeof(struct Giocatore)); 
                     if (!giocatori[i]) { printf("Errore: memoria non allocata per il giocatore %d.\n", i + 1); 
                     exit(1); 
                    
                    } 
                    
                // Imposta il nome del giocatore 
                printf("Inserisci il nome del giocatore %d: ", i + 1); 
                scanf("%s", giocatori[i]->nome_giocatore); 
                
                // Scegli la classe del giocatore 
                printf("Scegli la classe del giocatore %d (0 = Principe, 1 = Doppelganger): ", i + 1); 
                int classe; 
                scanf("%d", &classe); 
                
                if (classe == 0) {
                 principe_presente = 1; 
                } 
                else if (classe != 0 && classe != 1) { 
                    printf("Classe non valida. Imposto di default Principe.\n"); 
                    classe = 0; 
                    principe_presente = 1; 
                    } 
                    
                    giocatori[i]->classe_giocatore = (tipo_giocatore)classe; 
                    
                    // Imposta i valori iniziali 
                    giocatori[i]->p_vita = 3; 
                    giocatori[i]->p_vita_max = 3; 
                    giocatori[i]->dadi_attacco = 2; 
                    giocatori[i]->dadi_difesa = 2; 
                    giocatori[i]->numero_scappate = 0; 
                    giocatori[i]->trabocchetto_ignorato = 0; 
                    giocatori[i]->posizione = NULL;  // Da aggiornare quando la mappa è chiusa 
                    
                    } 
                    
                    // Se nessun Principe è stato scelto, forza il primo giocatore come Principe 
                    if (!principe_presente) { 
                        printf("Nessun Principe scelto. Il primo giocatore sarà impostato come Principe.\n");
                         giocatori[0]->classe_giocatore = principe; 
                         
                        } 
                        
                        printf("Giocatori impostati con successo!\n"); 
                        break;
                }
                

            case 2:
                ins_stanza();
                break;
            
            case 3:
                canc_stanza();
                break;
            
            case 4:
                stampa_stanze();
                break;

            case 5:
                genera_random();
                break;

            case 6:
                chiudi_mappa();
                return;

            default:
                printf("Scelta non valida.\n");
        }
    } while (1);
}

static void ins_stanza() {
    struct Stanza* nuova_stanza = (struct Stanza*)malloc(sizeof(struct Stanza));
    if (!nuova_stanza) {
        printf("Errore: memoria insufficiente per la creazione della stanza.\n");
        return;
    }

    printf("Inserisci il tipo della stanza (0-9): ");
    int tipo;
    if (scanf("%d", &tipo) != 1 || tipo < 0 || tipo > 9) {
        printf("Errore: tipo stanza non valido.\n");
        free(nuova_stanza);
        return;
    }
    nuova_stanza->stanza = tipo;
    nuova_stanza->trabocchetto = rand() % 5;
    nuova_stanza->tesoro = rand() % 6;

    nuova_stanza->stanza_destra = NULL;
    nuova_stanza->stanza_sinistra = NULL;
    nuova_stanza->stanza_sopra = NULL;
    nuova_stanza->stanza_sotto = NULL;

    if (!pFirst) {
        pFirst= nuova_stanza;
        printf("Prima stanza creata!\n");
        return;
    }

    printf("Scegli la direzione per collegare la stanza (0=destra, 1=sinistra, 2=sopra, 3=sotto): ");
    int direzione;
    if (scanf("%d", &direzione) != 1 || direzione < 0 || direzione > 3) {
        printf("Errore: direzione non valida.\n");
        free(nuova_stanza);
        return;
    }

struct Stanza* temp = pFirst;
while (1) {
    if (direzione == 0 && !temp->stanza_destra) {
        temp->stanza_destra = nuova_stanza;
        break;
    }
    else if (direzione == 1 && !temp->stanza_sinistra) {
        temp->stanza_sinistra = nuova_stanza;
        break;
    }
    else if (direzione == 2 && !temp->stanza_sopra) {
        temp->stanza_sopra = nuova_stanza;
        break;
    }
    else if (direzione == 4 && !temp->stanza_sotto) {
        temp->stanza_sotto = nuova_stanza;
        break;
    }
    else {
        printf("La direzione scelta e gia in uso.\n");
        free(nuova_stanza);
        return;
    }
}

    printf("Nuova stanza aggiunta!\n");
}

static void canc_stanza() {
    if (!pFirst) {
        printf("Errore: nessuna stanza da cancellare.\n");
        return;
    }

    struct Stanza* temp = pFirst;
    struct Stanza* prev = NULL;

    // Cerca l'ultima stanza disponibile
    while (temp->stanza_destra || temp->stanza_sinistra || temp->stanza_sopra || temp->stanza_sotto) {
        prev = temp;

        if (temp->stanza_destra) temp = temp->stanza_destra;
        else if (temp->stanza_sinistra) temp = temp->stanza_sinistra;
        else if (temp->stanza_sopra) temp = temp->stanza_sopra;
        else if (temp->stanza_sotto) temp = temp->stanza_sotto;
    }

    // Disconnetti la stanza dalla mappa
    if (prev) {
        if (prev->stanza_destra == temp) prev->stanza_destra = NULL;
        else if (prev->stanza_sinistra == temp) prev->stanza_sinistra = NULL;
        else if (prev->stanza_sopra == temp) prev->stanza_sopra = NULL;
        else if (prev->stanza_sotto == temp) prev->stanza_sotto = NULL;
    } else {
        // Se non c'era una stanza precedente, significa che pFirst era l'unica stanza
        pFirst = NULL;
    }

    free(temp);
    printf("Stanza/e cancellata/e con successo!\n");
}


static void stampa_stanze() {
    if (!pFirst) {
        printf("La mappa e vuota.\n");
        return;
    }

    struct Stanza* temp = pFirst;
    int count = 1;

    while (temp) {
        printf("Stanza %d:\n", count);
        printf("  Tipo: %d\n", temp->stanza);
        printf("  Trabocchetto: %d\n", temp->trabocchetto);
        printf("  Tesoro: %d\n", temp->tesoro);

        if (temp->stanza_destra) temp = temp->stanza_destra;
        else if(temp->stanza_sinistra) temp = temp->stanza_sinistra;
        else if(temp->stanza_sopra) temp = temp->stanza_sopra;
        else if(temp->stanza_sotto) temp = temp->stanza_sotto;
        else break;

        count++;
    }
}

static void genera_random() {
    struct Stanza* temp = pFirst;
    int test = !pFirst;
    //Cancellazione mappa esistente
        // printf("DEBUG pFIRST %p\n", (void*)pFirst);
        // printf("DEBUG !pFIRST %d\n", test);
    while (pFirst) {
        canc_stanza();
    }

    //Creazione 15 stanze casuali
    for (int i = 0; i < 15; i++) {
        struct Stanza* nuova_stanza = (struct Stanza*)malloc(sizeof(struct Stanza));
        if (!nuova_stanza) {
            printf("Errore durante l'allocazione di memoria\n");
            return;
        }
        
        //Campi casuali
        nuova_stanza->stanza = (Tipo_stanza)(rand() % 10);
        nuova_stanza->trabocchetto = (rand() % 100 < 65) ? nessuno : (Tipo_trabocchetto)(1 + rand() % 4);
        nuova_stanza->tesoro = (Tipo_tesoro)((rand() % 100 < 20) ? 0 : (rand() % 5 + 1));

        nuova_stanza->stanza_destra = nuova_stanza->stanza_sinistra = nuova_stanza->stanza_sopra = nuova_stanza->stanza_sotto = NULL;

        if (!pFirst) {
            pFirst = nuova_stanza;
            temp = nuova_stanza;
        }
        else {
            int direzione;
            do {
                direzione = rand()  % 4;
            } while ((direzione == 0 && temp->stanza_destra) ||
            (direzione == 1 && temp->stanza_sinistra) ||
            (direzione == 2 && temp->stanza_sopra) ||
            (direzione == 3 && temp->stanza_sotto));
        
        switch (direzione) {
            case 0:
                temp->stanza_destra = nuova_stanza;
                break;
            
            case 1:
                temp->stanza_sinistra = nuova_stanza;
                break;

            case 2:
                temp->stanza_sopra = nuova_stanza;
                break;
            
            case 3:
                temp->stanza_sotto = nuova_stanza;
                break;
        }
        temp = nuova_stanza;
        }
    }
    printf("Mappa generata con successo, per controllare premere 4\n");
}

static void chiudi_mappa() {

    if (conta_stanze() < 15) {
        printf("Errore: la mappa deve contenere almeno 15 stanze per essere chiusa.\n");
        return;
    }

    for  (int i = 0; i < n_giocatori; i++) {
        if (giocatori[i]) {
            giocatori[i]->posizione = pFirst;
        }
    }

    printf("La mappa e stata chiusa con successo.\n");
}

static int conta_stanze() {
    int count = 0; 
    struct Stanza* temp = pFirst;

    while (temp) {
        count++;
        if (temp->stanza_destra) temp = temp->stanza_destra;
        else if(temp->stanza_sinistra) temp = temp->stanza_sinistra;
        else if(temp->stanza_sopra) temp = temp->stanza_sopra;
        else if(temp->stanza_sotto) temp = temp->stanza_sotto;
        else break;
    }

    return count;
}


void inizializza_giocatori() {

    for (int i = 0; i < n_giocatori; i++) {
        giocatori[i] = malloc(sizeof(struct Giocatore));
        if (!giocatori[i]) {
            printf("Errore: allocazione memoria per il giocatore %d fallita.\n", i);
            exit(1);
        }

        // Inizializza i campi del giocatore
        snprintf(giocatori[i]->nome_giocatore, sizeof(giocatori[i]->nome_giocatore), "Giocatore%d", i + 1);
        giocatori[i]->p_vita = 3;
        giocatori[i]->p_vita_max = 3;
        giocatori[i]->dadi_attacco = 2;
        giocatori[i]->dadi_difesa = 2;
        giocatori[i]->posizione = pFirst;
        giocatori[i]->numero_scappate = 0;
        giocatori[i]->trabocchetto_ignorato = 0;
    }
}

void verifica_giocatori() {
    printf("Numero di giocatori: %d\n", n_giocatori);
    for (int i = 0; i < n_giocatori; i++) {
        if (giocatori[i]) {
            printf("Giocatore %d: %s, Vita: %d/%d, Posizione: %p\n",
                   i + 1, giocatori[i]->nome_giocatore,
                   giocatori[i]->p_vita, giocatori[i]->p_vita_max,
                   (void*)giocatori[i]->posizione);
        } else {
            printf("Giocatore %d: Non inizializzato.\n", i + 1);
        }
    }
}


void gioca() {
    if (!pFirst || n_giocatori == 0) {
        printf("Errore: il gioco non e stato impostato correttamente.\n");
        return;
    }

    //Nemico nemico_attuale = nessun_nemico; // Inizializza il nemico attuale
    printf("Inizia il gioco!\n");

    int turno = 0;
    while (1) {
        struct Giocatore* giocatore_corrente = giocatori[turno];

        // Controllo se il giocatore corrente e valido
        if (!giocatore_corrente) {
            printf("Errore: Giocatore non inizializzato al turno %d.\n", turno);
            

            //Controlla se esiste un giocatore valido
            int giocatori_validi = 0;
            for (int i = 0; i < n_giocatori; i++) {
                if (giocatori[i]) {
                    giocatori_validi++;
                }
            }

            if (giocatori_validi == 0) {
                printf("Errore critico: nessun giocatore iniziallizato\n");
                return;
            }

            turno = (turno + 1) % n_giocatori;
            continue;
        }

        printf("\nE il turno di %s:\n", giocatore_corrente->nome_giocatore);
        printf("1) Avanza\n2) Combatti\n3) Scappa\n4) Stampa Giocatore\n5) Stampa Zona\n6) Prendi Tesoro\n7) Cerca Porta Segreta\n8) Passa\n 9)DEBUG CHIUDI GIOCO\nScelta: ");

        int scelta;
        if (scanf("%d", &scelta) != 1 || scelta < 1 || scelta > 9) {
            printf("Errore: Scelta non valida.\n");
            while (getchar() != '\n'); // Pulisce il buffer
            continue;
        }

        switch (scelta) {
            case 1:
                    //printf("DEBUG: Chiamata alla funzione avanza()\n");
                avanza(giocatore_corrente);
                break;
            
            case 2:
                if (nemico_attuale != nessun_nemico) {
                        //printf("DEBUG: Chiamata alla funzione combatti()\n");
                    combatti(giocatore_corrente, nemico_attuale);
                    nemico_attuale = nessun_nemico; // Reset dopo combattimento
                } else {
                    printf("Non ci sono nemici da combattere.\n");
                }
                break;
            
            case 3:
                    //printf("DEBUG: Chiamata alla funzione scappa()\n");
                scappa(giocatore_corrente);
                break;
            
            case 4:
                    //printf("DEBUG: Chiamata alla funzione stampa_giocatore()\n");
                stampa_giocatore(giocatore_corrente);
                break;
            
            case 5:
                    //printf("DEBUG: Chiamata alla funzione stampa_zona()\n");
                stampa_zona(giocatore_corrente->posizione);
                break;
             
            case 6:
                    //printf("DEBUG: Chiamata alla funzione prendi_tesoro()\n");
                prendi_tesoro(giocatore_corrente);
                break;
            
            case 7:
                    //printf("DEBUG: Chiamata alla funzione cerca_porta_segreta()\n");
                cerca_porta_segreta(giocatore_corrente);
                break;
            
            case 8:
                    //printf("DEBUG: Chiamata alla funzione passa()\n");
                passa();
                break;

            case 9:
                return;
            
            default:
                printf("Scelta non valida!\n");
        }

        // Controllo per fine partita
        if (giocatore_corrente->posizione && 
            !giocatore_corrente->posizione->stanza_destra && 
            !giocatore_corrente->posizione->stanza_sinistra && 
            !giocatore_corrente->posizione->stanza_sopra && 
            !giocatore_corrente->posizione->stanza_sotto) {

            printf("Hai trovato Jaffar!\n");
            combatti(giocatore_corrente, jaffar);

            if (giocatore_corrente->p_vita > 0) {
                printf("Jaffar e stato sconfitto dal vincitore %s!\n", giocatore_corrente->nome_giocatore);
                salva_vincitore(giocatore_corrente->nome_giocatore);
                return;
            } else {
                printf("%s e caduto combattendo Jaffar. La partita continua.\n", giocatore_corrente->nome_giocatore);
                turno = (turno + 1) % n_giocatori;
                continue;
            }
        }

        // Controllo se tutti i giocatori sono stati eliminati
        int giocatori_vivi = 0;
        for (int i = 0; i < n_giocatori; i++) {
            if (giocatori[i]->p_vita > 0) {
                giocatori_vivi++;
            }
        }

        if (giocatori_vivi == 0) {
            printf("Tutti i Giocatori sono stati eliminati. Fine partita.\n");
            
            //RIpristino punti vita
            for (int i = 0; i < n_giocatori; i++) {
                if (giocatori[i]) {
                    giocatori[i]->p_vita = giocatori[i]->p_vita_max;
                }
            }
            return;
        }

        turno = (turno + 1) % n_giocatori;
    }

    printf("Fine del gioco\n");
}

void dealloca_stanze(struct Stanza* stanza) {
    while (stanza != NULL) {
        struct Stanza* stanza_successiva = NULL;

        //Segue le stanze
        if (stanza->stanza_destra) {
            stanza_successiva = stanza->stanza_destra;
        }
        else if (stanza->stanza_sinistra) {
            stanza_successiva = stanza->stanza_sinistra;
        }
        else if (stanza->stanza_sopra) {
            stanza_successiva = stanza->stanza_sopra;
        }
        else if(stanza->stanza_sotto) {
            stanza_successiva = stanza->stanza_sotto;
        }

        free(stanza);
        stanza = stanza_successiva;
    }
}

void combatti(struct Giocatore* giocatore, Nemico tipo_nemico) {
    if (!giocatore) {
        printf("Errore: Giocatore non valido.\n");
        return;
    }

    int dadi_attacco_nemico = 0, dadi_difesa_nemico = 0, punti_vita_nemico = 0;
    const char* nome_nemico;

    switch (tipo_nemico) {
        case scheletro:
            dadi_attacco_nemico = 1;
            dadi_difesa_nemico = 1;
            punti_vita_nemico = 1;
            nome_nemico = "Scheletro";
            break;

        case guardia:
            dadi_attacco_nemico = 2;
            dadi_difesa_nemico = 2;
            punti_vita_nemico = 2;
            nome_nemico = "Guardia";
            break;

        case jaffar:
        dadi_attacco_nemico = 3;
            dadi_difesa_nemico = 2;
            punti_vita_nemico = 3;
            nome_nemico = "Jaffar";
            break;

        default:
            printf("Errore: tipo nemico errato");
            return;
    }

printf("Inizio combattimento contro %s!\n", nome_nemico);

while (giocatore->p_vita > 0 && punti_vita_nemico > 0) {
    int attacca_prima = rand() % 2;
    if (attacca_prima) {
        printf("%s attacca!\n", giocatore->nome_giocatore);
        int colpi_riusciti = 0;
        for (int i = 0; i < giocatore->dadi_attacco; i++) {
            int dado = rand() % 6 + 1;
            if (dado == 6) {
                colpi_riusciti += 2; 
            }
            else if (dado >= 4) {
                colpi_riusciti++;
            }
        }

        int difese_nemico = 0;
for (int i = 0; i < dadi_difesa_nemico; i++) {
    int dado = rand() % 6 + 1;
    if (dado == 6) {
        difese_nemico += 2;
    }
    else if (dado >= 4){
        difese_nemico++;
    }
    
}

int danni = colpi_riusciti - difese_nemico;
if(danni > 0) {
    punti_vita_nemico -= danni;
    printf("%s infligge %d danni al nemico. Vita rimanente: %d\n", giocatore->nome_giocatore, danni, punti_vita_nemico);
}
else {
    printf("Il nemico para tutti gli attacchi.\n");
}
    }
else {
    printf("Il nemico attacca!\n");
    int colpi_riusciti = 0;
        for (int i = 0; i < dadi_attacco_nemico; i++) {
            int dado = rand() % 6 + 1;
            if (dado == 6) {
                colpi_riusciti += 2; 
            }
            else if (dado >= 4) {
                colpi_riusciti++;
    }
}

int difese_giocatore = 0;
for (int i = 0; i < giocatore->dadi_difesa; i++) {
    int dado = rand() % 6 + 1;
            if (dado == 6) {
                difese_giocatore += 2; 
            }
            else if (dado >= 4) {
                difese_giocatore++;
    }
}

int danni = colpi_riusciti - difese_giocatore;
if (danni > 0) {
    giocatore->p_vita -= danni;
    printf("Il nemico infligge %d danni a %s. Vita rimanente %d\n", danni, giocatore->nome_giocatore, giocatore->p_vita);
        }
    }
}

if (giocatore->p_vita > 0) {
    printf("%s ha vinto il combattimento\n", giocatore->nome_giocatore);
}
else {
    printf("%s e stato sconfitto,\n", giocatore->nome_giocatore);
}

}

void prendi_tesoro(struct Giocatore* giocatore) {
    if (!giocatore || !giocatore->posizione) {
        printf("Errore: giocatore o posizione non validi.\n");
        return;
    }
    struct Stanza* stanza_corrente = giocatore->posizione;

    if (stanza_corrente->tesoro == nessun_tesoro) {
        printf("Non c'e essun tesoro nella stanza!\n");
        return;
    }

    switch (stanza_corrente->tesoro) {
        case verde_veleno:
            printf("Hai trovato un tesoro: Veleno Verde!\n");
            giocatore->p_vita--;
            if (giocatore->p_vita < 0) {
                giocatore->p_vita = 0;
            }
            printf("La tua vita e diminuita a %d\n", giocatore->p_vita);
            break;

        case blu_guarigione:
            printf("Hai trovato un tesoro: Guarigione Blu!\n");
            giocatore->p_vita++;
            if (giocatore->p_vita > giocatore->p_vita_max) {
                giocatore->p_vita = giocatore->p_vita_max;
            }
            printf("La tua vita e aumentata a %d\n", giocatore->p_vita);
            break;

        case rosso_aumenta_vita:
            printf("Hai trovato un tesoro: Aumento Vita Rosso!\n");
            giocatore->p_vita_max++;
            giocatore->p_vita = giocatore->p_vita_max;
            printf("La tua vita e aumentata a %d. La tua vita attuale e %d.\n", giocatore->p_vita_max, giocatore->p_vita);
            break;

        case spada_tagliente:
            printf("Hai trovato un tesoro: Spada Tagliente!\n");
            giocatore->dadi_attacco++;
            printf("Ora hai %d dadi di attacco.\n", giocatore->dadi_attacco);
            break;

        case scudo:
            printf("Hai trovato un tesoro: Scudo!\n");
            giocatore->dadi_difesa++;
            printf("Ora hai %d dadi di difesa.\n", giocatore->dadi_difesa);
            break;

        default:
            printf("Tesoro sconosciuto. Nessun effetto applicato\n");
            break;
    }

    //Rimozzione tesoro dalla stanza
    stanza_corrente->tesoro = nessun_tesoro;
    printf("Il tesoro e stato preso\n");

}

void cerca_porta_segreta(struct Giocatore* giocatore) {
    if (!giocatore || !giocatore->posizione) {
        printf("Errore: Giocatore o posizione non validi.\n");
        return;
    }

    struct Stanza* stanza_corrente = giocatore->posizione;
    int probabilita[] = {33, 20, 15};
    int dir_disp = 0;

    //Controllo direzioni inaccessibili
    if (!stanza_corrente->stanza_destra) dir_disp++;
    if (!stanza_corrente->stanza_sinistra) dir_disp++;
    if (!stanza_corrente->stanza_sopra) dir_disp++;
    if (!stanza_corrente->stanza_sotto) dir_disp++;

    if (dir_disp == 0) {
        printf("Non ci sono direzioni disponibili per una porta segreta.\n");
        return;
    }

    static int tentativi = 0;
    if (tentativi >= 3) {
        printf("Hai gia cercato ogni posto possibile per una stanza segreta.\n");
        return;
    }

    //Determinazione porta segreta
    int chance = rand() % 100;
    printf("Tentativo %d: probabilita di successo %d%%.\n", tentativi + 1, probabilita[tentativi]);

    if (chance < probabilita[tentativi]) {
        printf("Hai trovato una porta segreta!\n");

        //Generazione stanza segreta
        struct Stanza* nuova_stanza = malloc(sizeof(struct Stanza));
        if (!nuova_stanza) {
            printf("Errore: memoria insufficiente per creare una nuova stanza.\n");
            return;
        }

        nuova_stanza->stanza_destra = NULL;
        nuova_stanza->stanza_sinistra = NULL;
        nuova_stanza->stanza_sopra = NULL;
        nuova_stanza->stanza_sotto = NULL;
        nuova_stanza->stanza_destra = nessuno; //trabocchetto

        //Aggiunta tesoro casuale
        int tesoro_segreto = rand() % 5;
        switch (tesoro_segreto) {
        case 0:
            printf("Hai trovato un tesoro: Veleno Verde!\n");
            giocatore->p_vita--;
            if (giocatore->p_vita < 0) {
                giocatore->p_vita = 0;
            }
            printf("La tua vita e diminuita a %d\n", giocatore->p_vita);
            break;

        case 1:
            printf("Hai trovato un tesoro: Guarigione Blu!\n");
            giocatore->p_vita++;
            if (giocatore->p_vita > giocatore->p_vita_max) {
                giocatore->p_vita = giocatore->p_vita_max;
            }
            printf("La tua vita e aumentata a %d\n", giocatore->p_vita);
            break;

        case 2:
            printf("Hai trovato un tesoro: Aumento Vita Rosso!\n");
            giocatore->p_vita_max++;
            giocatore->p_vita = giocatore->p_vita_max;
            printf("La tua vita e aumentata a %d. La tua vita attuale e %d.\n", giocatore->p_vita_max, giocatore->p_vita);
            break;

        case 3:
            printf("Hai trovato un tesoro: Spada Tagliente!\n");
            giocatore->dadi_attacco++;
            printf("Ora hai %d dadi di attacco.\n", giocatore->dadi_attacco);
            break;

        case 4:
            printf("Hai trovato un tesoro: Scudo!\n");
            giocatore->dadi_difesa++;
            printf("Ora hai %d dadi di difesa.\n", giocatore->dadi_difesa);
            break;

        default:
            printf("Tesoro sconosciuto. Nessun effetto applicato\n");
            break;
    }

    //Consumo del tesoro
    nuova_stanza->tesoro = nessun_tesoro;
    printf("Il tesoro e stato cosnumato");

    //Collegamento stanza segreta a stanza corrente
    if (!stanza_corrente->stanza_destra) {
        stanza_corrente->stanza_destra = nuova_stanza;
        printf("la porta segreta conduce a destra!\n");
    }
    else if (!stanza_corrente->stanza_sinistra) {
        stanza_corrente->stanza_precedente = nuova_stanza;
        printf("la porta segreta conduce a sinistra!\n");
    }
    else if(!stanza_corrente->stanza_sopra) {
        stanza_corrente->stanza_sopra = nuova_stanza;
        printf("la porta segreta conduce sopra!\n");
    }

    //Giocatore si sposta nella stanza segreta
    giocatore->posizione = nuova_stanza;
    printf("Ti sposti nella stanza segreta.\n");

    //Ritorno alla stanza iniziale
    giocatore->posizione = nuova_stanza;
    printf("Sei tornato alla stanza originale");
    }
    else {
        printf("Non hai trovato nessuna porta segreta in questa direzioe.\n");

        tentativi++;
    }
}

void termina_gioco() {
    printf("Grazie per aver giocato! Un saluto ai giocatori:\n");

    //stampa giocatori
    for (int i = 0; i < n_giocatori; i++) {
        if (giocatori[i] != NULL) {
            printf("- %s\n", giocatori[i]->nome_giocatore);
        }
    }

    //deallocazione stanze
    dealloca_stanze(pFirst);
    pFirst = NULL;

    //deallocazione giocatori
    for (int i = 0; i < n_giocatori; i++) {
        free(giocatori[i]);
        giocatori[i] = NULL;
    }
    n_giocatori = 0;

    printf("Memoria deallocata con successo!\n");
}

void crediti() {
    printf("\n--- Crediti ---\n");
    printf("Creatore del gioco: Filippo Castagnola");
    printf("\nUltimi Vincitori:\n");
    for (int i = 0; i < MAX_VINCITORI; i++) {
        if (strlen(ultimi_vincitori[i]) > 0) {
            printf("%d) %s\n", i + 1, ultimi_vincitori[i]);
        }
        else {
            printf("%d) Nessun vincitore ancora registrato\n", i + 1);
        }
    }
}

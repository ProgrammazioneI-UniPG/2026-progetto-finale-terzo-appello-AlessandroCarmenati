#include "gamelib.h"


static struct Giocatore* array_giocatori[4] = {NULL, NULL, NULL, NULL};
static int n_giocatori = 0;

static struct Zona_mondoreale* prima_zona_mondoreale = NULL;
static struct Zona_soprasotto* prima_zona_soprasotto = NULL;

static int mappa_pronta = 0;
static int undici_usata = 0;

static int game_over = 0;
static char vincitore[50] = "";
static int giocatori_vivi = 0;

static char ultimi_vincitori[3][50] = {"-", "-", "-"};

static const char* nome_z(Tipo_zona t);
static const char* nome_n(Tipo_nemico n);
static const char* nome_o(Tipo_oggetto o);

static void passa(int* turno_ok);
static void stampa_giocatore(struct Giocatore* g);
static void stampa_zona_gioco(struct Giocatore* g);
static void riassunto_zona(struct Giocatore* g);

static int nemico_blocca(struct Giocatore* g);

static void genera_mappa();
static void inserisci_zona();
static void cancella_zona();
static void stampa_mappa();
static void stampa_zona_setup();
static void chiudi_mappa();
static void pulisci_mappa();

static void pulisci_giocatori();
static void salva_vincitore(const char* nome);

static void avanza(struct Giocatore* g, int* ha_avanzato);
static void indietreggia(struct Giocatore* g);
static void cambia_mondo(struct Giocatore* g, int* ha_avanzato);
static void combatti(struct Giocatore* g);
static void raccogli_oggetto(struct Giocatore* g);
static void utilizza_oggetto(struct Giocatore* g);
static void compatta_zaino(struct Giocatore* g);

typedef struct {
    int atk;
    int def;
    int hp;
} StatNemico;

static const char* nome_nemico(Tipo_nemico n);
static StatNemico stat_nemico(Tipo_nemico n);

static const char* nome_z(Tipo_zona t) {
    const char* n[] = {"Bosco", "Scuola", "Laboratorio", "Caverna", "Strada", "Giardino", "Supermercato", "Centrale", "Deposito", "Polizia"};
    return n[t];
}

static const char* nome_n(Tipo_nemico n) {
    const char* nn[] = {"Nessuno", "Billi", "Democane", "Demotorzone"};
    return nn[n];
}

static const char* nome_o(Tipo_oggetto o) {
    const char* no[] = {"Nessuno", "Bicicletta", "Maglietta Fuocoinferno", "Bussola", "Schitarrata Metallica"};
    return no[o];
}

static void passa(int* turno_ok) {
    printf("Turno passato.\n");
    *turno_ok = 1;
}

static void stampa_giocatore(struct Giocatore* g) {
    if (g == NULL) {
        printf("Giocatore non valido.\n");
        return;
    }

    printf("\n--- STATO GIOCATORE ---\n");
    printf("Nome: %s\n", g->nome);
    printf("Mondo: %s\n", g->mondo == 0 ? "Reale" : "Soprasotto");
    printf("HP: %d\n", g->vite);
    printf("Attacco: %d\n", g->attacco_pischico);
    printf("Difesa: %d\n", g->difesa_pischica);
    printf("Fortuna: %d\n", g->fortuna);

    printf("Zaino:\n");
    for (int i = 0; i < 3; i++) {
        printf("  Slot %d: %s\n", i+1, nome_o(g->zaino[i]));
    }

    printf("------------------------\n");
}

static void stampa_zona_gioco(struct Giocatore* g) {
    if (!g) return;

    printf("\n--- ZONA CORRENTE ---\n");
    if (g->mondo == 0) {
        struct Zona_mondoreale* z = g->pos_mondoreale;
        printf("Mondo: Reale\n");
        printf("Zona: %s\n", nome_z(z->tipo));
        printf("Nemico: %s\n", nome_n(z->nemico));
        printf("Oggetto: %s\n", nome_o(z->oggetto));
        
    } else {
        struct Zona_soprasotto* z = g->pos_soprasotto;
        printf("Mondo: Soprasotto\n");
        printf("Zona: %s\n", nome_z(z->tipo));
        printf("Nemico: %s\n", nome_n(z->nemico));
        
    }
}

static void riassunto_zona(struct Giocatore* g) {
    if (!g) return;

    if (g->mondo == 0) {
        struct Zona_mondoreale* z = g->pos_mondoreale;
        printf("Sei nel REALE | Zona: %s | Nemico: %s | Oggetto: %s\n",
               nome_z(z->tipo), nome_n(z->nemico), nome_o(z->oggetto));
    } else {
        struct Zona_soprasotto* z = g->pos_soprasotto;
        printf("Sei nel SOPRASOTTO | Zona: %s | Nemico: %s\n",
               nome_z(z->tipo), nome_n(z->nemico));
    }
}

static int nemico_blocca(struct Giocatore* g) {
    if (g->mondo == 0) {
        Tipo_nemico n = g->pos_mondoreale->nemico;
        unsigned char m = g->pos_mondoreale->sconfitto_mask;

        if (n == nessun_nemico) return 0;
        return ((m & (1u << g->id)) == 0);
    } else {
        Tipo_nemico n = g->pos_soprasotto->nemico;
        unsigned char m = g->pos_soprasotto->sconfitto_mask;
        if (n == nessun_nemico) return 0;
        return ((m & (1u << g->id)) == 0);
    }
}

void imposta_gioco() {
    pulisci_mappa();
    pulisci_giocatori();
    undici_usata = 0;
    mappa_pronta = 0;

    printf("Numero giocatori (1-4): ");
    if (scanf("%d", &n_giocatori) != 1) {
        while (getchar() != '\n');
        n_giocatori = 1;
    }
    if (n_giocatori < 1) n_giocatori = 1;
    if (n_giocatori > 4) n_giocatori = 4;

    for (int i = 0; i < n_giocatori; i++) {
        array_giocatori[i] = malloc(sizeof(struct Giocatore));
        if (!array_giocatori[i]) {
            printf("Errore malloc giocatore.\n");
            pulisci_giocatori();
            return;
        }

        array_giocatori[i]->id = i;
        array_giocatori[i]->mondo = 0;

        printf("Nome G%d: ", i + 1);
        scanf("%49s", array_giocatori[i]->nome);

        array_giocatori[i]->attacco_pischico = (rand() % 20) + 1;
        array_giocatori[i]->difesa_pischica  = (rand() % 20) + 1;
        array_giocatori[i]->fortuna          = (rand() % 20) + 1;

        array_giocatori[i]->vite = 12; //hp iniziali

        for (int k = 0; k < 3; k++) array_giocatori[i]->zaino[k] = nessun_oggetto;

        printf("Bonus per %s:\n"
               "1. +3 ATK / -3 DEF\n"
               "2. -3 ATK / +3 DEF\n"
               "3. UndiciVirgolaCinque (solo una volta)\n"
               "4. Base\n"
               "Scelta: ", array_giocatori[i]->nome);

        int m = 4;
        if (scanf("%d", &m) != 1) { while(getchar() != '\n'); m = 4; }

        if (m == 1) {
            array_giocatori[i]->attacco_pischico += 3;
            array_giocatori[i]->difesa_pischica  -= 3;
        } else if (m == 2) {
            array_giocatori[i]->attacco_pischico -= 3;
            array_giocatori[i]->difesa_pischica  += 3;
        } else if (m == 3) {
            if (!undici_usata) {
                array_giocatori[i]->attacco_pischico += 4;
                array_giocatori[i]->difesa_pischica  += 4;
                array_giocatori[i]->fortuna          -= 7;
                if (array_giocatori[i]->fortuna < 1) array_giocatori[i]->fortuna = 1;

                undici_usata = 1;
                strcpy(array_giocatori[i]->nome, "UndiciVirgolaCinque");
            } else {
                printf("UndiciVirgolaCinque gia scelto: bonus Base assegnato.\n");
            }
        }

        if (array_giocatori[i]->attacco_pischico < 1) array_giocatori[i]->attacco_pischico = 1;
        if (array_giocatori[i]->difesa_pischica  < 1) array_giocatori[i]->difesa_pischica  = 1;
        if (array_giocatori[i]->fortuna          < 1) array_giocatori[i]->fortuna          = 1;

        printf("Creato: %s | HP:%d ATK:%d DEF:%d FORT:%d\n",
               array_giocatori[i]->nome,
               array_giocatori[i]->vite,
               array_giocatori[i]->attacco_pischico,
               array_giocatori[i]->difesa_pischica,
               array_giocatori[i]->fortuna);
    }

    int m_s;
    do {
        printf("\n--- MAPPA ---\n"
               "1. Genera 15 zone\n"
               "2. Inserisci\n"
               "3. Cancella\n"
               "4. Stampa\n"
               "5. Chiudi\n"
               "6. Stampa zona (posizione i)\n"
               "Scelta: ");

        if (scanf("%d", &m_s) != 1) {
            while (getchar() != '\n');
            continue;
        }

        switch (m_s) {
            case 1:
                genera_mappa();
                mappa_pronta = 0;
                break;

            case 2:
                inserisci_zona();
                mappa_pronta = 0;
                break;

            case 3:
                cancella_zona();
                mappa_pronta = 0;
                break;

            case 4:
                stampa_mappa();
                break;

            case 5:
                chiudi_mappa();
                break;

            case 6:
                stampa_zona_setup();
                break;

            default:
                printf("Comando non valido.\n");
                break;
        }
    } while (!mappa_pronta);
}

void gioca() {
    if (!mappa_pronta) {
        printf("Prima devi impostare e chiudere la mappa.\n");
        return;
    }

    game_over = 0;
    vincitore[0] = '\0';

    for (int i = 0; i < n_giocatori; i++) {
        if (array_giocatori[i] != NULL) {
            array_giocatori[i]->pos_mondoreale = prima_zona_mondoreale;
            array_giocatori[i]->pos_soprasotto = prima_zona_soprasotto;
            array_giocatori[i]->mondo = 0;
        }
    }

    giocatori_vivi = 0;
    for (int i = 0; i < n_giocatori; i++) {
        if (array_giocatori[i] != NULL) giocatori_vivi++;
    }

    while (!game_over) {

        if (giocatori_vivi == 0) {
            printf("\nTutti i giocatori sono morti. GAME OVER.\n");
            game_over = 1;
            break;
        }

        int ordine[4];
        int k = 0;
        for (int i = 0; i < n_giocatori; i++) {
            if (array_giocatori[i] != NULL) {
                ordine[k++] = i;
            }
        }

        for (int i = k - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int t = ordine[i];
            ordine[i] = ordine[j];
            ordine[j] = t;
        }

        for (int idx = 0; idx < k && !game_over; idx++) {
            int gi = ordine[idx];
            struct Giocatore* g = array_giocatori[gi];
            if (g == NULL) continue;

            int turno_ok = 0;
            int avanzato = 0;

            printf("\n========================\n");
            printf("Turno di %s\n", g->nome);
            stampa_giocatore(g);
            stampa_zona_gioco(g);

            while (!turno_ok && !game_over) {
                printf("\nAzioni: 1.Av 2.Ind 3.Mondo 4.Comb 5.Racc 6.Usa 7.Passa\nScelta: ");

                int s;
                if (scanf("%d", &s) != 1) {
                    while (getchar() != '\n');
                    continue;
                }

                switch (s) {
                    case 1: avanza(g, &avanzato); if (!game_over && array_giocatori[gi] != NULL) riassunto_zona(array_giocatori[gi]); break;
                    case 2: indietreggia(g); if (!game_over && array_giocatori[gi] != NULL) riassunto_zona(array_giocatori[gi]); break;
                    case 3: cambia_mondo(g, &avanzato); if (!game_over && array_giocatori[gi] != NULL) riassunto_zona(array_giocatori[gi]); break;
                    case 4: combatti(g); break;
                    case 5: raccogli_oggetto(g); break;
                    case 6: utilizza_oggetto(g); break;
                    case 7: passa(&turno_ok); break;
                    default: printf("Comando non valido.\n"); break;
                }

                g = array_giocatori[gi];
                if (g == NULL) {
                    turno_ok = 1;
                }
            }
        }
    }

    if (vincitore[0] != '\0') {
        printf("\n=== PARTITA FINITA ===\nVincitore: %s\n", vincitore);
    } else {
        printf("\n=== PARTITA FINITA ===\nNessun vincitore.\n");
    }
}

static void genera_mappa() {
    pulisci_mappa();

    struct Zona_mondoreale* last_r = NULL;
    struct Zona_soprasotto* last_s = NULL;

    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;

    for (int i = 0; i < 15; i++) {
        struct Zona_mondoreale* nr = malloc(sizeof(struct Zona_mondoreale));
        struct Zona_soprasotto* ns = malloc(sizeof(struct Zona_soprasotto));

        if (!nr || !ns) {
            printf("Errore malloc durante la generazione mappa.\n");
            if (nr) free(nr);
            if (ns) free(ns);
            pulisci_mappa();
            return;
        }

        nr->tipo = (Tipo_zona)(rand() % 10);
        ns->tipo = nr->tipo;

        nr->oggetto = (rand() % 100 < 40) ? (Tipo_oggetto)((rand() % 4) + 1) : nessun_oggetto;

        nr->sconfitto_mask = 0;
        ns->sconfitto_mask = 0;

        nr->nemico = nessun_nemico;
        ns->nemico = nessun_nemico;

        if (rand() % 100 < 30) {
            nr->nemico = (rand() % 2 == 0) ? billi : democane;
        }

        if (rand() % 100 < 30) {
            ns->nemico = democane;
        }

        nr->avanti = NULL;
        nr->indietro = last_r;
        nr->link_soprasotto = ns;

        ns->avanti = NULL;
        ns->indietro = last_s;
        ns->link_mondoreale = nr;

        if (prima_zona_mondoreale == NULL) prima_zona_mondoreale = nr;
        else last_r->avanti = nr;

        if (prima_zona_soprasotto == NULL) prima_zona_soprasotto = ns;
        else last_s->avanti = ns;

        last_r = nr;
        last_s = ns;
    }

    if (last_s) {
        last_s->nemico = demotorzone;
        last_s->sconfitto_mask = 0;
    }

    printf("Mappa creata! (15 zone)\n");
}

static void inserisci_zona() {
    int pos;

    printf("Posizione di inserimento (0 = inizio): ");
    if (scanf("%d", &pos) != 1 || pos < 0) {
        while (getchar() != '\n');
        printf("Posizione non valida.\n");
        return;
    }

    struct Zona_mondoreale* nr = malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* ns = malloc(sizeof(struct Zona_soprasotto));

    if (!nr || !ns) {
        printf("Errore malloc.\n");
        if (nr) free(nr);
        if (ns) free(ns);
        return;
    }

    int tipo;
    printf("Tipo zona (0-9): ");
    scanf("%d", &tipo);
    if (tipo < 0 || tipo > 9) tipo = 0;

    nr->tipo = (Tipo_zona)tipo;
    ns->tipo = nr->tipo;

    int ogg;
    printf("Oggetto Reale (0-4, 0=nessuno): ");
    scanf("%d", &ogg);
    if (ogg < 0 || ogg > 4) ogg = 0;
    nr->oggetto = (Tipo_oggetto)ogg;

    int nem_r;
    printf("Nemico Reale (0=nessuno, 1=billi, 2=democane): ");
    scanf("%d", &nem_r);
    if (nem_r < 0 || nem_r > 2) nem_r = 0;
    nr->nemico = (Tipo_nemico)nem_r;

    int nem_s;
    printf("Nemico Soprasotto (0=nessuno, 2=democane): ");
    scanf("%d", &nem_s);
    if (nem_s != 2) nem_s = 0;
    ns->nemico = (Tipo_nemico)nem_s;

    nr->sconfitto_mask = 0;
    ns->sconfitto_mask = 0;

    nr->link_soprasotto = ns;
    ns->link_mondoreale = nr;

    if (pos == 0 || prima_zona_mondoreale == NULL) {
        nr->avanti = prima_zona_mondoreale;
        nr->indietro = NULL;
        if (prima_zona_mondoreale)
            prima_zona_mondoreale->indietro = nr;
        prima_zona_mondoreale = nr;

        ns->avanti = prima_zona_soprasotto;
        ns->indietro = NULL;
        if (prima_zona_soprasotto)
            prima_zona_soprasotto->indietro = ns;
        prima_zona_soprasotto = ns;
    } else {
        struct Zona_mondoreale* curr_r = prima_zona_mondoreale;
        struct Zona_soprasotto* curr_s = prima_zona_soprasotto;

        int i = 0;
        while (curr_r->avanti && i < pos - 1) {
            curr_r = curr_r->avanti;
            curr_s = curr_s->avanti;
            i++;
        }

        nr->avanti = curr_r->avanti;
        nr->indietro = curr_r;
        if (curr_r->avanti)
            curr_r->avanti->indietro = nr;
        curr_r->avanti = nr;

        ns->avanti = curr_s->avanti;
        ns->indietro = curr_s;
        if (curr_s->avanti)
            curr_s->avanti->indietro = ns;
        curr_s->avanti = ns;
    }

    printf("Zona inserita correttamente.\n");
}

static void cancella_zona() {
    int pos;
    printf("Posizione da cancellare (0 = prima): ");
    if (scanf("%d", &pos) != 1 || pos < 0) {
        while (getchar() != '\n');
        printf("Posizione non valida.\n");
        return;
    }

    if (prima_zona_mondoreale == NULL || prima_zona_soprasotto == NULL) {
        printf("Mappa vuota.\n");
        return;
    }

    struct Zona_mondoreale* curr_r = prima_zona_mondoreale;
    struct Zona_soprasotto* curr_s = prima_zona_soprasotto;

    int i = 0;
    while (curr_r != NULL && curr_s != NULL && i < pos) {
        curr_r = curr_r->avanti;
        curr_s = curr_s->avanti;
        i++;
    }

    if (curr_r == NULL || curr_s == NULL) {
        printf("Posizione fuori range.\n");
        return;
    }

    if (curr_s->nemico == demotorzone) {
        printf("Non puoi cancellare la zona che contiene Demotorzone.\n");
        return;
    }

    if (curr_r->indietro) curr_r->indietro->avanti = curr_r->avanti;
    else prima_zona_mondoreale = curr_r->avanti;

    if (curr_r->avanti) curr_r->avanti->indietro = curr_r->indietro;

    if (curr_s->indietro) curr_s->indietro->avanti = curr_s->avanti;
    else prima_zona_soprasotto = curr_s->avanti;

    if (curr_s->avanti) curr_s->avanti->indietro = curr_s->indietro;

    curr_r->link_soprasotto = NULL;
    curr_s->link_mondoreale = NULL;

    free(curr_r);
    free(curr_s);

    printf("Zona in posizione %d cancellata.\n", pos);

    mappa_pronta = 0;
}

static void stampa_mappa() {
    if (prima_zona_mondoreale == NULL || prima_zona_soprasotto == NULL) {
        printf("Mappa vuota.\n");
        return;
    }

    int scelta;
    printf("Stampa mappa:\n1) Mondo Reale\n2) Soprasotto\nScelta: ");
    if (scanf("%d", &scelta) != 1) {
        while (getchar() != '\n');
        printf("Input non valido.\n");
        return;
    }

    if (scelta == 1) {
        printf("\n--- MAPPA MONDO REALE ---\n");
        struct Zona_mondoreale* z = prima_zona_mondoreale;
        int i = 0;

        while (z != NULL) {
            printf("[%02d] Zona: %s | Nemico: %s | Oggetto: %s\n",
                   i,
                   nome_z(z->tipo),
                   nome_n(z->nemico),
                   nome_o(z->oggetto));
            z = z->avanti;
            i++;
        }

    } else if (scelta == 2) {
        printf("\n--- MAPPA SOPRASOTTO ---\n");
        struct Zona_soprasotto* z = prima_zona_soprasotto;
        int i = 0;

        while (z != NULL) {
            printf("[%02d] Zona: %s | Nemico: %s\n",
                   i,
                   nome_z(z->tipo),
                   nome_n(z->nemico));
            z = z->avanti;
            i++;
        }

    } else {
        printf("Scelta non valida.\n");
        return;
    }
}


static void stampa_zona_setup() {
    if (prima_zona_mondoreale == NULL || prima_zona_soprasotto == NULL) {
        printf("Mappa vuota.\n");
        return;
    }

    int pos;
    printf("Posizione zona da stampare (0 = prima): ");
    if (scanf("%d", &pos) != 1 || pos < 0) {
        while (getchar() != '\n');
        printf("Input non valido.\n");
        return;
    }

    struct Zona_mondoreale* zr = prima_zona_mondoreale;
    struct Zona_soprasotto* zs = prima_zona_soprasotto;

    int i = 0;
    while (zr && zs && i < pos) {
        zr = zr->avanti;
        zs = zs->avanti;
        i++;
    }

    if (!zr || !zs) {
        printf("Posizione fuori range.\n");
        return;
    }

    printf("\n--- ZONA (setup) posizione %d ---\n", pos);

    printf("[Reale] Zona: %s | Nemico: %s | Oggetto: %s\n",
           nome_z(zr->tipo), nome_n(zr->nemico), nome_o(zr->oggetto));

    printf("[Soprasotto] Zona: %s | Nemico: %s\n",
           nome_z(zs->tipo), nome_n(zs->nemico));
}

static void chiudi_mappa() {
    int c = 0, d = 0;
    struct Zona_mondoreale* t = prima_zona_mondoreale;
    struct Zona_soprasotto* ts = prima_zona_soprasotto;
    while(t) { c++; t = t->avanti; }
    while(ts) { if(ts->nemico == demotorzone) d++; ts = ts->avanti; }
    if (c >= 15 && d == 1) { mappa_pronta = 1; printf("Mappa OK!\n"); }
    else printf("Mappa non valida (richieste 15 zone e 1 Demotorzone).\n");
}

static void avanza(struct Giocatore* g, int* ha_avanzato) {
    if (*ha_avanzato) {
        printf("Hai gia mosso in questo turno!\n");
        return;
    }

    if (nemico_blocca(g)) {
        printf("C'e un nemico qui: combatti prima!\n");
        return;
    }

    if (g->mondo == 0) {
        if (g->pos_mondoreale->avanti == NULL) {
            printf("Non puoi avanzare oltre.\n");
            return;
        }
        g->pos_mondoreale = g->pos_mondoreale->avanti;
        g->pos_soprasotto = g->pos_soprasotto->avanti;
    } else {
        if (g->pos_soprasotto->avanti == NULL) {
            printf("Non puoi avanzare oltre.\n");
            return;
        }
        g->pos_soprasotto = g->pos_soprasotto->avanti;
        g->pos_mondoreale = g->pos_mondoreale->avanti;
    }

    *ha_avanzato = 1;
}

static void indietreggia(struct Giocatore* g) {
    if (nemico_blocca(g)) {
        printf("C'e un nemico qui: combatti prima!\n");
        return;
    }

    if (g->mondo == 0) {
        if (g->pos_mondoreale->indietro == NULL) {
            printf("Non puoi tornare indietro.\n");
            return;
        }
        g->pos_mondoreale = g->pos_mondoreale->indietro;
        g->pos_soprasotto = g->pos_soprasotto->indietro;
    } else {
        if (g->pos_soprasotto->indietro == NULL) {
            printf("Non puoi tornare indietro.\n");
            return;
        }
        g->pos_soprasotto = g->pos_soprasotto->indietro;
        g->pos_mondoreale = g->pos_mondoreale->indietro;
    }
}

static void cambia_mondo(struct Giocatore* g, int* ha_avanzato) {
    if (*ha_avanzato) {
        printf("Hai gia fatto un'azione di movimento in questo turno.\n");
        return;
    }

    Tipo_nemico n = (g->mondo == 0) ? g->pos_mondoreale->nemico : g->pos_soprasotto->nemico;
    if (n != nessun_nemico) {
        printf("C'e un nemico qui. Combatti prima!\n");
        return;
    }

    if (g->mondo == 0) {
        g->mondo = 1;
        *ha_avanzato = 1;
        printf("Passi nel Soprasotto.\n");
        return;
    }

    int tiro = (rand() % 20) + 1;
    *ha_avanzato = 1;
    if (tiro <= g->fortuna) {
        g->mondo = 0;
        printf("Riesci a tornare nel Mondo Reale! (tiro=%d)\n", tiro);
    } else {
        printf("Fallito il ritorno al Mondo Reale. (tiro=%d)\n", tiro);
    }
}

static const char* nome_nemico(Tipo_nemico n) {
    switch(n) {
        case billi: return "Billi";
        case democane: return "Democane";
        case demotorzone: return "Demotorzone";
        default: return "Nessuno";
    }
}

static StatNemico stat_nemico(Tipo_nemico n) {
    switch(n) {
        case billi:       return (StatNemico){ .atk=6,  .def=4,  .hp=8  };
        case democane:    return (StatNemico){ .atk=9,  .def=6,  .hp=12 };
        case demotorzone: return (StatNemico){ .atk=12, .def=8,  .hp=18 };
        default:          return (StatNemico){ .atk=0,  .def=0,  .hp=0  };
    }
}

static void combatti(struct Giocatore* g) {
    if (!g) return;

    Tipo_nemico n = (g->mondo == 0) ? g->pos_mondoreale->nemico : g->pos_soprasotto->nemico;

    if (n == nessun_nemico) {
        printf("Nessun nemico qui.\n");
        return;
    }

    StatNemico sn = stat_nemico(n);
    int hpG = g->vite;
    int hpN = sn.hp;

    printf("\n--- COMBATTIMENTO contro %s ---\n", nome_nemico(n));

    int fine = 0;
    while (!fine && !game_over) {

        printf("\n%s (HP:%d, ATK:%d, DEF:%d, FORT:%d) vs %s (HP:%d, ATK:%d, DEF:%d)\n",
               g->nome, hpG, g->attacco_pischico, g->difesa_pischica, g->fortuna,
               nome_nemico(n), hpN, sn.atk, sn.def);

        printf("1) Attacca  2) Difendi  3) Fuggi (usa Fortuna)\nScelta: ");
        int s;
        if (scanf("%d", &s) != 1) { while(getchar()!='\n'); continue; }

        if (s == 1) {
            int tiro = (rand() % 20) + 1;
            int danno = (tiro + g->attacco_pischico) - sn.def;
            if (danno < 0) danno = 0;

            hpN -= danno;
            printf("Colpisci! (tiro=%d) Danno=%d\n", tiro, danno);

            if (hpN <= 0) {
                printf("Hai sconfitto %s!\n", nome_nemico(n));

                if (g->mondo == 0) {
                    g->pos_mondoreale->sconfitto_mask |= (1u << g->id);

                    if ((rand() % 2) == 0) {
                        g->pos_mondoreale->nemico = nessun_nemico;
                        g->pos_mondoreale->sconfitto_mask = 0;
                        printf("Il nemico scompare dalla zona.\n");
                    } else {
                        printf("Il nemico resta nella zona (altri potrebbero rincontrarlo).\n");
                    }
                } else {
                    g->pos_soprasotto->sconfitto_mask |= (1u << g->id);

                    if ((rand() % 2) == 0) {
                        g->pos_soprasotto->nemico = nessun_nemico;
                        g->pos_soprasotto->sconfitto_mask = 0;
                        printf("Il nemico scompare dalla zona.\n");
                    } else {
                        printf("Il nemico resta nella zona (altri potrebbero rincontrarlo).\n");
                    }
                }

                if (n == demotorzone) {
                    strcpy(vincitore, g->nome);
                    salva_vincitore(g->nome);
                    game_over = 1;
                }

                fine = 1;
                break;
            }

        } else if (s == 2) {
            printf("Ti metti in difesa.\n");

        } else if (s == 3) {
            int tiro = (rand() % 20) + 1;
            if (tiro <= g->fortuna) {
                printf("Fuga riuscita! (tiro=%d)\n", tiro);
                fine = 1;
                break;
            } else {
                printf("Fuga fallita! (tiro=%d)\n", tiro);
            }

        } else {
            printf("Scelta non valida.\n");
            continue;
        }

        if (!fine) {
            int tiroN = (rand() % 20) + 1;
            int dannoN = (tiroN + sn.atk) - g->difesa_pischica;
            if (dannoN < 0) dannoN = 0;

            if (s == 2) dannoN /= 2;

            hpG -= dannoN;
            printf("%s ti attacca! (tiro=%d) Danno subito=%d\n", nome_nemico(n), tiroN, dannoN);

            if (hpG <= 0) {
                printf("%s e' morto in combattimento.\n", g->nome);

                for (int i = 0; i < n_giocatori; i++) {
                    if (array_giocatori[i] == g) {
                        free(array_giocatori[i]);
                        array_giocatori[i] = NULL;
                        giocatori_vivi--;
                        break;
                    }
                }

                if (giocatori_vivi == 0) game_over = 1;

                fine = 1;
                break;
            }
        }
    }

    for (int i = 0; i < n_giocatori; i++) {
        if (array_giocatori[i] == g) {
            g->vite = hpG;
            break;
        }
    }
}

static void raccogli_oggetto(struct Giocatore* g) {
    if (g->mondo != 0) {
        printf("Nel Soprasotto non ci sono oggetti da raccogliere.\n");
        return;
    }

    if (nemico_blocca(g)) {
        printf("C'e un nemico: prima combatti, poi puoi raccogliere.\n");
        return;
    }

    if (g->pos_mondoreale->oggetto == nessun_oggetto) {
        printf("Nessun oggetto qui.\n");
        return;
    }

    for (int i = 0; i < 3; i++) {
        if (g->zaino[i] == nessun_oggetto) {
            g->zaino[i] = g->pos_mondoreale->oggetto;
            printf("Raccolto: %s\n", nome_o(g->zaino[i]));
            g->pos_mondoreale->oggetto = nessun_oggetto;
            return;
        }
    }

    printf("Zaino pieno. Usa prima un oggetto.\n");
}

static void compatta_zaino(struct Giocatore* g) {
    Tipo_oggetto tmp[3] = {nessun_oggetto, nessun_oggetto, nessun_oggetto};
    int idx = 0;
    for(int i=0; i<3; i++) {
        if (g->zaino[i] != nessun_oggetto) tmp[idx++] = g->zaino[i];
    }
    for(int i=0; i<3; i++) g->zaino[i] = tmp[i];
}

static void utilizza_oggetto(struct Giocatore* g) {
    int vuoto = 1;
    for (int i = 0; i < 3; i++) {
        if (g->zaino[i] != nessun_oggetto) { vuoto = 0; break; }
    }
    if (vuoto) {
        printf("Zaino vuoto.\n");
        return;
    }

    printf("Zaino di %s:\n", g->nome);
    for (int i = 0; i < 3; i++) {
        printf("%d) %s\n", i + 1, nome_o(g->zaino[i]));
    }

    printf("Scegli slot da usare (1-3, 0 annulla): ");
    int scelta;
    if (scanf("%d", &scelta) != 1) { while(getchar()!='\n'); return; }
    if (scelta == 0) return;
    if (scelta < 1 || scelta > 3) {
        printf("Scelta non valida.\n");
        return;
    }

    int idx = scelta - 1;
    Tipo_oggetto o = g->zaino[idx];
    if (o == nessun_oggetto) {
        printf("Slot vuoto.\n");
        return;
    }

    switch (o) {
        case bicicletta:
            g->fortuna += 3;
            if (g->fortuna > 20) g->fortuna = 20;
            printf("Usi %s: Fortuna +3 (ora %d)\n", nome_o(o), g->fortuna);
            break;

        case maglietta_fuocoinferno:
            g->difesa_pischica += 4;
            if (g->difesa_pischica > 25) g->difesa_pischica = 25;
            printf("Usi %s: Difesa +4 (ora %d)\n", nome_o(o), g->difesa_pischica);
            break;

        case bussola: {
            printf("Usi %s: analizzi cosa c'e piu avanti...\n", nome_o(o));

            Tipo_nemico n_reale = nessun_nemico;
            Tipo_nemico n_sotto = nessun_nemico;

            if (g->pos_mondoreale && g->pos_mondoreale->avanti)
                n_reale = g->pos_mondoreale->avanti->nemico;
            if (g->pos_soprasotto && g->pos_soprasotto->avanti)
                n_sotto = g->pos_soprasotto->avanti->nemico;

            printf("Davanti (Reale): %s\n", nome_n(n_reale));
            printf("Davanti (Soprasotto): %s\n", nome_n(n_sotto));

            if (n_sotto == demotorzone) {
                printf("La bussola impazzisce... Demotorzone e vicino.\n");
            }
            break;
        }

        case schitarrata_metallica:
            g->attacco_pischico += 4;
            if (g->attacco_pischico > 25) g->attacco_pischico = 25;
            printf("Usi %s: Attacco +4 (ora %d)\n", nome_o(o), g->attacco_pischico);
            break;

        default:
            printf("Oggetto non riconosciuto.\n");
            return;
    }

    g->zaino[idx] = nessun_oggetto;
    compatta_zaino(g);
}

static void pulisci_mappa() {
    struct Zona_mondoreale* tr = prima_zona_mondoreale;
    while (tr) {
        struct Zona_mondoreale* p = tr->avanti;
        free(tr->link_soprasotto);
        free(tr);
        tr = p;
    }
    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;
}

static void pulisci_giocatori() {
    for(int i=0; i<4; i++) {
        if(array_giocatori[i]) {
            free(array_giocatori[i]);
            array_giocatori[i] = NULL;
        }
    }
    n_giocatori = 0;
}

static void salva_vincitore(const char* nome) {
    strncpy(ultimi_vincitori[2], ultimi_vincitori[1], 49);
    ultimi_vincitori[2][49] = '\0';

    strncpy(ultimi_vincitori[1], ultimi_vincitori[0], 49);
    ultimi_vincitori[1][49] = '\0';

    strncpy(ultimi_vincitori[0], nome, 49);
    ultimi_vincitori[0][49] = '\0';
}

void termina_gioco() {
    pulisci_mappa();
    pulisci_giocatori();
    exit(0);
}

void crediti() {
    printf("\n--- CREDITI ---\n");
    printf("Creatore: ALESSANDRO CARMENATI 392266 \n");
    printf("Ultimi 3 vincitori:\n");
    printf("1) %s\n", ultimi_vincitori[0]);
    printf("2) %s\n", ultimi_vincitori[1]);
    printf("3) %s\n", ultimi_vincitori[2]);
}

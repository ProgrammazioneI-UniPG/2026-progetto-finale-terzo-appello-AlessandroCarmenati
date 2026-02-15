#ifndef GAMELIB_H
#define GAMELIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { bosco, scuola, laboratorio, caverna, strada, giardino, supermercato, centrale_elettrica, deposito_abbandonato, stazione_polizia } Tipo_zona;
typedef enum { nessun_nemico, billi, democane, demotorzone } Tipo_nemico;
typedef enum { nessun_oggetto, bicicletta, maglietta_fuocoinferno, bussola, schitarrata_metallica } Tipo_oggetto;

struct Zona_mondoreale {
    Tipo_zona tipo;
    Tipo_nemico nemico;
    Tipo_oggetto oggetto;
    struct Zona_mondoreale *avanti, *indietro;
    struct Zona_soprasotto *link_soprasotto;
    unsigned char sconfitto_mask;
};

struct Zona_soprasotto {
    Tipo_zona tipo;
    Tipo_nemico nemico;
    struct Zona_soprasotto *avanti, *indietro;
    struct Zona_mondoreale *link_mondoreale;
    unsigned char sconfitto_mask;
};

struct Giocatore {
    char nome[50];
    int mondo; // 0 Reale, 1 Soprasotto
    struct Zona_mondoreale* pos_mondoreale;
    struct Zona_soprasotto* pos_soprasotto;
    int attacco_pischico, difesa_pischica, fortuna;
    Tipo_oggetto zaino[3];
    int vite;
    int id;
};

void imposta_gioco();
void gioca();
void termina_gioco();
void crediti();

#endif

[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/5fsIc7xe)
# Progetto-finale-2025-Cosestrane
Progetto finale Programmazione Procedurale UniPG Informatica

## Nome: Alessandro

## Cognome: Carmenati

## Matricola: 392266

## Commenti/modifiche al progetto: 

Il progetto implementa un gioco testuale basato su due mappe parallele (Mondo Reale e Soprasotto), ciascuna composta da 15 zone collegate tra loro in modo speculare tramite liste doppiamente collegate.

Ad ogni nuova generazione della mappa, le strutture precedenti vengono deallocate per evitare memory leak. Anche i giocatori sono allocati dinamicamente e vengono deallocati alla morte o alla reimpostazione del gioco.

È stato introdotto un sistema di combattimento che utilizza le caratteristiche attacco_pischico, difesa_pischica, fortuna e punti vita (HP). Ogni nemico possiede statistiche proprie. Il combattimento prevede attacco, difesa e fuga basata sulla fortuna.

Per gestire correttamente la presenza dei nemici in una zona è stata implementata una bitmask (sconfitto_mask) che consente a un giocatore di non essere più bloccato da un nemico già sconfitto, mantenendo però il nemico visibile per altri giocatori. Dopo la sconfitta, il nemico ha una probabilità del 50% di scomparire definitivamente.

È stata implementata la memorizzazione degli ultimi tre vincitori, visualizzabili tramite la funzione crediti(). I vincitori vengono salvati solo in caso di sconfitta del Demotorzone.

Sono stati inoltre aggiunti miglioramenti di usabilità:

stampa automatica dello stato del giocatore a inizio turno

riassunto della zona dopo ogni movimento

gestione degli input errati

protezione contro overflow nelle stringhe

Non è stata implementata persistenza su file: tutti i dati sono mantenuti in memoria durante l’esecuzione del programma.


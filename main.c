#include "gamelib.h"

int main() {
    int scelta;
    srand((unsigned) time(NULL));

    do {
        printf("\n--- COSESTRANE - MENU ---\n");
        printf("1. Imposta gioco\n2. Gioca\n3. Termina gioco\n4. Visualizza crediti\nScelta: ");
        
        if (scanf("%d", &scelta) != 1) {
            printf("Comando sbagliato!\n");
            while(getchar() != '\n'); 
            scelta = 0;
            continue;
        }

        switch(scelta) {
            case 1: imposta_gioco(); break;
            case 2: gioca(); break;
            case 3: termina_gioco(); break;
            case 4: crediti(); break;
            default: printf("Comando non valido. Riprova.\n");
        }
    } while (scelta != 3);
    return 0;
}

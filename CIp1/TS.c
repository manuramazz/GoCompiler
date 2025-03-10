#include "TS.h"
#include "definiciones.h"
#include <stdio.h>
TABB TS;

void inicializarTS() {
    crearAbb(&TS);
    char * keywords[] = {"package", "import", "go", "func", "var"};
    int numeros[] = {PACKAGE, IMPORT, GO, FUNC,VAR};
    for (int i = 0; i < 4; i++) {
        TIPOELEMENTOABB elem;
        elem.lexema = keywords[i];
        elem.numero = numeros[i];
        insertarElementoAbb(&TS, elem);
    }
}

void liberarTS() {
    destruirAbb(&TS);
}

void buscar(char * lexema, TIPOELEMENTOABB *elem) {
    buscarNodoAbb(TS, lexema, elem);
}

void insertar(TIPOELEMENTOABB elem) {
    insertarElementoAbb(&TS, elem);
}

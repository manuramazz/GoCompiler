#include <stdio.h>
#include <stdlib.h>
#include "TS.h"
#include "compSintactico.h"
#include "abb.h"

/*Inicializa la tabla de símbolos -> crear árbol binario
 *y del componente léxico -> abrir archivo y cargar primer bloque*/
void init() {
    inicializarTS();
    inicializacion();
}

//Libera la tabla de símbolos
void liberar() {
    liberarTS();
}

int main(void) {
    init();
    componenteSintactico();
    liberar();
    return 0;
}
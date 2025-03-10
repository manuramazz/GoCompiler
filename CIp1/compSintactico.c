//
// Created by User on 14/02/2025.
//

#include "compSintactico.h"

#include <stdio.h>
#include <stdlib.h>


void componenteSintactico() {
    COMPONENTE_LEXICO cl;
    while ((cl = sigComponenteLexico()).numero != EOF) {
        printf("Componente lexico: <%-4d, %s>\n", cl.numero,cl.lexema);
    }
}

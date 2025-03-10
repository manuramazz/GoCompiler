//
// Created by User on 19/02/2025.
//

#include "errores.h"

#include <stdio.h>

void error(int codigo) {
    switch (codigo) {
        case 1:
            perror("Error al abrir el archivo");
            break;
    }
}

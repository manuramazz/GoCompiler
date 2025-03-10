//
// Created by User on 14/02/2025.
//

#ifndef COMPLEXICO_H
#define COMPLEXICO_H
#include <stdio.h>

/* La estructura COMPONENTE_LEXICO representa un componente léxico
 * Está formada por un número y un lexema
 * numero es un int representa un código del tipo de componente léxico
 * lexema es un puntero a char que que contendrá la cadena de caracteres que representa el valor del componente léxico
 */
typedef struct {
    int numero;
    char * lexema;
}COMPONENTE_LEXICO;


//Tareas de inicialización
void inicializacion();

//Función que procesa el siguiente componente léxico del fichero, invocada por el componente sintáctico
COMPONENTE_LEXICO sigComponenteLexico();

#endif //COMPLEXICO_H

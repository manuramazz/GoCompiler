//
// Created by User on 14/02/2025.
//

#ifndef TS_H
#define TS_H
#include "abb.h"
#endif //TS_H

//Función que crea la tabla de símbolos (árbol binario) e introduce las palabras reservadas
void inicializarTS();

//Función que busca un lexema en la tabla de símbolos
void buscar(char * lexema, TIPOELEMENTOABB *elem);

//Función que inserta un nuevo elemento en la tabla de símbolos
void insertar(TIPOELEMENTOABB elem);

//Función que destruye la tabla de símbolos
void liberarTS();
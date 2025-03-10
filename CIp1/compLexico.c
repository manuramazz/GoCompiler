#include "compLexico.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errores.h"
#include "definiciones.h"
#include "TS.h"

//Tamano de cada uno de los bloques de lectura
//Nota: se añade un caracter extra para el EOF (actúa de centinela) por lo que tienen tamaño SIZE+1, de los cuales hay SIZE caracteres del fichero
#define SIZE 64

FILE * fd;
//Par de bloques de tamaño SIZE+1 del sistema de entrada, donde se carga el contenido del fichero
char A[SIZE+1], B[SIZE+1];

//Apuntadores al inicio del componente léxico y al siguiente caracter a leer
int inicio, delantero;

//Indicador de bloque actual
int bloqueA = 0;


/*Indicador de si se debe devolver un semicolon tras el próximo salto de línea
 * Según la documentación de GO:
 * "When the input is broken into tokens, a semicolon is automatically inserted into the token stream immediately after a line's final token if that token is
 * an identifier
 * an integer, floating-point, imaginary, rune, or string literal
 * one of the keywords break, continue, fallthrough, or return
 * one of the operators and punctuation ++, --, ), ], or }
 */
int semicolon = 0;


/*********************************
 *FUNCIONES DEl SISTEMA DE ENTRADA*
 * ********************************/
//Función que carga un bloque del fichero de tamaño SIZE en el sistema de entrada
void cargarBloque() {
    //Si el bloque A está cargado, se carga el bloque B con fread y viceversa
    //Tienen un tamaño de SIZE+1 para poder añadir el EOF en el último caracter
    if (bloqueA) {
        memset(B,EOF,SIZE);
        fread(B,SIZE,1,fd);
        B[SIZE]=EOF;
        delantero = 0;
    }else {
        memset(A,EOF,SIZE);
        fread(A,SIZE,1,fd);
        A[SIZE]=EOF;
        delantero = 0;
    }
    bloqueA  = !bloqueA;
}

void inicializacion() {
    //apertura de fichero
    fd= fopen("concurrentSum.go", "r");
    if (fd == NULL) {
        error(1);
    }

    //carga del primer bloque y set de los apuntadores a 0
    cargarBloque();
    inicio = 0;
    delantero = 0;

    A[SIZE] = EOF;
    B[SIZE] = EOF;
}

//Función que devuelve el siguiente caracter a procesar
char siguienteChar() {
    char c;
    /* Dependiendo del bloque actual, se devuelve el siguiente caracter de A o B
     * Dentro hay una comprobación de EOF para cargar un nuevo bloque si se llega al final del actual
     * Si se llega al final del fichero, se devuelve EOF al componente sintáctico */
    switch (bloqueA) {
        case 1:
            c = A[delantero];
            delantero++;
            if (c == EOF) {
                if (!feof(fd)) {
                    cargarBloque();
                    c = siguienteChar();
                }
            }
            return c;

        case 0:
            c = B[delantero];
            delantero++;
            if (c == EOF) {
                if (!feof(fd)) {
                    cargarBloque();
                    c = siguienteChar();
                }
            }
            return c;
    }
    return EOF;
}

//Reserva memoria para el lexema
void reservarLexema(char **lexema, int tam){
    *lexema = (char*)malloc(tam*sizeof(char));
}

//Retrocede el apuntador delantero en una posición,
//para cuando se acepta un componente léxico no incluir el caracter que lo ha aceptado y no pertenece a él
void retroceder(){
    delantero--;
}

//Copia el lexema del bloque del sistema de entrada en el componente léxico con strncpy
void copiarLexema(COMPONENTE_LEXICO * cl,int tamaño,int mismo) {
    if(bloqueA){
        //Caso de que delantero se encuentra en el bloque A
        if (mismo) {
            //Caso de que inicio también se encuentre en el bloque A
            strncpy(cl->lexema, &A[inicio], delantero - inicio);
        }else{
            //Caso de que inicio se encuentre en el bloque A y delantero en B
            //En este caso  se copia el lexema en dos partes
            //se usa memset con '\0' previamente para evitar problemas con la concatenación
            memset(cl->lexema,'\0',tamaño);
            strncpy(cl->lexema, &B[inicio], SIZE-inicio);
            strncat(cl->lexema, &A[0], delantero);
        }
    }
    else{
        //Caso de que delantero se encuentra en el bloque B
        if (mismo) {
            //Ambos están en B
            strncpy(cl->lexema, &B[inicio], delantero - inicio);
        }else {
            //Delantero está en B e inicio en A
            memset(cl->lexema,'\0',tamaño);
            strncpy(cl->lexema, &A[inicio], SIZE-inicio);
            strncat(cl->lexema, &B[0], delantero);
        }
    }
    cl->lexema[tamaño-1] = '\0';
}

/* Función que acepta un componente léxico, copia el lexema en el componente léxico
 * y actualiza el apuntador inicio a la posición de delantero */
void aceptar (COMPONENTE_LEXICO * cl) {
    //Retroceder para no incluir el caracter que acepta el componente léxico
    retroceder();
    //Verifica si el lexema está en un solo bloque o en dos según si es mayor la posición de inicio o de delantero
    //(se asume que no hay errores de tamaño en los lexemas y no superan el tamaño de SIZE)
    if(inicio < delantero){
        reservarLexema(&cl->lexema, delantero - inicio+1);
        copiarLexema(cl,delantero - inicio+1, 1);
    }else{
        int tamaño = SIZE-inicio + delantero+1;
        reservarLexema(&cl->lexema, tamaño);
        copiarLexema(cl,tamaño,0);
    }
    inicio = delantero;
}


//Función que acepta un operador (se diferencia en la función general de aceptar en que no retrocede el apuntador delantero)
void aceptarOperador(COMPONENTE_LEXICO * cl) {
    if(inicio < delantero){
        reservarLexema(&cl->lexema, delantero - inicio+1);
        copiarLexema(cl,delantero - inicio+1, 1);
    }else{
        int tamaño = SIZE-inicio + delantero+1;
        reservarLexema(&cl->lexema, tamaño);
        copiarLexema(cl,tamaño,0);
    }
    inicio = delantero;
}

// Para aceptar comentarios, se actualiza el apuntador inicio a la posición de delantero
// No es necesario copiar el lexema o verificar su tamaño
void aceptarComentario(COMPONENTE_LEXICO * cl) {
    inicio = delantero;
}

// Función llamada cuando se debe insertar un semicolon tras un salto de línea
// No hay que leer de los bloques, se reserva memoria para el lexema ";" y se devuelve
void aceptarSemicolon(COMPONENTE_LEXICO * cl) {
    cl->lexema = (char*)malloc(sizeof(char));
    cl->lexema = ";";
    cl->numero = ';';
}



/************************
 *FUNCIONES DE AUTOMATAS*
 * **********************/
//Autómata para números decimales
COMPONENTE_LEXICO automata_decimal_lit(char c, COMPONENTE_LEXICO * componente, int op) {
    //Lee la parte entera (dígitos en bucle)
    //El lenguaje de aceptación de una cadena de dígitos en GO es [0-9] [ '_' || [0-9]]
    if (isdigit(c)) {
        op=0;
        while (isdigit(c=siguienteChar())|| c=='_') {}
    }
    //Si tiene un punto o un exponente es un float lit
    if (c=='.' || c=='e' || c=='E') {
        //Caso de no tener punto pero sí exponente
        if (c=='e'||c=='E'){
            op=0;
            c=siguienteChar();
            //Puede incluir un signo + o - tras el exponente/punto
            if (c=='+'||c=='-') {
                c=siguienteChar();
            }
            //A continuación sigue leyendo dígitos (base decimal)
            while (isdigit(c)|| c=='_') {
                c=siguienteChar();
            }
        }
        else {
            //Caso de tener punto
            //Primero se lee la base decimal
            while (isdigit(c=siguienteChar())|| c=='_') {op=0;}
            //Exponente seguido de signo opcional y dígitos
            if (c=='e'||c=='E') {
                op=0;
                c=siguienteChar();
                if (c=='+'||c=='-') {
                    c=siguienteChar();
                }
                while (isdigit(c)|| c=='_') {
                    c=siguienteChar();
                }
            }
        }
        //En caso de que lo único que se leyó en el autómata fue un punto, no es un número sino un operador
        //Se verifica mediante la variable op, que se anula si se leyó algún otro caracter aparte del '.'
        if (op) {
            componente->numero = '.';
            aceptar(componente);
            semicolon = 0;
            return *componente;
        }else {
            //Si se leyó un exponente, es un float lit
            //Todavía hay que comprobar si el exponente es un imaginary lit
            if (c=='i'){
                componente->numero = IMAGINARY_LIT;
                c=siguienteChar();
            }else { //En caso contrario, es un float lit
                componente->numero = FLOAT_LIT;
            }
            aceptar(componente);
            semicolon = 1;
            return *componente;
        }
    }
    //Si no tiene parte decimal ni exponencial, puede ser un integer lit o un imaginary lit dependiendo de si la pila de dígitos va seguida de una 'i'
    if (c=='i'){
        componente->numero = IMAGINARY_LIT;
        c=siguienteChar();
    }else {
        componente->numero = INTEGER_LIT;
    }
    aceptar(componente);
    semicolon = 1;
    return *componente;
}

//Función que reconoce identificadores y keywords
COMPONENTE_LEXICO automata_identificadores(char c, COMPONENTE_LEXICO * componente) {
    int i = 0;
    //Están formados por cadenas alfanuméricas y guiones bajos (no pueden empezar por un número)
    while (isalnum(c = siguienteChar()) || c == '_') {
    }
    //Cuando se acepta un identificador, se busca en la tabla de símbolos
    aceptar(componente);
    componente->numero = -1;
    buscar(componente->lexema,componente);
    //Si no se encuentra, se inserta
    if (componente->numero==-1) {
        componente->numero=ID;
        insertar(*componente);
    }
    if (componente->numero == ID) {
        semicolon = 1;
    }
    return *componente;
}

//Función que reconoce strings literales
COMPONENTE_LEXICO automata_strings(char c, COMPONENTE_LEXICO * componente) {
    int barra = 0;
    //Están formados por cualquier caracter entre comillas, pudiendo incluir secuencias de escape
    //Las secuencias de escape se reconocen con una barra invertida y se controlan con la variable barra
    while ((c = siguienteChar()) != '"' || barra) {
        barra=0;
        if (c=='\\') {
            barra = 1;
        }
    }
    //Se acepta cuando se encuentra el segundo caracter de comillas no escapado
    c = siguienteChar();
    aceptar(componente);
    componente->numero = STRING_LIT;
    semicolon = 1;
    return *componente;
}

//Función que reconoce literales hexadecimales
COMPONENTE_LEXICO automata_hexadecimal_lit(char c, COMPONENTE_LEXICO * componente) {
    //Se entra a esta función cuando se ha leído la base 0x
    while (isxdigit(c=siguienteChar()) || (c=='_')) {}
    //Se leen en bucle dígitos hexadecimales y guiones bajos
    if (c=='.') {
        //Si se encuentra un punto, se puede tratar de un hexadecimal float lit (o un imaginary lit)
        c=siguienteChar();
        if (c=='+'||c=='-') {
            c=siguienteChar();
        }
        while (isxdigit(c)|| c=='_') {
            c=siguienteChar();
        }
        //A su vez puede incluir exponentes
        if ((c=siguienteChar())=='p'||c=='P') {
            c=siguienteChar();
        }
        //Si el exponente incluye una 'i' al final, es un imaginary lit
        if (c=='i'){
            componente->numero = IMAGINARY_LIT;
            c=siguienteChar();
        }else {
            //En caso contrario se acepta como un float lit
            componente->numero = FLOAT_LIT;
        }
        aceptar(componente);
        semicolon = 1;
        return *componente;
    }
    //Si no se encuentra un punto, se hace la misma comprobación entre integer e imaginary lit
    if (c=='i'){
        componente->numero = IMAGINARY_LIT;
        c=siguienteChar();
    }else {
        componente->numero = INTEGER_LIT;
    }
    aceptar(componente);
    semicolon = 1;
    return *componente;
}


//Función que reconoce literales octales (no hay octales en punto flotante en go)
COMPONENTE_LEXICO automata_octal_lit(char c, COMPONENTE_LEXICO * componente) {
    //Lee en bucle dígitos octales y guiones bajos y acepta como integer lit o imaginary lit
    while ( ((c=siguienteChar()) >= '0' && c <= '7') || c=='_') {}
    if (c=='i'){
        componente->numero = IMAGINARY_LIT;
        c=siguienteChar();
    }else {
        componente->numero = INTEGER_LIT;
    }
    aceptar(componente);
    semicolon = 1;
    return *componente;
}

//Función que reconoce literales binarios (no hay binarios en punto flotante en go)
COMPONENTE_LEXICO automata_binario_lit(char c, COMPONENTE_LEXICO * componente) {
    //Lee en bucle dígitos binarios y guiones bajos y acepta como integer lit o imaginary lit
    while ( (c=siguienteChar()) == '0' || c <= '1' || c=='_') {}
    if (c=='i'){
        componente->numero = IMAGINARY_LIT;
        c=siguienteChar();
    }else {
        componente->numero = INTEGER_LIT;
    }
    aceptar(componente);
    semicolon = 1;
    return *componente;
}

//Función que reconoce operadores
COMPONENTE_LEXICO automata_operadores(char c, COMPONENTE_LEXICO * componente) {

    //Usan la función aceptarOperador en lugar de aceptar al no ser necesaria la retrocesión
    aceptarOperador(componente);
    componente->numero = c;
    //Verificación de si es un operador que necesita un semicolon tras él
    if (componente->numero == ')' || componente->numero == ']' || componente->numero == '}') {
        semicolon = 1;
    }else {
        semicolon = 0;
    }
    return *componente;
}

//Para los operadores que tienen posibilidad de ser compuestos, se usa una función específica
COMPONENTE_LEXICO automata_operadores_compuestos(char c, COMPONENTE_LEXICO * componente) {
    //Se leen los siguientes caracteres para determinar si es un operador compuesto válido
    //si es un operador simple de los siguientes, se acepta como tal y sería necesario retroceder antes de usar la función aceptarOperador
    if (c=='<') {
        semicolon = 0;
        c = siguienteChar();
        if (c == '-') {
            componente->numero = 299;
            aceptarOperador(componente);
            return *componente;
        }
    }
    if (c=='+') {
        c = siguienteChar();
        if (c == '=') {
            semicolon = 0;
            componente->numero = 298;
            aceptarOperador(componente);
            return *componente;
        }else {
            retroceder();
            componente->numero = '+';
            semicolon=0;
            aceptarOperador(componente);
            return *componente;
        }

    }
    if (c==':') {
        semicolon = 0;
        c = siguienteChar();
        if (c == '=') {
            componente->numero = 297;
            aceptarOperador(componente);
            return *componente;
        }else {
            retroceder();
            componente->numero = ':';
            semicolon=0;
            aceptarOperador(componente);
            return *componente;
        }
    }
    return *componente;
}



/****************************
 *FUNCIÓN INVOCADA POR EL CS*
 * **************************/

//Función que implementa el componente léxico
//Devuelve el siguiente componente léxico encontrado en el fichero
COMPONENTE_LEXICO sigComponenteLexico() {
    //Inicializa una variable de tipo COMPONENTE_LEXICO donde se almacenará la información del componente léxico a devolver
    // y un caracter que será el siguiente a procesar del fichero (llamada a la función siguienteChar)
    char c,charInicio;
    COMPONENTE_LEXICO * componente = (COMPONENTE_LEXICO *) malloc(sizeof(COMPONENTE_LEXICO));
    componente->lexema = NULL;
    c = siguienteChar();

    //Identificadores
    if (isalpha(c) || c == '_') {
        return automata_identificadores(c,componente);
    }

    //Strings literales
    if ( c == '"') {
        return automata_strings(c,componente);

    }

    //Numeros
    if (isdigit(c) || c == '.') {
        if (c == '0') {
            c=siguienteChar();
            //HEX LIT
            if (c=='x'||c=='X') {
                return automata_hexadecimal_lit(c,componente);
            }
            //OCTAL LIT
            if (c=='o'||c=='O') {
                return automata_octal_lit(c,componente);
            }
            //BINARY LIT
            if (c=='b'||c=='B') {
                return automata_binario_lit(c,componente);
            }
            return automata_decimal_lit(c,componente,0);
        }
        //si no empieza por 0 pero es un digito -> INT/FLOAT DECIMAL LIT
        if (isdigit(c) || c == '.') {
            return automata_decimal_lit(c,componente,1);
        }
    }

    //Delimitadores
    if ((c) == '\n' || c == '\t' || c == ' ') {

        inicio=delantero;
        //Comprobación de si se debe insertar un semicolon tras el salto de línea
        if (c=='\n' && semicolon) {
            aceptarSemicolon(componente);
            return *componente;
        }
        //Se actualiza el apuntador inicio a la posición de delantero en cada delimitador consecutivo encontrado, pues no se requerirá volver atrás
        while ((c = siguienteChar()) == '\n' || c == '\t' || c == ' ') {
            inicio=delantero;
        }
        delantero=inicio;
        return sigComponenteLexico();
    }

    //OPERADORES
    if (c == '{' || c == '}' || c == '(' || c == ')' || c == ';' || c == ',' || c == '[' || c == ']' || c == '=' || c=='*' || c=='-') {
        return automata_operadores(c,componente);
    }

    //Comentarios u operador de división
    if (c == '/') {
        //Comentarios
        if ((c=siguienteChar())=='/' || c == '*') {
            if (c == '/') {
                //comentario de linea
                while ((c = siguienteChar()) != '\n'){}
                aceptarComentario(componente);
                //Se acepta el comentario cuando se encuentra un salto de línea y se comprueba si se debe insertar un semicolon
                if (semicolon) {
                    aceptarSemicolon(componente);
                    return *componente;
                }
                return sigComponenteLexico();
            }else {
                //comentario multilinea
                if ((c = siguienteChar()) == '*'){
                    while ( (c = siguienteChar()) != '*' || (c = siguienteChar()) != '/') {
                    }
                    //Se hace la misma comprobación tras aceptar el comentario multilinea con '*' y '/' consecutivos
                    aceptarComentario(componente);
                    if (semicolon) {
                        aceptarSemicolon(componente);
                        return *componente;
                    }
                    return sigComponenteLexico();
                }
            }
        }else {
            //Operador de división
            retroceder();
            aceptarOperador(componente);
            componente->numero = '/';
            return *componente;
        }
    }
    //operadores compuestos
    if (c=='<' || c=='+' || c==':') {
        return automata_operadores_compuestos(c,componente);
    }
    componente->numero = EOF;
    return *componente;
}





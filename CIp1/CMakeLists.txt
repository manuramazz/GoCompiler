cmake_minimum_required(VERSION 3.30)
project(CIp1 C)

set(CMAKE_C_STANDARD 11)

add_executable(CIp1 main.c
        compLexico.c
        compLexico.h
        compSintactico.c
        compSintactico.h
        TS.c
        TS.h
        definiciones.h
        abb.c
        abb.h
        errores.c
        errores.h
)

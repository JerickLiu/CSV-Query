/*
 ============================================================================
 Name        : ppsServer.h
 Author      : Jerick Liu
 Description : Header file of ppsServer.c
 ============================================================================
 */

#ifndef HEADER_PERSON
#define HEADER_PERSON

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <ctype.h>
#include <semaphore.h>
#include <pthread.h>

/************************************************/
// DEFINE

#define MAXLENGTH 100
#define NUM_YEARS 5
#define SEPARATOR ","
#define SERVER_PORT 6000

/************************************************/
// structures

typedef struct PokemonData {
    int   number;
    char  name[MAXLENGTH];
    char  type1[MAXLENGTH];
    char  type2[MAXLENGTH];
    int   total;
    int   hp;
    int   attack;
    int   defense;
    int   spAttack;
    int   spDefense;
    int   speed;
    int   generation;
    char  legendary[MAXLENGTH];
} Pokemon;

typedef struct count_t {
    int count;
    FILE* allPokemon;
    Pokemon** pokemonPtrArr;
    char query[MAXLENGTH];
    sem_t mutex;
} count_t;

/************************************************/
// prototypes

void getFile(FILE*);
void* parsePokemon(void* arg);
void lineToPokemon(char*, Pokemon*, char*);
void pokemonToLine(char*, const Pokemon*);

#endif

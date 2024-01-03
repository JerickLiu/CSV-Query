/*
 ============================================================================
 Name        : pqcClient.c
 Author      : Jerick Liu
 Description : A program to read and write CSV files.
 ============================================================================
 */

 /* Includes */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<semaphore.h>

/* Constants */
#define SEPARATOR ','
#define MAXLENGTH 100

/* Structs */
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
    volatile int count;
    int numInc;
    sem_t mutex;
} count_t;

/* Function Prototypes */
int parsePokemon(FILE*, Pokemon**, char*, int);
void lineToPokemon(char*, Pokemon*, char);
void saveResults(Pokemon*, int);
void pokemonToLine(char*, const Pokemon*);
int countPokemon(FILE*);

/*
* Function: main
* Description: Entry point to the ReadCSV sample program
*/
int main(void) {

    pthread_t t1, t2;
    int inputMenu, flag;
    char* inputCSV;
    char* query;
    FILE* allPokemon;
    Pokemon* pokemonArr = NULL;
    int num = 0;
    int queries = 0;

    // Prompts gamer for input CSV, stores it dynamically
    printf("Enter the directory of the CSV file: ");
    scanf("%ms", &inputCSV);

    // Opens user-inputted CSV
    allPokemon = fopen(inputCSV, "r");

    // Loops if the file is NULL
    while (!allPokemon) {

        // Deallocates invalid input
        free(inputCSV);

        // Prompts user to reenter file, or exit the program
        printf("Pokemon file is not found. Enter the directory of the CSV file or \"q\" to quit: ");
        scanf("%ms", &inputCSV);

        // Checks if user wants to exit the program
        if (strcmp(inputCSV, "q") == 0) {
            free(inputCSV);
            return 0;
        }

        // Opens the user-inputted CSV
        allPokemon = fopen(inputCSV, "r");
    }


    printf("Successfully read file.\n");

    // Menu - Loops until user exits (selects 3)
    flag = 1;
    while (flag) {
        printf("Menu:\n\t1. Type Search\n\t2. Save Results\n\t3. Exit the Program\nEnter a numeric option (1, 2, 3): ");
        scanf("%d", &inputMenu);

        switch (inputMenu) {
        case 1:

            // Prompts gamer for type query - stores it dynamically
            printf("\nWhat type to query: ");
            scanf("%ms", &query);

            // Parses all pokemon matching search query from input CSV
            num = parsePokemon(allPokemon, &pokemonArr, query, num);

            // Closes and reopens file to reset positioning to beginning
            fclose(allPokemon);
            allPokemon = fopen(inputCSV, "r");

            // Increments number of queries completed
            ++queries;

            free(query);

            printf("Query successful. Proceed to save.\n\n");

            break;
        case 2:
            // Saves memory into file
            saveResults(pokemonArr, num);
            break;
        case 3:
            // User exits, outputs how many queries completed
            flag = 0;
            printf("Queries Completed: %d\n", queries);
            break;

        default:
            // User inputted value out of range
            printf("Invalid input. Please try again.\n");
            break;
        }
    }

    // Deallocates memory on the heap
    free(inputCSV);
    free(pokemonArr);

    // Closes file
    fclose(allPokemon);

    return(0);
}

/*
 * Function: parsePokemon
 * Description: Reads csv lines from a file, creates Pokemon structures,
 *              and dymamically grows the array that holds them.
 * Parameters:
 *   allPokemon: file pointer
 *   pokemonPtrArr: array pointer that holds Pokemon structures
 *   query: char pointer that holds type to query
 *   numPokemon: int that contains amount of elements is pokemonPtrArr
 * Returns:
 *   the number of pokemon read in from the file (header line excluded)
*/
int parsePokemon(FILE* allPokemon, Pokemon** pokemonPtrArr, char* query, int numPokemon) {

    // Int tracking line of file
    int lineNum = 0;

    // Char array to parse
    char* lineToParse;

    // Loops until reaches end of file
    while (fscanf(allPokemon, "%[^\n]\n", lineToParse) != EOF) {

        // Checks if header discarded and line contains query search
        if (lineNum > 0 && strstr(lineToParse, query)) {

            // Checks if the current line is the first line to parse and there are no elements in the array
            if (lineNum == 1 && numPokemon == 0) {
                // Dynamically allocates memory
                *pokemonPtrArr = calloc(1, sizeof(Pokemon));
                ++numPokemon;
            }
            else {
                // Reallocates memory
                *pokemonPtrArr = realloc(*pokemonPtrArr, (numPokemon + 1) * sizeof(Pokemon));
                ++numPokemon;
            }

            // Converts parsed line to pokemon structure at newly allocated index
            lineToPokemon(lineToParse, &(*pokemonPtrArr)[numPokemon - 1], SEPARATOR);

        }
        ++lineNum;
    }

    return numPokemon;
}



/*
 * Function: lineToPokemon
 * Description: Converts line from CSV to structure
 * Parameters:
 *   lineToParse: char pointer representing line from input file
 *   newPokemon: Pokemon pointer representing new structure
 *   sep: Seperator character to delimit entries
 * Returns:
 *   N/A
*/
void lineToPokemon(char* lineToParse, Pokemon* newPokemon, char sep) {
    // Str seperates all data
    int number = atoi(strsep(&lineToParse, &sep));
    char* name = strsep(&lineToParse, &sep);
    char* type1 = strsep(&lineToParse, &sep);
    char* type2 = strsep(&lineToParse, &sep);
    int total = atoi(strsep(&lineToParse, &sep));
    int hp = atoi(strsep(&lineToParse, &sep));
    int attack = atoi(strsep(&lineToParse, &sep));
    int defense = atoi(strsep(&lineToParse, &sep));
    int spAttack = atoi(strsep(&lineToParse, &sep));
    int spDefense = atoi(strsep(&lineToParse, &sep));
    int speed = atoi(strsep(&lineToParse, &sep));
    int generation = atoi(strsep(&lineToParse, &sep));
    char* legendary = strsep(&lineToParse, &sep);

    // Initiate values of new pokemon struct using parsed data
    newPokemon->number = number;
    strcpy(newPokemon->name, name);
    strcpy(newPokemon->type1, type1);
    strcpy(newPokemon->type2, type2);
    newPokemon->total = total;
    newPokemon->hp = hp;
    newPokemon->attack = attack;
    newPokemon->defense = defense;
    newPokemon->spAttack = spAttack;
    newPokemon->spDefense = spDefense;
    newPokemon->speed = speed;
    newPokemon->generation = generation;
    strcpy(newPokemon->legendary, legendary);

}

/*
 * Function: saveResults
 * Description: Exports memory to output file
 * Parameters:
 *   pokemonArr: pokemon array holding all pokemon structures in memory
 *   numPokemon: int representing amount of elements in array
 * Returns:
 *   N/A
*/
void saveResults(Pokemon* pokemonArr, int numPokemon) {

    FILE* outputCSV;
    char* outputName;
    int i;

    // Prompts gamer for output file - dynamically allocated
    printf("Enter the name of the output file to save to: ");
    scanf("%ms", &outputName);

    // Opens the input
    outputCSV = fopen(outputName, "w");

    // Loops if file is NULL (cannot be created)
    while (!outputCSV) {

        // Deallocates bad input
        free(outputCSV);

        // Prompts gamer for new input
        printf("Unable to create the new file. Please enter the name of the file again: ");
        scanf("%ms", &outputName);

        // Opens the input
        outputCSV = fopen(outputName, "w");
    }

    // Loops through pokemon array
    for (i = 0; i < numPokemon; ++i) {
        // Initiates line to write to file
        char lineToWrite[MAXLENGTH] = "";
        pokemonToLine(lineToWrite, &pokemonArr[i]);
        fprintf(outputCSV, "%s\n", lineToWrite);
    }

    // Success message
    printf("Pokemon data written to: %s\n\n", outputName);

    // Closes files and frees memory
    free(outputName);
    fclose(outputCSV);
}


/*
 * Function: pokemonToLine
 * Description: Converts pokemon struct to line
 * Parameters:
 *   lineToWrite: char array to hold line
 *   pokemon: Pokemon pointer of a pokemon
 * Returns:
 *   N/A
*/
void pokemonToLine(char* lineToWrite, const Pokemon* pokemon) {
    //#,Name,Type 1,Type 2,Total,HP,Attack,Defense,Sp. Atk,Sp. Def,Speed,Generation,Legendary
    char number[MAXLENGTH];
    sprintf(number, "%d", pokemon->number);
    lineToWrite = strcat(lineToWrite, number);
    lineToWrite = strcat(lineToWrite, ",");
    lineToWrite = strcat(lineToWrite, pokemon->name);
    lineToWrite = strcat(lineToWrite, ",");
    lineToWrite = strcat(lineToWrite, pokemon->type1);
    lineToWrite = strcat(lineToWrite, ",");
    lineToWrite = strcat(lineToWrite, pokemon->type2);
    lineToWrite = strcat(lineToWrite, ",");
    char total[MAXLENGTH];
    sprintf(total, "%d", pokemon->total);
    lineToWrite = strcat(lineToWrite, total);
    lineToWrite = strcat(lineToWrite, ",");
    char hp[MAXLENGTH];
    sprintf(hp, "%d", pokemon->hp);
    lineToWrite = strcat(lineToWrite, hp);
    lineToWrite = strcat(lineToWrite, ",");
    char attack[MAXLENGTH];
    sprintf(attack, "%d", pokemon->attack);
    lineToWrite = strcat(lineToWrite, attack);
    lineToWrite = strcat(lineToWrite, ",");
    char defense[MAXLENGTH];
    sprintf(defense, "%d", pokemon->defense);
    lineToWrite = strcat(lineToWrite, defense);
    lineToWrite = strcat(lineToWrite, ",");
    char spAttack[MAXLENGTH];
    sprintf(spAttack, "%d", pokemon->spAttack);
    lineToWrite = strcat(lineToWrite, spAttack);
    lineToWrite = strcat(lineToWrite, ",");
    char spDefense[MAXLENGTH];
    sprintf(spDefense, "%d", pokemon->spDefense);
    lineToWrite = strcat(lineToWrite, spDefense);
    lineToWrite = strcat(lineToWrite, ",");
    char speed[MAXLENGTH];
    sprintf(speed, "%d", pokemon->speed);
    lineToWrite = strcat(lineToWrite, speed);
    lineToWrite = strcat(lineToWrite, ",");
    char generation[MAXLENGTH];
    sprintf(generation, "%d", pokemon->generation);
    lineToWrite = strcat(lineToWrite, generation);
    lineToWrite = strcat(lineToWrite, ",");
    lineToWrite = strcat(lineToWrite, pokemon->legendary);
}

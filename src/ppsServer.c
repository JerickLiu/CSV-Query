/*
 ============================================================================
 Name        : ppsServer.c
 Author      : Jerick Liu
 Description : A program to manage the server side of a Query System
 ============================================================================
 */

#include "ppsServer.h"

int main() {
    pthread_t           t1, t2;
    int                 serverSocket, clientSocket, i;
    struct sockaddr_in  serverAddr, clientAddr;
    int                 status, addrSize, bytesReceived;
    fd_set              readfds, writefds;
    FILE* allPokemon;
    char                buffer[100];
    char* response = "OK";
    char* inputCSV;

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
            exit(-1);
        }

        // Opens the user-inputted CSV
        allPokemon = fopen(inputCSV, "r");
    }

    // Create the server socket
    serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0) {
        printf("*** SERVER ERROR: Could not open socket.\n");
        close(serverSocket);
        exit(-1);
    }

    // Setup the server address
    memset(&serverAddr, 0, sizeof(serverAddr)); // zeros the struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons((unsigned short)SERVER_PORT);

    // Bind the server socket
    status = bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (status < 0) {
        printf("*** SERVER ERROR: Could not bind socket.\n");
        close(serverSocket);
        exit(-1);
    }

    // Listening
    status = listen(serverSocket, 5);
    if (status < 0) {
        printf("*** SERVER ERROR: Could not listen\n");
        close(serverSocket);
        exit(-1);
    }

    // Wait for clients
    while (1) {
        // Wait for connection request
        addrSize = sizeof(clientAddr);

        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);

        if (clientSocket < 0) {
            printf("*** SERVER ERROR: Could not accept the connection\n");
            // Close socket
            close(serverSocket);
            exit(-1);
        }

        printf("SERVER: Received client connection.\n");

        Pokemon* pokemonArr = NULL;
        count_t* numPokemon = (count_t*)malloc(sizeof(count_t));

        if (sem_init(&numPokemon->mutex, 0, 1) < 0) {
            printf("Error: failed to initialize semaphore.");
            exit(1);
        }

        // Menu starts here
        while (1) {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

            buffer[bytesReceived] = '\0';

            if (strcmp(buffer, "1\0") == 0) {
                printf("SERVER: Client chose Type Search. Awaiting input ... \n");

                bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                buffer[bytesReceived] = '\0';

                printf("SERVER: Received \"%s\" type from client... Preparing to query\n", buffer);

                numPokemon->count = 0;
                numPokemon->allPokemon = allPokemon;
                numPokemon->pokemonPtrArr = &pokemonArr;
                strcpy(numPokemon->query, buffer);

                // Parses all pokemon matching search query from input CSV
                pthread_create(&t1, NULL, parsePokemon, (void*)numPokemon);
                pthread_join(t1, NULL);

                // Loops through pokemon array: sends each entry to client
                printf("SERVER: Sending client %d query results.\n", numPokemon->count);
                send(clientSocket, &numPokemon->count, sizeof(numPokemon->count), 0);

                for (i = 0; i < numPokemon->count; i++) {
                    char lineToSend[MAXLENGTH] = { "\0" };
                    pokemonToLine(lineToSend, &pokemonArr[i]);
                    send(clientSocket, lineToSend, strlen(lineToSend), 0);
                }


                free(*(numPokemon->pokemonPtrArr));

                // Closes and reopens file to reset positioning to beginning
                fclose(allPokemon);
                allPokemon = fopen(inputCSV, "r");

                if (strcmp(buffer, "quit") == 0 || strcmp(buffer, "stop") == 0)
                    break;
            }
            else if (strcmp(buffer, "2\0") == 0) {
                break;
            }
            else if (strcmp(buffer, "3\0") == 0) {
                break;
            }

        }

        printf("SERVER: Closing Client Connection\n");
        close(clientSocket);

        if (strcmp(buffer, "stop") == 0)
            break;
    }

    close(serverSocket);
    printf("SERVER: Shutting down.\n");
}

void* parsePokemon(void* arg) {

    // FILE* allPokemon, Pokemon** pokemonPtrArr, char* query
    count_t* args = (count_t*)arg;

    // Int tracking line of file
    int lineNum = 0;

    // Char array to parse
    char lineToParse[MAXLENGTH];

    // Loops until reaches end of file
    while (fscanf(args->allPokemon, "%[^\n]\n", lineToParse) != EOF) {

        // Checks if header discarded and line contains query search
        if (lineNum > 0 && strstr(lineToParse, args->query)) {

            // Checks if the current line is the first line to parse and there are no elements in the array
            if (lineNum == 1 && args->count == 0) {

                // Dynamically allocates memory
                *(args->pokemonPtrArr) = calloc(1, sizeof(Pokemon));

                if (sem_wait(&args->mutex) < 0) {
                    printf("Error: Semaphore Wait failed.\n");
                    exit(1);
                }

                args->count++;

                if (sem_post(&args->mutex) < 0) {
                    printf("Error: Semaphore Post failed.\n");
                    exit(1);
                }
            }
            else {
                // Reallocates memory
                *(args->pokemonPtrArr) = realloc(*(args->pokemonPtrArr), ((args->count) + 1) * sizeof(Pokemon));

                if (sem_wait(&args->mutex) < 0) {
                    printf("Error: Semaphore Wait failed.\n");
                    exit(1);
                }

                args->count++;

                if (sem_post(&args->mutex) < 0) {
                    printf("Error: Semaphore Post failed.\n");
                    exit(1);
                }
            }

            if (sem_wait(&args->mutex) < 0) {
                printf("Error: Semaphore Wait failed.\n");
                exit(1);
            }

            // Converts parsed line to pokemon structure at newly allocated index
            lineToPokemon(lineToParse, &(*(args->pokemonPtrArr))[(args->count) - 1], SEPARATOR);

            if (sem_post(&args->mutex) < 0) {
                printf("Error: Semaphore Post failed.\n");
                exit(1);
            }

        }
        ++lineNum;
    }

    pthread_exit(NULL);
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
void lineToPokemon(char* lineToParse, Pokemon* newPokemon, char* sep) {
    // Str seperates all data
    int number = atoi(strsep(&lineToParse, sep));
    char* name = strsep(&lineToParse, sep);
    char* type1 = strsep(&lineToParse, sep);
    char* type2 = strsep(&lineToParse, sep);
    int total = atoi(strsep(&lineToParse, sep));
    int hp = atoi(strsep(&lineToParse, sep));
    int attack = atoi(strsep(&lineToParse, sep));
    int defense = atoi(strsep(&lineToParse, sep));
    int spAttack = atoi(strsep(&lineToParse, sep));
    int spDefense = atoi(strsep(&lineToParse, sep));
    int speed = atoi(strsep(&lineToParse, sep));
    int generation = atoi(strsep(&lineToParse, sep));
    char* legendary = strsep(&lineToParse, sep);

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
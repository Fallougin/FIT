/*
    IZP Projekt 1 - prace s textem
    Autor: Artur Suvorkin, xsuvor00
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <stdbool.h>
#define MAXSIZE 10000
#define STR_SIZE 100

bool search(int delim_length, char delim[], char symbol) // Najde symbol "delim" v retezci
{
    bool found = false;
    for (int i = 0; i < delim_length; i++)
    {
        if (delim[i] == symbol) {
            found = true;
            break;
        }
        else {
            continue;      
        }  
    }
    return found;
}

int set_intr_cntr(bool value) // Pokud mame "delim" zacneme 3. argumentem, jinak 1.
{
    if (value)
        return 3;
    else
    {
        return 1;
    }
    
}
int parse_arguments (char* arguments [], int count_of_arguments) // Zpracovani prikazu
{
    bool delim_defined = false;
    char delim[STR_SIZE];
    if (strcmp(arguments[1], "-d") == 0) //delim
    {
        strcpy(delim, arguments[2]);
        delim_defined = true;
    }
    else
    {
        strcpy(delim, " ");
    }
    int instruction_counter;	
    instruction_counter = set_intr_cntr(delim_defined);

    while (instruction_counter < count_of_arguments)
    { 
        char buffer[MAXSIZE];
        //Uprava tabulky
        if(strcmp(arguments[instruction_counter], "irow") == 0) // irow R - vlozi radek tabulky pred radek R > 0 (insert-row).
        {
         if (instruction_counter == count_of_arguments-1) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }     
            int line_number = atoi(arguments[instruction_counter+1]); // Appropriation of argument R
            if (line_number <= 0) { // Zkontroluje hodnoty argumentu
                fprintf(stderr, "irow argument must be > 0!\n");
                return 1;
            }

            int line_counter = 0;
            while (fgets(buffer, MAXSIZE, stdin) != NULL) // Nacte tabulku radek po radku
            {
                line_counter++;
                if (line_counter == line_number) {
                    printf("\n");
                }
                printf("%s", buffer);
            } 
        }
        else if (strcmp(arguments[instruction_counter], "arow") == 0) // arow - prida novy radek tabulky na konec tabulky (append-row).
        {
            while (fgets(buffer, MAXSIZE, stdin) != NULL) {
                printf("%s", buffer);
            }
            printf("\n");

        }
        else if (strcmp(arguments[instruction_counter], "drow") == 0) // drow R - odstrani radek cislo R > 0 (delete-row).
        {
            if (instruction_counter == count_of_arguments-1) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int line_number = atoi(arguments[instruction_counter+1]);
            if (line_number <= 0) {
                fprintf(stderr, "drow argument must be > 0!\n");
                return 1;
            }
            int line_counter = 0;
            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                line_counter++;
                if (line_counter == line_number) {
                continue;
            }
                printf("%s", buffer);
            }
        }
        else if (strcmp(arguments[instruction_counter], "drows") == 0) // drows N M - odstrani radky N az M (N <= M). V pripade N=M se prikaz chova stejne jako drow N.
        {
            if (instruction_counter == count_of_arguments-2) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int line_numberN = atoi(arguments[instruction_counter+1]);
            int line_numberM = atoi(arguments[instruction_counter+2]); 
            if (line_numberN > line_numberM) {
                fprintf(stderr, "drows arguments must be N <= M !\n");
                return 1;
            }
            int line_counter = 0;
            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                line_counter++;
                if ((line_counter < line_numberN) || (line_counter > line_numberM)) {
                    printf("%s", buffer);
                }
                else {
                    continue;
                }    
            } 
        }
        else if (strcmp(arguments[instruction_counter], "icol") == 0) // icol C - vlozi prazdny sloupec pred sloupec dany cislem C.
        {
            if (instruction_counter == count_of_arguments-1) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int numberC = atoi(arguments[instruction_counter+1]);
            if (numberC <= 0){
                fprintf(stderr, "C must be > 0\n"); 
                return 1;
            }
            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                int column_counter = 0;
                int i = 0;

                if (numberC == 1) {
                    printf("%c", delim[0]);
                }
                while (buffer[i] != '\n') // Nacte tabulku znak po znaku
                {
                    if(search(strlen(delim), delim, buffer[i])) { // Prace ze sloupcemi
                        column_counter++;
                        if (column_counter == numberC-1) {
                            printf("%c", delim[0]);
                        }
                        printf("%c", delim[0]);
                        i++;
                        continue;
                    }
                    printf("%c", buffer[i]);
                    i++;   
                }
                printf("\n");
            }    
        }
        else if (strcmp(arguments[instruction_counter], "acol") == 0) // acol - prida prazdny sloupec za posledni sloupec.
        {
            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                int i = 0;
                while (buffer[i] != '\n')
                {
                    printf("%c", buffer[i]);
                    i++;   
                }
                if (strlen(buffer) == 1){ // Pokud 1. bunka tabulky obsahuje 1 prvek 
                    continue;
                }
                else
                {
                    printf("%c", delim[0]);
                }
                printf("\n");
            }             
        }
        else if (strcmp(arguments[instruction_counter], "dcol") == 0) // dcol C - odstranĂ­ sloupec cislo C.
        {
            if (instruction_counter == count_of_arguments-1) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int numberC = atoi(arguments[instruction_counter+1]);
            if (numberC <= 0){
                fprintf(stderr, "C must be > 0\n"); 
                return 1;
            }

            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                int column_counter = 0;
                int i = 0;
                if (numberC == 1) {
                    while (!search(strlen(delim), delim, buffer[i]))
                    {
                        i++;
                    }
                    i++;   
                }

                while (buffer[i] != '\n') // kontroluje "\n" v radku
                {
                    if(search(strlen(delim), delim, buffer[i])) {
                        column_counter++;
                        if (column_counter == numberC -1) {
                            printf("%c", delim[0]);
                            i++;
                            while (!search(strlen(delim), delim, buffer[i])){
                                i++;
                            }
                            i++;
                        }
                    } 
                    printf("%c", buffer[i]);
                    
                    i++;   
                }
                printf("\n");
            }
        }
        else if (strcmp(arguments[instruction_counter], "dcols") == 0) // dcols N M - odstrani sloupce N az M (N <= M). 
        {             
            if (instruction_counter == count_of_arguments-2) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }                                                   // V pĹĂ­padÄ N=M se pĹĂ­kaz chovĂĄ stejnÄ jako dcol N.
            int numberN = atoi(arguments[instruction_counter+1]);
            int numberM = atoi(arguments[instruction_counter+2]);
            if (numberN > numberM ){
                fprintf(stderr, "dcols arguments must be N <= M !\n"); 
                return 1;
            }
            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                int column_counter = 0;
                int i = 0;

                if (numberN == 1) {
                    while (!search(strlen(delim), delim, buffer[i]))
                    {
                        i++;
                    } 
                }
                
                while (buffer[i] != '\n')
                {
                    if(search(strlen(delim), delim, buffer[i])) {
                        column_counter++;
                        if(column_counter == numberN-1) {
                            printf("%c", delim[0]);
                        }
                        while ((column_counter >= numberN-1) && (column_counter <= numberM-1)) {
                            
                            column_counter++;
                            i++;
                            while (!search(strlen(delim), delim, buffer[i])){
                                i++;
                            }
                            i++;
                        }
                    }
                    printf("%c", buffer[i]);
                    i++;   
                }
                printf("\n");
            }
        }   
    // ZpracovĂĄnĂ­ dat
        else if (strcmp(arguments[instruction_counter], "cset") == 0) // cset C STR - do bunky ve sloupci C bude nastaven retezec STR.
        {
            if (instruction_counter == count_of_arguments-2) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int numberC = atoi(arguments[instruction_counter+1]);
            char put_string[MAXSIZE]; 
            strcpy(put_string, arguments[instruction_counter+2]); // kopiruje STR argument
            
            if (numberC <= 0){
                fprintf(stderr, "C must be > 0\n"); 
                return 1;
            }

            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                int column_counter = 0;
                int i = 0;
                
                if (numberC == 1) {
                    while (!search(strlen(delim), delim, buffer[i]))
                    {
                        i++;
                    } printf("%s", put_string);
                }
                
                while (buffer[i] != '\n')
                {
                    if(search(strlen(delim), delim, buffer[i])) {
                        column_counter++;
                        
                        if (column_counter == numberC-1) {
                            printf("%c", delim[0]);
                            printf("%s%c", put_string, delim[0]);
                            i++;
                            while (!search(strlen(delim), delim, buffer[i])){
                                i++;
                            }
                            i++;
                        }
                    }
                    printf("%c", buffer[i]);
                    i++;   
                }
                printf("\n");
            }
        }
        else if (strcmp(arguments[instruction_counter], "tolower") == 0)// tolower C - retezec ve sloupci C bude preveden na mala pismena.
        {
            if (instruction_counter == count_of_arguments-1) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int numberC = atoi(arguments[instruction_counter+1]);
            
            if (numberC <= 0){
                fprintf(stderr, "C must be > 0\n"); 
                return 1;
            }

            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                int column_counter = 0;
                int i = 0;
                
                if ((numberC == 1) && (buffer[i] != delim[0])) {
                    while (!search(strlen(delim), delim, buffer[i]))
                    {
                        if ((buffer[i] >= 'A') && (buffer[i] <='Z')) {
                            buffer[i] = buffer[i]+32;
                            printf("%c", buffer[i]);
                        }
                        else {
                            printf("%c", buffer[i]);
                        }
                        i++;
                    }
                }
                
                while (buffer[i] != '\n')
                {
                    if(search(strlen(delim), delim, buffer[i])) {
                        column_counter++;
                        if (column_counter == numberC-1) {
                            printf("%c", buffer[i]);
                            i++;
                            while (!search(strlen(delim), delim, buffer[i])){
                                if ((buffer[i] >= 'a') && (buffer[i] <='z')) {
                                    printf("%c", buffer[i]);
                                }
                                else if ((buffer[i] >= 'A') && (buffer[i] <='Z')){
                                    buffer[i] = buffer[i]+32;
                                    printf("%c", buffer[i]);
                                }
                                else if (!(((buffer[i] >= 65) || (buffer[i] >= 90)) && ((buffer[i] >= 97) || (buffer[i] >= 122)))){
                                    printf("%c", buffer[i]);
                                }
                                i++;
                            }
                            printf("%c", delim[0]);
                            i++;
                        }
                    }
                    printf("%c", buffer[i]);
                    i++;   
                }
                printf("\n");
            }
        }
        else if (strcmp(arguments[instruction_counter], "toupper") == 0)// toupper C - retezec ve sloupce C bude preveden na velka pismena.
        {
            if (instruction_counter == count_of_arguments-1) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int numberC = atoi(arguments[instruction_counter+1]);
            
            if (numberC <= 0){
                fprintf(stderr, "C must be > 0\n"); 
                return 1;
            }

            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                int column_counter = 0;
                int i = 0;
                
                if ((numberC == 1) && (buffer[i] != delim[0])) {
                    while (!search(strlen(delim), delim, buffer[i]))
                    {
                        if ((buffer[i] >= 'a') && (buffer[i] <='z')) { // Overi ASCII hodnotu 
                            buffer[i] = buffer[i]-32;
                            printf("%c", buffer[i]);
                        }
                        else {
                            printf("%c", buffer[i]);
                        }
                        i++;
                    }
                }
                
                while (buffer[i] != '\n')
                {
                    if(search(strlen(delim), delim, buffer[i])) {
                        column_counter++;
                        
                        if (column_counter == numberC-1) {
                            printf("%c", buffer[i]);
                            i++;
                            while (!search(strlen(delim), delim, buffer[i])){
                                if ((buffer[i] >= 'A') && (buffer[i] <='Z')) {
                                    printf("%c", buffer[i]);
                                }
                                else if ((buffer[i] >= 'a') && (buffer[i] <='z')){
                                    buffer[i] = buffer[i]-32;
                                    printf("%c", buffer[i]);
                                }
                                else if (!(((buffer[i] >= 65) || (buffer[i] >= 90)) && ((buffer[i] >= 97) || (buffer[i] >= 122)))){
                                    
                                    printf("%c", buffer[i]);
                                }
                                i++;
                            }
                            printf("%c", delim[0]);
                            i++;
                        }
                       
                    }
                    printf("%c", buffer[i]);
                    i++;   
                }
                printf("\n");
            }
        }
        else if (strcmp(arguments[instruction_counter], "round") == 0)// round C - ve sloupci C zaokrouhli cislo na cele cislo.
        {
            if (instruction_counter == count_of_arguments-1) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int numberC = atoi(arguments[instruction_counter+1]);
            
            if (numberC <= 0){
                fprintf(stderr, "C must be > 0\n"); 
                return 1;
            }

            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                int column_counter = 0;
                int i = 0;
                int j = 0;
                int k = 0;
                char array_buffer[STR_SIZE];
                char array_num[STR_SIZE];
                if ((numberC == 1) && (buffer[i] != delim[0])) {
                    while (!search(strlen(delim), delim, buffer[i]))
                    {                       
                        array_buffer[j] = buffer[i];
                        i++;
                        j++;
                    }
                    while(array_buffer[k] != '.'){
                        array_num[k] = array_buffer[k];
                        k++;
                    }
                    double round_number = atof(array_buffer);
                    double int_number = atof(array_num);
                    double diffrence = round_number - int_number;
                    if (diffrence >= 0.5){
                        int_number++;
                    }
                    int result = (int)int_number;
                    printf("%d", result);
                }
                
                while (buffer[i] != '\n')
                {
                    if(search(strlen(delim), delim, buffer[i])) {
                        column_counter++;
                        
                        if (column_counter == numberC-1) {
                            printf("%c", buffer[i]);
                            i++;
            
                            while (!search(strlen(delim), delim, buffer[i]))
                            {                       
                                array_buffer[j] = buffer[i];
                                i++;
                                j++;
                            }
                            while(array_buffer[k] != '.'){
                                array_num[k] = array_buffer[k];
                                k++;
                            }
                            double round_number = atof(array_buffer);
                            double int_number = atof(array_num);
                            double diffrence = round_number - int_number;
                            if (diffrence >= 0.5){
                                int_number++;
                            }
                            int result = (int)int_number;
                            printf("%d%c", result, delim[0]);
                            i++;
                        }
                       
                    }
                    printf("%c", buffer[i]);
                    i++;   
                }
                printf("\n");
            }
        }
        else if (strcmp(arguments[instruction_counter], "int") == 0)// int C - odstrani desetinnou cast cisla ve sloupci C.
        {
            if (instruction_counter == count_of_arguments-1) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int numberC = atoi(arguments[instruction_counter+1]);
            
            if (numberC <= 0){
                fprintf(stderr, "C must be > 0\n"); 
                return 1;
            }

            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                int column_counter = 0;
                int i = 0;
                int j = 0;
  
                char array_buffer[STR_SIZE];
                if ((numberC == 1) && (buffer[i] != delim[0])) {
                    while (!search(strlen(delim), delim, buffer[i]))
                    {                       
                        array_buffer[j] = buffer[i];
                        i++;
                        j++;
                    }
                    while (array_buffer[j] != '.')
                    {
                        j++;
                    }
                    
                    int round_number = 0;
                    round_number = atoi (array_buffer);
                    printf("%i", round_number);
                }
                
                while (buffer[i] != '\n')
                {
                    if(search(strlen(delim), delim, buffer[i])) {
                        column_counter++;
                        if (column_counter == numberC-1) {
                            printf("%c", buffer[i]);
                            i++;
            
                            while (!search(strlen(delim), delim, buffer[i]))
                            {                       
                                array_buffer[j] = buffer[i];
                                i++;
                                j++;
                            }
                            while (array_buffer[j] != '.'){
                                j++;
                            }       
                            int round_number = 0;
                            round_number = atoi(array_buffer);
                            printf("%i%c", round_number, delim[0]);
                            i++;
                        }
                       
                    }
                    printf("%c", buffer[i]);
                    i++;   
                }
                printf("\n");
            }
        }
        else if (strcmp(arguments[instruction_counter], "copy") == 0) // copy N M - prepise obsah bunek ve sloupci M hodnotami ze sloupce N.
        {
            if (instruction_counter == count_of_arguments-2) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int numberN = atoi(arguments[instruction_counter+1]);
            int numberM = atoi(arguments[instruction_counter+2]);

            if ((numberN > numberM)){
                fprintf(stderr, "copy arguments must be N <= M !\n"); 
                return 1;
            }
            else if ((numberM <= 0) || (numberN <= 0)){
                printf("M and N must be > 0 ");
            }

            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                int column_counter = 0;
                int i = 0;
                char array_to_copy[STR_SIZE] = "";
                if (numberN == 1) {
                    while (!search(strlen(delim), delim, buffer[i]))
                    {                       
                        printf("%c", buffer[i]);
                        strncat(array_to_copy, &buffer[i], 1);
                        i++;
                    }
                }
                 
                while (buffer[i] != '\n')
                {
                    if(search(strlen(delim), delim, buffer[i])) {
                        column_counter++;
                        if (column_counter == numberN-1)
                        {
                            printf("%c", delim[0]);
                            i++;
                            while (!search(strlen(delim), delim, buffer[i]))
                            {
                                printf("%c", buffer[i]);
                                strncat(array_to_copy, &buffer[i], 1);
                                i++;
                            }
                            
                        }
                    }
                    if (search(strlen(delim), delim, buffer[i])) 
                    {
                        int k=0;
                        (numberN == 1) ? (k = numberM-1) : (k = numberM-2);
                        if (column_counter == k)
                        {
                            i++;
                            printf("%c", delim[0]);
                            while (!search(strlen(delim), delim, buffer[i]))
                            {
                                i++;
                            }
                            for(unsigned int j = 0; j < strlen(array_to_copy); j++)
                            {
                                printf("%c",array_to_copy[j]);
                            }
                        }
                    }
                    printf("%c", buffer[i]);
                    i++;   
                }
                printf("\n");
            }
        }
        else if (strcmp(arguments[instruction_counter], "swap") == 0) // swap N M - zameni hodnoty bunek ve sloupcich N a M.
        {
            if (instruction_counter == count_of_arguments-2) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int numberN = atoi(arguments[instruction_counter+1]);
            int numberM = atoi(arguments[instruction_counter+2]);
            if ((numberN > numberM)){
                fprintf(stderr, "copy arguments must be N <= M !\n"); 
                return 1;
            }
            if ((numberM <= 0) || (numberN <= 0)){
                printf("M and N must be > 0 ");
                return 1;
            }
            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                char *token;
   
                token = strtok(buffer, delim);
                char* array [MAXSIZE];
                int counter = 0;
                while( token != NULL ) {
                    array[counter] = token;
                    token = strtok(NULL, delim);
                    counter++;
                }
                counter = 0;
                while (array[counter]!= NULL)
                {
                    if(counter == numberN-1) {
                        printf("%c%s", delim[0], array[numberM-1]);
                        counter++;
                    }
                    if (counter == numberM-1){
                        printf("%c%s", delim[0], array[numberN-1]);
                        counter++;
                    }
                    else{
                        printf("%c%s", delim[0], array[counter]);
                        counter++;
                    }
                }
            }
        }
        else if (strcmp(arguments[instruction_counter], "move") == 0) // move N M - presune sloupec N pred sloupec M.
        {
            if (instruction_counter == count_of_arguments-2) {
                fprintf(stderr, "Wrong count of arguments!\n");
                return 1;
            }  
            int numberN = atoi(arguments[instruction_counter+1]);
            int numberM = atoi(arguments[instruction_counter+2]);
            if ((numberN > numberM)){
                fprintf(stderr, "copy arguments must be N <= M !\n"); 
                return 1;
            }
            if ((numberM <= 0) || (numberN <= 0)){
                printf("M and N must be > 0 \n");
                return 1;
            }
            while (fgets(buffer, MAXSIZE, stdin) != NULL)
            {
                char *token;
   
                token = strtok(buffer, delim); 
                char* array [MAXSIZE];
                int counter = 0;
                while( token != NULL ) {
                    array[counter] = token;
                    token = strtok(NULL, delim);
                    counter++;
                }
                counter = 0;
                while (array[counter]!= NULL)
                {
                    if(counter == numberN-1) {
                        counter++;
                    }
                    if (((numberM - numberN) == 1) && (counter == numberN)){
                        
                        printf("%c%s", delim[0], array[numberN-1]);
                        printf("%c%s", delim[0], array[numberN]);
                        counter++;
                    }
                    if (counter == numberM-2){
                        printf("%c%s", delim[0], array[numberM-2]);
                        printf("%c%s", delim[0], array[numberN-1]);
                        counter++;
                    }
                    else{
                        printf("%c%s", delim[0], array[counter]);
                        counter++;
                    }
                }
            }
        }
        else {
            instruction_counter++;
            continue;        
        }
        instruction_counter++;    
    }
    return 0;
}


int main (int argc, char* argv []) // Kontroluje pocet argumentu
{   
    if (argc == 1)
    {
        fprintf(stderr, "Put arguments\n");
        return 1;
    }
    else
    {
        if (parse_arguments(argv, argc) == 1)
        {
            return 1;
        }
    }
    return 0;
}
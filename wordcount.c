/*
 * By: Kyle McKenzie
 * File: wordcount.c
 * Version: 1.0
 * Last modified on Tue Jun 09 2020 by Kyle McKenzie
 * -----------------------------------------------------
 * This following code creates "n" number of child processes
 * to read and count the words in "n" number of files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

int countWords(FILE *fp);
char *ReadLineFile(FILE *infile);

int main(int argc, char *argv[])
{   
    // variables
    int i;
    int numOfChildren = 0;
    int numFilesCounted = 0;
    int numFilesFailed = 0;
    int exitStatus = 0;
    long waitPID;
    
    for(i = 1; i < argc; i++) // loop through files
    {        
        if(fork() == 0) // execute the following if a child
        {
            FILE *filePointer;
            // open file. If null print message and EXIT_FAILURE
            if((filePointer = fopen(argv[i], "r")) == NULL)
            {
                printf("Child process %ld for file %s: does not exist\n"
                    , (long) getpid()
                    , argv[i]);
                exit(EXIT_FAILURE);
            }
            // declare int to store the total words.
            int totalWords = 0;
            // call count words function
            totalWords = countWords(filePointer);
            // print child PID, file name, and # of words
            printf("Child process %ld for file %s: number of words is %d\n"
                    , (long) getpid()
                    , argv[i]
                    , totalWords);
            // close file and EXIT_SUCCESS
            fclose(filePointer);
            exit(EXIT_SUCCESS);
        }
        numOfChildren += 1;
    }
    
    // wait for all processes
    while((waitPID = wait(&exitStatus)) > 0)
    {
        // success case
        if(WIFEXITED(exitStatus) && !WEXITSTATUS(exitStatus))
        {
            numFilesCounted++;
        }
        
        // failure case
        if(WIFEXITED(exitStatus) && WEXITSTATUS(exitStatus))
        {
            numFilesFailed++;
        }
    }
    
    // print stats
    printf("Main process created %d child process(es) to count words in %d file(s).\n", numOfChildren, numFilesCounted);
    printf("%d file(s) have been counted successfully!\n", numFilesCounted);
    printf("%d file(s) did not exist.\n", numFilesFailed);
 
    return 0;
}

/* 
 * Function: countWords
 * Usage: total = countWords(filePointer);
 * ----------------------------
 * countWords counts the number of words in a file and returns it.
 * Before calling this function a check to ensure the file exists
 * must be performed!
 */
int countWords(FILE *filePointer)
{
    // variables
    int totalWords = 0;
    char *fileLine = NULL;
    char delimeters[] = " \t\n";
    
    // Read one line of file at a time
    while((fileLine = ReadLineFile(filePointer)))
    {
        // loop through words in the line and increment count for each one
        if((strtok(fileLine, delimeters)))
        {
            totalWords += 1;
            
            while(strtok(NULL, delimeters))
            {
                totalWords += 1;
            }
        }
        free(fileLine); // free space
    }
    return totalWords;
}

/* 
 * Function: ReadLineFile
 * Usage: s = ReadLineFile(infile);
 * ----------------------------
 * ReadLineFile reads a line of text from the input file which 
 * is already open and pointed by infile. It then reads the line, 
 * dynamically allocates space, and returns the line as a string. 
 * The newline character that terminates the input is not stored 
 * as part of the string.  
 * The ReadLineFile function returns NULL if infile is at the 
 * end-of-file position. 
 */

char *ReadLineFile(FILE *infile)
{
    // allocate memory for input buffer.
    char *inputBuffer;
    inputBuffer = (char *)malloc(sizeof(char) * (100));
    if(inputBuffer == NULL) // confirm allocation. exit if failed.
    {
        printf("Could not allocate buffer!\n");
        exit(1);
    }
    
    int tempChar;
    int index = 0;
    int currentBufferSize = 100;
    
    while(1)
    {
        tempChar = fgetc(infile);
        if(tempChar == EOF && index == 0) // if input stream empty free memory and return NULL
        {
            free(inputBuffer);
            return NULL;
        }
        
        // newline or EOF reached, allocate exact size and copy data. 
        if(tempChar == '\n' || tempChar == EOF)
        {
            char *newBuffer;
            newBuffer = (char *)malloc(sizeof(char) * (index + 1));
            if(newBuffer == NULL)
            {
                printf("Could not allocate new buffer!\n");
                exit(1);
            }
            memcpy(newBuffer, inputBuffer, index);
            newBuffer[index] = '\0';
            free(inputBuffer); // free old buffer
            return newBuffer;
        }
        
        if(index > currentBufferSize) // if size is not big enough reallocate
        {
            inputBuffer = (char *)realloc(inputBuffer, currentBufferSize * 2);
            if(inputBuffer == NULL)// confirm reallocation
            {
                printf("Could not re-allocate buffer!\n");
                free(inputBuffer);
                exit(1);
            }
            currentBufferSize = currentBufferSize * 2;// adjust current buffer size
        }

        inputBuffer[index] = tempChar; //add character to input buffer
        index++; // advance index
    }
    
  return(NULL);   // if there is any error!
}
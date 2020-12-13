#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <omp.h>
#include "../hash/hash.h"
#include "dictionary-util.c"


int dictionary_crack(char *password_hash, char *dictionary_path, int verbose);

void compare_candidates(FILE **file, char *password_hash, int verbose, int *result, char **password_text);


/* 
    dictionary_crack()
        - OpenMP Implementation

        password_hash:      hashed password string to crack
        dictionary_path:    full path, including filename (OpenMP)
        verbose:            set to 1 for verbose mode
*/
int dictionary_crack(char *password_hash, char *dictionary_path, int verbose)
{
    int result = NOT_FOUND;         /* default: match not found */
    int file_failure = SUCCESS;     /* default: no failure */

    char *password = NULL;

    FILE *file = NULL;
    open_dictionary_file(dictionary_path, &file, OMP, &file_failure);

    if(verbose)
    {
        printf("\n>>> Using dictionary path: %s\n\n", dictionary_path);
        print_password_hash(password_hash);
    }

    #pragma omp parallel
      #pragma omp single
        compare_candidates(&file, password_hash, verbose, &result, &password);

    close_dictionary_file(&file);

    if(result == NOT_FOUND)
        print_not_found(verbose);
    else if(result == FOUND)
        print_password_found(password, verbose);

    free(password);

    return result;
}



/* 
    compare_candidates()
        - 1. manages iterating through the dictionary file and initiating the has comparisons
        - 2. returns the result value (FOUND or NOT_FOUND) and the plain text password, if found

        file:               pointer to the dictionary file in memory
        password_hash:      hashed value of the password to be cracked
        verbose:            set to 1 for verbose mode
        result:             (output) FOUND or NOT_FOUND result
        password_text:      (output) plain text of the discovered password

*/
void compare_candidates(FILE **file, char *password_hash, int verbose, int *result, char **password_text)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int count = 0;  /* for implementations that require a counter */

    while ((read = getline(&line, &len, *file)) != -1)
    {
        char *candidate_buffer = NULL;
        remove_new_line(line, &candidate_buffer);
     
        
        #pragma omp task firstprivate(candidate_buffer)
        {
            //TODO: Make an OMP_COUNT_LIMIT or make these use the same macro??
            if(count == MPI_COUNT_LIMIT)
            {
                // do something to check
            }

            /* if NOT_FOUND, keep looking */
            do_comparison(password_hash, candidate_buffer, verbose, result, password_text);

            /* This STOPS the processing of the file on the process that FOUND the password */
            if(*result == FOUND)
            {
                if(verbose)
                    printf("We've got OMP things to figure out here....\n");
                //return;
            }
        }
    }
}

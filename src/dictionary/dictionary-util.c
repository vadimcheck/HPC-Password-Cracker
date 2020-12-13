#include "dictionary.h"


/* 
    remove_new_line()
        - checks for the new line character, removes if found

        input:  a string that might have a new line character at the end
        ouput:  a string with no newline character at the end
*/
void remove_new_line(char *input, char **output)
{
    int len = strlen(input);

    if (input[len - 1] == '\n')
    {
        input[len - 1] = '\0';
        *output = (char *)malloc(sizeof(char) * len);
        strncpy(*output, input, len);
    }
    else
    {
        *output = (char *)malloc(sizeof(char) * (len + 1));
        sprintf(*output, "%s\0", input);
    }
}

/* 
    return_password_text()
        - copies the text of the candidate password into the location

        input:  the string to be copied
        ouput:  a copy of the input string, as the requested location
*/
void return_password_text(char *input, char **output)
{
    int len = strlen(input);

    *output = (char *)malloc(sizeof(char) * len);
    memcpy(*output, input, len);

    // TODO: fix the printing problem with the MPI *output value. Often has extra junk. But the comparison is accurate.
}

/* 
    print_password_hash()
        - pretty print of the provided password hash

        password_hash:    the hash of the password the program is looking for
*/
void print_password_hash(char *password_hash)
{
    printf("\n---------------------------------------------------------------------------------------------------------------------------------\n");
    printf("Looking for this password hash:\t\t\t\t\t%s\n", password_hash);
    printf("---------------------------------------------------------------------------------------------------------------------------------\n");
    printf("\n");
}

/* 
    print_password_found()
        - print the found password; verbose mode enables pretty print

        match:     the plain text password to be printed
        verbose:   set to 1 for verbose mode
*/
void print_password_found(char *match, int verbose)
{
    if(verbose)
        printf("\n>>> SUCCESS!! ");

    printf("Password found: %s\n", match);

    if(verbose)
        printf("\n");

}

/* 
    print_not_found()
        - print statement that match was not found; verbose mode enables pretty print

        verbose:   set to 1 for verbose mode
*/
void print_not_found(int verbose)
{
    if(verbose)
        printf("\n >>> ");
        
    printf("Password not found.\n");

    if(verbose)
        printf("\n");
}

/* 
    open_dictionary_file()
        - opens the directory file from the provided location

        dictionary_path:    the full path to the dictionary file
        file:               pointer to the dictionary file in memory
        mode:               the implementation mode (SERIAL, MPI, OMP, etc.)
        verbose:            set to 1 for verbose mode

*/
void open_dictionary_file(char *dictionary_path, FILE **file, int mode, int *failure)
{
    *file = fopen(dictionary_path, "r");

    if (file == NULL)
    {
        printf("Error reading dictionary file: %s\n", dictionary_path);

        switch(mode)
        {
            case SERIAL:
                //*failure = FAILURE;   // Not needed, as the program exits
                printf("Exiting with error code %d.\n", EXIT_FAILURE);
                exit(EXIT_FAILURE);
                break;
            case MPI:
                *failure = FAILURE;
                break;
            case OMP:
                *failure = FAILURE;
                exit(EXIT_FAILURE);        // IF this works, it's the same as SERIAL... consolidate?
                break;
            default:
                // nothing here?
                break;

        }
    }

    *failure = SUCCESS;
}

/* 
    close_dictionary_file()
        - closes the file

        file:               pointer to the file in memory
*/
void close_dictionary_file(FILE **file)
{
    fclose(*file);
}

/* 
    do_comparison()
        - 1. hashes the plaintext password candidate from the dictionary file
        - 2. compares two hashed values
        - 3. returns the result value (FOUND or NOT_FOUND) and the plain text password, if found

        password_hash:      hashed value of the password to be cracked
        candidate_buffer:   plain text password candidate from the dictionary file
        verbose:            set to 1 for verbose mode
        result:             (output) FOUND or NOT_FOUND result
        password_text:      (output) plain text of the discovered password

*/
void do_comparison(char *password_hash, char *candidate_buffer, int verbose, int *result, char **password_text)
{
    unsigned char candidate_hash[65];
    hash(candidate_buffer, candidate_hash);

    if (verbose)
        printf("Password candidate from file:\t%16s\t--->\t%s\n", candidate_buffer, candidate_hash);   

    if (!strcmp(password_hash, candidate_hash))
    {
        *result = FOUND;
        return_password_text(candidate_buffer, password_text);
        free(candidate_buffer);

    }
}

/* 
    compare_candidates()
        - 1. manages iterating through the dictionary file and initiating the has comparisons
        - 2. returns the result value (FOUND or NOT_FOUND) and the plain text password, if found

        file:               pointer to the dictionary file in memory
        password_hash:      hashed value of the password to be cracked
        mode:               the implementation mode (SERIAL, MPI, OMP, etc.)
        verbose:            set to 1 for verbose mode
        result:             (output) FOUND or NOT_FOUND result
        password_text:      (output) plain text of the discovered password

*/
void compare_candidates(FILE **file, char *password_hash, int mode, int verbose, int *result, char **password_text)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int count = 0;  /* for implementations that require a counter */

    while ((read = getline(&line, &len, *file)) != -1)
    {
        char *candidate_buffer = NULL;
        remove_new_line(line, &candidate_buffer);
     
        switch(mode)
        {
            case SERIAL:
                do_comparison(password_hash, candidate_buffer, verbose, result, password_text);

                if(*result == FOUND)
                    return;

                break;

            case MPI:
                /* First check if it is already FOUND, and return if FOUND */
                if(count == MPI_COUNT_LIMIT)
                {
                    if( mpi_result_check(NOT_FOUND) == FOUND)
                        return;

                    count = 0;
                }

                /* if NOT_FOUND, keep looking */
                do_comparison(password_hash, candidate_buffer, verbose, result, password_text);
                count++;

                /* This STOPS the processing of the file on the process that FOUND the password */
                if(*result == FOUND)
                {
                    /* report back that the match is found */
                    mpi_result_check(FOUND);
                    return;
                }

                break;

            case OMP:            
                #pragma omp task firstprivate(candidate_buffer)
                {
                    do_comparison(password_hash, candidate_buffer, verbose, result, password_text);

                    if(*result == FOUND)
                        printf("We've got OMP things to do, Miss Donna!\n");
                }
                break;
        }        
    }
}


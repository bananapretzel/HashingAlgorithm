#include <stdio.h>
#include <stdlib.h>
#include "mylib.h"
#include "htable.h"
#include <getopt.h>

#define DEFAULT_SIZE 113

static int next_prime(int n);
static int is_prime(int n);
static void print_help(FILE *stream);

/**
 * Main function. Interprets the arguments passed to the program at execution.
 * Uses the arguments to control the following program functionality. If no
 * flags are specified the default functionality is creating a hash table and
 * reading strings into it from stdin. Linear probing is used by default.
 * 
 * @param argc - the number of command line arguments passed to the program.
 * @param argv - array to point to each argument passed to the program.
 * @return - integer value. 0 if program exits successfully.
 */
int main(int argc, char **argv)
{
    /* variable for getopt */
    const char *optstring = "deps:t:h";
    char option;
    char word[256];
    hashing_t hash_method = LINEAR_P;
    int capacity = DEFAULT_SIZE;
    int print_table = 0, print_stats = 0, snapshots = 0;
    htable h;

    while ((option = getopt(argc, argv, optstring)) != EOF)
    {
        switch (option)
        {
        case 'd':
            hash_method = DOUBLE_H;
            break;

        case 'e':
            print_table = 1;
            break;

        case 'p':
            print_stats = 1;
            break;

        case 's':
            snapshots = atoi(optarg);
            break;

        case 't':
            capacity = next_prime(atoi(optarg));
            break;

        case 'h':
        default:
            print_help(stdout);
            return EXIT_SUCCESS;
        }
    }
    h = htable_new(capacity, hash_method);

    while (getword(word, sizeof word, stdin) != EOF)
    {
        htable_insert(h, word);
    }
    if (print_table)
    {
        htable_print_entire_table(h, stderr);
    }
    if (print_stats)
    {
        if (snapshots > 0)
        {
            htable_print_stats(h, stdout, snapshots);
        }
        else
        {
            htable_print_stats(h, stdout, 10);
        }
    }
    else
    {
        htable_print(h, stdout);
    }
    htable_free(h);
    return EXIT_SUCCESS;
}

/**
 * Finds the smallest prime greater than integer n, assuming n > 0.
 * Uses function is_prime to test each candidate for prime status.
 * 
 * @param n - integer to start iterating upwards from.
 * @return - the smallest prime larger than n.
 */
static int next_prime(int n)
{
    int candidate = n;

    while (!is_prime(candidate))
    {
        candidate++;
    }
    return candidate;
}

/**
 * Test if the proposed integer n is prime.
 * 
 * @param n - integer to test if it is a prime.
 * @return - an integer of the result. Value 1 means n is prime, 0 if not.
 */
static int is_prime(int n)
{
    int i;

    /* If n is 1 or 0. */
    if (n <= 1)
    {
        return 0;
    }

    /* Test if proposed number is prime. */
    for (i = 2; i * i <= n; i++)
    {
        if (n % i == 0)
        {
            return 0;
        }
    }
    return 1;
}

/**
 * Print the program's help menu. Used if requested with -h flag or an
 * unsupported flag.
 * 
 * @param  stream - a stream to print the menu to.
 */
static void print_help(FILE *stream)
{
    fprintf(stream, "Usage: ./sample-asgn [OPTION]... <STDIN>\n\nPerform "
                    "various operations using a hash table.  By default,"
                    " words are\nread from stdin and added to the hash "
                    "table, before being printed out\nalongside their "
                    "frequencies to stdout.\n\n");
    fprintf(stream, " -d           Use double "
                    "hashing (linear probing is the default)\n -e           "
                    "Display entire contents of hash table on stderr\n -p"
                    "           Print stats info instead of frequencies & "
                    "words\n -s SNAPSHOTS Show SNAPSHOTS stats snapshots "
                    "(if -p is used)\n -t TABLESIZE Use the first prime >= "
                    "TABLESIZE as htable size\n\n -h           Display this "
                    "message\n\n");
}

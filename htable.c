#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mylib.h"
#include "htable.h"

void htable_print_stats(htable h, FILE *stream, int num_stats);
static void print_stats_line(htable h, FILE *stream, int percent_full);

struct htablerec
{
    int capacity;
    int num_keys;
    char **keys;
    int *freqs;
    int *stats;
    hashing_t method;
};

/**
 * Converts a given string into an int value
 * ready to give a hash value to.
 *
 * @param word - the string to convert to an unsigned int.
 * @return the result as an unsigned int. 
 */
static unsigned int htable_word_to_int(char *word)
{
    unsigned int result = 0;
    while (*word != '\0')
    {
        result = (*word++ + 31 * result);
    }
    return result;
}

/**
 * Calculates the step to be used for double hashing
 *
 * @param h - the hash table.
 * @param i_key - an integer to calculate step on.
 * @return the step as an unsigned int.
 */
static unsigned int htable_step(htable h, unsigned int i_key)
{
    return 1 + (i_key % (h->capacity - 1));
}

/**
 * Frees the memory allocated to the hash table.
 *
 * @param h - the hash table.
 */
void htable_free(htable h)
{
    int i;
    for (i = 0; i < h->capacity; i++)
    {
        if (h->keys[i] != NULL)
        {
            free(h->keys[i]);
        }
    }
    free(h->keys);
    free(h->freqs);
    free(h);
}

/**
 * Inserts given string into the hash table
 * Tries to insert in the first available slot, if unable then steps based
 * on the method (linear probing or double hashing)
 * Updates the freqs array accordingly.
 * 
 * @param h - the hash table.
 * @param str - a string to insert into the table.
 * @return an int of how many times that string has been 
 * inserted, 0 if table is full.
 */
int htable_insert(htable h, char *str)
{
    unsigned int str_to_idx = htable_word_to_int(str);
    unsigned int index = str_to_idx % h->capacity;
    unsigned int collisions = 0;
    unsigned int step;

    if (h->method == DOUBLE_H)
    {
        step = htable_step(h, str_to_idx);
    }
    else
    {
        step = 1;
    }
    /* If the cell is empty, add the string to the cell. */
    if (h->keys[index] == NULL)
    {
        h->keys[index] = emalloc(strlen(str) + 1 * sizeof h->keys[0][0]);
        strcpy(h->keys[index], str);
        h->freqs[index]++;
        h->num_keys++;
        return 1;
    }
    /* If cell is populated with the string, increment freq count. */
    else if (strcmp(h->keys[index], str) == 0)
    {
        return ++h->freqs[index];
    }
    /* If the cell is populated with another string decide what to do. */
    else
    {
        /* Try to find vacant cell */
        while (h->keys[index] != NULL && strcmp(h->keys[index], str) != 0)
        {
            collisions++;
            index = (index+step)%h->capacity;
            if (collisions >= (unsigned int)h->capacity)
            {
                return 0;
            }
        }
        /* If cell is found and vacant add string to it. */
        if (h->keys[index] == NULL)
        {
            h->keys[index] = emalloc(strlen(str) + 1 * sizeof h->keys[0][0]);
            strcpy(h->keys[index], str);
            h->freqs[index]++;
            h->stats[h->num_keys] += collisions;
            h->num_keys++;

            return 1;
        }
        /* If cell is found but populated with the same string just increment the freq count. */
        else
        {
            return ++h->freqs[index];
        }
    }
}

/**
 * Creates a new hash table, allocates the required memory for the freqs,
 * keys and stats arrays. Initialises the appropriate struct members.
 * 
 * @param capacity - the desired size of hash table.
 * @param hash_method - the desired hashing method, linear probing or double hashing.
 * @return the created hash table.
 */
htable htable_new(int capacity, hashing_t hash_method)
{
    int i;
    /* Create a table and allocate memory for it. */
    htable result = emalloc(sizeof *result);

    /* Initialises capacity, the number of keys, and hashing method. */
    result->capacity = capacity;
    result->num_keys = 0;
    result->method = hash_method;

    /* Allocate memory for freqs, keys, and stats. */
    result->freqs = emalloc(result->capacity * sizeof result->freqs[0]);
    result->keys = emalloc(result->capacity * sizeof result->keys[0]);
    result->stats = emalloc(result->capacity * sizeof result->stats[0]);

    /* Set keys to NULL and freqs to 0. */

    for (i = 0; i < capacity; i++)
    {
        result->keys[i] = NULL;
        result->freqs[i] = 0;
        result->stats[i] = 0;
    }

    return result;
}

/**
 * Prints the hash table, frequency in column 1 and key in column 2.
 * Only prints the keys which are in table
 * 
 * @param h - the hash table.
 * @param stream - the stream to send output to.
 */
void htable_print(htable h, FILE *stream)
{
    int i;
    for (i = 0; i < h->capacity; i++)
    {
        if (h->keys[i] != NULL)
        {
            fprintf(stream, "%d    %s\n", h->freqs[i], h->keys[i]);
        }
    }
}

/**
 * Prints the hash table with extra information, including position,
 * frequency, stats, and key.
 * Prints out the whole table in order, shows the empty positions
 * in table as well.
 * 
 * @param h - the hash table.
 * @param stream - the stream to send output to.
 */
void htable_print_entire_table(htable h, FILE *stream)
{
    int i;

    /* Print header. */
    fprintf(stderr, "  Pos  Freq  Stats  Word\n");
    fprintf(stderr, "----------------------------------------\n");

    /* Print hash table data. */
    for (i = 0; i < h->capacity; i++)
    {
        fprintf(stream, "%5d %5d %5d", i, h->freqs[i]
        , h->stats[i]);
        
        if (h->keys[i] != 0)
        {
            fprintf(stream, "   %s\n", h->keys[i]);
        } else {
            fprintf(stream, "\n");
        }
    }
}

/**
 * Searches the table for string, goes through the same process as the
 * insert function to find the string. If not found returns 0.
 * 
 * @param h - the hash table.
 * @param str - the the string to search for.
 * @return - an int of the frequency, how many times that specific
 * string has been inserted.
 */
int htable_search(htable h, char *str)
{
    int collisions = 0;

    int index = (htable_word_to_int(str)) % h->capacity;

    int step = htable_step(h, index);

    while (h->keys[index] != NULL && strcmp(h->keys[index], str) != 0)
    {
        if (collisions == h->capacity)
        {
            return 0;
        }

        index = (index + step) % h->capacity;

        collisions++;
    }

    return h->freqs[index];
}

/**
 * Prints out a table showing what the following attributes were like
 * at regular intervals (as determined by num_stats) while the
 * hashtable was being built.
 *
 * @li Percent At Home - how many keys were placed without a collision
 * occurring.
 * @li Average Collisions - how many collisions have occurred on
 *  average while placing all of the keys so far.
 * @li Maximum Collisions - the most collisions that have occurred
 * while placing a key.
 *
 * @param h the hashtable to print statistics summary from.
 * @param stream the stream to send output to.
 * @param num_stats the maximum number of statistical snapshots to print.
 */

void htable_print_stats(htable h, FILE *stream, int num_stats)
{
    int i;

    fprintf(stream, "\n%s\n\n",
            h->method == LINEAR_P ? "Linear Probing" : "Double Hashing");
    fprintf(stream, "Percent   Current    Percent    Average      Maximum\n");
    fprintf(stream, " Full     Entries    At Home   Collisions   Collisions\n");
    fprintf(stream, "------------------------------------------------------\n");
    for (i = 1; i <= num_stats; i++)
    {
        print_stats_line(h, stream, 100 * i / num_stats);
    }
    fprintf(stream, "------------------------------------------------------\n\n");
}

/**
 * Prints out a line of data from the hash table to reflect the state
 * the table was in when it was a certain percentage full.
 * Note: If the hashtable is less full than percent_full then no data
 * will be printed.
 *
 * @param h - the hash table.
 * @param stream - a stream to print the data to.
 * @param percent_full - the point at which to show the data from.
 */
static void print_stats_line(htable h, FILE *stream, int percent_full)
{
    int current_entries = h->capacity * percent_full / 100;
    double average_collisions = 0.0;
    int at_home = 0;
    int max_collisions = 0;
    int i;

    if (current_entries > 0 && current_entries <= h->num_keys)
    {
        for (i = 0; i < current_entries; i++)
        {
            if (h->stats[i] == 0)
            {
                at_home++;
            }
            if (h->stats[i] > max_collisions)
            {
                max_collisions = h->stats[i];
            }
            average_collisions += h->stats[i];
        }

        fprintf(stream, "%4d %10d %11.1f %10.2f %11d\n", percent_full,
                current_entries, at_home * 100.0 / current_entries,
                average_collisions / current_entries, max_collisions);
    }
}

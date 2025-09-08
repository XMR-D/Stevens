#include <sys/ioctl.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "list-handling.h"
#include "opt_parser.h"
#include "utility.h"

#include "printing.h"

/* TODO DEBUG THIS LAST STEP + ADD options handling */
static void PrintListing(FileList * list, int row_nb)
{
    /* Setup the start of the list (first elm) */
    if (!list->fname && list->next)
        list = list->next;

    FileList * start = list;
    FileList * curr = list;
    char * name;

    for (int i = 0; i < row_nb; i++)
    {
        name = strrchr(list->fname, '/');
        printf("%s  ", ++name);

        for (int i = 0; i < row_nb; i++)
        {
            if (curr->next)
                curr = curr->next;
            else
            {
                printf("\n");
                start = start->next;
                break;
            }
        }
        if (!curr->next)
        {
            name = strrchr(curr->fname, '/');
            printf("%s  ", ++name);
            break;
        }
        curr = start;
    }
    printf("\n");
}


static int GetWinWidth(void)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

static int GetMaxLen(FileList * list, int * nb_files)
{
    int max = 0;
    int curr_len = 0;

    if (!list->fname && list->next)
        list = list->next;

    while (list)
    {
        curr_len = strlen(list->fname);

        (*nb_files)++;

        if (max < curr_len)
            max = curr_len;

        list = list->next;
    }
    return max;
}


void ClassicPrinter(FileList * list)
{
    /* If the list is empty (no files to print) return */
    if (!list->next)
        return;
    /* 
     * Compute rows and columns to print the files
     * The width of a column is the size of the largest file + 2 for padding
     * Given by GetMaxLen(list);
     * Get the width of the terminal to compute how to print the output
     * Given by GetWinWidth();
    */

    int nb_files = 0;
    int col_nb = GetWinWidth() / GetMaxLen(list, &nb_files);
    int row_nb = (nb_files + col_nb - 1) / col_nb;
    PrintListing(list, row_nb);
}

void LongFormatPrinter(FileList * list)
{
    ClassicPrinter(list);
    return;
}
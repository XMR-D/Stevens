#include <sys/ioctl.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "list-handling.h"
#include "opt-parser.h"
#include "utility.h"

#include "printing.h"

/* TODO DEBUG THIS LAST STEP + ADD options handling */
static void PrintListingInColumn(FileList * list, int longest, int row_nb, int col_nb)
{
    /* Setup the start of the list (first elm) */
    if (!list->fname && list->next)
        list = list->next;

    FileList * start = list;
    FileList * curr = list;
    int step = row_nb;

    char * name = strrchr(curr->fname, '/');
    printf("%s", ++name);

    Padding(curr->fname, longest);

    for (int i = 0; i < row_nb; i++)
    {
        for (int j = 0; j < col_nb; j++)
        {
            
            while (step > 0)
            {
                if (!curr->next)
                    break;

                curr = curr->next;
                step--;
            }

            if (!curr->next)
                break;

            step = row_nb;

            /* Print the file with the correct padding*/
            name = strrchr(curr->fname, '/');
            printf("%s", ++name);
            Padding(curr->fname, longest);

        }
 
        start = start->next;
        curr = start;
        printf("\n");
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
     *
     * First, we get the number of files and the width of a column (size of the largest file + 2 for padding)
     * Given by GetMaxLen(list, &nb_files);
     *
     * Then we get the width of the terminal Given by GetWinWidth() to compute the number of columns and rows
     *
     * Then we call PrintListingInColumn with the listing of the current dir
     * with the good format options to print it in columns.
    */
    int nb_files = 0;
    int longest = GetMaxLen(list, &nb_files);
    int col_nb = GetWinWidth() / longest;
    int row_nb = (nb_files + col_nb - 1) / col_nb;
    PrintListingInColumn(list, longest, row_nb, col_nb);
}

void LongFormatPrinter(FileList * list)
{
    ClassicPrinter(list);
    return;
}
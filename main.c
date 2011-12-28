
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "sequence.h"
#include "coloring.h"
#include "recurse.h"
#include "filters.h"
#include "check.h"

#define strmatch(s, r) (!strcasecmp ((s), (r)))
#define MATCH_THEN_SET(tok, text)		\
  ((tok) != NULL && strmatch ((tok), #text))	\
    {						\
      char *new_tok = strtok (NULL, " \t\n");	\
      (text) = strtoul (new_tok, NULL, 0);	\
    }

int main (int argc, char *argv[])
{
  /* Runtime data */
  long iterations = 0;
  int min_gap = 1;
  int max_gap = 0;
  int n_colors = 3;
  int ap_length = 3;
  Sequence *alphabet = sequence_parse ("[1 2 3 4]");
  filter_func filter = cheap_check_sequence3;
  /* END Runtime data */

  char buf[1024];
  FILE *fh;
  /* Open script */
  if (argc > 1)
    {
      fh = fopen (argv[1], "r");
      if (fh == NULL)
        {
          fprintf (stderr, "Failed to open script ``%s''\n", argv[1]);
          exit (EXIT_FAILURE);
        }
    }
  else fh = stdin;

  /* Parse */
  while (fgets (buf, sizeof buf, fh))
    {
      char *tok = strtok (buf, " \t\n");
      /* skip comments and blank lines */
      if (tok == NULL || *tok == '#')
        continue;

      /* set <min_gap|max_gap|n_colors|ap_length|alphabet> <N> */
      if (strmatch (tok, "set"))
        {
          tok = strtok (NULL, " \t\n");
          if MATCH_THEN_SET (tok, min_gap)
          else if MATCH_THEN_SET (tok, max_gap)
          else if MATCH_THEN_SET (tok, n_colors)
          else if MATCH_THEN_SET (tok, ap_length)
          else if MATCH_THEN_SET (tok, iterations)
          else if (strmatch (tok, "alphabet"))
            {
              tok = strtok (NULL, "\n");
              sequence_delete (alphabet);
              alphabet = sequence_parse (tok);
            }
        }
      /* filter <no-double-3-aps|no-additive-squares> */
      else if (strmatch (tok, "filter"))
        {
          tok = strtok (NULL, " \t\n");
          if (tok && strmatch (tok, "no-double-3-aps"))
            filter = cheap_check_sequence3;
          else if (tok && strmatch (tok, "no-additive-squares"))
            filter = cheap_check_additive_square;
          else
            fprintf (stderr, "Unknown filter ``%s''\n", tok);
        }
      /* search <seqences|colorings|words> [seed] */
      else if (strmatch (tok, "search"))
        {
          time_t start = time (NULL);
          reset_max ();

          tok = strtok (NULL, " \t\n");
          if (tok && strmatch (tok, "sequences"))
            {
              Sequence *seek;

              tok = strtok (NULL, " \t\n");
              if (tok && *tok == '[')
                seek = sequence_parse (tok);
              else
                {
                  seek = sequence_new ();
                  sequence_append (seek, 1);
                }

              puts ("#### Starting sequence search ####");
              if (iterations > 0)
                printf ("  Stop after: \t%ld iterations\n", iterations);
              printf ("  Minimum gap:\t%d\n", min_gap);
              printf ("  Maximum gap:\t%d\n", max_gap);
              printf ("  AP length:\t%d\n", ap_length);
              printf ("  Seed Seq.:\t"); sequence_print (seek);
              puts("\n");

              if (seek == NULL)
                {
                  fprintf (stderr, "Failed to allocate sequence.");
                  exit (EXIT_FAILURE);
                }

              recurse_sequence (seek, min_gap, max_gap, filter, iterations);

              sequence_delete (seek);
            }
          else if (strmatch (tok, "colorings") ||
                   strmatch (tok, "partitions"))
            {
              Coloring *seek;

/*
              tok = strtok (NULL, " \t\n");
              if (tok && *tok == '[')
                seek = sequence_parse (tok);
              else
*/
                {
                  seek = coloring_new (n_colors);
                  coloring_append (seek, 1, 0);
                }

              puts ("#### Starting coloring search ####");
              if (iterations > 0)
                printf ("  Stop after: \t%ld iterations\n", iterations);
              printf ("  Minimum gap:\t%d\n", min_gap);
              printf ("  Maximum gap:\t%d\n", max_gap);
              printf ("  AP length:\t%d\n", ap_length);
              printf ("  Seed Col.:\t"); coloring_print (seek);
              puts("");

              recurse_colorings (seek, 1, min_gap, max_gap, filter, iterations);

              coloring_delete (seek);
            }
          else if (tok && strmatch (tok, "words"))
            {
              Sequence *seek = sequence_new ();

              puts ("#### Starting word search ####");
              if (iterations > 0)
                printf ("  Stop after: \t%ld iterations\n", iterations);
              printf ("  Alphabet:\t"); sequence_print (alphabet);
              printf ("\n  Seed Seq.:\t"); sequence_print (seek);
              puts("\n");

              recurse_words (seek, alphabet, filter, iterations);

              sequence_delete (alphabet);
              sequence_delete (seek);
            }
          else
            fprintf (stderr, "Unrecognized search space ``%s''\n", tok);
          printf ("Done. Time taken: %ds. Iterations: %ld\n", (int) (time (NULL) - start), get_iterations());
          puts("\n");
        }
    }

  /* Cleanup */
  if (fh != NULL && fh != stdin)
    fclose (fh);
  return 0;
}


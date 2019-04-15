#include <stdlib.h>
#include <stdio.h>
#include "canvas.h"

int main(int argc, char const *argv[])
{
    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        perror("file");
        exit(1);
    }


    Canvas *c = canvas_readf(f);
    fclose(f);

    printf("New canvas of size %dx%d\n", c->num_cols, c->num_rows);
    canvas_print(c);

    char* outfname = "out.txt";
    FILE *outf = fopen(outfname, "w");
    if (outf == NULL) {
        perror("outfile");
        exit(1);
    }

    canvas_writef(c, outf);
    printf("Wrote to %s", outfname);
    fclose(outf);

    return 0;
}

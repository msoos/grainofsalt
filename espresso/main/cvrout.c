/*
    module: cvrout.c
    purpose: cube and cover output routines
*/

#include "espresso.h"

void fprint_pla(PLA, output_type, no_inputs, no_outputs, input, output, no_lines)
IN pPLA PLA;
IN int output_type;
int** input;
int** output;
int* no_lines;
{
    int num;
    register pcube last, p;
   {
	num = 0;
	if (output_type & F_type) {
		num += (PLA->F)->count;
		no_lines[1] = (PLA->F)->count;
	}
	if (output_type & D_type) {
		num += (PLA->D)->count;
		no_lines[2] = (PLA->D)->count;
	}
	if (output_type & R_type) {
		num += (PLA->R)->count;
		no_lines[0] = (PLA->R)->count;
	}
	
	int line = 0;
	    if (output_type & F_type) {
		foreach_set(PLA->F, last, p) {
		    	print_cube(p, "~1", input[line], output[line]);
		    	line++;
		}
	    }
	    if (output_type & D_type) {
		foreach_set(PLA->D, last, p) {
			print_cube(p, "~2", input[line], output[line]);
		    	line++;
		}
	    }
	    if (output_type & R_type) {
		foreach_set(PLA->R, last, p) {
			print_cube(p, "~0", input[line], output[line]);
		    	line++;
		}
	    }
    }
}


/*
    eqntott output mode -- output algebraic equations
*/
void eqn_output(PLA)
pPLA PLA;
{
    register pcube p, last;
    register int i, var, col, len;
    int x;
    bool firstand, firstor;

    if (cube.output == -1)
	fatal("Cannot have no-output function for EQNTOTT output mode");
    if (cube.num_mv_vars != 1)
	fatal("Must have binary-valued function for EQNTOTT output mode");
    makeup_labels(PLA);

    /* Write a single equation for each output */
    for(i = 0; i < cube.part_size[cube.output]; i++) {
	printf("%s = ", OUTLABEL(i));
	col = strlen(OUTLABEL(i)) + 3;
	firstor = TRUE;

	/* Write product terms for each cube in this output */
	foreach_set(PLA->F, last, p)
	    if (is_in_set(p, i + cube.first_part[cube.output])) {
		if (firstor)
		    printf("("), col += 1;
		else
		    printf(" | ("), col += 4;
		firstor = FALSE;
		firstand = TRUE;

		/* print out a product term */
		for(var = 0; var < cube.num_binary_vars; var++)
		    if ((x=GETINPUT(p, var)) != DASH) {
			len = strlen(INLABEL(var));
			if (col+len > 72)
			    printf("\n    "), col = 4;
			if (! firstand)
			    printf("&"), col += 1;
			firstand = FALSE;
			if (x == ZERO)
			    printf("!"), col += 1;
			printf("%s", INLABEL(var)), col += len;
		    }
		printf(")"), col += 1;
	    }
	printf(";\n\n");
    }
}


char *fmt_cube(c, out_map, s)
register pcube c;
register char *out_map, *s;
{
    register int i, var, last, len = 0;

    for(var = 0; var < cube.num_binary_vars; var++) {
	s[len++] = "?01-" [GETINPUT(c, var)];
    }
    for(var = cube.num_binary_vars; var < cube.num_vars - 1; var++) {
	s[len++] = ' ';
	for(i = cube.first_part[var]; i <= cube.last_part[var]; i++) {
	    s[len++] = "01" [is_in_set(c, i) != 0];
	}
    }
    if (cube.output != -1) {
	last = cube.last_part[cube.output];
	s[len++] = ' ';
	for(i = cube.first_part[cube.output]; i <= last; i++) {
	    s[len++] = out_map [is_in_set(c, i) != 0];
	}
    }
    s[len] = '\0';
    return s;
}


void print_cube(c, out_map, input, output)
register pcube c;
register char *out_map;
int* output;
int* input;
{
    register int i, var, ch;
    int last;

    int where = 0;
    for(var = 0; var < cube.num_binary_vars; var++) {
	ch = "?01-" [GETINPUT(c, var)];
	//printf("%c", ch);
	switch(ch) {
		case '0':
			input[where] = 0;
			break;
		case '1':
			input[where] = 1;
			break;
		case '-':
			input[where] = 2;
			break;
	}
	where++;
    }
    for(var = cube.num_binary_vars; var < cube.num_vars - 1; var++) {
	    //printf(" ", ch);
	for(i = cube.first_part[var]; i <= cube.last_part[var]; i++) {
	    ch = "01" [is_in_set(c, i) != 0];
	    //printf("%c", ch);
	    switch(ch) {
		    case '0':
			    input[where] = 0;
			    break;
		    case '1':
			    input[where] = 1;
			    break;
		    case '-':
			    input[where] = 2;
			    break;
	    }
	    where++;
	}
    }
    
    where = 0;
    if (cube.output != -1) {
	last = cube.last_part[cube.output];
	//printf(" ", ch);
	for(i = cube.first_part[cube.output]; i <= last; i++) {
	    ch = out_map [is_in_set(c, i) != 0];
	    //printf("%c", ch);
	    switch(ch) {
		    case '0':
			    output[where] = 0;
			    break;
		    case '1':
			    output[where] = 1;
			    break;
		    case '-':
			    output[where] = 2;
			    break;
	    }
	    where++;
	}
    }
    //printf("\n");
}

char *pc1(c) pcube c;
{static char s1[256];return fmt_cube(c, "01", s1);}
char *pc2(c) pcube c;
{static char s2[256];return fmt_cube(c, "01", s2);}


void debug_print(T, name, level)
pcube *T;
char *name;
int level;
{
    register pcube *T1, p, temp;
    register int cnt;

    cnt = CUBELISTSIZE(T);
    temp = new_cube();
    if (verbose_debug && level == 0)
	printf("\n");
    printf("%s[%d]: ord(T)=%d\n", name, level, cnt);
    if (verbose_debug) {
	printf("cofactor=%s\n", pc1(T[0]));
	for(T1 = T+2, cnt = 1; (p = *T1++) != (pcube) NULL; cnt++)
	    printf("%4d. %s\n", cnt, pc1(set_or(temp, p, T[0])));
    }
    free_cube(temp);
}


void debug1_print(T, name, num)
pcover T;
char *name;
int num;
{
    register int cnt = 1;
    register pcube p, last;

    if (verbose_debug && num == 0)
	printf("\n");
    printf("%s[%d]: ord(T)=%d\n", name, num, T->count);
    if (verbose_debug)
	foreach_set(T, last, p)
	    printf("%4d. %s\n", cnt++, pc1(p));
}


void cprint(T)
pcover T;
{
    register pcube p, last;

    foreach_set(T, last, p)
	printf("%s\n", pc1(p));
}


int makeup_labels(PLA)
pPLA PLA;
{
    int var, i, ind;

    if (PLA->label == (char **) NULL)
	PLA_labels(PLA);

    for(var = 0; var < cube.num_vars; var++)
	for(i = 0; i < cube.part_size[var]; i++) {
	    ind = cube.first_part[var] + i;
	    if (PLA->label[ind] == (char *) NULL) {
		PLA->label[ind] = ALLOC(char, 15);
		if (var < cube.num_binary_vars)
		    if ((i % 2) == 0)
			(void) sprintf(PLA->label[ind], "v%d.bar", var);
		    else
			(void) sprintf(PLA->label[ind], "v%d", var);
		else
		    (void) sprintf(PLA->label[ind], "v%d.%d", var, i);
	    }
	}
}


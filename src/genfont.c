/* 
 * Apple // emulator for Linux: Font compiler
 *
 * Copyright 1994 Alexander Jean-Claude Bottema
 * Copyright 1995 Stephen Lee
 * Copyright 1997, 1998 Aaron Culliney
 * Copyright 1998, 1999, 2000 Michael Deutschmann
 *
 * This software package is subject to the GNU General Public License
 * version 2 or later (your choice) as published by the Free Software 
 * Foundation.
 *
 * THERE ARE NO WARRANTIES WHATSOEVER. 
 *
 */

#define _GNU_SOURCE


/* I'm not sure if this is the correct way to detect libc 5/4. I long
 * since removed it from my system.
 */
#ifdef __GLIBC__ 
#if __GLIBC__ == 1

/* Older Linux C libraries had getline removed (to humor programs that 
 * used that name for their own functions), but kept getdelim */
#define getline(l,s,f) getdelim(l,s,'\n',f)

#endif /* __GLIBC__ == 1 */
#endif /* __GLIBC__ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(void)
{
    unsigned char byte;

    char *line = 0; 
    size_t line_size = 0;

    int i,mx=0;

    printf("/* Apple II text font data\n"
           " * \n"
           " * THIS FILE IS AUTOMATICALLY GENERATED --- DO NOT EDIT\n"
           " */\n");
 
    i = 0x100;

    while (getline(&line,&line_size,stdin) != -1)
    {
        if (line[0] == ';') continue;

	if (line[0] == '=')
        {
            char *name,*size;

	    name = line + 1;
	    while (isspace(*name)) name++;
	    size = strchr(name,',');
            *size++ = 0;
	    mx = i = strtol(size,0,0);

            printf("\nconst unsigned char %s[%d] =\n{\n  ",name,i*8);
        
	    continue;
	}
        
        i--;

        if (line[0] == ':')
        {
            int j = 8;

            while (j--)
            {
                int k;

                if (getline(&line,&line_size,stdin) == -1) {
                    // ERROR ...
                }
                k = 8;
                byte = 0;
                while (k--)
                {
                    byte <<= 1;
                    byte += (line[k] == '#');
                }
	
		if (j)
                   printf("0x%02x, ",byte);
		else if (i)
                   printf("0x%02x,\n  ",byte);	/* last byte in glyph */
		else
                   printf("0x%02x\n};\n",byte); /* last item in array */
            }
        }
        else break;
    }


    if (i)
    { 
        fprintf(stderr,
                "Trouble with font file at character 0x%02x\n",
                mx-i-1);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);

}


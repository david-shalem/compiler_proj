/*************************/
/*                       */
/* general include files */
/*                       */
/*************************/
#include <stdio.h>
#include <iostream>

/*************************/
/*                       */
/* project include files */
/*                       */
/*************************/
#include "Compiler.h"

/************/
/*          */
/* usage 📖 */
/*          */
/************/
int usage(int argc, char **argv)
{
    std::cout << "compi <input> <output>\n";
    return 0;
}

/**************/
/*            */
/* utility 🛠 */
/*            */
/**************/
bool fileExists(const char *filename)
{
    FILE *fl = fopen(filename, "rt");
    if (fl != NULL)
    {
        (void) fclose(fl);
        return true;
    }
    return false;
}

/*****************/
/*               */
/* code start 🌅 */
/*               */
/*****************/
int main(int argc, char **argv)
{
    if (argc == 3)
    {
        const char *input = argv[1];
        const char *output = argv[2];
        if (fileExists(input))
        {
            Compiler c(input, output);
            return c.compile();
        }
    }

    return usage(argc,argv);
}

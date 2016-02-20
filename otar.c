#include <stdio.h>
#include <unistd.h>

static int g_debugLevel = 0;

void parseopt(int argc, char ** argv);
void parseopt(int argc, char ** argv)
{
    int arg;
    while (-1 != (arg = getopt(argc, argv, "vhVatTed")))
    {
        printf("%d\n", arg);
    }
}

int main(int argc, char ** argv)
{
    parseopt(argc, argv);
    return 0;
}
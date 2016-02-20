#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>

static int g_debugLevel = 0;

struct t_program_opts
{
    int debugLevel;
    bool showHelp;
};

void DebugOutput(int level, const char * message, ...);
void DebugOutput(int level, const char * message, ...)
{
    va_list args;
    va_start(args, message);
    if (level <= g_debugLevel)
    {
        vfprintf(stderr, message, args);
        //fprintf(stderr, message);
    }
    va_end(args);
}

void Construct_t_program_opts(struct t_program_opts *newStruct);
void Construct_t_program_opts(struct t_program_opts *newStruct)
{
    newStruct->debugLevel = 0;
    newStruct->showHelp = false;
}

void parseopt(int argc, char ** argv, struct t_program_opts *options);
void parseopt(int argc, char ** argv, struct t_program_opts *options)
{
    int arg;
    while (-1 != (arg = getopt(argc, argv, "vhVatTed")))
    {
        if ('v' == arg)
        {
            ++(options->debugLevel);
        }
        else if ('h' == arg)
        {
            options->showHelp = true;
        }
        else if ('V' == arg)
        {
            
        }
        else if ('a' == arg)
        {
            
        }
        else if ('t' == arg)
        {
            
        }
        else if ('T' == arg)
        {
            
        }
        else if ('e' == arg)
        {
            
        }
        else if ('d' == arg)
        {
            
        }
    }
}


int main(int argc, char ** argv)
{
    struct t_program_opts options;
    Construct_t_program_opts(&options);
    
    parseopt(argc, argv, &options);
    g_debugLevel = options.debugLevel;
    DebugOutput(1, "Debug level: %d\n", g_debugLevel);
    if (options.showHelp)
    {
        DebugOutput(2, "Show help text option selected.\n");
    }
    return 0;
}
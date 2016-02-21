#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>

#define MEM_ALLOC_ERROR 127
#define FILE_READ_ERROR 126

static int g_debugLevel = 0;

void DebugOutput(int level, const char * message, ...);
void DebugOutput(int level, const char * message, ...)
{
    va_list args;
    va_start(args, message);
    if (level <= g_debugLevel)
    {
        vfprintf(stderr, message, args);
    }
    va_end(args);
}

struct t_string_list
{
    const char ** list;
    int size;
};

void Construct_t_string_list(struct t_string_list * newStruct);
void Construct_t_string_list(struct t_string_list * newStruct)
{
   newStruct->size = 0; 
   newStruct->list = NULL; 
}

void Destruct_t_string_list(struct t_string_list * oldStruct);
void Destruct_t_string_list(struct t_string_list * oldStruct)
{
    // Up to user of struct to clean up strings they have put in, unless
    // they specifically asked for it with FreeStrings_t_string_list
    free(oldStruct->list);
    oldStruct->list = NULL;
}

void FreeStrings_t_string_list(struct t_string_list * oldStruct);
void FreeStrings_t_string_list(struct t_string_list * oldStruct)
{
    for (int index = 0; index < oldStruct->size; ++index)
    {
        free((void *)((oldStruct->list)[index]));
        (oldStruct->list)[index] = NULL;
    }
}

void AddString_t_string_list(struct t_string_list * container, const char * string);
void AddString_t_string_list(struct t_string_list * container, const char * string)
{
    size_t newSizeBytes;
    ++(container->size);
    newSizeBytes = sizeof(char *)*(container->size);
    container->list = realloc(container->list, newSizeBytes);
    if (NULL != (container->list))
    {
        container->list[container->size-1] = string;
    }
    else
    {
        container->size = 0;
        exit(127);
    }
}

int GetStringCount_t_string_list(struct t_string_list * container);
int GetStringCount_t_string_list(struct t_string_list * container)
{
    return container->size;
}

const char * GetStringByIndex_t_string_list(struct t_string_list * container, int index);
const char * GetStringByIndex_t_string_list(struct t_string_list * container, int index)
{
    return container->list[index];
}

struct t_program_opts
{
    int debugLevel;
    bool showHelp;
    bool showVersion;
    bool addFiles;
    bool showContentsLong;
    bool showContentsShort;
    bool extractFiles;
    bool deleteFiles;
    struct t_string_list files;
};

void Construct_t_program_opts(struct t_program_opts *newStruct);
void Construct_t_program_opts(struct t_program_opts *newStruct)
{
    newStruct->debugLevel = 0;
    newStruct->showHelp = false;
    newStruct->showVersion = false;
    newStruct->addFiles = false;
    newStruct->showContentsLong = false;
    newStruct->showContentsShort = false;
    newStruct->extractFiles = false;
    newStruct->deleteFiles = false; 
    Construct_t_string_list(&(newStruct->files));
}

void AddFile_t_program_opts(struct t_program_opts * options, const char * string);
void AddFile_t_program_opts(struct t_program_opts * options, const char * string)
{
    AddString_t_string_list(&(options->files), string);
}

int GetFileCount_t_program_opts(struct t_program_opts options);
int GetFileCount_t_program_opts(struct t_program_opts options)
{
    return GetStringCount_t_string_list(&(options.files));
}

const char * GetFileNameByIndex_t_program_opts(struct t_program_opts options, int index);
const char * GetFileNameByIndex_t_program_opts(struct t_program_opts options, int index)
{
    return GetStringByIndex_t_string_list(&(options.files), index);
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
            options->showVersion = true;
        }
        else if ('a' == arg)
        {
            options->addFiles = true;
        }
        else if ('t' == arg)
        {
            options->showContentsShort = true;
        }
        else if ('T' == arg)
        {
            options->showContentsLong = true;
        }
        else if ('e' == arg)
        {
            options->extractFiles = true;
        }
        else if ('d' == arg)
        {
           options->deleteFiles = true; 
        }
    }
    if (optind < argc) {
    DebugOutput(2, "non getopt parameters:\n");
    for (/*optind already set*/; optind < argc; ++optind)
        printf ("%s \n", argv[optind]);
        AddFile_t_program_opts(options, argv[optind]);
    }
}

// I know a bytes buffer framework may seem like a bit much, but I think that
// by spending some time up front to write function that I always use instead
// if direct buffer manipulation, I can be fairly sure that I won't have 
// buffer overflows happen
struct t_bytes_buffer
{
    int size;
    char * bytes;
};

void Construct_t_bytes_buffer(t_bytes_buffer * newStruct);
void Destruct_t_bytes_buffer(t_bytes_buffer * oldStruct);
void Deallocate_t_bytes_buffer(t_bytes_buffer * container);
void Allocate_t_bytes_buffer(t_bytes_buffer * container, int bytes);
void ReadInBytesTo_t_bytes_buffer(t_bytes_buffer * container, int fdin, int bytes);


void Construct_t_bytes_buffer(t_bytes_buffer * newStruct)
{
    newStruct->bytes = NULL;
    newStruct->size = 0;
}

void Destruct_t_bytes_buffer(t_bytes_buffer * oldStruct)
{
    Deallocate_t_bytes_buffer(oldStruct);
}


void Allocate_t_bytes_buffer(t_bytes_buffer * container, int bytes);
{
   if (0 != container->size) || (NULL != (container->bytes))
   {
       Deallocate_t_bytes_buffer(container);
   }
   
   container->bytes = (char *)(malloc(bytes));
   if (NULL != (container->bytes))
   {
       container->size = bytes;
   }
   else
   {
       DebugOutput(4, "t_bytes_buffer Allocate could not allocate enough memory.\n");
       exit(MEM_ALLOC_ERROR);
   }
}

void Deallocate_t_bytes_buffer(t_bytes_buffer * container)
{
    container->size = 0;
    free((container->bytes));
    container->bytes = NULL;
}

void ReadInBytesTo_t_bytes_buffer(t_bytes_buffer * container, int fdin, int bytes)
{
    ssize_t readCount;
    readCount = 0;
    if (container->size != bytes)
    {
        if (container->size < bytes)
        {
            DebugOutput(3, "Enlarging bytes buffer on file read to avoid buffer overflow.\n");
        }
        else // We've ruled out equal, we are reading less than the buffer size
        {
            DebugOutput(3, "Shrinking bytes buffer on file read to avoid possible garbage at end of buffer.\n");
        }
        // Alloc atomatically cleans up buffer before setting new size
        Allocate_t_bytes_buffer(container, bytes);
    }
    
    readCount = read(fdin, container->bytes, bytes);
    if (bytes != readCount)
    {
        if (-1 == readCount)
        {
            DebugOutput(2, "Failed to read anything from file into t_bytes_buffer.\n");
        }
        else
        {
            DebugOutput(2, "Read %d bytes from file into t_bytes_buffer instead of %d bytes requested.\n", readCount, bytes);
        }
        exit(FILE_READ_ERROR);
    }
}


bool ValidateOtarFile(int fdin, bool thorough=false);
bool ValidateOtarFile(int fdin, bool thorough=false)
{
    struct t_bytes_buffer buffer;
    Construct_t_bytes_buffer(&buffer);
    
    // Start at the start of the file
    lseek(fdin, 0, SEEK_SET);
    
    // Look for the header
    ReadInBytesTo_t_bytes_buffer(&buffer, fdin, sizeof(char)*OTAR_ID_LEN);
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
    if (options.showVersion)
    {
        printf("Version: %s\n", GIT_VERSION);
    }
    return 0;
}
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>

#include "otar.h"

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

typedef struct s_string_list
{
    const char ** list;
    int size;
} t_string_list;

void Construct_t_string_list(t_string_list * newStruct);
void Construct_t_string_list(t_string_list * newStruct)
{
   newStruct->size = 0; 
   newStruct->list = NULL; 
}

void Destruct_t_string_list(t_string_list * oldStruct);
void Destruct_t_string_list(t_string_list * oldStruct)
{
    // Up to user of struct to clean up strings they have put in, unless
    // they specifically asked for it with FreeStrings_t_string_list
    free(oldStruct->list);
    oldStruct->list = NULL;
}

void FreeStrings_t_string_list(t_string_list * oldStruct);
void FreeStrings_t_string_list(t_string_list * oldStruct)
{
    for (int index = 0; index < oldStruct->size; ++index)
    {
        free((void *)((oldStruct->list)[index]));
        (oldStruct->list)[index] = NULL;
    }
}

void AddString_t_string_list(t_string_list * container, const char * string);
void AddString_t_string_list(t_string_list * container, const char * string)
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

int GetStringCount_t_string_list(t_string_list * container);
int GetStringCount_t_string_list(t_string_list * container)
{
    return container->size;
}

const char * GetStringByIndex_t_string_list(t_string_list * container, int index);
const char * GetStringByIndex_t_string_list(t_string_list * container, int index)
{
    return container->list[index];
}

typedef struct s_program_opts
{
    int debugLevel;
    bool showHelp;
    bool showVersion;
    bool addFiles;
    bool showContentsLong;
    bool showContentsShort;
    bool extractFiles;
    bool deleteFiles;
    char * archiveFile;
    int archiveFileLen;
    t_string_list files;
} t_program_opts;

void Construct_t_program_opts(t_program_opts *newStruct);
void Construct_t_program_opts(t_program_opts *newStruct)
{
    newStruct->debugLevel = 0;
    newStruct->showHelp = false;
    newStruct->showVersion = false;
    newStruct->addFiles = false;
    newStruct->showContentsLong = false;
    newStruct->showContentsShort = false;
    newStruct->extractFiles = false;
    newStruct->deleteFiles = false;
    newStruct->archiveFileLen = 0;
    newStruct->archiveFile = NULL;
    Construct_t_string_list(&(newStruct->files));
}

void Cleanup_t_program_opts(t_program_opts * oldStruct);
void Cleanup_t_program_opts(t_program_opts * oldStruct)
{
    oldStruct->archiveFileLen = 0;
    free(oldStruct->archiveFile);
    oldStruct->archiveFile = NULL;
    // Does not free up strings pointed to, but in this case, those are in argv and should not be freed
    Destruct_t_string_list(&(oldStruct->files));
}

void AddFile_t_program_opts(t_program_opts * options, const char * string);
void AddFile_t_program_opts(t_program_opts * options, const char * string)
{
    AddString_t_string_list(&(options->files), string);
}

int GetFileCount_t_program_opts(t_program_opts options);
int GetFileCount_t_program_opts(t_program_opts options)
{
    return GetStringCount_t_string_list(&(options.files));
}

const char * GetFileNameByIndex_t_program_opts(t_program_opts options, int index);
const char * GetFileNameByIndex_t_program_opts(t_program_opts options, int index)
{
    return GetStringByIndex_t_string_list(&(options.files), index);
}

void parseopt(int argc, char ** argv, t_program_opts *options);
void parseopt(int argc, char ** argv, t_program_opts *options)
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
typedef struct s_bytes_buffer
{
    int size;
    // Void so that you get a warning if you do pointer math on it directly
    // without a cast to say you know about the risks
    void * bytes;
} t_bytes_buffer;

void Construct_t_bytes_buffer(t_bytes_buffer * newStruct);
void Destruct_t_bytes_buffer(t_bytes_buffer * oldStruct);
void Deallocate_t_bytes_buffer(t_bytes_buffer * container);
void Allocate_t_bytes_buffer(t_bytes_buffer * container, int bytes);
void ReadInBytesTo_t_bytes_buffer(t_bytes_buffer * container, int fdin, int bytes);
bool CompareBuffer_t_bytes_buffer(t_bytes_buffer * container, void * otherBuffer, int otherBufferBytes);


void Construct_t_bytes_buffer(t_bytes_buffer * newStruct)
{
    newStruct->bytes = NULL;
    newStruct->size = 0;
}

void Destruct_t_bytes_buffer(t_bytes_buffer * oldStruct)
{
    Deallocate_t_bytes_buffer(oldStruct);
}


void Allocate_t_bytes_buffer(t_bytes_buffer * container, int bytes)
{
   if ((0 != container->size) || (NULL != (container->bytes)))
   {
       Deallocate_t_bytes_buffer(container);
   }
   
   container->bytes = malloc(bytes);
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

bool CompareBuffer_t_bytes_buffer(t_bytes_buffer * container, void * otherBuffer, int otherBufferBytes)
{
    if (otherBufferBytes > container->size)
    {
        return false;
    }
    else
    {
        return (0 == memcmp(container->bytes, otherBuffer, \
        MIN(otherBufferBytes, container->size)));
    }
}

bool readOtarMainHeader(int fdin);
bool readOtarMainHeader(int fdin)
{
    t_bytes_buffer buffer;
    
    Construct_t_bytes_buffer(&buffer);
    
    // Start at the start of the file
    lseek(fdin, 0, SEEK_SET);
    
    // Look for the otar file type signature
    ReadInBytesTo_t_bytes_buffer(&buffer, fdin, sizeof(char)*OTAR_ID_LEN);
    
    if (!CompareBuffer_t_bytes_buffer(&buffer, OTAR_ID, sizeof(char)*OTAR_ID_LEN))
    {
        DebugOutput(5, "Did not find otar signature at beginning of file during validation.\n");
        return false;
    }
    else
    {
        DebugOutput(5, "Found otar signature at beginning of file.\n");
        return true;
    }
    Destruct_t_bytes_buffer(&buffer);
}

otar_hdr_t * ReadOtarFileHeader(int fdin);
otar_hdr_t * ReadOtarFileHeader(int fdin)
{
    return NULL;
}

/*bool ValidateHeaderOtarFile(int fdin, bool thorough);
bool ValidateHeaderOtarFile(int fdin, bool thorough)
{
    t_bytes_buffer buffer;
    otar_hdr_t header;
    ssize_t readBytes;
    bool readSuccess;
    
    Construct_t_bytes_buffer(&buffer);
    readSuccess = false;
    
    // Start at the start of the file
    lseek(fdin, 0, SEEK_SET);
    
    // Look for the otar file type signature
    ReadInBytesTo_t_bytes_buffer(&buffer, fdin, sizeof(char)*OTAR_ID_LEN);
    
    if (!CompareBuffer_t_bytes_buffer(&buffer, OTAR_ID, sizeof(char)*OTAR_ID_LEN))
    {
        DebugOutput(5, "Did not find otar signature at beginning of file during validation.\n");
        return false;
    }
    else
    {
        DebugOutput(5, "Found otar signature at beginning of file.\n");
    }
    
    readBytes = read(fdin, &header, sizeof(otar_hdr_t));
    while ((readBytes = read(fdin, destination, sizeof(otar_hdr_t))) > 0) 
    {
        if (sizeof(otar_hdr_t) != readBytes)
        {
            DebugOutput(5, "Found junk at end of file.\n");
            return false;
        }
        
        if (memcmp(header.otar_hdr_end, OTAR_HDR_END, OTAR_HDR_END_LEN) != 0)
        {
            DebugOutput(5, "Found invalid end to otar header.\n");
        }
        

    }
    
    // If you got this far, you have passed all the checks so far
    return true;
}*/


int main(int argc, char ** argv)
{
    t_program_opts options;
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
    Cleanup_t_program_opts(&options);
    return 0;
}
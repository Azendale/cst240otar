#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "otar.h"

#define MEM_ALLOC_ERROR 127
#define FILE_READ_ERROR 126

static int g_debugLevel = 0;

void ShowHelp(void);

// Unsigned, bounded atoi
// Credit goes to http://stackoverflow.com/a/1086059/962918 for the start of this
int nuatoi(char * string, int count);
long nuatol(char * string, int count);
int nuatoiOctal(char * string, int count);

int nuatoi(char * string, int count)
{
    int total = 0;
    int charVal = 0;
    while(isdigit(string[0]) && count--)
    {
        charVal = string[0] - '0';
        total = total * 10 + charVal;      
        ++string;
    }
    return total;
}

// Unsigned, bounded atol
// Credit goes to http://stackoverflow.com/a/1086059/962918 for the start of this
long nuatol(char * string, int count)
{
    long total = 0;
    int charVal = 0;
    while(isdigit(string[0]) && count--)
    {
        charVal = string[0] - '0';
        total = total * 10 + charVal;      
        ++string;
    }
    return total;
}

// Unsigned, bounded atoi octal version
// Credit goes to http://stackoverflow.com/a/1086059/962918 for the start of this
int nuatoiOctal(char * string, int count)
{
    int total = 0;
    int charVal = 0;
    while(isdigit(string[0]) && count--)
    {
        charVal = string[0] - '0';
        total = total * 8 + charVal;      
        ++string;
    }
    return total;
}
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
    bool overwriteFiles;
    char * archiveFile;
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
    newStruct->overwriteFiles = false;
    newStruct->archiveFile = NULL;
    Construct_t_string_list(&(newStruct->files));
}

void Cleanup_t_program_opts(t_program_opts * oldStruct);
void Cleanup_t_program_opts(t_program_opts * oldStruct)
{
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

void SetArchiveFile(t_program_opts * container, char * filename);
void SetArchiveFile(t_program_opts * container, char * filename)
{
    container->archiveFile = filename;
}

void parseopt(int argc, char ** argv, t_program_opts *options);
void parseopt(int argc, char ** argv, t_program_opts *options)
{
    int arg;
    while (-1 != (arg = getopt(argc, argv, "vhVatTedo")))
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
        else if ('o' == arg)
        {
           options->overwriteFiles = true; 
        }
    }
    if (optind < argc)
    {
        SetArchiveFile(options, argv[optind]);
        ++optind;
        for (/*optind already set*/; optind < argc; ++optind)
        {
            AddFile_t_program_opts(options, argv[optind]);
        }
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
ssize_t ReadInBytesTo_t_bytes_buffer(t_bytes_buffer * container, int fdin, int bytes);
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

ssize_t ReadInBytesTo_t_bytes_buffer(t_bytes_buffer * container, int fdin, int bytes)
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
    }
    return readCount;
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

void ShowHelp(void)
{
    printf("Help text"); // TODO: write manual
}

typedef struct s_int_otar_header
{
   int size; 
   int fname_len;
   char * fname;
   long adate;
   long mdate;
   int uid;
   int gid;
   int mode;
} t_int_otar_header;

void Construct_t_int_otar_header(t_int_otar_header * container);
void Construct_t_int_otar_header(t_int_otar_header * container)
{
    container->size = 0;
    // Size, not counting null terminating guard, -1 means no string/don't derefence fname
    container->fname_len = -1;
    container->fname = NULL;
    container->uid = 0;
    container->gid = 0;
    container->mode = 0;
}

void Cleanup_t_int_otar_header(t_int_otar_header * container);
void Cleanup_t_int_otar_header(t_int_otar_header * container)
{
    free(container->fname);
    // -1 means no string, not even empty (so a check for -1 is to gaurd against segfaults)
    container->fname_len = -1;
}

t_int_otar_header * Copy_otar_hdr_t_To_t_int_otar_header(otar_hdr_t * in);
t_int_otar_header * Copy_otar_hdr_t_To_t_int_otar_header(otar_hdr_t * in)
{
    t_int_otar_header * out = malloc(sizeof(t_int_otar_header));
    Construct_t_int_otar_header(out);
    
    out->fname_len = nuatoi(in->otar_fname_len, OTAR_FNAME_LEN_SIZE);
    out->fname = strndup(in->otar_fname, MIN(OTAR_MAX_FILE_NAME_LEN, out->fname_len));
    // Length, as determined by null termination
    out->fname_len = MIN(strlen(out->fname), out->fname_len);
    
    out->adate = nuatol(in->otar_adate, OTAR_DATE_SIZE);
    out->mdate = nuatol(in->otar_mdate, OTAR_DATE_SIZE);

    out->uid = nuatoi(in->otar_uid, OTAR_GUID_SIZE);
    out->gid = nuatoi(in->otar_gid, OTAR_GUID_SIZE);
    
    out->mode = nuatoiOctal(in->otar_mode, OTAR_MODE_SIZE);
    out->size = MIN(nuatoi(in->otar_size, OTAR_FILE_SIZE), OTAR_MAX_MEMBER_FILE_SIZE);
    
    return out;
}

void CharFieldFromInt(const char * format, const long value, char * destFileField, const int fileFieldSize);
void CharFieldFromInt(const char * format, const long value, char * destFileField, const int fileFieldSize)
{
    // Size of working buffer
    const int workingBufferSize = fileFieldSize + 1; // +1 is room for null snprintf adds
    // Working buffer that is one byte longer than the file's field so it can hold the snprintf null
    char * workingBuffer = (char *)malloc(workingBufferSize);
    // Unfortunately, adds a null at the end, so we have to use a buffer and then drop the null
    snprintf(workingBuffer, workingBufferSize, format, fileFieldSize, value);
    // Copy without the null to field that will go in file
    memcpy(destFileField, workingBuffer, fileFieldSize);
    free(workingBuffer);
}

otar_hdr_t * Copy_t_int_otar_header_To_otar_hdr_t(t_int_otar_header * in);
otar_hdr_t * Copy_t_int_otar_header_To_otar_hdr_t(t_int_otar_header * in)
{
    otar_hdr_t * out = (otar_hdr_t *)malloc(sizeof(otar_hdr_t));
    
    int nameLen = MIN(in->fname_len, OTAR_MAX_FILE_NAME_LEN);
    // Pad with spaces
    memset(out->otar_fname, ' ', OTAR_MAX_FILE_NAME_LEN);
    // Set the string without a null getting added
    memcpy(out->otar_fname, in->fname, nameLen);
    
    CharFieldFromInt("%*ld", in->fname_len, out->otar_fname_len, OTAR_FNAME_LEN_SIZE);
    
    CharFieldFromInt("%*ld", in->adate, out->otar_adate, OTAR_DATE_SIZE);
    CharFieldFromInt("%*ld", in->mdate, out->otar_mdate, OTAR_DATE_SIZE);
    
    CharFieldFromInt("%*ld", in->uid, out->otar_uid, OTAR_GUID_SIZE);
    CharFieldFromInt("%*ld", in->gid, out->otar_gid, OTAR_GUID_SIZE);
    
    CharFieldFromInt("%*o", in->mode, out->otar_mode, OTAR_MODE_SIZE);
    CharFieldFromInt("%*ld", in->size, out->otar_size, OTAR_FILE_SIZE);
    
    return out;
}

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
        ShowHelp();
    }
    if (options.showVersion)
    {
        printf("Version: %s\n", GIT_VERSION);
    }
    if (options.archiveFile)
    {
        otar_hdr_t * header = (otar_hdr_t *)malloc(sizeof(otar_hdr_t));
        int fdin = open(options.archiveFile, O_RDONLY);
        
        if (-1 == fdin)
        {
            exit(OTAR_FILE_COULD_NOT_OPEN);
        }
        
        if (options.showContentsLong)
        {
            if (!readOtarMainHeader(fdin))
            {
                DebugOutput(1, "Not a valid otar file.\n");
                exit(OTAR_FILE_CORRUPT);
            }
            while (sizeof(otar_hdr_t) == read(fdin, header, sizeof(otar_hdr_t)))
            {
                /*// Convert size from lengthstring to int
                int fileSize = nuatoi(header.otar_fname_len, OTAR_FILE_SIZE);
                
                // Convert name size from lengthstring to int
                // Does not count null term
                int fileNameSize = nuatoi(header.otar_fname_len, OTAR_FNAME_LEN_SIZE);
                
                *//*int nullTFnameLen = min(OTAR_MAX_FILE_NAME_LEN, fileNameSize)+1;
                char * nullTFname = (char *)(malloc(nullTFnameLen));
                nullTFname[nullTFnameLen-1] = 0;*//*
                char * nullTFname = strndup(header.otar_fname, min(OTAR_MAX_FILE_NAME_LEN, fileNameSize));
                
                printf("%s\n", nullTFname);
                free(nullTFname);
                lseek(header->ator_size)*/
            }
        }
        else if (options.showContentsShort)
        {
            
        }
        else if (options.addFiles)
        {
            
        }
        else if (options.extractFiles)
        {
            
        }
        else if (options.deleteFiles)
        {
            
        }
        
        free(header);
        header = NULL;
    }
    else
    {
        fprintf(stderr, "No archive file specified.\n");
        ShowHelp();
    }
    Cleanup_t_program_opts(&options);
    return 0;
}

/*
 int debugLevel;
 bool showHelp;
 bool showVersion;
 bool addFiles;
 bool showContentsLong;
 bool showContentsShort;
 bool extractFiles;
 bool deleteFiles;
 char * archiveFile;
 t_string_list files;*/
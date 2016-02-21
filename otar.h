/*
 * R Jesse Chaney
 *
 * otar.h
 */

#pragma once

# include <string.h>
# include <sys/cdefs.h>

/*
 * otar archive files start with the OTAR_ID identifying string. Then follows a
 * 'struct otar_hdr_s', and as many bytes of member file data as its 'otar_size'
 * member indicates, for each member file.
 */

# define OTAR_ID	"!<otar>\n"     /* String that begins an otar archive file. */
# define OTAR_ID_LEN	8               /* Size of the otar id string string. */

# define OTAR_HDR_END	"++\n"		/* String at end of each member header. */
# define OTAR_HDR_END_LEN  3

# define OTAR_MAX_FILE_NAME_LEN     30
# define OTAR_FNAME_LEN_SIZE         2
# define OTAR_DATE_SIZE	            10
# define OTAR_GUID_SIZE	             5
# define OTAR_MODE_SIZE	             6
# define OTAR_FILE_SIZE	            16
# define OTAR_MAX_MEMBER_FILE_SIZE  1000000 /* Just make things a little easier. */

// Some exit() codes.
# define OTAR_FILE_COULD_NOT_OPEN     2
# define OTAR_FILE_CORRUPT            3
# define OTAR_FILE_MEMBER_BAD         4
# define OTAR_FILE_AR_WRITE_FAILED    5
# define OTAR_FILE_MEM_FILE_OPEN_FAIL 6
# define OTAR_FILE_MEM_FILE_READ_FAIL 7
# define OTAR_FILE_AR_FILE_WRITE_FAIL 8

typedef struct otar_hdr_s
{
    char otar_fname[OTAR_MAX_FILE_NAME_LEN];   /* Member file name, may not be NULL terminated. */
    char otar_fname_len[OTAR_FNAME_LEN_SIZE];  /* The length of the member file name */

    char otar_adate[OTAR_DATE_SIZE];	       /* File access date, decimal sec since Epoch. */
    char otar_mdate[OTAR_DATE_SIZE];	       /* File modify date, decimal sec since Epoch. */

    char otar_uid[OTAR_GUID_SIZE];	       /* user id in ASCII decimal */
    char otar_gid[OTAR_GUID_SIZE];             /* group id in ASCII decimal. */

    char otar_mode[OTAR_MODE_SIZE];	       /* File mode, in ASCII octal. */
    char otar_size[OTAR_FILE_SIZE];	       /* File size, in ASCII decimal. */

    char otar_hdr_end[OTAR_HDR_END_LEN];       /* Always contains OTAR_HDR_END. */
} otar_hdr_t;


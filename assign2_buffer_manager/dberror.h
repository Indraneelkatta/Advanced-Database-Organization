#ifndef DBERROR_H
#define DBERROR_H

#include <stdio.h>

/* module wide constants */
#define PAGE_SIZE 4096  // page size: 4096 bytes

/* return code definitions */
typedef int RC; // return code: typedef to int

#define RC_OK 0                     // ok
#define RC_FILE_NOT_FOUND 1         // file not found
#define RC_FILE_HANDLE_NOT_INIT 2   // file handler not inited
#define RC_WRITE_FAILED 3
#define RC_READ_NON_EXISTING_PAGE 4

#define RC_SEEK_FILE_POSITION_ERROR 100	// seek the current position of the file failed
#define RC_SEEK_FILE_TAIL_ERROR 101 // seek file tail position failed
#define RC_CLOSE_FILE_FAILED 102 // close file failed
#define RC_REMOVE_FILE_FAILED 103 // remove file failed
#define RC_ENOUGH_PAGES 104 // enough pages, no need to append new pages

#define RC_NO_REMOVABLE_PAGE 105 // no removable page for replacement
#define RC_PAGELIST_NOT_INIT 106 // PageList is not initialized
#define RC_PAGE_NOT_FOUND 107 // page not found when searching the requested page in the PageList
#define RC_INVALID_NUMPAGES 108 // numPages of the Buffer Pool is invalid
#define RC_PAGE_FOUND 109 // search the requested page and found it in the BUffer Pool
#define RC_FLUSH_POOL_ERROR 110 // force flush pool meets error (some pages are in use)
#define RC_RS_NOT_IMPLEMENTED 111 // the requested replacement strategy is not implemented

#define RC_READ_FILE_FAILED 110 // read file failed

#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
#define RC_RM_NO_MORE_TUPLES 203
#define RC_RM_NO_PRINT_FOR_DATATYPE 204
#define RC_RM_UNKOWN_DATATYPE 205

#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_IM_NO_MORE_ENTRIES 303

/* holder for error messages */
extern char *RC_message;

/* print a message to standard out describing the error */
extern void printError(RC error);
extern char *errorMessage(RC error);

/* throw message */
#define THROW(rc,message) \
  do {			  \
    RC_message=message;	  \
    return rc;		  \
  } while (0)		  \

/* check the return code and exit if it is an error */
#define CHECK(code)							\
  do {									\
    int rc_internal = (code);						\
    if (rc_internal != RC_OK)						\
      {									\
	char *message = errorMessage(rc_internal);			\
	printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n",__FILE__, __LINE__, __TIME__, message); \
	free(message);							\
	exit(1);							\
      }									\
  } while(0);

#endif

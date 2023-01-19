#ifndef DBERROR_H
#define DBERROR_H

#include "stdio.h"

/* module wide constants */
#define PAGE_SIZE 4096

/* return code definitions */
typedef int RC;

#define RC_OK 0
#define RC_FILE_NOT_FOUND 1
#define RC_FILE_HANDLE_NOT_INIT 2
#define RC_WRITE_FAILED 3
#define RC_READ_NON_EXISTING_PAGE 4
#define RC_FILE_PRESENT 5
#define RC_FILE_READ_ERROR 6
#define RC_WRITE_OUT_OF_BOUND_INDEX 7

#define RC_BUFFER_POOL_NOT_INIT 8
#define RC_BUFFER_POOL_ALREADY_INIT 9
#define RC_BUFFER_POOL_PINPAGE_ERROR 10
#define RC_BUFFER_POOL_PINPAGE_ALREADY_PRESENT 11
#define RC_BUFFER_POOL_PAGE_INUSE 12
#define RC_BUFFER_POOL_UNPINPAGE_ERROR 13
#define RC_BUFFER_POOL_FORCEPAGE_ERROR 14
#define RC_BUFFER_POOL_MARKDIRTY_ERROR 15
#define RC_BUFFER_POOL_EMPTY 16
#define RC_MEMORY_ALLOCATION_FAILED 17
#define RC_PIN_PAGE_FAILED 18
#define RC_UNPIN_PAGE_FAILED 19
#define RC_SCHEMA_NOT_INITIALISED 20
#define RC_MARK_DIRTY_FAILED 21
#define RC_FILE_DESTROY_FAILED 22
#define RC_BUFFER_SHUTDOWN_FAILED 23
#define RC_INVALID_PAGE_SLOT_NUM 24
#define RC_NULL_IP_PARAM 25
#define RC_OPEN_TABLE_FAILED 26
#define RC_IVALID_PAGE_SLOT_NUM 27
#define RC_CREATE_PAGE_FAILED 28
#define RC_OPEN_PAGE_FAILED 29
#define RC_MELLOC_MEM_ALLOC_FAILED 30

#define RC_SEEK_FILE_POSITION_ERROR 100	
#define RC_FILE_TAIL_ERROR 101
#define RC_CLOSE_FILE_FAILED 102
#define RC_READ_FILE_FAILED 103

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
#define RC_ERROR_CLOSING 304
#define RC_ERROR_DESTROYING 305

#define RC_ERROR 400
#define RC_PINNED_PAGES_IN_BUFFER 500 

#define RC_RM_NO_TUPLE_WITH_GIVEN_RID 600
#define RC_SCAN_CONDITION_NOT_FOUND 601

/* holder for error messages */
extern char *RC_message;

/* print a message to standard out describing the error */
extern void printError (RC error);
extern char *errorMessage (RC error);

#define THROW(rc,message) \
  do {			  \
    RC_message=message;	  \
    return rc;		  \
  } while (0)		  \

// check the return code and exit if it is an error
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

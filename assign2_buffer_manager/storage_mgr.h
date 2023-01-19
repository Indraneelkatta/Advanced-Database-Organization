#ifndef STORAGE_MGR_H
#define STORAGE_MGR_H

#include "dberror.h"

/*_____________________________________________________________
                   handling data structures                
_____________________________________________________________*/
typedef struct SM_FileHandle {
	char *fileName;
	int totalNumPages;
	int curPagePos;
	void *mgmtInfo;
} SM_FileHandle;

typedef char* SM_PageHandle;

/*___________________________________________________________
 *                    interface                             
 ____________________________________________________________*/
// manipulating page files 
extern void initStorageManager (void);
extern RC createPageFile (char *fileName);
extern RC openPageFile (char *fileName, SM_FileHandle *fHandle);
extern RC closePageFile (SM_FileHandle *fHandle);
extern RC destroyPageFile (char *fileName);


// reading blocks from disc
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);
extern int getBlockPos (SM_FileHandle *fHandle);
extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);

// writing blocks to a page file 
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC appendEmptyBlock (SM_FileHandle *fHandle);
extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle);

#endif

#define open_file(sp,file,mode)     			sp=fopen(file,mode);
#define calloc_allocation						calloc(PAGE_SIZE, sizeof(char));
#define seekSuccess(handle,fSet, SEEK_SET) 		fseek(handle,fSet, SEEK_SET);
#define read_file(memPage,PAGE_SIZE, f_Handle)	fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);
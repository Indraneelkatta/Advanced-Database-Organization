#include "storage_mgr.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "dberror.h"
//final
FILE *fp;

/*********************************************** MANIPULATING PAGE FILES ************************************************/

//Initialising storage manager
void initStorageManager(void)
{
	printf("The storage manager has been initialized successfully.\n");
	printf("Creation of a page file\n");
}
//Creation of page file
extern RC createPageFile(char *fName) {
	RC rc = -99; // return value.

	//Configure the file pointer with wb+
	//If the file already exists, make it empty
	open_file(fp,fName,"wb+");

	//If file pointer is NULL return RC_FILE_NOT_FOUND
	if (fp == NULL) {
		return RC_FILE_NOT_FOUND;
	}

	//Allocating memory to PAGE_SIZE elements
	SM_PageHandle blankPage = (SM_PageHandle) calloc(PAGE_SIZE, sizeof(char));

	//Writing an empty page to the file
	int wPStatus = fwrite(blankPage, sizeof(char), PAGE_SIZE, fp);

	if (wPStatus == 0) {
		rc = RC_WRITE_FAILED;
	} 
	else {
		rc = RC_OK;
	}

	free(blankPage);

	//Closing the file
	int cFile = fclose(fp);

	if (cFile == EOF) {
		rc = RC_CLOSE_FILE_FAILED;
	}

	return rc;
}

extern RC openPageFile(char *fName, SM_FileHandle *fHandle) {
	
	//Updating the file with r+ to read & write
	open_file(fp,fName,"r+");
	
	if (fp == NULL) {
		return RC_FILE_NOT_FOUND;
	}

	//Calculating files total pages and placing the pointer to the last page.
	int SeekPosition = fseek(fp, 0, SEEK_END);

	if (SeekPosition != 0) {
		return RC_SEEK_FILE_POSITION_ERROR;
	}

	//Stream − This is the pointer to a FILE object that identifies the stream.
	//Retriving last position of the file
	long lPosition = ftell(fp); //Returns the current value of the position of the stream.

	if (lPosition == -1) {
		return RC_FILE_TAIL_ERROR;
	}

	int lFile = (int) lPosition + 1;
	int totalNum = lFile / PAGE_SIZE;

	//Offset − This is the number of bytes to offset from whence.
	//Changing position of the pointer to the start of the file
	SeekPosition = fseek(fp, 0, SEEK_SET); //sets the file position of the stream to the given offset.

	if (SeekPosition != 0) {
		return RC_SEEK_FILE_POSITION_ERROR;
	}

	//Passing the file information to the fHandle
	fHandle->fileName = fName;
	fHandle->totalNumPages = totalNum;
	fHandle->curPagePos = 0;
	fHandle->mgmtInfo = fp;
	return RC_OK;
}

//Closing the page file
extern RC closePageFile(SM_FileHandle *fHandle)
{
	printf("Closing the file\n");
	
	int cFile = fclose(fHandle->mgmtInfo);

	if(!cFile)
	{
		return RC_OK;
	}
	else
	{    
		return RC_FILE_NOT_FOUND;
	}
}

//Deleting the page file
extern RC destroyPageFile(char *fName)
{
	//Checking whether the file was deleted or not using remove()
	int dFile = remove(fName);

	//Returns 0 if file was successfully deleted
	if(!dFile)
	{
		return RC_OK;
	}
	else
	{
		return RC_FILE_NOT_FOUND;
	}
}


/******************************************** READING BLOCKS FROM DISK****************************************/

//Read blocks from the file and store the data in memory
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) 
{	
	if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
		return RC_READ_NON_EXISTING_PAGE;
	}
	
	int set = sizeof(char) * (pageNum * PAGE_SIZE);
		
	int pointer_set = seekSuccess(fHandle->mgmtInfo,set, SEEK_SET);

	if (pointer_set != 0) {
		return RC_SEEK_FILE_POSITION_ERROR;
	}

	int read_val = read_file(memPage,PAGE_SIZE,fHandle->mgmtInfo);
	if (read_val != PAGE_SIZE) {
		return RC_READ_FILE_FAILED;
	}

	// sets the current page to given page Number
	fHandle->curPagePos = pageNum;
	return RC_OK;
}

//gives out the current pointer position
extern RC getBlockPos(SM_FileHandle *fHandle)
{
	return fHandle -> curPagePos;
}

//Read the first Block of the File 
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	return readBlock(0,fHandle,memPage);
}

//Read the block that is previous to current position
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	int pageNum = fHandle->curPagePos - 1;
  		return readBlock(pageNum, fHandle, memPage);
}

//Read the block that is currently pointed to by the page position pointer
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	return readBlock(fHandle->curPagePos, fHandle, memPage);
}

//Read the block that is next to current position
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  	int pageNum = fHandle->curPagePos + 1;
  		return readBlock(pageNum, fHandle, memPage);
}

//Read the last block in the file
extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{ 	
	int pageNum = fHandle->totalNumPages - 1;
  		return readBlock(pageNum, fHandle, memPage);
}

/*********************************************** Writing blocks to the page file ******************************************s*/	
//To write a block of memory to page file	
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {	
	//If pageNum < 0 or file's totalNumPages <= pageNum,	
	//then the page cannot be written to the file.	
	//Return RC_WRITE_FAILED.	
	if (pageNum < 0 || fHandle->totalNumPages < (pageNum + 1)) {	
		return RC_READ_NON_EXISTING_PAGE;	
	}	
	//Get the offset and seek the position in the file.	
	//If the position that supposes to be the start point is not found,	
	//then the page cannot be written to the file.	
	//Return RC_SEEK_FILE_POSITION_ERROR	
	int offset = pageNum * PAGE_SIZE * sizeof(char); // offset in the file from the absolute position	
	int SeekPosition = seekSuccess(fHandle->mgmtInfo,offset, SEEK_SET); // return label 	
	if (SeekPosition != 0) {	
	return RC_SEEK_FILE_POSITION_ERROR;	
	}	
	//If the writing operation fails,page si not successfully return to the file.	
	//Return RC_WRITE_FAILED		
	int writeSize = fwrite(memPage, sizeof(char), PAGE_SIZE,fHandle->mgmtInfo); 	
	if (writeSize != PAGE_SIZE) {	
		return RC_WRITE_FAILED;	
	}	
	//set current position of the to PageNum	
	fHandle->curPagePos = pageNum;	
	return RC_OK;	
}	
//To writing current block of the memory to the page file	
extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)	
{	
	return writeBlock (fHandle->curPagePos, fHandle, memPage);	
}	
extern RC appendEmptyBlock (SM_FileHandle *fHandle){	
	int seekDone;	
	size_t writeSizeofBlock;	
    SM_PageHandle ptr;	
     /*Allocates memory and return a pointer to it */	
    ptr = (char *) calloc(PAGE_SIZE, sizeof(char));;	
    int set = (fHandle->totalNumPages + 1)*PAGE_SIZE*sizeof(char) ;	
    //Set pointer to end of file	
    seekDone = seekSuccess(fHandle->mgmtInfo,set, SEEK_END); 	
    if (seekDone == 0){	
        writeSizeofBlock = fwrite(ptr, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);  //writes data from the memory block pointed by pointer to the file i.e last page is filled with zero bytes. 	
        fHandle->totalNumPages = fHandle->totalNumPages + 1;	
        fHandle->curPagePos = fHandle->totalNumPages;	
		rewind(fHandle->mgmtInfo);	
		fprintf(fHandle->mgmtInfo, "%d\n" , fHandle->totalNumPages); // update total number of pages	
        seekSuccess(fHandle->mgmtInfo, set, SEEK_SET);	
        free(ptr);	
        return RC_OK;	
	}	
	else{	
        free(ptr);	
		return RC_WRITE_FAILED;	
	}	
}	
//To ensure that the file has a more number of pages than what is requested, less page then increase it	
extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)	
{	
	if(numberOfPages > fHandle->totalNumPages)	
	{	
		int a;	
		int extra_pages = numberOfPages - fHandle->totalNumPages;	
		for(a = 0; a< extra_pages; a++)	
		{	
			appendEmptyBlock(fHandle);	
		}	
	}	
		return RC_OK;
}




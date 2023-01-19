#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"
#include "dt.h"

// GLOBAL VARIABLES
int noOfRPages; 
int wPages;
SM_FileHandle *fHandle; 
PageNumber *pageNumArray; 
int *fixCountArr;
bool *dirtyFlagArray; 
pthread_rwlock_t rwlock; 


//Searches the requested page in the buffer pool with searchPage.
RC searchPage(BM_BufferPool * const bm, BM_PageHandle * const page, PageNumber pageNum) 
{
	ListPage *row = (ListPage *) bm->mgmtData;
    printf("\n Row size is %d\n",row->size);
	if (row->size < 0) 
	{
		return_value(RC_PAGELIST_NOT_INIT) ;
	} 
	else if (row->size == 0) 
	{
		return_value(RC_PAGE_NOT_FOUND) ;
	}
	row->current = row->start;

	while (row->current != row->end && row->current->page->pageNum != pageNum) 
	{
		row->current = row->current->next;
	}

	if (row->current->page->pageNum != pageNum && row->current == row->end  ) 
	{
		return_value(RC_PAGE_NOT_FOUND) ;
	}

	// Loads the content into BM_PageHandle
	page->data = row->current->page->data;
	page->pageNum = row->current->page->pageNum;

	return_value(RC_PAGE_FOUND);
} 

//Adds information to the page's end
RC appendPage(BM_BufferPool * const bm, BM_PageHandle * const page, PageNumber pageNum) 
{
	ListPage *row = (ListPage *) bm->mgmtData;
	RC rc; 

	// Require lock
	pthread_rwlock_init(&rwlock, NULL);
	pthread_rwlock_wrlock(&rwlock);

	rc = -99;
	rc = openPageFile(bm->pageFile, fHandle);

	if (rc != RC_OK) {
		return_value(rc) ;
	}

	if (row->size == 0) {
		row->start->count = 1;
		row->start->writeIO = 1;

		// Call ensureCapacity to add it if the requested page is missing from the file. 
		if (fHandle->totalNumPages < pageNum + 1) {
			int entireNoPages = fHandle->totalNumPages;
			rc = -99;
			rc = ensureCapacity(pageNum + 1, fHandle);
			wPages += pageNum + 1 - entireNoPages;

			if (rc != RC_OK) 
			{
				rc = -99;
				rc = closePageFile(fHandle);
				if (rc != RC_OK) 
				{
					return_value(rc) ;
				}
				pthread_rwlock_unlock(&rwlock); 
				pthread_rwlock_destroy(&rwlock);
				return_value(rc) ;
			}
		}
		row->start->writeIO = 0;

		// We may now read the requested page from the file after ensureCapacity.
		row->start->readIO++;
		rc = -99;
		rc = readBlock(pageNum, fHandle, row->start->page->data);
		noOfRPages++;
		row->start->readIO--;

		if (rc != RC_OK) 
		{
			rc = -99;
			rc = closePageFile(fHandle);

			if (rc != RC_OK) 
			{
				return_value(rc) ;
			}
			pthread_rwlock_unlock(&rwlock);
			pthread_rwlock_destroy(&rwlock);
			return_value(rc) ;
		}

		//  fixCount = 1,  numReadIO = 0, numWriteIO = 0
		row->start->page->pageNum = pageNum;
		row->start->flag = FALSE;
		row->start->flagClock = FALSE;
	} else {
		row->end->next->count = 1;
		row->end->next->writeIO = 1;
		// if the page does not exist call ensureCapacity to add the requested page to the file
		if (fHandle->totalNumPages < (pageNum + 1)) {
			int entireNoPages = fHandle->totalNumPages;
			rc = -99;
			rc = ensureCapacity(pageNum + 1, fHandle);
			wPages += pageNum + 1 - entireNoPages;

			if (rc != RC_OK) 
			{
				rc = -99;
				rc = closePageFile(fHandle);

				if (rc != RC_OK) 
				{
					return_value(rc) ;
				}
				pthread_rwlock_unlock(&rwlock);
				pthread_rwlock_destroy(&rwlock);
				return_value(rc) ;
			}
		}
		row->end->next->writeIO = 0;
		// Now we can read the requested page after we add it to  the file
		row->end->next->readIO++;
		rc = -99;
		rc = readBlock(pageNum, fHandle, row->end->next->page->data);
		noOfRPages++;
		row->end->next->readIO--;

		if (rc != RC_OK) 
		{
			rc = -99;
			rc = closePageFile(fHandle);
			if (rc != RC_OK) 
			{
				return_value(rc) ;
			}
			pthread_rwlock_unlock(&rwlock);
			pthread_rwlock_destroy(&rwlock);

			return_value(rc) ;
		}

		// fixCount = 1,  numReadIO = 0, numWriteIO=0 
		row->end->next->page->pageNum = pageNum;
		row->end->next->flag = FALSE;
		row->end->next->flagClock = FALSE;

		row->end = row->end->next;

		// The current pointer now points to the requested page, this now acts as the tail of the page list
		row->current = row->end;
	}

	// After adding the requested page, the size of the pagelist is increased.
	row->size++;

	// Load the requested page into BM_PageHandle
	page->data = row->current->page->data;
	page->pageNum = row->current->page->pageNum;
	rc = -99;
	rc = closePageFile(fHandle);
	if (rc != RC_OK) 
	{
		return_value(rc) ;
	}
	pthread_rwlock_unlock(&rwlock);
	pthread_rwlock_destroy(&rwlock);
	return_value(RC_OK) ;
}

//The requested page will be replaced for the current page and read from the disk.
RC replacePage(BM_BufferPool * const bm, BM_PageHandle * const page, PageNumber pageNum) 
{
	ListPage *row = (ListPage *) bm->mgmtData;
	RC rc;

	// Require lock
	pthread_rwlock_init(&rwlock, NULL);
	pthread_rwlock_wrlock(&rwlock);

	// Open file
	rc = -99;
	rc = openPageFile(bm->pageFile, fHandle);
	if (rc != RC_OK) 
	{
		return_value(rc) ;
	}

	// If the removable page is dirty, then write it back to the disk before removing it.
	// Now fixCount = 0,numReadIO = 0 and numWriteIO = 0
	row->current->count = 1;
	row->current->writeIO = 1;

	// if the removable page is dirty, then write it back to the file
	if (row->current->flag == TRUE) {
		rc = -99;
		rc = writeBlock(row->current->page->pageNum, fHandle,
				row->current->page->data);
		wPages++;

		if (rc != RC_OK) 
		{
			rc = -99;
			rc = closePageFile(fHandle);
			if (rc != RC_OK) 
			{
				return_value(rc) ;
			}
			pthread_rwlock_unlock(&rwlock);
			pthread_rwlock_destroy(&rwlock);
			return_value(rc) ;
		}

		// After writeBlock, set the PageFrame back to clean
		row->current->flag = FALSE;
	}
	//We may now read the requested page from the file after ensureCapacity.
	if (fHandle->totalNumPages < pageNum + 1) 
	{
		int entireNoPages = fHandle->totalNumPages;
		rc = -99;
		rc = ensureCapacity(pageNum + 1, fHandle);
		wPages += pageNum + 1 - entireNoPages;

		if (rc != RC_OK) 
		{
			rc = -99;
			rc = closePageFile(fHandle);
			if (rc != RC_OK) 
			{
				return_value(rc) ;
			}
			pthread_rwlock_unlock(&rwlock);
			pthread_rwlock_destroy(&rwlock);

			return_value(rc) ;
		}
	}
	row->current->writeIO = 0;

	// We may now read the requested page from the file after ensureCapacity.
	row->current->readIO++;
	rc = -99;
	rc = readBlock(pageNum, fHandle, row->current->page->data);
	noOfRPages++;
	row->current->readIO--;
	if (rc != RC_OK) {
		// Do not change fixCount and NumWriteIO back, for this indicates write IO error and need more info to proceed
		rc = -99;
		rc = closePageFile(fHandle);
		if (rc != RC_OK) 
		{
			return_value(rc) ;
		}
		pthread_rwlock_unlock(&rwlock);
		pthread_rwlock_destroy(&rwlock);
		return_value(rc) ;
	}
	// Now the fixCount = 1, the numReadIO = 0, and the numWriteIO = 0
	row->current->page->pageNum = pageNum;
	row->current->flagClock = FALSE;

	// Load the requested into BM_PageHandle
	page->data = row->current->page->data;
	page->pageNum = row->current->page->pageNum;
	rc = -99;
	rc = closePageFile(fHandle);
	if (rc != RC_OK) 
	{
		return_value(rc) ;
	}
	pthread_rwlock_unlock(&rwlock);
	pthread_rwlock_destroy(&rwlock);
	return_value(RC_OK) ;
} // replacePage

//FIFO replacement algorithm
RC FIFO(BM_BufferPool * const bm, BM_PageHandle * const page,
		PageNumber pageNum) {
	ListPage *row = (ListPage *) bm->mgmtData;
	RC rc; // init return_value() code

	// Search the page in the page list by calling the searchPage function
	printf("searchPage number: Page-%d\n", pageNum);
	rc = -99;
	rc = searchPage(bm, page, pageNum);

	//  If the requested page is found, return RC PAGE FOUND; otherwise, return rc.
	if (rc == RC_PAGE_FOUND) 
	{
		row->current->count++;
		printf("Page-%d found\n", pageNum);
		return_value(rc) ;
	} else if (rc != RC_PAGE_NOT_FOUND) 
	{
		return_value(rc) ;
	}
	printf("Page-%d not found\n", pageNum);

	// If the code enters this condition then the Buffer Manager doesn't have the requested page
	// We have to read the page from the disk and load it into BM_PageHandle.
	 
	// Read the requested page from the disk and append it to the next section of the PageList if the Buffer Manager has space available for it.
	if (row->size < bm->numPages) 
	{
		printf("appendPage: Page-%d\n", pageNum);
		rc = -99; // reset return_value() code
		rc = appendPage(bm, page, pageNum);
		if (rc == RC_OK) 
		{
			printf("Page-%d successfully appended\n", pageNum);
		}
		return_value(rc) ;
	}
	 //replacing an already existing page in the Bmanager as per request made.
	row->current = row->start;
	//fixCount=0
	while (row->current != row->end
			&& (row->current->count != 0 || row->current->readIO != 0
					|| row->current->writeIO != 0)) {
		row->current = row->current->next;
	}
	// If the current pointer comes to the tail then we still to determine if the tail's fixCount = 0
	if (row->current == row->end
			&& (row->current->count != 0 || row->current->readIO != 0
					|| row->current->writeIO != 0)) {
		return_value(RC_NO_REMOVABLE_PAGE) ;
	}
	// Here, we replace out the requested page for the one that needs to be removed.	
	printf("Replace the Page: Page-%d\n", pageNum);
	rc = -99; // reset the return_value() value
	rc = replacePage(bm, page, pageNum);
	if (rc == RC_OK && row->current != row->end) 
	{
		if (row->current == row->start) 
		{
			row->start = row->start->next;
			row->current->next->pre = NULL;
		} else 
		{
			row->current->pre->next = row->current->next;
			row->current->next->pre = row->current->pre;
		}
		// Add the requested page to the tail of the PageList
		// connect tail and current
		row->current->pre = row->end;
		row->end->next = row->current;
		row->current->next = NULL;
		row->end = row->end->next;
		printf("Page-%d is replaced\n", pageNum);
	}
	return_value(rc) ;
} 

//LRU replacement algorithm
RC LRU(BM_BufferPool * const bm, BM_PageHandle * const page, PageNumber pageNum) {
	ListPage *row = (ListPage *) bm->mgmtData;
	RC rc = -99;
	rc = FIFO(bm, page, pageNum);

	if (rc != RC_OK && rc != RC_PAGE_FOUND) 
	{
		return_value(rc) ;
	} else if (rc == RC_PAGE_FOUND && row->current != row->end) 
	{
	
		if (row->current == row->start) 
		{
			row->start = row->start->next;
			row->current->next->pre = NULL;
		} else 
		{
			row->current->pre->next = row->current->next;
			row->current->next->pre = row->current->pre;
		}
		row->current->pre = row->end;
		row->end->next = row->current;
		if (row->size < bm->numPages) 
		{
			row->current->next = row->end->next;
			row->end->next->pre = row->current;
		} else 
		{
			row->current->next = NULL;
		}
		row->end = row->end->next;
		// Now the current pointer still points to the requested page
	}

	// If the code enters, then LRU complete Now the current pointer points to the requested page
	return_value(RC_OK) ;
}

//CLOCK replacement algorithm
RC CLOCK(BM_BufferPool * const bm, BM_PageHandle * const page, PageNumber pageNum) 
{
	ListPage *row = (ListPage *) bm->mgmtData;
	RC rc;
	printf("Search the Page: Page-%d\n", pageNum);
	rc = -99;
	rc = searchPage(bm, page, pageNum);
	
	if (rc == RC_PAGE_FOUND) 
	{
		row->current->count++;
		row->current->flagClock = TRUE;
		printf("Page-%d is found\n", pageNum);
		return_value(rc) ;
	} 
	else if (rc != RC_PAGE_NOT_FOUND) 
	{
		return_value(rc) ;
	}
	printf("Page-%d not found\n", pageNum);

	// Load the page in the buffer manager if its not loaded in it. Read the requested page from the disk and append it if the buffer manager has space for it. 
	if (row->size < bm->numPages) 
	{
		printf("Append the Page: Page-%d\n", pageNum);
		rc = -99;
		rc = appendPage(bm, page, pageNum);
		if (rc == RC_OK) 
		{
			if (row->size < bm->numPages)
			{
				row->clk = row->current->next;
			} 
			else if (row->size == bm->numPages) 
			{
				row->clk = row->start;
			}

			printf("Page-%d is appended\n", pageNum);
		}

		return_value(rc) ;
	}
	// Find the first page with clockFlag = TRUE and fixCount = 0
	while (row->clk->flagClock == TRUE || row->clk->count != 0 || row->clk->readIO != 0 || row->clk->writeIO != 0) 
	{
		row->clk->flagClock = FALSE;
		// Move the clock pointer to the next. If it points to the tail, move it to head
		if (row->clk == row->end) {
			row->clk = row->start;
		} else {
			row->clk = row->clk->next;
		}
	}

	// We find the first PageFrame whose clockFlag = FALSE and fixCount = 0
	row->current = row->clk;

	// Replace the removable page with the requested page
	printf("Replace the Page: Page-%d\n", pageNum);
	rc = -99;
	rc = replacePage(bm, page, pageNum);
	if (rc == RC_OK) {
		// Set the clockFlag of the requested page to TRUE after it has been replaced.
		row->clk->flagClock = TRUE;
		// Set the clock pointer to the next to the current pointer
		if (row->clk == row->end) {
			row->clk = row->start;
		} else {
			row->clk = row->clk->next;
		}
		printf("Page-%d is replaced\n", pageNum);
	}
	return_value(rc) ;
}

//Initialize the PageList 
void initPageList(BM_BufferPool * const bm) {
	ListPage *row = (ListPage *) bm->mgmtData;
	FramePage *pf[bm->numPages];
	int i;
	for (i = 0; i < bm->numPages; i++) 
	{
		pf[i] = (FramePage *) allocate_value(FramePage);
		// Initialize pageframe content
		pf[i]->page = MAKE_PAGE_HANDLE();
		pf[i]->page->data = (char *) memory_allocation;
		pf[i]->page->pageNum = NO_PAGE;
		pf[i]->numberOfFrame = i;
		pf[i]->readIO = 0;
		pf[i]->writeIO = 0;
		pf[i]->count = 0;
		pf[i]->flag = FALSE;
		pf[i]->flagClock = FALSE;

		// Add this new PageFrame to the tail
		if (i == 0) 
		{
			pf[i]->pre = NULL;
			pf[i]->next = NULL;
		} else 
		{
			pf[i - 1]->next = pf[i];
			pf[i]->pre = pf[i - 1];
			pf[i]->next = NULL;
		}
	}
	// Reset all pointers and row's size to the initial state
	row->start = pf[0];
	row->end = row->start;
	row->current = row->start;
	row->clk = row->start;
	row->size = 0;
	return_value();
}

/**********************************************Buffer Manager Interface Pool Handling*************************************************/

//Initialize the Buffer Pool
RC initBufferPool(BM_BufferPool * const bm, const char * const pageFileName,
		const int numPages, ReplacementStrategy strategy, void *stratData) {
	if (numPages <= 0) {
		return_value(RC_INVALID_NUMPAGES) ;
	}
	// init fHandle, pageNumArray, dirtyFlagArray, fixCountArr -> position 1
	// free_space them in shutdownBufferPool -> position 4
	fHandle = (SM_FileHandle *) allocate_value(SM_FileHandle);
	pageNumArray = (PageNumber *) allocate_memPage(PageNumber); 
	dirtyFlagArray = (bool *) allocate_memPage(bool); 
	fixCountArr = (int *) allocate_memPage(int); 

	noOfRPages = 0;
	wPages = 0;

	bm->pageFile = (char *) pageFileName; // set the name of the requested page file
	bm->numPages = numPages; // set the capacity of the Buffer Pool
	bm->strategy = strategy; // set the replacement strategy
	
	ListPage *row = (ListPage *) allocate_value(ListPage);
	bm->mgmtData = row;

	initPageList(bm);
	return_value(RC_OK);
}

//Close the buffer pool
RC shutdownBufferPool(BM_BufferPool * const bm) {
	ListPage *row = (ListPage *) bm->mgmtData;
	RC rc = -99; 
	rc = forceFlushPool(bm);
	if (rc != RC_OK) 
	{
		return_value(rc) ;
	}
	// Setting the current pointer to the tail
	row->current = row->end;
	if (bm->numPages == 1) 
	{
		free_space(row->start->page->data);
		free_space(row->start->page);
	} else {
		while (row->current != row->start) 
		{
			row->current = row->current->pre;
			free_space(row->current->next->page->data);
			free_space(row->current->next->page);
		}
		// The current pointer leads to the head after the while loop, then free space  the remaining block.	
		free_space(row->start->page->data);
		free_space(row->start->page);
	}
	free_space(row);

	free_space(fHandle);
	free_space(pageNumArray);
	free_space(dirtyFlagArray);
	free_space(fixCountArr);

	return_value(RC_OK) ;
}

//Write data in all dirty pages
RC forceFlushPool(BM_BufferPool * const bm) {
	ListPage *row = (ListPage *) bm->mgmtData;
	int noWriteCount = 0;
	row->current = row->start;
	// check if fixcount=0
	// point the current pointer to tail
	while (row->current != row->end) 
	{	
		if (row->current->flag == TRUE && row->current->count > 0) 
		{
			noWriteCount++;
		} 
		else if (row->current->flag == TRUE && row->current->count == 0)
		 {
			forcePage(bm, row->current->page);
		}
		row->current = row->current->next;
	}

	//  the current points to the tail of the PageList
	if (row->current == row->end) 
	{
		if (row->current->flag == TRUE && row->current->count > 0) 
		{
			noWriteCount++;
		} 
		else if (row->current->flag == TRUE && row->current->count == 0) 
		{
			forcePage(bm, row->current->page);
		}
	}

	// if there is any unwritable page, then return_value() error code
	if (noWriteCount != 0) 
	{
		return_value(RC_FLUSH_POOL_ERROR) ;
	}
	return_value(RC_OK) ;
}

/**************************************************Buffer Manager Interface Access Pages****************************************************/

//In the buffer pool, pin the page with the requested pageNum. Load the page from the file into the buffer pool if it isn't already there.
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page, const PageNumber pageNum) 
{
	RC rc = -99; 
	if (bm->strategy == RS_FIFO) 
	{
		rc = FIFO(bm, page, pageNum);

		if (rc == RC_PAGE_FOUND) 
		{
			rc = RC_OK;
		}
	} 
	else if (bm->strategy == RS_LRU) 
	{
		rc = LRU(bm, page, pageNum);
	} 
	else if (bm->strategy == RS_CLOCK) 
	{
		rc = CLOCK(bm, page, pageNum);
	} 
	else if (bm->strategy == RS_LFU) 
	{	
		return_value(RC_RS_NOT_IMPLEMENTED);
	} 
	else if (bm->strategy == RS_LRU_K) 
	{
		return_value(RC_RS_NOT_IMPLEMENTED);
	}
	return_value(rc) ;
} 

//Set the requested page as dirty
RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page) 
{
	ListPage *row = (ListPage *) bm->mgmtData;
	// Search the requested page in the PageList
	RC rc = -99;
	rc = searchPage(bm, page, page->pageNum);

	// if page not found, then return rc
	if (rc != RC_PAGE_FOUND) {
		return_value(rc) ;
	}
	row->current->flag = TRUE;
	return_value(RC_OK) ;
} 

//Unpin a page
RC unpinPage(BM_BufferPool * const bm, BM_PageHandle * const page) 
{
	ListPage *row = (ListPage *) bm->mgmtData;
	RC rc = -99;
	rc = searchPage(bm, page, page->pageNum);

	if (rc != RC_PAGE_FOUND) {
		return_value(rc) ;// since page is not found
	}
	row->current->count--;
	return_value(RC_OK) ; 
} 

//Write the requested page back to the page file on disk
RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page) {
	ListPage *row = (ListPage *) bm->mgmtData;
	RC rc = -99;
	pthread_rwlock_init(&rwlock, NULL);
	pthread_rwlock_wrlock(&rwlock);
	rc = -99;
	rc = openPageFile(bm->pageFile, fHandle);

	if (rc != RC_OK) 
	{
		return_value(rc) ;
	}

	row->current->writeIO = 1;

	// Write the requested page back to the disk
	rc = -99;
	rc = writeBlock(page->pageNum, fHandle, page->data);
	wPages++;

	if (rc != RC_OK) 
	{
		rc = -99;
		rc = closePageFile(fHandle);
		if (rc != RC_OK) {
			return_value(rc) ;
		}
		pthread_rwlock_unlock(&rwlock);
		pthread_rwlock_destroy(&rwlock);
		return_value(rc) ;
	}
	row->current->flag = FALSE;
	row->current->writeIO = 0;
	rc = -99;
	rc = closePageFile(fHandle);


	if (rc != RC_OK) {
		return_value(rc) ;
	}
	pthread_rwlock_unlock(&rwlock);
	pthread_rwlock_destroy(&rwlock);
	return_value(RC_OK) ;
} 

/**********************************************************Statistics Interface***********************************************************/


// The NO PAGE is used to represent an empty page frame.
PageNumber *getFrameContents(BM_BufferPool * const bm) {
	ListPage *row = (ListPage *) bm->mgmtData;
	row->current = row->start;
	int index = 0; 

	while (row->current != row->end) 
	{
		pageNumArray[index] = row->current->page->pageNum;

		row->current = row->current->next;
		index++;
	}
	// Include the information from the tail in the array, then increase the pos so that it matches the PageList's size.
	pageNumArray[index++] = row->current->page->pageNum;

	// check if the PageList is full or not, add the values to the pool
	if (index < bm->numPages) 
	{
		int i;
		for (i = index; i < bm->numPages; i++) 
		{
			pageNumArray[i] = NO_PAGE;
		}
	}
	row->current = row->end;
	return_value(pageNumArray) ;
}

//return_value()s an array of bools (of size numPages). Empty page frames are considered as clean
bool *getDirtyFlags(BM_BufferPool * const bm) 
{
	ListPage *row = (ListPage *) bm->mgmtData;
	row->current = row->start; 
	int index = 0; 
	while (row->current != row->end) {
		// Put the value of the page pointed to by the current pointer into the current position of array
		dirtyFlagArray[index] = row->current->flag;
		row->current = row->current->next;
		index++; 
	}
	dirtyFlagArray[index++] = row->current->flag;

	// check if the PageList is full or not, add the values to the pool
	if (index < bm->numPages) 
	{
		int i;
		for (i = index; i < bm->numPages; i++) 
		{
			dirtyFlagArray[i] = FALSE;
		}
	}
	row->current = row->end;
	return_value(dirtyFlagArray) ;
}

 //return_value()s an array of ints (of size numPages)
int *getFixCounts(BM_BufferPool * const bm) 
{
	ListPage *row = (ListPage *) bm->mgmtData;
	row->current = row->start; //set current to head
	int index = 0; 
	while (row->current != row->end) {
		// Put the value of the page pointed to by the current pointer into the current position of array
		fixCountArr[index] = row->current->count;
		row->current = row->current->next;
		index++;
	}
	fixCountArr[index++] = row->current->count;
	// check if the PageList is full or not, add the values to the pool
	if (index < bm->numPages) 
	{
		int i;
		for (i = index; i < bm->numPages; i++) 
		{
			fixCountArr[i] = 0;
		}
	}
	row->current = row->end;
	return_value(fixCountArr) ;
} 
 //no of pages read from disk since the BufferPool initialized
int getNumReadIO(BM_BufferPool * const bm) 
{
	return_value(noOfRPages) ;
} 

 //no of pages written to page file since the BufferPool initialized
int getNumWriteIO(BM_BufferPool * const bm) 
{
	return_value(wPages) ;
}

#include<stdio.h>
#include<stdlib.h>
#include "storage_mgr.h"
#include <math.h>
#include "buffer_mgr.h"

//page - struture - one page structure in  a pool
typedef struct Page
{
	SM_PageHandle data;
	//Data of the page 
	PageNumber pageNum;
	//ID for each page
	int refNumber;
	bool dirtyBit;
	//tracks the page modification
	int Numhit;
	int fixCount;
	
} PageFrame;

int buffSize = 0;
int test(int frontIndex)
{
	if(frontIndex % buffSize == 0)
            {
                frontIndex= 0;
            }
    else
	{
		frontIndex= frontIndex;
	}
            
	return frontIndex; 
}

//The function that writes page to disk
int writeCount = 0;
extern void writeNewBlock(BM_BufferPool *const bm, PageFrame *pageFrame, int pageFrameIndex)
{
	SM_FileHandle filehandle;
	//Initializing the page frame's content in the buffer pool after reading a page from disk
	openPageFile(bm->pageFile, &filehandle);
	writeBlock(pageFrame[pageFrameIndex].pageNum, &filehandle, pageFrame[pageFrameIndex].data);
	writeCount++;
}

//function that sets the page frame content to the new page Content
extern void setNewPage(PageFrame *pageFrame, PageFrame *page, int pageFrameIndex)
{
	
	pageFrame[pageFrameIndex].data = page->data;
	pageFrame[pageFrameIndex].pageNum = page->pageNum;
	pageFrame[pageFrameIndex].dirtyBit = page->dirtyBit;
	pageFrame[pageFrameIndex].fixCount = page->fixCount;
	pageFrame[pageFrameIndex].Numhit = page->Numhit;
}


int rearIndex = 0;
//THE First In First Out Function
extern void FIFO(BM_BufferPool *const bm, PageFrame *page)
{
	
	PageFrame *pageFrame = (PageFrame *) bm->mgmtData;
	
	int i=0;
    int frontIndex= rearIndex % buffSize;

	//Interpretting through the page frames
	while(i < buffSize)
	{
		if(pageFrame[frontIndex].fixCount == 0)
		{
			//Write the page if the memory has a modification/update.

			if(pageFrame[frontIndex].dirtyBit == 1)
			{
				
				//The function that writes page to disk
				
				writeNewBlock(bm,pageFrame,frontIndex);
			}

			//function that sets the page frame content to the new page Content
			setNewPage(pageFrame,page,frontIndex);

			break;
		}
		else
		{
			//Move to next loc if busy.
			frontIndex++;

			//this function returns out the front index..
			theFrontIndex(frontIndex);
			
		}
		i++;
	
	}


}

//Custom function that returns the Front Index..
void theFrontIndex(int frontIndex){

	frontIndex = (frontIndex % buffSize == 0) ? 0 : frontIndex;
}

//Funtion that interacts through all the page frames in the buffer pool
int allPageFrames(PageFrame *pageFrame,int min,int leastIndex, int j)
{
	for(j = 0; j < buffSize; j++)
	{
		if(pageFrame[leastIndex].fixCount == 0)
		{
			leastIndex = (leastIndex + j) % buffSize;
			min = pageFrame[leastIndex].refNumber;
			break;
		}
	}
	return j;
}  

int lfuPtr = 0;
extern void LFU(BM_BufferPool *const bm, PageFrame *page)
{
	PageFrame *pageFrame = (PageFrame *) bm->mgmtData;

    int i,
	leastIndex= lfuPtr;
	int min, count=0;
	
	//allPageFrames function to interpret over all pages 
	allPageFrames(pageFrame,min,leastIndex, i);

	i = (leastIndex + 1) % buffSize;

	// for loop - finds out the pageframe with min reference number(refNumber)

	for(count = 0; count < buffSize; count++)
	{
		if(pageFrame[i].refNumber < min)
		{
			leastIndex = i;
			min = pageFrame[i].refNumber;
		}
		i = (i + 1) % buffSize;


	}
	//Write the page if the memory has a modification/update.
		
	if(pageFrame[leastIndex].dirtyBit == true)
	{
		//The function that writes page to disk
			writeNewBlock(bm,pageFrame,leastIndex);	 
	} 

	//function that sets the page frame content to the new page Content
		setNewPage(pageFrame,page,leastIndex);
	lfuPtr = leastIndex + 1;

}

//Function to find the frame
void findFrame(int i,int leastHit,int min,PageFrame *pageFrame){
	leastHit=i;
    min=pageFrame[i].Numhit;
	return min;

}

//Least Recently Used Function
extern void LRU(BM_BufferPool *const bm, PageFrame *page)
{	
	PageFrame *pageFrame = (PageFrame *) bm->mgmtData;	

	int i=0;
    int leastHit, min;
    leastHit=0;

	//For Loop - interprets over all the page frames in the buffer pool
	for(i = 0; i < buffSize; i++)
	{

		// pageFrame[i].fixCount == 0 - denotes that it page frames isnt being used..
		if(pageFrame[i].fixCount == 0)
		findFrame(i,leastHit,min,pageFrame);
		//Function to find the frame

	}

	for(i=leastHit + 1; i<buffSize; i++)
	{
    	if(pageFrame[i].Numhit< min)
    	findFrame(i,leastHit,min,pageFrame);
		//Function to find the frame
	}

	if(pageFrame[leastHit].dirtyBit == true)
	//if the page is dirty/modified then write that page on to disk
	{
		//The function that writes page to disk
		writeNewBlock(bm,pageFrame,leastHit);	 
	} 

	//function that sets the page frame content to the new page Content
	setNewPage(pageFrame,page,leastHit);
	}

//
/******
 extern void LRU_K(BM_BufferPool *const bm, PageFrame *page)
 {
  PageFrame *pageFrame = (PageFrame *) bm->mgmtData;
	int i=0;
    int leastHit, min;
    leastHit=0;
    min= pageFrame[0].Numhit;

for(i=leastHit + 1; i<buffSize; i++)
{
    if(pageFrame[i].Numhit< min)
    {
        leastHit=i;
        min=pageFrame[i].Numhit;
    }
}

if(pageFrame[leastHit].dirtyBit == true)
{
    writeNewBlock(bm, pageFrame, leastHit);
} 

    setNewPage(pageFrame, page, leastHit);

 }


*/

int clockptr = 0;
//CLOCK - circular list of pages in memory, with the "hand" (iterator) pointing to the last examined page frame in the list
extern void CLOCK(BM_BufferPool *const bm, PageFrame *page)
{
	PageFrame *pageFrame = (PageFrame *) bm->mgmtData;
	while(1)
	{
        while (clockptr % buffSize == 0)
            {
                 clockptr= 0;
				 break;
            }

        if(pageFrame[clockptr].Numhit==0)
            {
				//Write the page if the memory has a modification/update.
                while(pageFrame[clockptr].dirtyBit== true)
                    {
						
						//The function that writes page to disk
						writeNewBlock(bm,pageFrame,clockptr);
						break;
                     }

				//function that sets the page frame content to the new page Content
				setNewPage(pageFrame,page,clockptr);
                clockptr++;
            }
        else
            {
				//NUmhitv set  to zero to stop infinite iterations..
                pageFrame[clockptr++].Numhit = 0;		
				//clockptr shift to next location.

            }
    }
}

//initBufferPool - function to create and intialize the Buffer pool
extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numofPages, ReplacementStrategy strategy, 
		  void *stratData)
{
	

	PageFrame *page = malloc(sizeof(PageFrame) * numofPages);
	

	//buffSize - it represents the total no of pages in the bufferpool
	buffSize = numofPages;	


	int i=0;

//intialiing the pages in BufferPool
	while(i<buffSize)
	{
		page[i].data = NULL;
		page[i].pageNum = -1;
		page[i].dirtyBit = false;
		page[i].fixCount = 0;
		page[i].Numhit = 0;	
		page[i].refNumber = 0;
		i++;
	}

	bm->mgmtData = page;
    bm->pageFile = (char *)pageFileName;
	bm->numPages = numofPages;
	bm->strategy = strategy;

	writeCount = clockptr = lfuPtr = 0;

//return ack success
	return RC_OK;
		
}

//shutdownBufferPool - function to shut down the bufferpool and free up the space and other holded components ..

extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;

	//forceFlushPool - writes all the dirty pages into the disk.
	forceFlushPool(bm);

	shutdownP( bm, pageFrame );
	
	return RC_OK;
}

void shutdownP(BM_BufferPool *const bm, PageFrame *pageFrame )
{
	int j=0;	
	while(j < buffSize)
	{
		if(pageFrame[j].fixCount != 0)
		{
			return RC_PINNED_PAGES_IN_BUFFER;
		}
		j++;
	}

	bm->mgmtData = NULL;
 //freeing the occupied space
	free(pageFrame);
	
}

//funtion to write dirty pages to disk
extern RC forceFlushPool(BM_BufferPool *const bm)
{
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	
	flushP(pageFrame, bm);
	
	return RC_OK;
}

//function to store all dirty pages into the disk
void flushP(PageFrame *pageFrame, BM_BufferPool *const bm)
{
	int j=0;
	while(j < buffSize)
	{
		if(pageFrame[j].fixCount == 0 && pageFrame[j].dirtyBit == true)
		{
			SM_FileHandle filehandle;
			//Initializing the page frame's content in the buffer pool after reading a page from disk
			openPageFile(bm->pageFile, &filehandle);

			//writeBlock - writes data into disk
			writeBlock(pageFrame[j].pageNum, &filehandle, pageFrame[j].data);
			
			//marking the as Not dirty
			pageFrame[j].dirtyBit = false;
			writeCount++;
		}
		j++;
	}	
}

//function to Mark the modified page as dirty
extern RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	
	dirtyP(pageFrame, page);

	return RC_ERROR;
}

void dirtyP(PageFrame *pageFrame, BM_PageHandle *const page)
{


	// Interpretting  through pages in the buffer pool
	int i=0;
	while(i < buffSize)
	{
		if(pageFrame[i].pageNum == page->pageNum)
		{

			//setting the dirty bit to true
			pageFrame[i].dirtyBit = true;
			return RC_OK;		
		}	
		i++;
	}	
}
// used to remove a page from the memory
extern RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{	
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	//going through every page in the buffer pool iteratively
	unPage(pageFrame, page);
	
	return RC_OK;
}

void unPage(PageFrame *pageFrame, BM_PageHandle *const page)
{
	int i=0;
	while(i < buffSize)
	{
		//going through every page in the buffer pool iteratively
		while(pageFrame[i].pageNum == page->pageNum)
		{
			pageFrame[i].fixCount--;
			break;		
		}	
		//Reduce fixCount (which indicates the client has finished work on that page) and exit loop if the current page is the one that has to be unpinned.	
		i++;
	}
}


void fPage(PageFrame *pageFrame, BM_BufferPool *const bm, BM_PageHandle *const page)
{
	int i=0;
	while(i < buffSize)
	{
		while(pageFrame[i].pageNum == page->pageNum)
		{		
			SM_FileHandle filehandle;
			//Initializing the page frame's content in the buffer pool after reading a page from disk
			openPageFile(bm->pageFile, &filehandle);
			writeBlock(pageFrame[i].pageNum, &filehandle, pageFrame[i].data);
			pageFrame[i].dirtyBit = false;
			writeCount++;
			break;
		}
		i++;
	}	
}

extern RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	//Initializing the page frame's content in the buffer pool after reading a page from disk
	fPage(pageFrame, bm, page);	
	return RC_OK;
}

int hit = 0;
//To indicate that this was the final page frame inspected, set hitNum = 1 to the buffer pool
PageFrame *pinpageS(BM_BufferPool *const bm, PageFrame *newPage)
{
	switch (bm->strategy)
	{
		case RS_LRU:
			newPage->Numhit = hit;
			
			break;
		case RS_CLOCK:
			newPage->Numhit = 1;
			//To indicate that this was the final page frame inspected, set hitNum = 1 to the buffer pool
			break;

		default:
			break;
	}
	
	return newPage;
}
//When the buffer pool is full, a page in memory is replaced with the newly pinned page using the proper page replacement approach.
void pinpageSwit(BM_BufferPool *const bm, PageFrame *pageFrame, int i )
{
	switch (bm->strategy)
	{
		case RS_LRU:
			pageFrame[i].Numhit = hit;
			break;
		case RS_CLOCK:
			pageFrame[i].Numhit = 1;
			//To indicate that this was the final page frame inspected, set hitNum = 1 to the buffer pool
			break;
		case RS_LFU:
			pageFrame[i].refNumber++;
			break;
		default:
			break;
	}
}
// When the buffer pool is full, a page in memory is replaced with the newly pinned page using the proper page replacement approach.
extern RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum)
{
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	const int firstpage=0;
	if(pageFrame[firstpage].pageNum == -1)
	{
		
		SM_FileHandle filehandle;
		//Initializing the page frame's content in the buffer pool after reading a page from disk
		openPageFile(bm->pageFile, &filehandle);
		pageFrame[firstpage].data = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageNum,&filehandle);
		readBlock(pageNum, &filehandle, pageFrame[firstpage].data);
		pageFrame[firstpage].pageNum = pageNum;
		pageFrame[firstpage].fixCount++;
		rearIndex = hit = 0;
		pageFrame[firstpage].Numhit = hit;	
		//To indicate that this was the final page frame inspected, set hitNum = 1 to the buffer pool
		pageFrame[firstpage].refNumber = 0;
		page->pageNum = pageNum;
		page->data = pageFrame[firstpage].data;
		
		return RC_OK;		
	}
	else
	{	
		int i=0;
		bool isBufferFull = true;
		
		while(i < buffSize)
		{
			if(pageFrame[i].pageNum != -1)
			{
				if(pageFrame[i].pageNum == pageNum)
				{
					
					pageFrame[i].fixCount++;
					isBufferFull = false;
					hit++;
					//To indicate that this was the final page frame inspected, set hitNum = 1 to the buffer pool
					pinpageSwit(bm, pageFrame, i );

					page->pageNum = pageNum;
					page->data = pageFrame[i].data;
					clockptr++;
					break;
				}				
			} 
			else {
				SM_FileHandle filehandle;
				//Initializing the page frame's content in the buffer pool after reading a page from disk
				openPageFile(bm->pageFile, &filehandle);
				pageFrame[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageNum, &filehandle, pageFrame[i].data);
				pageFrame[i].pageNum = pageNum;
				pageFrame[i].fixCount = 1;
				pageFrame[i].refNumber = 0;
				rearIndex++;	
				hit++;
				//To indicate that this was the final page frame inspected, set hitNum = 1 to the buffer pool
				buffStat (page,pageFrame, bm, i);
		
				page->pageNum = pageNum;
				page->data = pageFrame[i].data;
				
				isBufferFull = false;
				break;
			}
			i++;
		}
		
		if(isBufferFull == true)
		{
			PageFrame *newPage = (PageFrame *) malloc(sizeof(PageFrame));		
			//Initializing the page frame's content in the buffer pool after reading a page from disk
			SM_FileHandle filehandle;
			//Initializing the page frame's content in the buffer pool after reading a page from disk
			openPageFile(bm->pageFile, &filehandle);
			newPage->data = (SM_PageHandle) malloc(PAGE_SIZE);
			//used to read block with page number
			readBlock(pageNum, &filehandle, newPage->data);
			newPage->pageNum = pageNum;
			newPage->dirtyBit = false;	
			newPage->refNumber = 0;	
			newPage->fixCount = 1;
			hit++;
			//To indicate that this was the final page frame inspected, set hitNum = 1 to the buffer pool
			rearIndex++;
			//The least recently used page is determined by the LRU algorithm using the hit value.
			pinpageS(bm, newPage);

			page->pageNum = pageNum;
			page->data = newPage->data;			
			// calling function based on page replacement strategy.
			bmStrategy(newPage, bm);			
		}		
		return RC_OK;
	}	
 }


//Initializing the page frame's content in the buffer pool after reading a page from disk
void buffStat (BM_PageHandle *const page,PageFrame *pageFrame, BM_BufferPool *const bm, int i)
{
	switch (bm->strategy)
	{
		case RS_LRU:
			pageFrame[i].Numhit = hit;
			//To indicate that this was the final page frame inspected, set hitNum = 1 to the buffer pool
			break;
			// least resently used page
		case RS_CLOCK:
			pageFrame[i].Numhit = 1;
			// last page frame
			break;
		default:
			break;
	}
}


void bmStrategy(PageFrame *newPage, BM_BufferPool *const bm)
{
	// calling function based on page replacement strategy.
	switch(bm->strategy)
	{		
		case RS_LFU:
			LFU(bm, newPage);
			//calling LFU
			break;
		case RS_CLOCK:
			CLOCK(bm, newPage);
			//calling CLOCK
			break;
		case RS_LRU:
			LRU(bm, newPage);
			//Calling LRU
			break;
		case RS_LRU_K:
			printf("\n LRU-k not defined");
			break;
		case RS_FIFO:
			FIFO(bm, newPage);
			//calling FIFO
			break;	
		default:
			printf("\nAlgorithm undefined\n");
			break;
	}
}

// retreives array of pages
PageNumber frameCountS(PageNumber *frameContents, PageFrame *pageFrame)
{
	//changing the value of frameContents to the pageNum of the page after iterating through every page in the buffer pool
	int k = 0;
	while(k < buffSize)
	{	
		if(pageFrame[k].pageNum != -1)
		{
			frameContents[k]=pageFrame[k].pageNum;
			// setting frame content using pageframe's pageNumber.
		}
		else 
		{
			frameContents[k]= NO_PAGE;
			// setting frame content to nopage
		}
		k++;
	}
	
	return frameContents;
} 

extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	// retreives array of pages
	PageNumber *frameContents = malloc(sizeof(PageNumber) * buffSize);
	PageFrame *pageFrame = (PageFrame *) bm->mgmtData;
	//changing the value of frameContents to the pageNum of the page after iterating through every page in the buffer pool
	frameCountS(frameContents, pageFrame);
	return frameContents;
}


extern int getNumReadIO (BM_BufferPool *const bm)
{
	return (rearIndex++ );
	// rearindex start with 0 so add 1.
}

bool dirtyS(bool *dirtyFlags, PageFrame *pageFrame, int j)
{
	//Each bool in the array returned by this function denotes the dirtyBit of the corresponding page.
	switch (pageFrame[j].dirtyBit)
	{
		case true:
			dirtyFlags[j]= true;
			//the dirtyFlags value being set to TRUE if a dirty page is present
			break;

		case false:
			dirtyFlags[j]= false;
			//the dirtyFlags value being set to FALSE if a dirty page is present
			break;
		
		default:
			printf("Error in the dirtyFlag");
			break;
	}

	return dirtyFlags;
}

extern bool *getDirtyFlags (BM_BufferPool *const bm)
{
	bool *dirtyFlags = malloc(sizeof(bool) * buffSize);
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	//Each bool in the array returned by this function denotes the dirtyBit of the corresponding page.
	int k= 0;
	while(k < buffSize)
	{
		//all the pages in the buffer pool are iterated over.
        dirtyS(dirtyFlags, pageFrame, k);
		//the dirtyFlags value being set to TRUE if a dirty page is present, otherwise. FALSE
		k++;
	}	
	return dirtyFlags;
}


extern int getNumWriteIO (BM_BufferPool *const bm)
{
	return writeCount;
	//returns no of pages once the buffer pool is initialised
}

int fCount(PageFrame *pageFrame, int *fixCounts, int j)
{
		// Iterating through the buffer pool's pages and setting fixCounts to the page's fixCount
		fixCounts[j]=0;
		while(pageFrame[j].fixCount != -1)
		{
			//jth element is the fixed page frame's recorded page count.
			fixCounts[j]=pageFrame[j].fixCount;
			break;
		}
	return fixCounts;
}
// retrieves array of numPages
extern int *getFixCounts (BM_BufferPool *const bm)
{
	int *fixCounts = malloc(sizeof(int) * buffSize);
	PageFrame *pageFrame= (PageFrame *)bm->mgmtData;
	// Iterating through the buffer pool's pages and setting fixCounts to the page's fixCount
	int k = 0;
	while(k < buffSize)
	{	
		//kth element is the fixed page frame's recorded page count.
		fCount(pageFrame, fixCounts, k);
		k++;
	}	
	return fixCounts;
}



#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

// Include return codes and methods for logging errors
#include "dberror.h"

#include "dt.h"

// Replacement Strategies
typedef enum ReplacementStrategy {
	RS_FIFO = 0,
	 RS_LRU = 1, 
	 RS_CLOCK = 2,
	  RS_LFU = 3, 
	  RS_LRU_K = 4
} ReplacementStrategy;

/* Data Types and Structures */
typedef int PageNumber;
#define NO_PAGE -1

// Buffer Manager Buffer Pool Handler
typedef struct BM_BufferPool {
	char *pageFile;
	int numPages;
	ReplacementStrategy strategy;
	void *mgmtData; // use this one to store the bookkeeping info your buffer
					// manager needs for a buffer pool
} BM_BufferPool;

// Buffer Manager Page Handler
typedef struct BM_PageHandle {
	PageNumber pageNum;
	char *data;
} BM_PageHandle;


typedef struct FramePage {
	struct BM_PageHandle *page;//page handler in buffer manager, it stores all the page content and maintains a log of all page numbers
	int numberOfFrame;
	int readIO;//number of read input outputs on the page
	int writeIO; //number of write input outputs on the page
	int count; //sets status of page if its in use or not busy
	bool flag;//it is set to TRUE if the content in the page is modified
	bool flagClock;
	struct FramePage *pre; //points to previous pageframe
	struct FramePage *next;//points to the next pageframe
} FramePage;


typedef struct ListPage {
	struct FramePage *start;//pointer to point at the start element of the list	
	struct FramePage *end;//pointer to point at the end element of the list	
	struct FramePage *current;//pointer to point at the current element of the list	
	struct FramePage *clk;//pointer used for clock algo
	int size;
} ListPage;

#define MAKE_POOL()					\
  ((BM_BufferPool *) malloc (sizeof(BM_BufferPool)))

#define MAKE_PAGE_HANDLE()				\
  ((BM_PageHandle *) malloc (sizeof(BM_PageHandle)))

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool * const bm, const char * const pageFileName,
		const int numPages, ReplacementStrategy strategy, void *stratData);
RC shutdownBufferPool(BM_BufferPool * const bm);
RC forceFlushPool(BM_BufferPool * const bm);

// Buffer Manager Interface Access Pages
RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page);
RC unpinPage(BM_BufferPool * const bm, BM_PageHandle * const page);
RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page);
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page,
		const PageNumber pageNum);

// Statistics Interface
PageNumber *getFrameContents(BM_BufferPool * const bm);
bool *getDirtyFlags(BM_BufferPool * const bm);
int *getFixCounts(BM_BufferPool * const bm);
int getNumReadIO(BM_BufferPool * const bm);
int getNumWriteIO(BM_BufferPool * const bm);

#endif

#define free_space(element) 	free(element);
#define return_value(value)		return value;
#define memory_allocation 		malloc(PAGE_SIZE * sizeof(char));
#define allocate_value(x) 		malloc(sizeof(x));
#define allocate_memPage(a)  	malloc(bm->numPages * sizeof(a));






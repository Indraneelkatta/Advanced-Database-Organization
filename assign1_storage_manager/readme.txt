Group - Indraneel Katta Balaji (A20511759)
        Venu Prakash Gunda (A20513398)
	    Neha Gawali (A20523722)


PROCEDURE TO RUN THE ASSIGNMENT

1. Open terminal and go to the Project directory.
      cd <project-path>
2. Enter 'make clean' to remove all the object files
3. Enter 'make' to compile and create the new object(.o) files
4. Enter 'make run' to execute the output of the project

It consists of following files:

storagemanager.c - It has all the function definitions of all the functions requried for the storage manager to work
storagemanager.h - It is a Headerfile consists of all the declared functions that are defined in the .c counterpart
dberror.h - its a headerfile consists of all the error message constants
dberror.c - consists of functions definition for error messages
test_assign1_1.c - is a Test case file which tests all the requried functions

FUNCTIONS

*FILE MANIPULATION*(Executed by: Indraneel Katta Balaji (A20511759))

initStorageManager()

- it initializes the Storage manager.

createPageFile()

- this function is used to create a page file,use 'fopen' to create a new file and give permission to read and write by defining "wb+" that is to write and read the file

openPageFile()

- this function is used to open the page file.

closePageFile()

- this function is used to close the file

destoryPageFile()

- this function is used to delete the page file


*READING BLOCKS FROM DISK* (Executed by:Venu Prakash Gunda (A20513398))

readBlock()

-The readBlock() method reads the page from the file and reads it into memory. Checks if the pg number is valid or not then sets the pointer. After this the read operation is performed., it returns an OK message if the read is performed successfully

getBlockPos()

-This gives current position of the pointer

readFirstBlock()

-it reads the first page from the file

readPreviousBlock()

-it reads the previous page from the file

readCurrentBlock()

-it reads the current page from the file

readNextBlock()

--it reads the next page from the file

readLastBlock()

--it reads the last page from the file

*WRITEING BLOCKS FROM DISK*(Executed by:Neha Gawali(A20523722))

writeBlock()

- The writeBlock() method is used to write a  memory block to the file.We check if the page is writable to the file. If the starting point is not found, we cannot write to the page. It returns RC_WRITE_FAILED if write operation is failed, returns RC_OK if the write operation is complete.


writeCurrentBlock()

- this function write one page to the file from the current position. 

appendEmptyBlock()

- this function writes an empty page to the file by appending to the end

ensureCapacity()

- To ensure the file has enough pages, function increases the size of the number of Pages if the file has less than number of Pages




















 

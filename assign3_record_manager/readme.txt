Group4 -  Indraneel Katta Balaji (A20511759)
          Venu Prakash Gunda (A20513398)
	  	  Neha Gawali (A20523722)

Project Execution:

1.Open the terminal and go to the project directory : cd<project-path>

2.Run 'make' to compile and create the new object(.o) files.

3.Run 'make run' to run the files 

Project File Structure:

1. buffer_mgr.c
2. buffer_mgr.h
3. buffer_mgr_stat.c
4. buffer_mgr_stat.h
5. dberror.c
6. dberror.h
7. dt.h
8. storage_mgr.c
9. storage_mgr.h
10. record_mgr.h
11. record_mgr.c
12. test_assign3_1.c
13. test_helper.h
14. readme.txt

FUNCTIONS EXECUTED:

    	Functions			                            Description
	
	initRecordManager()			    -	The function to intialize the Record Manager.
	
	shutdownRecordManager()			- The function to ShutDown the respective Record Manager.
  
  createTable()               - The Function to creates a Table with name As "name" and the respectively mentioned schema..
  
  openTable()                 - The function to open a Table with Name as "name"(given as param).
  
  closeTable()                - The function to close a table.
  
  deleteTable()               - The function for Deleting the table with Name as "name"
  
  getNumTuples()              - The function to get Number of tuples i.e tuple count in the table(using rel)
  
  insertRecord()              - The function to insert a new record into the table(using rel as ref) 
                                                               
  deleteRecord()              - The function deletes record with RID as id, table
	                              
                               
  updateRecord()              - The function that updates the record.

  getRecord()                 - The funtion to retrieve a record from a page and slot mentioned. Page is pinned and then the record is retrieved. The page is then unpinned.
  
  startScan()                 - The function that used to scan all the records under condition.
  
  next()                      - The function scans the table for each record and stores the resultant record satisfying the condition
                                
  closeScan()                 - The function  to indicate to the record manager that all associated resources can be cleaned up.

  getRecordSize()             - Checks if the schema is created; if not it throws an error. If schema exists then it returns the size in bytes of records for a given schema
  
  createSchema()              - Used to create a new schema
  
  freeSchema()                - The function used to free the space associated with that particular schema in the memory.
  
  createRecord()              - These function is used to create a new record. Initially, page and slot are set to -1 as it has not inserted into table/page/slot	
  
  freeRecord()                - Checks if the record is free. If the record is free then it returns record free. If not free then it frees the record by removing the data from the record.
  
  getAttr()                   - The attribue value of a specific record can be obtained using it. Float, String, or Integer are the three possible values for the request.
  
  setAttr()                   - The value of the attribute is set using it. It is possible for an attribute in a record to be of the Integer, String, or Float type, and it is set to the supplied value.







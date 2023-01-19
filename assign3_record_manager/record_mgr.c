#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
//varchanges
const int AtrSize = 15;
typedef struct RcMgr
{
    BM_PageHandle pointerH;
    BM_BufferPool bufferPool;
    RID rID;
    Expr *condition;
    int tpCount;
    int fPage;
    int scCount;
} RcMgr;

RcMgr *rm;

//The function to intialize the Record Manager.
extern RC initRecordManager (void *mgmtData)
{

//The Function to intialize Storage Manager
    initStorageManager();
    printf("----------------------Record Manager Initialized--------------------- \n");


    return RC_OK;
    //Success Acknowledgement - Record Manager Intialization.
}


//The function to ShutDown the respective Record Manager.
extern RC shutdownRecordManager ()
{
    free(rm);
    //free the memory related to the RM.
    rm = NULL;
    printf("\n-----------------Shut down Success----------------------------------- \n ");
    return RC_OK;
    //Success Acknowledgement - Record Manager Shutdown.
}



    //The Function to creates a Table with name As "name" and the respectively mentioned schema.
extern RC createTable (char *name, Schema *schema)
{
    rm=(RcMgr*) malloc(sizeof(RcMgr));
    //Memory allocation to the Rm -data structure.

    SM_FileHandle fHandle;

    //Intializing the buffer Pool.
    initBufferPool(&rm->bufferPool, name, 100, RS_LRU, NULL);
    char data[PAGE_SIZE];
    char *pointerH = data;
    
    *(int*)pointerH = 0; 
    //Intial tuple count = 0;
    pointerH = sizeof(int)+pointerH; 
    //pointer incremented by sizeof(int).
    
    *(int*)pointerH = 1;
     //First page is set to "1"
    pointerH = sizeof(int)+pointerH; 
    //pointer incremented by sizeof(int).

    *(int*)pointerH = schema->numAttr; 
    //numAttr - Number od attributes...
    pointerH = sizeof(int)+pointerH;   
     //pointer incremented by sizeof(int).

    *(int*)pointerH = schema->keySize;  
    // attribute key size
    pointerH =  sizeof(int)+pointerH;  
     //pointer incremented by sizeof(int).
    
    for(int temp1=0;temp1<schema->numAttr;temp1++)
    {
        strncpy(pointerH,schema->attrNames[temp1],AtrSize);
        pointerH = AtrSize+pointerH;
        *(int*)pointerH = (int)schema->dataTypes[temp1];
        pointerH = sizeof(int)+pointerH;
        *(int*)pointerH = (int) schema->typeLength[temp1];
        pointerH = sizeof(int)+pointerH;
    
    }
    

    //creating a page file. 
    int rcode=createPageFile(name); 
    opearteFile(rcode);
    

    //opening a page file
    rcode=openPageFile(name, &fHandle);
    opearteFile(rcode);


    //Writing the schema at first part of page.    
    rcode=writeBlock(0, &fHandle, data);
    opearteFile(rcode);

    
    //closing the page file
    rcode=closePageFile(&fHandle);
    opearteFile(rcode);

return RC_OK;
//returning w code defination.
}


//The function to close a table 
extern RC closeTable (RM_TableData *rel)
{

// rm stores the tables meta data
    RcMgr *rm;

    rm= rel->mgmtData;

    //Bufferpool shutdown.
    shutdownBufferPool(&rm->bufferPool);
    return RC_OK;

}


//The function to open a Table with Name as "name"(given as param)
extern RC openTable (RM_TableData *rel, char *name)
{
    SM_PageHandle pointerH;

    Schema *schema;
    rel->mgmtData = rm;

    //Tables name to "name"
    rel->name = name;
    int CountA;

    //arrangment of a page into Bufferpool w help of Buffermanager
    pinPage(&rm->bufferPool, &rm->pointerH, 0);

    //Memory allocation to schema
    schema = (Schema*) malloc(sizeof(Schema));
    pointerH = (char*) rm->pointerH.data;
    

    rm->tpCount= *(int*)pointerH; //getting the tuple count.

    pointerH = sizeof(int)+pointerH;

    rm->fPage= *(int*) pointerH;
    pointerH = sizeof(int)+pointerH;
    
    CountA = *(int*)pointerH;
    pointerH = sizeof(int)+pointerH;
     
    schema->attrNames = (char**) malloc(sizeof(char*) *CountA);
    schema->numAttr = CountA;
    schema->typeLength = (int*) malloc(sizeof(int) *CountA);
    schema->dataTypes = (DataType*) malloc(sizeof(DataType) *CountA);
    

    //space to save the Atr name for each attribute.
    for(int var=0;var<CountA;var++){
        schema->attrNames[var]= (char*) malloc(AtrSize);
    }

    
    //
    for(int var1=0;var1<schema->numAttr;var1++){
    strncpy(schema->attrNames[var1],pointerH,AtrSize);
            pointerH = pointerH + AtrSize;
        schema->dataTypes[var1]= *(int*) pointerH;
        pointerH=sizeof(int)+pointerH;
        schema->typeLength[var1]= *(int*)pointerH;
            pointerH=sizeof(int)+ pointerH;
    }

    //New schema to the tables schema arrangement.
    rel->schema = schema;

    //Removal page from BufferPool with the help of Buffermanager.
    unpinPage(&rm->bufferPool,&rm->pointerH);

    //writing that page back on to the disk with the help of Buffermanager.
    forcePage(&rm->bufferPool,&rm->pointerH);
    return RC_OK;
}

//The function for Deleting the table with Name as "name"..
extern RC deleteTable (char *name)
{
    destroyPageFile(name);
    
    return RC_OK;
}


//The function to get Number of tuples i.e tuple count in the table(using rel)..
extern int getNumTuples (RM_TableData *rel)
{
    RcMgr *r;
    r= rel->mgmtData;
    int arr;
    arr=r->tpCount;
    return arr;
}

//Custom Function
int opearteFile(int rcode)
{
    if(RC_OK != rcode)
        {
            return rcode;
        }
}


//////////////////////////// Function for Handling Record Data /////////////////////////////////////////

//slotvacancy() - checks for Free slot
int slotVacancy(char *data, int rSize)
{
    int n;
    n= PAGE_SIZE / rSize;
    for(int temp=0;temp<n;temp++){
        if(data[temp * rSize]!='+')
        {
            return temp;
        }
    }
    return -1;
}


//The function to insert a new record into the table(using rel as ref).
extern RC insertRecord (RM_TableData *rel, Record *record)
{
    RcMgr *rm;
    rm= rel->mgmtData;

    //rId is the record ID to this record.
    RID *rID;
    rID= &record->id;
    rID->page = rm->fPage;
    char *dt;
    int SizeR;

    //getRecordSize() returns out the size of the record
    SizeR=getRecordSize(rel->schema);
    char *slotptr;

    //Pinnig the page 
    pinPage(&rm->bufferPool,&rm->pointerH,rID->page);
    dt = rm->pointerH.data;

    //slotvacancy() - checks for Free slot
    rID->slot = slotVacancy(dt, SizeR);

    while(rID->slot == -1)
    {       
        //unpin a page from bufferPool if page dosent have a free slot 
        unpinPage(&rm->bufferPool,&rm->pointerH);
        //incrementing the page.
        rID->page=rID->page+1;
        //pining a page into bufferpool
        pinPage(&rm->bufferPool,&rm->pointerH,rID->page);
     //Data-to-initial position
        dt = rm->pointerH.data;

        //slotvacancy() - checks for Free slot
        rID->slot = slotVacancy(dt, SizeR);
    }
    
    slotptr = dt;

    //Marking the modified page as dirty
    markDirty(&rm->bufferPool, &rm->pointerH);

    //getting the slotstart point.
    slotptr = (rID->slot * SizeR)+slotptr;

    //'+' This symbol indicates that the record is new and it is supposed to be remove in case of Memory outage 
    *slotptr = '+';
    memcpy((slotptr+1),record->data + 1,SizeR - 1);

    //Removing a page from bufferpool w help of Buffermanage
    unpinPage(&rm->bufferPool, &rm->pointerH);


    rm->tpCount=rm->tpCount+1;

    //Pining the Page Back
    pinPage(&rm->bufferPool,&rm->pointerH, 0);

    return RC_OK;
}


//The function deletes record with RID as id, table (using rel as reference)
extern RC deleteRecord (RM_TableData *rel, RID id)
{

    //getting the meta data from table
    RcMgr *r;
    r= rel->mgmtData;

    //Pinnig the  page with respective record
    pinPage(&r->bufferPool, &r->pointerH, id.page);
    r->fPage = id.page;
    int sizeR;
    char *dt;

    //getRecordSize() returns out the size of the record
    sizeR= getRecordSize(rel->schema);
    dt= r->pointerH.data;
    dt = (id.slot * sizeR)+dt ;

    //- this symbol says the page is deleted..
    *dt = '-';

    //Marking the modified page as dirty
    markDirty(&r->bufferPool, &r->pointerH);

    //Removing a page from bufferpool w help of Buffermanage
    unpinPage(&r->bufferPool, &r->pointerH);
    return RC_OK;
}

extern RC updateRecord (RM_TableData *rel, Record *record)
{
    char *dt;

    //getting the meta data from the table..
    RcMgr *rm = rel->mgmtData;

    //pinning the page to update respective record.
    pinPage(&rm->bufferPool, &rm->pointerH, record->id.page);

    int rSize;
    //getRecordSize() returns out the size of the record
    rSize= getRecordSize(rel->schema);
    RID id;
    id= record->id;
    dt = rm->pointerH.data;
    dt = (rSize * id.slot)+dt;
    *dt = '+';
    memcpy(1+dt,1+record->data,rSize - 1 );

    //Marking modified page as dirty
    markDirty(&rm->bufferPool,&rm->pointerH);

    //Removing a page from bufferpool w help of Buffermanage
    unpinPage(&rm->bufferPool,&rm->pointerH);
    return RC_OK;
}

extern RC getRecord (RM_TableData *rel, RID id, Record *record)
{
    //rec4 pagenum... extra funcs/ddecs
    RcMgr *rm;
    rm= rel->mgmtData;
    pinPage(&rm->bufferPool, &rm->pointerH, id.page);
    int sizeR;

    //getRecordSize() returns out the size of the record
    sizeR= getRecordSize(rel->schema);
    char *dPtr;
    dPtr= rm->pointerH.data;
    dPtr = (id.slot * sizeR)+dPtr;
    
    if('+'==*dPtr)
    {
        char *data = record->data;
        record->id = id;
        memcpy(++data, dPtr + 1,sizeR - 1);
    }
    else
    {
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }
    //Removing a page from bufferpool w help of Buffermanage
    unpinPage(&rm->bufferPool, &rm->pointerH);
    return RC_OK;
}


// scans

void sctbMan( RcMgr *tbManager, RcMgr *scManager,RM_ScanHandle *scan, Expr *cond, RM_TableData *rel)
{
    
    // Setting the metadata, tuple count, scan's table, allocating memory to scan manager
    // Initialising the scan
    scManager->scCount = 0;
    scan->mgmtData = scManager;
    //scan from the 1st slot
    scManager->rID.slot = 0;
    scManager->condition = cond;
    //scan from the 1st page
    scManager->rID.page = 1;
    scan->rel= rel;
    tbManager = rel->mgmtData;
    tbManager->tpCount = AtrSize;
}
// It is used to scan all the records under condition
extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
    RcMgr *tbManager;
    RcMgr *scManager;
    
    scManager = (RcMgr*) malloc(sizeof(RcMgr));
    

    while (cond == NULL)
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
        break;
    }
    
    openTable(rel, "ScanTable");
    // Open the memory table

    // Setting the metadata, tuple count, scan's table, allocating memory to scan manager
    sctbMan(tbManager, scManager, scan, cond, rel);
    return RC_OK;
}
//scans the table for each record and stores the resultant record satisfying the condition
extern RC next (RM_ScanHandle *scan, Record *record)
{
    Schema *schema;
    schema= scan->rel->schema;
    // Initializing the data
    RcMgr *scManager;
    scManager= scan->mgmtData;

    RcMgr *tbManager;
    tbManager= scan->rel->mgmtData;
    // Veryfying the test expression
    while (scManager->condition == NULL)
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
        break;
    }
    
    Value *rs;
    char *data;
    int rSize;
    int sltt;
    int tCount;

    rs= (Value *) malloc(sizeof(Value));
    
    //getRecordSize() returns out the size of the record
    rSize= getRecordSize(schema);
    
    sltt= PAGE_SIZE / rSize;
    
    tCount= tbManager->tpCount;
    // Verifying the able wether it contains the tuples or not
    while (0 == tCount)
    {
        return RC_RM_NO_MORE_TUPLES;
        break;
    }

    int scCount = scManager->scCount;
    while (scCount <= tCount)
    {
        bool SC;
        if(0 < scCount)
        {
            SC = true;
        }
        else
        {
            SC = false;
        }

        switch (SC)
        {
        case true:
            scManager->rID.slot++;
            while(sltt<= scManager->rID.slot)
            {
                // Setting slot to 1st position
                scManager->rID.page++;
                scManager->rID.slot = 0;
                break;
            }
            break;
        
        default:
            // Setting slot to 1st position
            scManager->rID.slot = 0;
            scManager->rID.page = 1;
            break;
        } // Put page buffer pool
        pinPage(&tbManager->bufferPool,&scManager->pointerH,scManager->rID.page);

        char *dpointer;  // Retreive the data

        data = scManager->pointerH.data;
        data = (scManager->rID.slot * rSize)+data;
        record->id.slot = scManager->rID.slot; // Setting record slot & page to scans slot and page  
        record->id.page = scManager->rID.page; // Initializaing the records 1st place 
        dpointer= record->data;
        *dpointer = '-'; //Tombstone mechanism
        memcpy(1+dpointer, 1+data, rSize - 1);
        scManager->scCount=scManager->scCount+1;

        evalExpr(record,schema,scManager->condition,&rs);

        switch (rs->v.boolV)
        {
        case TRUE :

        //Removing a page from bufferpool w help of Buffermanage
            unpinPage(&tbManager->bufferPool,&scManager->pointerH);
            return RC_OK;
            break;
        
        default:
            break;
        }
        scCount++;
    }

    //Removal page from BufferPool with the help of Buffermanager.
    unpinPage(&tbManager->bufferPool,&scManager->pointerH);
    scManager->rID.slot=0;
    scManager->scCount=0;
    scManager->rID.page=1;
    // reset manager values
    return RC_RM_NO_MORE_TUPLES;
    
}

extern RC closeScan (RM_ScanHandle *scan)
{
    RcMgr *rcManager;
    rcManager= scan->rel->mgmtData;
    RcMgr *scManager;
    scManager= scan->mgmtData;

    while(0<scManager->scCount)
    {
        //Removal page from BufferPool with the help of Buffermanager.
        unpinPage(&rcManager->bufferPool, &scManager->pointerH);
        scManager->rID.page = 1;
        scManager->scCount= 0;
        scManager->rID.slot = 0;
        // reset values of manager
        break;
    }
    //free memory space allocated
    free(scan->mgmtData);
    scan->mgmtData=NULL;
    return RC_OK;
}


extern RC freeSchema (Schema *schema)
{   
    int i=0;
    free(schema->typeLength);

    while (i < schema->numAttr)
    {
        free(schema->attrNames[i]);
        ++i;
    }

    //free memory space allocated
    free(schema->attrNames);
    free(schema->dataTypes);
    free(schema->keyAttrs);
    free(schema);
    return RC_OK;
}

//getRecordSize() returns out the size of the record
extern int getRecordSize (Schema *schema)
{
    int lens = 0;
    int k = 0;
    while(k < schema->numAttr)
    {
        switch (schema->dataTypes[k])
        {
        case DT_STRING:
            lens += schema->typeLength[k]; // Adding int string to length
            break;
        case DT_INT:
            lens += sizeof(int); // Adding int size to length
            break;
        case DT_FLOAT:
            lens += sizeof(float); // Adding float size to length
            break;
        default:
            lens += sizeof(bool); // Adding bool size to length
            break;
        }
        k++;
    }
    return (lens+1);
}

extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
    Schema *schemaN;
    schemaN= (Schema *) malloc(sizeof(Schema));   
    schemaN->numAttr = numAttr;
    schemaN->attrNames = attrNames;
    schemaN->dataTypes = dataTypes;
    schemaN->typeLength = typeLength;
    schemaN->keySize = keySize;
    schemaN->keyAttrs = keys;
    // allocate memory to schema, set names, key size, data type and length
    return schemaN;
}
extern RC freeRecord (Record *record)
{
    // free space occupied 
    free(record->data);
     free(record);
     return RC_OK;
}
extern RC createRecord (Record **record, Schema *schema)
{
    // used to create  new record
    Record *recN;
    int recordL;
    char *dpointer;
    recN= (Record*) malloc(sizeof(Record));

    //getRecordSize() returns out the size of the record
    recordL= getRecordSize(schema);
    recN->id.page = recN->id.slot = -1;
    recN->data= (char*) malloc(recordL);
    dpointer= recN->data;
    *dpointer = '-';
    *(1+dpointer) = '\0';
    *record = recN;
    return RC_OK;
}

RC getoffsetvalue (Schema *schema, int attrNum, int *result)
{
    *result = 1;
    for(int temp=0;temp<attrNum;temp++)
    {
        switch (schema->dataTypes[temp])
        {
        case DT_STRING:
            *result =schema->typeLength[temp]+ *result; // add size of string to result
            break;
        case DT_INT:
             *result =sizeof(int)+ *result; // add size of int to result
             break;
        case DT_FLOAT:
             *result =sizeof(float)+ *result; // add size of float to result
             break;
        case DT_BOOL:
            *result = sizeof(bool)+ *result ; // add size of bool to result
            break; 
        default:
            break;
        }
    }
    return RC_OK;
}

void attributeNum(Schema *schema, int attrNum)
{
    switch (attrNum)
    {
    case 1:
        schema->dataTypes[attrNum]=1;
        break;
    
    default:
        schema->dataTypes[attrNum] =schema->dataTypes[attrNum];
        break;
    }
}

Value *attributeGet(Schema *schema, int attrNum, Value *atr, char *dpointer)
{
    // retrives data from given record
    bool bVal;
    float fVal;
    int size;
    int iVal = 0;
    switch (schema->dataTypes[attrNum])
    {
                
        // retreives value of type bool
        case DT_BOOL:
            
            memcpy(&bVal,dpointer, sizeof(bool));
            atr->v.boolV = bVal;
            atr->dt = DT_BOOL;
            break;
        // retreives value of type float
        case DT_FLOAT:
            
            memcpy(&fVal, dpointer, sizeof(float));
            atr->v.floatV = fVal;
            atr->dt = DT_FLOAT;
            break;
        // retreives value of type string
        case DT_STRING:
            
            size= schema->typeLength[attrNum];
            atr->v.stringV = (char *) malloc(1+size);
            strncpy(atr->v.stringV, dpointer, size);
            atr->v.stringV[size] ='\0';
            atr->dt = DT_STRING;
            break;
        // retreives value of type int
        case DT_INT:
            
            memcpy(&iVal, dpointer, sizeof(int));
            atr->v.intV = iVal;
            atr->dt = DT_INT;
            break;
    
        default:
            printf("Undefined Serializer for datatype provided. \n");
            break;
    }

    return atr;

}

// gets data from required record under given schema
extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
    // allocating  memory space, getting start position, adding offset
    Value *atr;
    atr= (Value*) malloc(sizeof(Value));
    int offset;
    offset= 0;
    // used to get the off set value
    getoffsetvalue(schema,attrNum,&offset);
    char *dpointer;
    dpointer= record->data;
    dpointer = offset+dpointer;

    attributeNum(schema, attrNum);
    //get data based on datatype
    attributeGet(schema, attrNum, atr, dpointer);

    *value = atr;
    return RC_OK;
}


void attributeSet(Schema *schema, int attrNum, char *dpointer, Value *value)
{
    int size;
    switch (schema->dataTypes[attrNum])
    {
    // set size of bool to the pointer
    case DT_BOOL:
        *(bool *) dpointer = value->v.boolV;
        dpointer =sizeof(bool)+ dpointer ;
        break;
     // set size of floaat to the pointer   
    case DT_FLOAT:
        *(float *) dpointer = value->v.floatV;
        dpointer = sizeof(float)+dpointer;
        break;
    // set size of string to the pointer
    case DT_STRING:
        
        size= schema->typeLength[attrNum];
        strncpy(dpointer, value->v.stringV, size);
        dpointer = schema->typeLength[attrNum]+dpointer;
        break;
    // set size of int to the pointer
    case DT_INT:
        *(int *) dpointer = value->v.intV;
        dpointer = sizeof(int)+dpointer;
        break;

    default:
        printf("Undefined Serializer for datatype provided. \n");
        break;
    }
}
// sets data to required record under given schema
extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
    int offset ;
    offset= 0;
    // used to get the off set value
    getoffsetvalue(schema, attrNum, &offset);
    char *dpointer;
    dpointer= record->data;
    dpointer = dpointer + offset;
        
    //set data based on datatype
    attributeSet(schema, attrNum, dpointer, value);
    return RC_OK;
}


/*
 * storage_mgr.c
 *
 *  Created on: Sep 2, 2015
 */

#include "storage_mgr.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* manipulating page files */
void initStorageManager (void)
{

}

/*
 * It creates a new page file with "fileName". The initial file size is of one page.
 * This method fills this single page with '\0' bytes.
 */
RC createPageFile (char *fileName)
{
	FILE *fptr;		//file pointer

	char *firstPage, *headerPage;

	fptr = fopen(fileName, "w");	//Open the filePage in write mode

	if(fptr!=NULL)	//if file exists
	{
		//allocate firstPage to store the actual data
		firstPage = (char*)calloc(PAGE_SIZE, sizeof(char));

		//allocate headgerPage to store file infor like total number of pages
		headerPage = (char*)calloc(PAGE_SIZE, sizeof(char));

		//intial page number will be 1, and fptr points to headerPage
		fputs("1",fptr);

		//write the headerPage with total no. of Pages
		fwrite(headerPage, PAGE_SIZE, 1, fptr);

		//write the firstPage with '\0' bytes
		fwrite(firstPage, PAGE_SIZE, 1, fptr);

		//free memory to avoid memory leaks
		free(headerPage);
		free(firstPage);

		fclose(fptr);	//Close the file

		return RC_OK;
	}
	else	//If file does not exist
		return RC_FILE_NOT_FOUND;
}

/*
 * This method Opens an existing page file & returns RC_FILE_NOT_FOUND if the file does not exist.
 * If file opening is a success, then the fields of file handle are initialized with the information about the opened file.
 */
RC openPageFile (char *fileName, SM_FileHandle *fHandle)
{
	FILE *fptr;		//file pointer

	fptr = fopen(fileName, "r+");	//open the pageFile

	if(fptr!=NULL)	//if file exists
	{
		/*update the fileHandle attributes*/

		fHandle->fileName = fileName;	//store the file name

		/*read headerPage to get the Total Number of Pages*/
		char* readHeader;
		readHeader = (char*)calloc(PAGE_SIZE,sizeof(char));
		fgets(readHeader,PAGE_SIZE,fptr);
		char* totalPage;
		totalPage = readHeader;

		fHandle->totalNumPages = atoi(totalPage); //convert to integer
		fHandle->curPagePos = 0;	//store the current page position

		//store the File pointer information in the Management info of Page Handle
		fHandle->mgmtInfo = fptr;

		free(readHeader);		//free memory to avoid memory leaks

		return RC_OK;
	}
	else	//if file does not exists
	{
		return RC_FILE_NOT_FOUND;
	}
}

/*
 * This method is used to close the pageFile
 */
RC closePageFile (SM_FileHandle *fHandle)
{
	//if closing the file is success
	if(fclose(fHandle -> mgmtInfo) == 0)
	{
		return RC_OK;
	}
	else
	{
		return RC_FILE_NOT_FOUND;
	}
}

/*
 * This method delete's the pageFile or destory's the pageFile
 */
RC destroyPageFile (char *fileName)
{
	//if remove pageFile is successful
	if(!remove(fileName))
	{
		return RC_OK;
	}
	else
	{
		return RC_FILE_NOT_FOUND;
	}
}

/* reading blocks from disc */

/*
 * This method reads the pageNum(th) block from a file and stores its content in the memory pointed to by the memPage page handle.
 * If the file has less than pageNum pages, it return RC_READ_NON_EXISTING_PAGE.
 */
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	//check if PageNum is Valid
	if(pageNum<0 || pageNum>fHandle->totalNumPages - 1)
	{
		return RC_READ_NON_EXISTING_PAGE;
	}

	//if seeking the file ptr is successful then read the page into memPage
	if(!fseek(fHandle->mgmtInfo,(pageNum+1)*PAGE_SIZE,SEEK_SET))
	{
		//read the block into memPage
		fread(memPage,sizeof(char),PAGE_SIZE,fHandle->mgmtInfo);
		fHandle->curPagePos = pageNum;	//update the curr page pos to most recently read page

		return RC_OK;
	}
	else
		return RC_READ_NON_EXISTING_PAGE;
}

/*
 * This method returns the current page position
 */
int getBlockPos (SM_FileHandle *fHandle)
{
	return fHandle->curPagePos;
}

/*
 * This method is used to read the First Block from the pageFile into memPage.
 * It returns an error if there are no pages in the pageFile
 */
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{

	//call to readBlock with page No. 0 i.e first block
	if(RC_OK == readBlock(0,fHandle,memPage))
		return RC_OK;
	else
		return RC_READ_NON_EXISTING_PAGE;
}

/*
 * This method is used to read the previous Block from the pageFile into memPage.
 * It returns an error if there is no previous block in the pageFile
 */
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	//call to readBlock with page No. (currentPage-1) i.e previous block
	if(RC_OK == readBlock(getBlockPos(fHandle)-1,fHandle,memPage))
		return RC_OK;
	else
		return RC_READ_NON_EXISTING_PAGE;
}

/*
 * This method is used to read the Current Block from the pageFile into memPage.
 * It returns an error if there are no blocks in the pageFile
 */
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	//call to readBlock with page No. (currentPage) i.e current block
	if(RC_OK == readBlock(getBlockPos(fHandle),fHandle,memPage))
		return RC_OK;
	else
		return RC_READ_NON_EXISTING_PAGE;
}

/*
 * This method is used to read the First Block from the pageFile into memPage.
 * It returns an error if there are no pages in the pageFile
 */
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	//call to readBlock with page No. (currentPage+1) i.e next block
	if(RC_OK == readBlock(getBlockPos(fHandle)+1,fHandle,memPage))
		return RC_OK;
	else
		return RC_READ_NON_EXISTING_PAGE;
}

/*
 * This method is used to read the Last Block from the pageFile into memPage.
 * It returns an error if there are no pages in the pageFile
 */
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	//call to readBlock with page as Last block
	if(RC_OK == readBlock(fHandle->totalNumPages - 1,fHandle,memPage))
		return RC_OK;
	else
		return RC_READ_NON_EXISTING_PAGE;
}

/* writing blocks to a page file */

/*
 * This method is used to write onto the block specified in the pageNum field.
 * If the block is not present it gives an error RC_WRITE_FAILED
 */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	//check if pageNum is valid
	if(pageNum > fHandle->totalNumPages - 1 || pageNum < 0)
	{
		return RC_WRITE_FAILED;
	}
	else
	{
		//seek to page number specified
		fseek(fHandle->mgmtInfo,(pageNum+1)*PAGE_SIZE,SEEK_SET);

		//write the block
		fwrite(memPage,PAGE_SIZE,1,fHandle->mgmtInfo);

		//update the current page position
		fHandle->curPagePos = pageNum;

		return RC_OK;
	}

}

/*
 * This method is used to write onto the currently pointed block.
 * If the block is not present it gives and error RC_WRITE_FAILED
 */
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	//write on the current block
	if(RC_OK == writeBlock(getBlockPos(fHandle),fHandle,memPage))
		return RC_OK;
	else
		return RC_WRITE_FAILED;
}

/*
 * This method is used to append a new Empty block into the pageFile.
 * The empty block will contain '\0' bytes
 */
RC appendEmptyBlock (SM_FileHandle *fHandle)
{
	//allocate a new empty page
	char * newPage;
	newPage = (char*)calloc(PAGE_SIZE, sizeof(char));

	//seek to the page
	fseek(fHandle->mgmtInfo,(fHandle->totalNumPages+1)*PAGE_SIZE,SEEK_SET);

	//if write is possible, then write the empty block onto the pageFile
	if(fwrite(newPage, PAGE_SIZE, 1, fHandle->mgmtInfo))
	{
		//update the fileHandle attributes

		fHandle->totalNumPages +=1;		//increment total number of pages
		fHandle->curPagePos = fHandle->totalNumPages - 1;	//update the current page position

		/*write the new total number of pages into the headerPage*/

		//seek to the start of the headerPage
		fseek(fHandle->mgmtInfo,0L,SEEK_SET);
		//write in file with formatting, the total number of pages
		fprintf(fHandle->mgmtInfo, "%d", fHandle->totalNumPages);

		//seek the pointer back again where it was last pointing
		fseek(fHandle->mgmtInfo,(fHandle->totalNumPages+1)*PAGE_SIZE,SEEK_SET);

		//free the memory allocated to avoid memory leaks
		free(newPage);

		return RC_OK;
	}
	else
	{
		free(newPage);
		return RC_WRITE_FAILED;
	}
}

/*
 * This method is used to ensure the capacity of pageFile.
 * The pageFile must have numberOfPages that is specified,
 * if not then add those many number of pages to achieve that capacity
 */
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)
{
	//Check if the capacity is already maintained
	if(fHandle->totalNumPages >= numberOfPages)
	{
		return RC_OK;
	}
	else	//if capacity less
	{
		int i, numOfPagesToAdd;
		//calculate the number of pages to be appended
		numOfPagesToAdd = numberOfPages - fHandle->totalNumPages;

		//loop through and append empty blocks to attain the required capacity
		for(i=0; i < numOfPagesToAdd ; i++)
		{
			//call to appendEmptyBlock()
			appendEmptyBlock(fHandle);
		}
		return RC_OK;
	}
}

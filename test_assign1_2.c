#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"

/* prototypes for test functions */
static void testCreateOpenClose(void);
static void testSinglePageContent(void);
//
/* main function running all tests */
int
main (void)
{
  testName = "";

  initStorageManager();

  testCreateOpenClose();
  testSinglePageContent();

  return 0;
}


/* check a return code. If it is not RC_OK then output a message, error description, and exit */
/* Try to create, open, and close a page file */
void
testCreateOpenClose(void)
{
  SM_FileHandle fh;

  testName = "test create open and close methods";

  TEST_CHECK(createPageFile (TESTPF));

  TEST_CHECK(openPageFile (TESTPF, &fh));
  ASSERT_TRUE(strcmp(fh.fileName, TESTPF) == 0, "filename correct");
  ASSERT_TRUE((fh.totalNumPages == 1), "expect 1 page in new file");
  ASSERT_TRUE((fh.curPagePos == 0), "freshly opened file's page position should be 0");

  TEST_CHECK(closePageFile (&fh));

  TEST_CHECK(destroyPageFile (TESTPF));

// after destruction trying to open the file should cause an error
  ASSERT_TRUE((openPageFile(TESTPF, &fh) != RC_OK), "opening non-existing file should return an error.");

  TEST_DONE();
}

/* Try to create, open, and close a page file */
void
testSinglePageContent(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test single page content";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");

  TEST_CHECK(appendEmptyBlock(&fh));
  printf("appended Empty Block\n");

  TEST_CHECK(appendEmptyBlock(&fh));
  printf("appended Empty Block\n");

  for (i=0; i < PAGE_SIZE; i++)
	ph[i] = 'a';
  TEST_CHECK(writeBlock (0, &fh, ph));
  printf("writing A block\n");

  for (i=0; i < PAGE_SIZE; i++)
	ph[i] = 'b';
  TEST_CHECK(writeBlock (1, &fh, ph));
  printf("writing B block\n");

  for (i=0; i < PAGE_SIZE; i++)
	ph[i] = 'c';
  TEST_CHECK(writeBlock(2,&fh, ph));
  printf("writing C block\n");


  // read first page into handle
  printf("Reading pageNum 0\n");
  TEST_CHECK(readFirstBlock (&fh, ph));

  printf("Reading PageNum 2\n");
  TEST_CHECK(readBlock (2,&fh, ph));

  printf("Reading PageNum 1\n");
  TEST_CHECK(readPreviousBlock (&fh, ph));

  printf("Reading PageNum 1 Again\n");
  TEST_CHECK(readCurrentBlock (&fh, ph));

  printf("Reading PageNum 2\n");
  TEST_CHECK(readNextBlock (&fh, ph));

  printf("Reading PageNum 2 again as it is last block\n");
  TEST_CHECK(readLastBlock (&fh, ph));

  for (i=0; i < PAGE_SIZE; i++)
      ASSERT_TRUE((ph[i] == 'c'), "expected C Block as the lastBlock to be read");

  printf("Ensuring Capacity for already allocated 3 blocks\n");
  TEST_CHECK(ensureCapacity(3,&fh));

  printf("Increasing Page Capacity\n");
  TEST_CHECK(ensureCapacity(10,&fh));

  printf("New reading of Pages after Capacity Increased\n");
  TEST_CHECK(readFirstBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
        ASSERT_TRUE((ph[i] == 'a'), "expected A Block as the firstBlock to be read");

  printf("Reading pageNum 9\n");
  TEST_CHECK(readBlock (9,&fh, ph));

  printf("Reading pageNum 8\n");
  TEST_CHECK(readPreviousBlock (&fh, ph));

  printf("Writing to current Block 8\n");
  for (i=0; i < PAGE_SIZE; i++)
  	ph[i] = 'd';
  TEST_CHECK(writeCurrentBlock(&fh, ph));

  printf("Reading pageNum 8\n");
  TEST_CHECK(readCurrentBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
        ASSERT_TRUE((ph[i] == 'd'), "expected D Block as the lastBlock to be read");

  printf("Reading 2nd time Completed\n");

  TEST_CHECK(appendEmptyBlock(&fh));
  printf("Appended Empty Block");
  // destroy new page file
  TEST_CHECK(destroyPageFile (TESTPF));

  TEST_DONE();
}





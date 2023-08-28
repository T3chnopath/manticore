/* This test is designed to test a simple sleep for 18 ticks.  */

#include   <stdio.h>
#include   "tx_api.h"

//static unsigned long   thread_0_counter =  0;
static TX_THREAD       thread_0;


/* Define thread prototypes.  */

static void    thread_0_entry(ULONG thread_input);


/* Prototype for test control return.  */
void  test_control_return(UINT status);


/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    threadx_thread_simple_sleep_application_define(void *first_unused_memory)
#endif
{

UINT    status;
CHAR    *pointer;

    /* Put first available memory address into a character pointer.  */
    pointer =  (CHAR *) first_unused_memory;


    /* Put system definition stuff in here, e.g. thread creates and other assorted
       create information.  */

    status =  tx_thread_create(&thread_0, "thread 0", thread_0_entry, 1,  
            pointer, TEST_STACK_SIZE_PRINTF, 
            16, 16, 3, TX_AUTO_START);

    /* Check for status.  */
    if (status != TX_SUCCESS)
    {

        printf("Running Thread Simple Sleep for 18 Ticks Test....................... ERROR #1\n");
        test_control_return(1);
    }
}



/* Define the test threads.  */

static void    thread_0_entry(ULONG thread_input)
{


    /* Inform user.  */
    printf("Running Thread Simple Sleep for 18 Ticks Test....................... ");

    /* Clear the tick count. */
    tx_time_set(0);

    /* Sleep for 18 ticks.  */
    tx_thread_sleep(18);

    /* Determine if the sleep was accurate.  */
    if ((tx_time_get() == 18) ||
        (tx_time_get() == 19))
    {

        /* Successful Simple Sleep test.  */
        printf("SUCCESS!\n");
        test_control_return(0);
    }
    else
    {

        /* Thread Simple Sleep error.  */
        printf("ERROR #2\n");
        test_control_return(1);
    }
}


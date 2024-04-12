/*
 * addexec.c
 *
 * Simple program to add two numbers (given in as arguments). Used to
 * test argument passing to child processes via execv().
 *
 * Intended for the basic system calls assignment; this should work
 * once execv() argument handling is implemented, but command line
 * arguments does not need to work.
 *
 * Author: Kuei (Jack) Sun <kuei.sun@mail.utoronto.ca>
 *
 * University of Toronto
 *
 * 2016
 */

#include <sys/wait.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>

#define TEST(expr, val) do { \
        printf("Running test case %d... ", ++testno); \
        ret = expr;     \
        if ( ret >= 0 ) { \
                printf("failed. Expecting negative return value, " \
                "got %d.\n", ret); \
        } else if ( errno != val ) { \
                printf("failed. Expecting error number %d, got %d.\n", \
                val, ret); \
        } else { \
                printf("passed.\n"); \
        } \
} while(0)

static void test_bad_rw2(void)
{
        int ret;
        int testno = 0;
        char * badbuf  = (char *)0xbadbeef;
        char * badbuf2 = (char *)0x0;
        char * badbuf3 = (char *)0xdeadbeef;
        char buf[16]; 


        // stdin is read-only
        TEST(write(STDIN_FILENO, "c", 1), EBADF);

        // fd 5 is not opened (and cannot be opened until we implement it...)
        TEST(write(5, "hello", 5), EBADF);

        // buf points to invalid address
        TEST(write(STDOUT_FILENO, badbuf, 10), EFAULT); 
        TEST(write(STDERR_FILENO, badbuf2, 2), EFAULT); 
        TEST(write(STDERR_FILENO, badbuf3, 7), EFAULT);

        // stdout and stderr are write-only
        TEST(read(STDOUT_FILENO, buf, 1), EBADF);
        TEST(read(STDERR_FILENO, buf, 1), EBADF);

        // fd 9 not opened
        TEST(read(9, buf, 1), EBADF);

        printf("Press any key 3 times in the next set of tests.\n");

        // buf points to invalid address
        TEST(read(STDIN_FILENO, badbuf,  1), EFAULT);
        TEST(read(STDIN_FILENO, badbuf2, 1), EFAULT);
        TEST(read(STDIN_FILENO, badbuf3, 1), EFAULT);
}

int main(void) {

    printf("hello.");
    printf("hello.\n");
    write(STDOUT_FILENO, "hello", 2);
    write(STDERR_FILENO, "hello", 5);



    test_bad_rw2();

	return 0;
}

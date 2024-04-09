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



int main(void) {

    printf("hello.");
    return 0;
}

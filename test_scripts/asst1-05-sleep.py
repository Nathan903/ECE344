#!/usr/bin/python
#
# asst1-sleep
#
# tests the sleep() system call
#
# Author: Kuei (Jack) Sun <kuei.sun@mail.utoronto.ca>
#
# University of Toronto
# 2016
#

import core
import sys

mark = 5

def main():
    test = core.TestUnit("sleep")
    test.runprogram("/testbin/sleep")
    test.set_timeout(15)
    check = 'sleep: test completed.'
    test.look_for_and_print_result(check, mark, menu=1)

if __name__ == "__main__":
    main()


#!/usr/bin/python
#
# asst1-timeit
#
# simply tests the timeit program to run sieve of eratosthenes twice. 
#
# Author: Kuei (Jack) Sun <kuei.sun@mail.utoronto.ca>
#
# University of Toronto
# 2016
#

import core
import sys

mark = 9

def main():
    test = core.TestUnit("timeit")
    test.runprogram("/testbin/timeit")
    test.set_timeout(30)
    check = 'sieve: 2 runs took about \d+ seconds'
    test.look_for_and_print_result(check, mark)

if __name__ == "__main__":
    main()


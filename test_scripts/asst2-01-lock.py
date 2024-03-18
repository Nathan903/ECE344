#!/usr/bin/python

import core
import sys

mark = 15

def testlock(i):
    global test
    test = core.TestUnit("lock")
    test.send_command("sy2")
    return test.look_for(['Test failed', 'Lock test done.'])


def main():
    # run the test several times because the test can succeed 
    # even if locks are not implemented correctly
    result = 0
    for i in range(10):
        index = testlock(i+1)
        if index <= 0:
            result = -1 # failure
            break
    if (result == 0):
        test.print_result(mark, mark)
    else: # no partial mark
        test.print_result(0, mark)


if __name__ == "__main__":
	main()

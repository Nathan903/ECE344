#!/usr/bin/python
#
# asst0-badcall
#
# tests invalid arguments for __time()
#
# Author: Kuei (Jack) Sun <kuei.sun@mail.utoronto.ca>
#
# University of Toronto
# 2017
#

import core
import sys
import pexpect

syscalls = {
    'e' : [
        "passed: __time with invalid seconds pointer",
        "passed: __time with kernel seconds pointer",
        "passed: __time with invalid nsecs pointer",
        "passed: __time with kernel nsecs pointer",
    ],
}

def main():
    test = core.TestUnit("badcall")
    test.runprogram("/testbin/badcall")
    for ch in syscalls:
        test.look_for("Choose:")
        # the second argument is 0, so don't wait for menu prompt
        test.send_command(ch, 0)
        for output in syscalls[ch]:
            test.look_for_and_print_result(output, 1) 

if __name__ == "__main__":
	main()


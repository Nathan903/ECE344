#!/usr/bin/python
#
# asst1-dbflags
#
# tests debug flags
#
# Author: Kuei (Jack) Sun <kuei.sun@mail.utoronto.ca>
#
# University of Toronto
# 2020
#

import core
import sys
import random

def checkDBValue(test, res):
    check = 'Current value of dbflags is ' + res
    test.send_command("dbflags")
    test.look_for_and_print_result(check, 1)

def setDBValue(test, value, on):
    path = "df " + str(value) + " " + on
    test.send_command(path)

def failDBValue(test, value, on):
    print("Turning " + on + " dbflags value " + str(value))
    check = 'Usage: df nr on\/off'
    setDBValue(test, value, on)
    test.look_for_and_print_result(check, 1)

def testDBValue(test, value, on, res):
    print("Turning " + on + " dbflags value " + str(value))
    setDBValue(test, value, on)
    return checkDBValue(test, res)

MAX_NR = 12

def main():
    test = core.TestUnit("dbflags")
    test.set_timeout(5)

    #Check if we have the dbflags menu option
    test.send_command("?o")
    test.look_for_and_print_result('\[dbflags\] Debug flags', 1)
    checkDBValue(test, "0x0")

    failDBValue(test, "foo", "on")
    failDBValue(test, 2, "bar")
    failDBValue(test, MAX_NR + 1, "on")
    failDBValue(test, -2, "off")

    testDBValue(test, 1, "on", "0x1")
    testDBValue(test, 5, "on", "0x11")
    testDBValue(test, 1, "off", "0x10")
    testDBValue(test, 6, "on", "0x30")
    

if __name__ == "__main__":
    main()

#!/usr/bin/python
#
# asst1-name
#
# tests updating name of group
#
# Author: Kuei (Jack) Sun <kuei.sun@mail.utoronto.ca>
#
# University of Toronto
# 2020
#

import core
import sys

mark = 2

def main():
    test = core.TestUnit("name")
    out = test.look('Put-your-group-name-here')
    if out < 0:
        # we found the menu
        test.print_result(mark, mark)
    else:
        # the default is printed - fail
        test.print_result(0, mark)        

if __name__ == "__main__":
    main()

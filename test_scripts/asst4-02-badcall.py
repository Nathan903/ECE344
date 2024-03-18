#!/usr/bin/python
#
# tests invalid arguments for waitpid(), execv(), sbrk(), and __time()
#
# Author: Kuei (Jack) Sun <kuei.sun@mail.utoronto.ca>
#
# University of Toronto
# 2016
#

import core, sys, pexpect

syscalls = (
    ('d', 5, [
        r"passed: sbrk\(too-large negative",
        r"passed: sbrk\(huge positive",
        r"passed: sbrk\(huge negative",
        r"passed: sbrk\(unaligned positive",
        r"passed: sbrk\(unaligned negative",
    ]),
     
    ('c', 4, [
        r"passed: wait for pid -8",
        r"passed: wait for pid -1",
        r"passed: pid zero",
        r"passed: nonexistent pid",
        r"passed: wait with NULL status",
        r"passed: wait with invalid pointer status",
        r"passed: wait with kernel pointer status",
        r"passed: wait with unaligned status",
        r"passed: wait with bad flags",
        r"passed: wait for self. Invalid argument",
        r"passed: wait for parent \(from child\): Invalid argument",
        r"passed: wait for parent test \(from parent\): Operation succeeded", 
    ]),
    
    ('b', 4, [
        r"passed: exec NULL",
        r"passed: exec invalid pointer",
        r"passed: exec kernel pointer",
        r"passed: exec the empty string",
        r"passed: exec .*? with NULL arglist",
        r"passed: exec .*? with invalid pointer arglist",
        r"passed: exec .*? with kernel pointer arglist",
        r"passed: exec .*? with invalid pointer arg",
        r"passed: exec .*? with kernel pointer arg",
    ]),
)

# lose 2 marks per failure
loss = 2

def main():
    test = core.TestUnit("badcall")
    test.runprogram("/testbin/badcall")
    
    res = [0] * len(syscalls)
    total = 0
    j = 0
    for (ch, mark, testcases) in syscalls:
        # set up the mark for this syscall
        res[j] = mark
        total += mark
        
        out = test.look_for([test.menu, "Choose:"])
        if out <= 0:
            print ("FAIL")
            res[j] = 0
            continue
            
        fail = 0
        test.send_command(ch, 0)
        
        for output in testcases:
            # (jsun): we check for failure messages to speed up the tester
            i = test.look_for(["UH-OH", "FAILURE", "unimplemented", output])
            if i <= 2:
                res[j] -= loss
                fail = 1
                
        if fail:
            print ("FAIL")
        else:
            print ("PASS") 
        j += 1

    # you get zero instead of negative marks for too many failures
    res = [ 0 if m < 0 else m for m in res ]
    test.print_result(sum(res), total)

if __name__ == "__main__":
	main()


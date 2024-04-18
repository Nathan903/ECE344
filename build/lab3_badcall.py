#!/usr/bin/python
#
# core.py
#
# Wrapper TestUnit class which uses pexpect to control sys161 
#
# Authors:
#
# Kuei (Jack) Sun   <kuei.sun@mail.utoronto.ca>
# Prof. Ding Yuan   <yuan@ece.utoronto.ca>
# Prof. Ashvin Goel <ashvin@eecg.utoronto.edu>
#
# University of Toronto
# 2016
#

from __future__ import print_function
import pexpect, sys, os, shutil, signal

# user wants to stop the tester scripts, start shutdown cleanly
def shutdown(signum, frame):
    print("Stopping current script")
    sys.exit(1)
    
signal.signal(signal.SIGHUP, shutdown)  # 1
signal.signal(signal.SIGINT, shutdown)  # 2
signal.signal(signal.SIGQUIT, shutdown) # 3
signal.signal(signal.SIGTERM, shutdown) # 15

class TestUnit(object):
    """
    Infrastructure for running a test script
    """

    def set_timeout(self, timeout):
        self.kernel.timeout = timeout
        if self.verbose > 0:
            print('This test has a timeout of ' + str(timeout) + ' seconds')

    def _start(self):
        self.kernel = pexpect.spawn(self.path, timeout = 10, encoding='utf-8')
        self.kernel.logfile = open('os161.log', 'a')
    
    # restarts the kernel    
    def restart(self):
        self.kernel.logfile.close()
        try:
            self.kernel.close()
        except pexpect.ExceptionPexpect:
            if self.verbose > 1:
                print("WARNING: could not shutdown cleanly")
        self._start()

    def __init__(self, message, path_to_kernel = "kernel"):
        try:
            self.verbose = os.environ['OS161_TESTER_VERBOSE']
        except KeyError:
            self.verbose = 0
        try:
            self.verbose = int(self.verbose)
        except ValueError:
            self.verbose = 0
        self.path = 'sys161 ' + str(path_to_kernel)
        self._start()
        self.mark = 0
        self.total = 0
        self.message = message
        self.cwd = os.path.join(os.getcwd(), "root") 
        self.prog = "testbin/os161testerprog"
        self.menu = 'Operation took \d+\.\d+ seconds'
        self.prompt = r'OS/161 kernel \[\? for menu\]:'
        self.marker = open('os161-mark.txt', 'a')

    def __del__(self):
        self.kernel.logfile.close()
        if (self.total > 0):
            print('Mark for ' + self.message + ' is ' + \
                  str(self.mark) + ' out of ' + str(self.total))
            self.marker.write(self.message + ', ' + str(self.total) + \
                             ', ' + str(self.mark) + '\n')
        self.marker.close()
        try:
            os.remove(self.cwd + self.prog)
        except OSError:
            pass

    # By default, we wait before we send a command.
    # However, if wait is set to 0, then we don't wait AND
    # we don't send the newline.
    def send_command(self, cmd, wait = 1):
        if wait:
            try:
                self.kernel.expect(self.prompt)
            except pexpect.TIMEOUT:
                print('WARNING: PROMPT NOT FOUND')
            except Exception:
                # this is probably not recoverable. don't send command
                print('OS HAS CRASHED')
                return       
        # The fun bit is, we need to send the command character by
        # character to the simulator, otherwise we are going to have
        # a lot of fun ;-)
        if self.verbose > 1:
            print("SENDING: " + cmd)
        cmd_char = list(cmd)

        for i in cmd_char:
            self.kernel.send(i)
        if wait:
            self.kernel.send('\n')

    def runprogram(self, cmd, args = ""):
        # make a copy of the program, so that students don't try 
        # to guess the output of a program by its name
        output = ""
        try:                
            shutil.copy(self.cwd + cmd, os.path.join(self.cwd, self.prog))
            self.send_command("p " + self.prog + " " + args);
            self.send_command('b', 0)
            while 1:
                try:
                    index = self.kernel.expect([self.prompt, pexpect.EOF], timeout=None)
                    if self.kernel.before:
                        output+=self.kernel.before+"\n"
                    if index == 0 or "sys161:" in self.kernel.before:
                        break  # Prompt found, end of output
                except pexpect.TIMEOUT:
                    break  # No more output

        except IOError:
                print("NOT FOUND: %s"%cmd)
        return output

    def look(self, result, silent=0):
        if isinstance(result, list):
            result = [ self.prompt ] + result
        else:
            result = [ self.prompt, result ]
        return self.look_for(result, silent) - 1

    def look_for(self, result, silent=0):
        display = lambda msg, rv=0 : print(msg) or rv if not silent else rv
        try:
            if self.verbose > 1:
                display("EXPECTING: " + str(result))
            index = self.kernel.expect(result)
            if self.verbose > 0:
                display("FOUND: " + self.kernel.match.group(0))
        except pexpect.TIMEOUT:
            return display("TIMEOUT ERROR", -1)
        except pexpect.EOF:
            return display("END OF FILE ERROR", -2)
        except Exception:
            return display("UNEXPECTED ERROR %s\n"%sys.exc_info()[0] +
                   "\nPLEASE REPORT THIS TO THE INSTRUCTOR OR A TA:\n", -3)
        return index

    def print_result(self, mark_obtained, mark):
        self.total += mark
        self.mark += mark_obtained
        if mark_obtained == mark:
            print("PASS")
        else:
            print("FAIL")

    def look_for_and_print_result(self, result, mark, menu=0):
        if menu:
            if isinstance(result, list):
                result = [ self.menu ] + result
            else:
                result = [ self.menu, result ]
        out = self.look_for(result)
        if menu and out > 0:
            self.print_result(mark, mark)
        elif not menu and out >= 0:
            self.print_result(mark, mark)
        else:
            self.print_result(0, mark)
        return out
        
# Define the message for the test
message = "Running sys161 kernel and sending input 'test'"

# Create an instance of TestUnit
test = TestUnit(message)
o = test.runprogram("/testbin/badcall")

for i in o.strip().split('\n')[1:]:
    if 'sys161:' in i and 'cycles' in i:
        break
    print(i)


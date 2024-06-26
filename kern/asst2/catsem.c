/*

111)                                                                                            [][[}}}}[[}[>             ...''```
1{{}}[                                                                                          ]          ?l                .''``
1{}}[}[[                                                                                        ]   ]^^++  ];                 ...'
{}[]]]]]??                                                                                      ?   ]  ~+  ],                  ...
}[[[[]]??-_~.                                                                                   ?    ]_~+  ]"                     
[[[]]???--_+++'                                                                                 ?   -  ?I  ]                      
+][]??--____+++~.                                                                               ??i ?-??-?_-                      
``<??--___++++~~~+                                                                              ]          _                      
``''>--___+++~+~~<                                                                              ]  ++ ;?.' ?                      
'''''.!+_++++~+~~<                                                                              ]  <<?:]?  ]                      
''''..  I++++~+~~<                                           `t\rf.                              [_ii!lIII:]                      
''...     :~~~~~~>                                          \vzLmYJ\                                                              
''...       :~~~~>                                          XCCJ]1OU                                                              
....          ^~~~                                          i_;?i??                                                               
 ...            ^>                                        ooool_++                                                                
                                                     `qhbk #ooo}I# a/                                                             
                                                    ]hao**\&##haoM`aaoQ                                                           
                                                  Ldooo**#i#***ao-z1ooou                                                          
                                                bbkooo*##8Y*****h}*o*ooabb                                                        
                                              pkkho*##**&;*****#l%Z   /#*akaX.                                                    
                                            dahd((M#M##o"##****c&         j*khoa('                                                
                                          0*o#    LCJQOt#&#*#a`               \Yqbb                                               
                                         qho      nYYYzYJLQ00Lr            _}+>~}l]l                                              
                                        1aa       cnxucccvuccnf^          <)~_+([[{:                                              
                                       li l        |1)tjxuxjt/\\|         <t)/()|t\                                               
                                        "          1[]}(x CYuruf/|,        :z\xjxn                                                
                                                   ;]??]!  "zvurf||                                                               
                                                    1}[[1    ~Xvr/\\`                                                             
                                                     }[][      xvxj/|                                                             
                                            .       ~[{{{!      ]Xunxf                                                            
                                       .........   :)/jn(       \zzcc                                                             
                                    ........''...  (/rn         nvcc                                                              
''''''.                                ''````````"{/xu.        ]vccc                                                              
^^^^^`''''''.......'''''''''''''''`^^^,"","""""",,|juu``''''''^uccz,.''''`'''``````''''''''''''''''````````^^^^^^^^"",,,,::;;IIIIl
^^^^```''''.....................''''```''''''''''\nnn\...'''' ZpXzz ................................''''''''''''```^^^^",,,::;;III
^^`'''''...            .          ..............>pdk..    .. 0dkbO....... ........................'''......''''''''``^^^^"",,::;II
''.......                  ...... .'............\L0b`...... >kkkbqU'.''''...............     ..........         ..''```^^^^,,:::;;
''.....                                    .....tOdq '.....'`":,wbpwm````'''......                                ...'''`^^^"",,::
'...                                           .?JZw, '.....`:IIIlI:`.......                                       ....''`````^^,:
                                                '^,"         `""^`.                                                 .....''``^^,,:


 * catsem.c
 *
 * Please use SEMAPHORES to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include "catmouse.h"

/*
 * 
 * Function Definitions
 * 
 */

/*
 * catsem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
catsem(void * unusedpointer, 
       unsigned long catnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) catnumber;
}
        

/*
 * mousesem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
mousesem(void * unusedpointer, 
         unsigned long mousenumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) mousenumber;
}


/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int
catmousesem(int nargs,
            char ** args)
{
        int index, error;
   
        /*
         * Start NCATS catsem() threads.
         */

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catsem Thread", 
                                    NULL, 
                                    index, 
                                    catsem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catsem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
        
        /*
         * Start NMICE mousesem() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mousesem Thread", 
                                    NULL, 
                                    index, 
                                    mousesem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mousesem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * wait until all other threads finish
         */

        while (thread_count() > 1)
                thread_yield();

        (void)nargs;
        (void)args;
        kprintf("catsem test done\n");

        return 0;
}


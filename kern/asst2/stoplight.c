#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>
#define NCARS 20

static const char *directions[] = { "N", "W", "S", "E" };
static const char *msgs[] = {
  "approaching:",
  "region1:    ",
  "region2:    ",
  "region3:    ",
  "leaving:    "
};
/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

static void message(int msg_nr, int carnumber, int cardirection, int destdirection) {
  kprintf(
    "%s car = %2d, direction = %s, destination = %s\n",
    msgs[msg_nr], carnumber,
    directions[cardirection], directions[destdirection]);
}

struct semaphore *grids[4];
struct semaphore *grids_edit_mutex;
static void general(unsigned long cardirection, unsigned long carnumber, unsigned long nturns) {
    //nturns = how many regions needed
    unsigned long destdirection = (cardirection + nturns) % 4;
    unsigned long d, reg;

    //APPROACHING
    message(APPROACHING, carnumber, cardirection, destdirection);
   
    // critical region. we either reserve all or none of the grids
    P(grids_edit_mutex);
        // reserve the needed grids
        for (d = 0; d < nturns; d++) { 
            P(grids[(cardirection + d) % 4]);
        }
    V(grids_edit_mutex);

    // do the turns 
    for (reg = 1; reg <= nturns; reg++) {
        message(reg, carnumber, cardirection, destdirection);
    }
    // LEAVE
    message(LEAVING, carnumber, cardirection, destdirection);
    // un-reserve the needed grids 
    for (d = 0; d < nturns; d++) {
        V(grids[(cardirection + d) % 4]); 
    }

    // int i;
    // for (i = 0; i < 4; i++) {
    //     kprintf("%d", grids[i]->count);
    // }
    // kprintf("\n");
}

/* gostraight()
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */
static void gostraight(unsigned long cardirection, unsigned long carnumber) {
  general(cardirection, carnumber, 2); 
}
static void turnleft(unsigned long cardirection, unsigned long carnumber) {
  general(cardirection, carnumber, 3);
}
static void turnright(unsigned long cardirection, unsigned long carnumber) {
  general(cardirection, carnumber, 1);
}


/*
 * approachintersection()
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 * Returns:
 *      nothing.
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
static void approachintersection(void * unusedpointer, unsigned long carnumber) {
  int cardirection = random() % 4;
  int way = random() % 3;
  if (way== 0) { gostraight(cardirection, carnumber);} 
  else if (way == 1) { turnleft(cardirection, carnumber); } 
  else { turnright(cardirection, carnumber); }
  (void) unusedpointer;
}


/* createcars()
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 * Returns:
 *      0 on success.
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int createcars(int nargs, char ** args) {
  int i;
  for ( i = 0; i < 4; i++) {
      grids[i] = sem_create("sem", 1);
  }
  grids_edit_mutex=sem_create("grids_edit_mutex", 1);
  int index, error;
  //Start NCARS approachintersection() threads.
  for (index = 0; index < NCARS; index++) {
    error = thread_fork("approachintersection thread", NULL, index, approachintersection, NULL);
    // panic() on error.
    if (error) { 
      panic("approachintersection: thread_fork failed: %s\n", strerror(error));
    }
  }
  
  // wait until all other threads finish
  while (thread_count() > 1)
    thread_yield();
  (void)message;
  (void)nargs;
  (void)args;
  for (i = 0; i < 4; i++) {
      sem_destroy(grids[i]);
  }
  sem_destroy(grids_edit_mutex);
  kprintf("stoplight test done\n");
  return 0;
}


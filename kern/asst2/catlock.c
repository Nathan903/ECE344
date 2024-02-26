#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include "catmouse.h"
#include <synch.h>
/*
 * catlock()
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 * Returns: nothing.
 * Notes: Write and comment this function using locks/cv's.
 */
#define EMPTY 0
#define CCC 1
#define MMM 2
int dishes[2] = {0, 0};
struct lock* dishes_lock;

static void catlock(void * unusedpointer, unsigned long catnumber) {
  int i;
  for ( i = 0; i < 4; i++) {
    int no_mouses, have_space,selected_dish;
    while (1) {
      lock_acquire(dishes_lock);
      no_mouses = (dishes[0] != MMM) && (dishes[1] != MMM);
      have_space = (dishes[0] == EMPTY) || (dishes[1] == EMPTY);
      if (no_mouses && have_space) {      
        if (dishes[0] == EMPTY) {
            selected_dish = EMPTY;
            dishes[0] = CCC;
        } else if (dishes[1] == EMPTY) {
            selected_dish = 1;
            dishes[1] = CCC;
        }
        lock_release(dishes_lock);
        break;
      }
      lock_release(dishes_lock);
    }
    catmouse_eat("cat", catnumber, 1+selected_dish, i);
    lock_acquire(dishes_lock);
    dishes[selected_dish] = EMPTY;
    lock_release(dishes_lock);
  }
  (void) unusedpointer;
}



/* mouselock()
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 * Returns: nothing.
 * Notes: Write and comment this function using locks/cv's.
 */

static void mouselock(void * unusedpointer, unsigned long mousenumber) {
  (void) unusedpointer;
int i;
  for ( i = 0; i < 4; i++) {
    int no_cats, have_space,selected_dish;
    while (1) {
      lock_acquire(dishes_lock);
      no_cats= (dishes[0] != CCC) && (dishes[1] != CCC);
      have_space = (dishes[0] == EMPTY) || (dishes[1] == EMPTY);
      if (no_cats&& have_space) {      
        if (dishes[0] == EMPTY) {
            selected_dish = EMPTY;
            dishes[0] = MMM;
        } else if (dishes[1] == EMPTY) {
            selected_dish = 1;
            dishes[1] =MMM;
        }
        lock_release(dishes_lock);
        break;
      }
      lock_release(dishes_lock);
    }
    catmouse_eat("mouse", mousenumber, selected_dish+1, i);
    lock_acquire(dishes_lock);
    dishes[selected_dish] = EMPTY;
    lock_release(dishes_lock);
  }
(void) mousenumber;
}

/*
 * catmouselock()
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 * Returns: 0 on success.
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */
int catmouselock(int nargs, char ** args) {
  int index, error;
  dishes_lock= lock_create("dishes_lock");

  //Start NCATS catlock() threads.
  for (index = 0; index < NCATS; index++) {
    error = thread_fork("catlock thread",
      NULL,
      index,
      catlock,
      NULL
    );

    if (error) {
      panic("catlock: thread_fork failed: %s\n", strerror(error));
    }
  }
  // Start NMICE mouselock() threads.
  for (index = 0; index < NMICE; index++) {
    error = thread_fork("mouselock thread",
      NULL,
      index,
      mouselock,
      NULL
    );
    if (error) {
      panic("mouselock: thread_fork failed: %s\n", strerror(error));
    }
  }

  // wait until all other threads finish
  while (thread_count() > 1)
    thread_yield();

  (void) nargs;
  (void) args;
  lock_destroy(dishes_lock);
  kprintf("catlock test done\n");
  return 0;
}

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <clock.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <syscall.h>


/*
 * System call handler.
 *
 * A pointer to the trapframe created during exception entry (in
 * exception.S) is passed in.
 *
 * The calling conventions for syscalls are as follows: Like ordinary
 * function calls, the first 4 32-bit arguments are passed in the 4
 * argument registers a0-a3. In addition, the system call number is
 * passed in the v0 register.
 *
 * On successful return, the return value is passed back in the v0
 * register, like an ordinary function call, and the a3 register is
 * also set to 0 to indicate success.
 *
 * On an error return, the error code is passed back in the v0
 * register, and the a3 register is set to 1 to indicate failure.
 * (Userlevel code takes care of storing the error code in errno and
 * returning the value -1 from the actual userlevel syscall function.
 * See src/lib/libc/syscalls.S and related files.)
 *
 * Upon syscall return the program counter stored in the trapframe
 * must be incremented by one instruction; otherwise the exception
 * return code will restart the "syscall" instruction and the system
 * call will repeat forever.
 *
 * Since none of the OS/161 system calls have more than 4 arguments,
 * there should be no need to fetch additional arguments from the
 * user-level stack.
 *
 * Watch out: if you make system calls that have 64-bit quantities as
 * arguments, they will get passed in pairs of registers, and not
 * necessarily in the way you expect. We recommend you don't do it.
 * (In fact, we recommend you don't use 64-bit quantities at all. See
 * arch/mips/include/types.h.)
 */
#define ATOMIC_START int spl = splhigh()
#define ATOMIC_END splx(spl) 
int /*err*/ sys_write(int fd, const void* buf, size_t nbytes, int32_t* return_value){ 
  if (!(fd==1 || fd==2)){
    (*return_value)=-1;
    return EBADF;
  }
  ATOMIC_START;
  char* kernel_buffer = kmalloc(nbytes+1);
  int copyin_failure= copyin(buf,(void *) kernel_buffer, nbytes); //0 on success, EFAULT if a memory
  if(!copyin_failure) {
    kernel_buffer[nbytes]='\0';
    kprintf(kernel_buffer);
    (*return_value)=nbytes;
  } else{
    (*return_value)=-1;
  }

  ATOMIC_END;
  kfree((void *)kernel_buffer);
  return copyin_failure;
}


int sys___time(time_t *seconds, time_t *nanoseconds, int32_t* return_value){
  time_t s; u_int32_t ns;

  gettime(&s, &ns);
  int copyout_failure;
  if(seconds==NULL && nanoseconds==NULL){
    *return_value=s;

    return 0;
  }
  if(seconds!=NULL){
    copyout_failure = copyout( (const void *) &s , (userptr_t) seconds, sizeof(time_t));
    if(copyout_failure!=0){
    	*return_value=-1;

    	return EFAULT;
    }
  }
  if(nanoseconds!=NULL){
    copyout_failure = copyout( (const void *) &ns , (userptr_t) nanoseconds, sizeof(u_int32_t));
    if(copyout_failure!=0){

    	*return_value=-1;
    	return EFAULT;
    }
  }

  (*return_value)=(int32_t)s;
  return 0;
}

void mips_syscall(struct trapframe *tf) {
  int callno;
  int32_t retval;
  int err;

  assert(curspl == 0);

  callno = tf->tf_v0;

  /*
   * Initialize retval to 0. Many of the system calls don't
   * really return a value, just 0 for success and -1 on
   * error. Since retval is the value returned on success,
   * initialize it to 0 by default; thus it's not necessary to
   * deal with it except for calls that return other values,
   * like write.
   */

  retval = 0;

  switch (callno) {
  case SYS_reboot:
    err = sys_reboot(tf->tf_a0);
    break;
  case SYS_write:
    err = sys_write(tf->tf_a0,(const void *)tf->tf_a1,tf->tf_a2, &retval);
    break;
  case SYS___time:

      err = sys___time((time_t *)tf->tf_a0,(time_t *)tf->tf_a1,&retval);
    break;

  default:
    kprintf("Unknown syscall %d\n", callno);
    err = ENOSYS;
    break;
  }

  if (err) {
    /*
     * Return the error code. This gets converted at
     * userlevel to a return value of -1 and the error
     * code in errno.
     */
    tf->tf_v0 = err;
    tf->tf_a3 = 1; /* signal an error */
  } else {
    /* Success. */
    tf->tf_v0 = retval;
    tf->tf_a3 = 0; /* signal no error */
  }

  /*
   * Now, advance the program counter, to avoid restarting
   * the syscall over and over again.
   */

  tf->tf_epc += 4;

  /* Make sure the syscall code didn't forget to lower spl */
  assert(curspl == 0);
}

void md_forkentry(struct trapframe *tf) {
  /*
   * This function is provided as a reminder. You need to write
   * both it and the code that calls it.
   *
   * Thus, you can trash it and do things another way if you prefer.
   */

  (void)tf;
}

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <clock.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <test.h>
#define INVALID_IDX -69
extern u_int32_t curkstack;
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
int /*err*/ sys_read(int fd, const void* buf, size_t nbytes, int32_t* return_value ) { 
  if (!(fd==0)){
    (*return_value)=-1;
    return EBADF;
  }
  if (nbytes!=1) {
    (*return_value)=-1;
    return EUNIMP;
  }
  ;
  char input = getch();
  int copyout_failure = copyout( (const void *) &input , (userptr_t) buf, sizeof(char));
  if(!copyout_failure) {
  (*return_value)=1;
  } else{
    (*return_value)=-1;
  }

  return copyout_failure;
  
}

int sys_sleep( int sec){
clocksleep(sec); return 0;
}

int sys___time(time_t *seconds, time_t *nanoseconds, int32_t* return_value){
  time_t s; u_int32_t ns;

  gettime(&s, &ns);
  int copyout_failure;
  if(seconds==NULL && nanoseconds==NULL){
    (*return_value)=s;

    return 0;
  }
  if(seconds!=NULL){
    copyout_failure = copyout( (const void *) &s , (userptr_t) seconds, sizeof(time_t));
    if(copyout_failure!=0){
    	(*return_value)=-1;

    	return EFAULT;
    }
  }
  if(nanoseconds!=NULL){
    copyout_failure = copyout( (const void *) &ns , (userptr_t) nanoseconds, sizeof(u_int32_t));
    if(copyout_failure!=0){

    	(*return_value)=-1;
    	return EFAULT;
    }
  }

  (*return_value)=(int32_t)s;
  return 0;
}
int sys_getpid(int32_t* return_value){
  (*return_value)=curthread->pid;
  return 0;
}
int sys__exit(int32_t exit_code){
  curthread->exit_code = exit_code;
  curthread->has_exited = 1;
  
  int i;for(i=0; i<PID_TABLE_LEN ;i++){
    if(pid_to_threadptr[i].pid==curthread->pid){
      pid_to_threadptr[i].exit_code=exit_code;
      pid_to_threadptr[i].has_exited=curthread->pid+69;
      break;
    }
  }
  ATOMIC_START;
  thread_wakeup((void*) curthread->pid);
  ATOMIC_END;
  
  thread_exit();
  return 0;
}
int sys_waitpid(pid_t pid, int *status, int options, int32_t* return_value){
  if(options!=0 || pid<1 || pid>= cur_max_pid){
    (*return_value) = -1;
    return EINVAL;
  }
  int kernel_status;
  int child_idx = INVALID_IDX;
  ATOMIC_START;

  int i;for(i=0; i<PID_TABLE_LEN ;i++){
    if (pid_to_threadptr[i].pid==pid ){
      child_idx = i; break;
    }
  }
  
  if (child_idx == INVALID_IDX){
    (* return_value)=-1;
    //kprintf("\nCHILD DIED\n");
    ATOMIC_END;
    return EINVAL;
  }
  if(pid_to_threadptr[child_idx].parent_thread != curthread){
    (* return_value)=-1;
    //kprintf("\nNO PERMISSION\n");
    ATOMIC_END;
    return EINVAL;
  
  }
  //kprintf("\nCHILD alive %d %d %d\n",pid_to_threadptr[child_idx].has_exited, pid, child_idx);
    
  while(pid_to_threadptr[child_idx].has_exited==0){
    thread_sleep((void*)pid);
  }
  //kprintf("\nCHILD just died\n");
  
  kernel_status = pid_to_threadptr[child_idx].exit_code;
  //destory child from table
  pid_to_threadptr[child_idx].pid = 0;
  pid_to_threadptr[child_idx].has_exited= 0;
  
  int copyout_failure; copyout_failure = copyout( (const void *) &kernel_status, (userptr_t) status,sizeof(int));
    ATOMIC_END;
    if(copyout_failure!=0){
      (*return_value)=-1;
      return EFAULT;
    }
    (*return_value)=pid;
    return 0;
}

void md_forkentry2(struct trapframe *tf, struct addrspace * parent_as) {

  struct trapframe new_tf;
  memcpy(&new_tf, tf, sizeof(struct trapframe));
  // kprintf("made tf\n");
  // kprintf("copied tf\n");
  new_tf.tf_a3 = 0;
  new_tf.tf_v0 = 0;
  new_tf.tf_epc +=4;
  // kprintf("curthread [%p] \n", curthread);
  curthread->t_vmspace = parent_as; 
  as_activate(parent_as);
  assert(curthread->pid <=cur_max_pid);
  // int i;for(i=0; i<PID_TABLE_LEN ;i++){
  //   if (pid_to_threadptr[i].pid==0){
  //     pid_to_threadptr[i].pid =cur_max_pid;
  //     pid_to_threadptr[i].threadptr=curthread;
  //     break;
  //   }
  // } cur_max_pid++;
  kfree(tf); // THIS MUST BE IN CHILD, CUZ PARENT MIGHT FREE IT TOO EARLY
  mips_usermode(&new_tf);
  assert(1==2);
  
}

int sys_fork(struct trapframe* tf, int32_t* return_value){

  // kprintf("start forking\n");
  ATOMIC_START;
  struct addrspace * new_as;
  int result; result = as_copy(curthread -> t_vmspace, &new_as);
  if (result!=0) {
    (*return_value) = -1;
    ATOMIC_END;
    return ENOMEM;
  }

  // kprintf("copied as\n");
  struct thread* child_thread /*##*/=NULL;
  struct trapframe * copied_tf = kmalloc(sizeof(struct trapframe));
  memcpy(copied_tf, tf, sizeof(struct trapframe));
  // kprintf("parent curthread [%p] \n", curthread);
  int t_result; t_result =thread_fork("a", (void *) copied_tf, (unsigned long) new_as, (void (*)(void *, unsigned long) )md_forkentry2, &child_thread);
  // kprintf("forked\n");
  // kprintf("parent curthread [%p] \n", curthread);

  if (t_result!=0){
    kprintf("\nFORK BAD!\n");
    ATOMIC_END;
  }
  (*return_value) = (int32_t) child_thread->pid;

  ATOMIC_END;
  return 0;
}
#define MAX_STR_LENGTH2 30//########
#define MAX_ARG_LEN 30//########
#define MAX_ARGS 15 //########

#define CHECK_FAILURE(N) \
    do { \
        if (copyin_failure == EFAULT) { \
            (*return_value) = -1; \
            return EFAULT; \
        } \
        if (str_len >= (N) ) { \
            (*return_value) = -1; \
            kprintf("\nEXEC STR TOO BIG\n"); \
            return E2BIG; \
        } \
        if (str_len <=1) { \
            (*return_value) = -1; \
            return EINVAL; \
        } \
    } while(0)
int sys_execv(const char *userland_program, char **userland_args, int32_t* return_value){

  if(userland_program==NULL || userland_args==NULL){
    (*return_value) = -1;
    return EFAULT;
  
  }
  size_t str_len;
  char program[MAX_STR_LENGTH2];
  int j; for(j=0; j<MAX_STR_LENGTH2; j++)  program[j]='\0';
  int copyin_failure= copyinstr(
    (const void*) userland_program,
    (void *) program, 
    (size_t) MAX_STR_LENGTH2,
    &str_len
  ); 
  CHECK_FAILURE(MAX_STR_LENGTH2);
  //kprintf("%s \n",program);

  //kprintf("\tprogram:%s strlen:%d\n",program, str_len);
  
  char * args[MAX_ARGS];  
  copyin_failure= copyin(
    (const void*) userland_args,
    (void *) args, 
    (size_t) sizeof(char*)*MAX_ARGS
  ); 
  if(copyin_failure ==EFAULT){
    (*return_value) = -1;
    return EFAULT;
  }

  int argc; for (argc =0; argc<MAX_ARGS; argc++){
	char * userland_argi = args[argc];
        //kprintf("%p\n", args[argc]);
	if(userland_argi==NULL){
	  break;
	}
	args[argc] = kmalloc( sizeof(char)* MAX_ARG_LEN);
	copyin_failure= copyinstr(
          (const void*) userland_argi,
          (void *) args[argc], 
          (size_t) sizeof(char*)*MAX_STR_LENGTH2,
          &str_len
        );
        CHECK_FAILURE(MAX_ARG_LEN);
        //kprintf("%s\n", args[argc]);
	
  }
  if(argc>=MAX_ARGS){
    (*return_value) = -1;
    return E2BIG;
  }
  //kprintf("\tprogram:%s argc:%d\n",program, argc);
  runprogram_with_args(program, args, argc);

  (*return_value) = -1;
  //kprintf("EXEC SOMEHOW REUTRNED");
  assert(1==2);
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
  case SYS_read:
    err = sys_read(tf->tf_a0,(const void *)tf->tf_a1,tf->tf_a2, &retval);
    break;
  case SYS_sleep:
    err = sys_sleep(tf->tf_a0);
    break;
  case SYS_getpid:
    err = sys_getpid(&retval);
    break;
  case SYS__exit:
    err = sys__exit(tf->tf_a0);
    break;
  case SYS_waitpid:
    err = sys_waitpid((pid_t) tf->tf_a0, (int *) tf->tf_a1, (int) tf->tf_a2,&retval);
    break;
  case SYS_fork:
    err = sys_fork(tf, &retval );
    break;
  case SYS_execv:
    err = sys_execv( (const char *)tf->tf_a0, (char **)tf->tf_a1, &retval );
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

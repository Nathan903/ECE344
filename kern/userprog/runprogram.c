/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <lib.h>
#include <test.h>
void rp(const char * s);
/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname)
{
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &v);
	if (result) {
		return result;
	}

	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	// vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}

	/* Warp to user mode. */
	md_usermode(0 /*argc*/, NULL /*userspace addr of argv*/,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}
#define MAX_STR_LENGTH 100 //########
#define NOT_VALID_STR 69

int strnlen(const char *s){
	int ret = 0;
	while (s[ret]) { 
	ret++;
	  if(ret>MAX_STR_LENGTH){ 
	    return -1;
	  }
	}
	return ret;
}
int runprogram_with_args(char *progname,char ** args, unsigned long nargs){
  struct vnode *v;
  vaddr_t entrypoint, stackptr;
  int result;

  /* Open the file. */
  result = vfs_open(progname, O_RDONLY, &v);
  if (result) {
  rp("1");
    return result;
  }

  /* We should be a new thread. */
  if(curthread->t_vmspace!=NULL){
    as_destroy(curthread->t_vmspace);
    curthread->t_vmspace=NULL;
  }
  assert(curthread->t_vmspace == NULL);

  /* Create a new address space. */
  curthread->t_vmspace = as_create();
  if (curthread->t_vmspace == NULL) {
    vfs_close(v);
    rp("2");
    return ENOMEM;
  }

  /* Activate it. */
  as_activate(curthread->t_vmspace);

  /* Load the executable. */
  result = load_elf(v, &entrypoint);
  if (result) {
    /* thread_exit destroys curthread->t_vmspace */
    vfs_close(v);
    rp("3");
    return result;
  }

  /* Done with the file now. */
  // vfs_close(v);

  /* Define the user stack in the address space */
  result = as_define_stack(curthread->t_vmspace, &stackptr);
  if (result) {
    /* thread_exit destroys curthread->t_vmspace */
    rp("4");return result;
  }
  
  
    int args_copy_len = 4*(nargs+1);
  vaddr_t *lengths = (vaddr_t*)kmalloc((nargs + 1) * sizeof(vaddr_t));
  lengths[nargs] = (vaddr_t) 0;

  int curlen = 0; int padding =0;

  //kprintf("\n args_copy_len %d \n",args_copy_len);
  unsigned long i;for(i=0; i<nargs;i++){
    curlen =  strnlen(args[i]);
    if (curlen ==-1){rp("6"); return NOT_VALID_STR;}
    padding = (4-(curlen+1)%4)%4;
    assert( (curlen+padding+1)%4 == 0);
    args_copy_len+= curlen+padding+1;
    lengths[i] = (vaddr_t) (curlen+padding+1);
  } assert( args_copy_len%4 == 0);
  
  stackptr -= (vaddr_t) args_copy_len;
  //kprintf("\n args_copy_len %d \n",args_copy_len);
  char *args_copy = (char *)kmalloc( args_copy_len );
  int ii; for (ii = 0; ii < args_copy_len; ii++) { args_copy[ii] = '\0';}
  unsigned int  idx=4*(nargs+1);
  for(i=0; i<nargs;i++){
    //kprintf("%u %d\n",idx,args_copy_len);
    strcpy( (char *) (args_copy+idx), args[i]);
    //kprintf("%u %d\n",idx,args_copy_len);
    int final_loc = stackptr+idx;
    idx += lengths[i];
    lengths[i]= final_loc;
  } 
  /*
  for (ii = 0; ii < args_copy_len; ii++) { 
    if(args_copy[ii])
    kprintf("%c", args_copy[ii]);
    else kprintf("_");
  } kprintf("\n");
 for (ii = 0; ii < (int)nargs; ii++) { 
    kprintf("%x ",lengths[ii]);
  } kprintf("\n");
  kprintf("%x\n", stackptr);
  */
  
  copyout( (const void *)(args_copy), (userptr_t)stackptr, (size_t) args_copy_len);
  copyout( (const void *)(lengths), (userptr_t)stackptr, (size_t) ((nargs + 1) * sizeof(vaddr_t)));
  //kprintf("%x\n", stackptr);
  
  kfree(args_copy);
  kfree(lengths);
  /* Warp to user mode. */
  md_usermode(
    (int) nargs, //argc
    (userptr_t) stackptr /*userspace addr of argv*/,
    stackptr,
    entrypoint);
rp("8");
  /* md_usermode does not return */
  panic("md_usermode returned\n");
  return EINVAL;
}


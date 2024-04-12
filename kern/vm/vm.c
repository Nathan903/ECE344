#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <vfs.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <synch.h>

/* under dumbvm, always have 48k of user stack */
#define DUMBVM_STACKPAGES    30

#define nullptr '\0'
extern u_int32_t firstfree;   /* first free virtual address; set by start.S */
extern u_int32_t firstpaddr;  /* address of first free physical page */
extern u_int32_t lastpaddr;   /* one past end of last free physical page */
char vm_bootstrapped=0;
#include "common.c"
#include "coremap.c"
void vm_bootstrap(void) { 
  kprintf("[vm_bootstrap]: firstaddr%u lastaddr%u firstfree%u freesize:%uK\n",firstpaddr, lastpaddr, firstfree, (lastpaddr-firstpaddr)/1024);
  vm_bootstrapped=1;
  //dirtyqueue= q_create(COREMAP_SIZE);
}
paddr_t getppages(unsigned long npages) {
  struct addrspace* as = (curthread==nullptr) ? NULL : curthread->t_vmspace;
  return getppages_vm(npages, as, (as==NULL)? FIXED_STATE : DIRTY_STATE);
}
paddr_t getppages_vm(unsigned long npages, struct addrspace* as,char state) {
  if (npages>20){
    rp("getppageBIG"); kprintf("%ld\n",npages);
    npages=20;
  }
  int spl;int pagei;
  spl = splhigh();
  /////////////////////////////  
  assert((vm_bootstrapped) || ((!vm_bootstrapped)&&((curthread==nullptr) || (curthread->pid==0))));
  if(vm_bootstrapped){
    //gp("[alc]");kprintf("%d\n",free_page_size());
    lock_acquire(&cmlock);
    pagei = get_pages(npages, as,state );
    lock_release(&cmlock);
  } else{ pagei = get_pages(npages, as,state );}
  ///////////////////////////
  //kprintf("[addr] %u %u %u %u\n", I_TO_ADDR(pagei), firstpaddr,(lastpaddr-firstpaddr)/1024,(lastpaddr-firstpaddr)%1024);
  splx(spl);
  assert(pagei>=0);
  // kprintf("[allocd]%d\n",free_page_size());
  // if(free_page_size()==1){
  //   print_core_map();
  // }
  return I_TO_ADDR(pagei);
}

vaddr_t alloc_kpages(int npages) {
  if(npages!=1) rp("NPAGES!=1"); //assert(npages==1);
  //kprintf("alloc: firstaddr%u lastaddr%u firstfree%u freesize:%uK\n",firstpaddr, lastpaddr, firstfree, (lastpaddr-firstpaddr)/1024);
  paddr_t pa;
  pa = getppages(npages);
  if (pa == 0) {
    return 0;
  }
  if(vm_bootstrapped){
    //rp("a"); kprintf("%d ",free_page_size());
  }
  return PADDR_TO_KVADDR(pa);
}



void free_kpages(vaddr_t addr) {
    
  //gp("e"); kprintf("%u %u %d %x\n",addr,KADDR_I(addr),free_page_size(),coremap[ KADDR_I(addr) ].npages);
  assert(COREMAP_SIZE>KADDR_I(addr) ); 
  lock_acquire(&cmlock);
  char i; for(i=0;i<coremap[ KADDR_I(addr) ].npages;i++){
    coremap[ KADDR_I(addr) +i].state=FREE_STATE;
  }

  lock_release(&cmlock);
}


#include "vm_fault.c"
#include "new_addrspace.c"
#include "swap.c"

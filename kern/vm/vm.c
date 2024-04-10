#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>

/* under dumbvm, always have 48k of user stack */
#define DUMBVM_STACKPAGES    12

#define nullptr '\0'
extern u_int32_t firstfree;   /* first free virtual address; set by start.S */
extern u_int32_t firstpaddr;  /* address of first free physical page */
extern u_int32_t lastpaddr;   /* one past end of last free physical page */
char vm_bootstrapped=0;
#include "coremap.c"
void vm_bootstrap(void) { 
  kprintf("[vm_bootstrap]: firstaddr%u lastaddr%u firstfree%u freesize:%uK\n",firstpaddr, lastpaddr, firstfree, (lastpaddr-firstpaddr)/1024);
  vm_bootstrapped=1;

}

static paddr_t getppages(unsigned long npages) {
  int spl;int pagei;
  spl = splhigh();
  /////////////////////////////  
  assert((vm_bootstrapped) || ((!vm_bootstrapped)&&((curthread==nullptr) || (curthread->pid==0))));
  if(vm_bootstrapped){
    lock_acquire(&cmlock);
    pagei = get_pages(npages, (curthread==nullptr) ? 0 : curthread->pid);
    lock_release(&cmlock);
  } else{ pagei = get_pages(npages, (curthread==nullptr) ? 0 : curthread->pid);}
  ///////////////////////////
  //kprintf("[addr] %u %u %u %u\n", I_TO_ADDR(pagei), firstpaddr,(lastpaddr-firstpaddr)/1024,(lastpaddr-firstpaddr)%1024);
  splx(spl);
  assert(pagei>=0);
  return I_TO_ADDR(pagei);
}

vaddr_t alloc_kpages(int npages) {
  if(npages!=1) rp("NPAGES!=1"); assert(npages==1);
  
  //kprintf("alloc: firstaddr%u lastaddr%u firstfree%u freesize:%uK\n",firstpaddr, lastpaddr, firstfree, (lastpaddr-firstpaddr)/1024);
  paddr_t pa;
  pa = getppages(npages);
  if (pa == 0) {
    return 0;
  }
  if(vm_bootstrapped){
    rp("a"); kprintf("%d ",free_page_size());
  }
  return PADDR_TO_KVADDR(pa);
}



void free_kpages(vaddr_t addr) {
  //rp("e"); kprintf("%u %u %d\n",addr,KADDR_I(addr),free_page_size());
  assert(COREMAP_SIZE>KADDR_I(addr) ); 
  lock_acquire(&cmlock);
  coremap[ KADDR_I(addr) ].state=FREE_STATE;
  lock_release(&cmlock);
}









int vm_fault(int faulttype, vaddr_t faultaddress) {
  vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
  paddr_t paddr;
  int i;
  u_int32_t ehi, elo;
  struct addrspace *as;
  int spl;

  spl = splhigh();

  faultaddress &= PAGE_FRAME;

  //DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

  switch (faulttype) {
  case VM_FAULT_READONLY:
    /* We always create pages read-write, so we can't get this */
    panic("dumbvm: got VM_FAULT_READONLY\n");
  case VM_FAULT_READ:
  case VM_FAULT_WRITE:
    break;
  default:
    splx(spl);
    DEBUG(DB_VM, "\x1b[31m\n[vmfault]EINVAL 0x%x\n\x1b[0m",faultaddress);
    return EINVAL;
  }

  as = curthread->t_vmspace;
  if (as == NULL) {
    /*
     * No address space set up. This is probably a kernel
     * fault early in boot. Return EFAULT so as to panic
     * instead of getting into an infinite faulting loop.
     */
    DEBUG(DB_VM, "\x1b[31m\n[vmfault]EFAULT 0x%x\n\x1b[0m",faultaddress);
    return EFAULT;
  }

  /* Assert that the address space has been set up properly. */
  assert(as->as_vbase1 != 0);
  assert(as->as_pbase1 != 0);
  assert(as->as_npages1 != 0);
  assert(as->as_vbase2 != 0);
  assert(as->as_pbase2 != 0);
  assert(as->as_npages2 != 0);
  assert(as->as_stackpbase != 0);
  assert((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
  assert((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
  assert((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
  assert((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
  assert((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);

  vbase1 = as->as_vbase1;
  vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
  vbase2 = as->as_vbase2;
  vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
  stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
  stacktop = USERSTACK;

  if (faultaddress >= vbase1 && faultaddress < vtop1) {
    paddr = (faultaddress - vbase1) + as->as_pbase1;
  } else if (faultaddress >= vbase2 && faultaddress < vtop2) {
    paddr = (faultaddress - vbase2) + as->as_pbase2;
  } else if (faultaddress >= stackbase && faultaddress < stacktop) {
    paddr = (faultaddress - stackbase) + as->as_stackpbase;
  } else {
    splx(spl);
    DEBUG(DB_VM, "\x1b[31m\n[vmfault]EFAULT 0x%x\n\x1b[0m",faultaddress);

    return EFAULT;
  }

  /* make sure it's page-aligned */
  assert((paddr & PAGE_FRAME) == paddr);

  for (i = 0; i < NUM_TLB; i++) {
    TLB_Read(&ehi, &elo, i);
    if (elo & TLBLO_VALID) {
      continue;
    }
    ehi = faultaddress;
    elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
    DEBUG(DB_VM, "\x1b[32mvm: 0x%x -> 0x%x [%d]\x1b[0m\n", faultaddress, paddr,i);
    TLB_Write(ehi, elo, i);
    splx(spl);
    return 0;
  }

  rp("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
  splx(spl);
  return EFAULT;
}

#include "new_addrspace.c"

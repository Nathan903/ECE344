void rp(const char *text) {
    kprintf("\x1b[31m%s\x1b[0m", text);
}
void gp(const char *text) {
    kprintf("\x1b[32m%s\x1b[0m", text);
}
void sr() {
    kprintf("\x1b[32m");
}

void er() {
    kprintf("\x1b[0m\n");
}
#include <types.h>
#include <machine/tlb.h>
#include <queue.h>
// struct queue* dirtyqueue;

typedef struct{
  char state;
  // pid_t pid;
  struct addrspace* as;
  char npages;

  // addr = firstfree + i * 4096
} coremap_entry;

#define FREE_STATE 0
#define FIXED_STATE 1
#define DIRTY_STATE 2
#define CLEAN_STATE 3
#define I_TO_ADDR(i) ((i) * PAGE_SIZE+firstpaddr)
#define rPADDR_TO_KVADDR(paddr) ((paddr)-MIPS_KSEG0)
#define ADDR_I(paddr) (((paddr)-firstpaddr)/PAGE_SIZE)
#define KADDR_I(paddr) (((paddr)-firstpaddr-MIPS_KSEG0)/PAGE_SIZE)
#define COREMAP_SIZE 77//77
//308k/4k
coremap_entry coremap[COREMAP_SIZE];
struct lock cmlock;
#define SWAPSIZE 1280
#define SMARTVM_STACKPAGES 100
paddr_t unevict(u_int32_t swapii, paddr_t* as);
int evict();
paddr_t getppages_vm(unsigned long npages, struct addrspace* as,char state);
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <uio.h>
#include <vfs.h>
#include <elf.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vnode.h>
void loadseg(struct vnode *v, vaddr_t ph_p_vaddr_plus_cc, Elf_Phdr ph);

void tlbclear(){
  int i, spl;
  spl = splhigh();
  u_int32_t ehi, elo;
  unsigned int p =  (unsigned int)(curthread->pid)<<6;
  

  for (i = 0; i < NUM_TLB; i++) {
    TLB_Read(&ehi, &elo, i);
    if ( ((ehi& 0xFFF)>>6)!=(p & 0xFFF) || (((ehi& 0xFFF)>>6)!=0)) {
        TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
    }
  }
  splx(spl);
}
u_int32_t  swapi=1;


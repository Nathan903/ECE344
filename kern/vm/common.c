void rp(const char *text) {
    kprintf("\x1b[31m%s\x1b[0m", text);
}
void gp(const char *text) {
    kprintf("\x1b[32m%s\x1b[0m", text);
}
#include <types.h>
#include <machine/tlb.h>
#include <queue.h>
// struct queue* dirtyqueue;
void tlbclear(){
  int i, spl;
  spl = splhigh();
  for (i = 0; i < NUM_TLB; i++) {
    TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
  }
  splx(spl);
}


typedef struct{
  char state;
  // pid_t pid;
  struct addrspace* as;
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
#define COREMAP_SIZE 70 //77
//308k/4k
coremap_entry coremap[COREMAP_SIZE];
struct lock cmlock;
#define SWAPSIZE 1280
paddr_t unevict(u_int32_t swapii);
int evict();
paddr_t getppages_vm(unsigned long npages, struct addrspace* as,char state);



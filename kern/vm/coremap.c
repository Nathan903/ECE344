void rp(const char *text) {
    kprintf("\x1b[31m%s\x1b[0m", text);
}
#include <types.h>

void printBinary(unsigned int num) {
    if (num > 1) printBinary(num >> 1); 
    kprintf("%d", num & 1); 
}
#include <machine/tlb.h>
void print_TLB_entries() {
    u_int32_t entryhi, entrylo;
    int i;

    kprintf("TLB Entries:\n");
    for (i = 0; i < NUM_TLB; i++) {
        TLB_Read(&entryhi, &entrylo, i);
        kprintf("  %d\t", i);printBinary(entryhi);kprintf(" ");printBinary(entrylo);
        kprintf(" %x %x\n", entryhi, entrylo);
    }
}


#include <synch.h>
struct spinlock {
    volatile char held;
};

void spinlock_acquire(struct spinlock *lock) {
  assert(lock != NULL);
  int spl = splhigh();
  while(1){
    int old = lock->held;
    lock->held = 1;
    if(old==0){break;}
  }
  splx(spl);
}

void spinlock_release(struct spinlock *lock) {
    assert(lock != NULL);
    assert(lock->held); // Ensure lock is locked before releasing
    lock->held= 0;
}


#define printf kprintf
typedef struct{
  char state;
  pid_t pid;
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
#define COREMAP_SIZE 77
//308k/4k

coremap_entry coremap[COREMAP_SIZE];
struct lock cmlock;

void print_core_map() {
    kprintf("Coremap Entries:\n");
    kprintf("----------------\n");
    kprintf("| Index | State | PID   |\n");
    kprintf("------------------------\n");
    unsigned int i;for (i=0; i < COREMAP_SIZE; ++i) {
        kprintf("| %-6d| %-6d| %-6d| %d %u\n", i, coremap[i].state, coremap[i].pid, I_TO_ADDR(i), PADDR_TO_KVADDR(I_TO_ADDR(i)));
    }
    kprintf("------------------------\n");
}
int print_core_map2(int nargs, char **args){
  print_TLB_entries();
  print_core_map();
  if( 0 && (nargs==69 || args==NULL )){rp("f");}return 0;
}
int free_page_size() {
    int free_size = 0;
    unsigned int i; for (i = 0; i < COREMAP_SIZE; ++i) {
        if (coremap[i].state == FREE_STATE) {
            free_size += 1;
        }
    }
    return free_size;
}

int get_pages(int npages, pid_t pid) {
    char state = (pid==0)? FIXED_STATE : DIRTY_STATE;
     int start = 0;
     int count = 0;
     int i;int j;

    for (i = 0; i < COREMAP_SIZE; ++i) {
        if (coremap[i].state == FREE_STATE) {
            if (count == 0) {
                start = i; // start of a potential free block
            }
            count++;
            if (count == npages) {
                for (j = start; j < start + npages; ++j) {
                    coremap[j].state = state; 
                    coremap[j].pid = pid; 
                }

                return start; 
            }
        } else {
            count = 0;
        }
    }
    rp("\n\n OUT OF MEMORY \n\n");
    assert( 1+1==11415); // no contiguous block of free pages
    return -1;
}
#undef printf







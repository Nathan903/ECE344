
void printBinary(unsigned int num) {
    if (num > 1) printBinary(num >> 1); 
    kprintf("%d", num & 1); 
}
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



void print_core_map() {
    kprintf("Coremap Entries:\n");
    kprintf("----------------\n");
    kprintf("| Index | State | as   |    size |\n");
    kprintf("------------------------\n");
    unsigned int i;for (i=0; i < COREMAP_SIZE; ++i) {
        kprintf("| %-6d| %-6d| %p|%x | %d  |%u\n", i, coremap[i].state, coremap[i].as, coremap[i].npages, I_TO_ADDR(i), PADDR_TO_KVADDR(I_TO_ADDR(i)));
    }
    kprintf("------------------------\n");
}
int print_core_map2(int nargs, char **args){
    // make_swap();
    // print_TLB_entries();
    print_core_map();

    // int i = evict();
    // kprintf("%d\n",i);
    // print_core_map();
    // paddr_t paddr; 
    // struct addrspace* as = coremap[i].as;
    // unsigned int j=0; for(j=0; j<PT_LENGTH;j++){
    //     kprintf("%u\n",as->pagetable[j].pa);
    //     if(as->pagetable[j].pa>0 && as->pagetable[j].pa<10){
    //       paddr= as->pagetable[j].pa;
    //     }
    // }
    // paddr = unevict(paddr);


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

int get_pages(int npages, struct addrspace* as, char state) {
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
                    
                    coremap[j].as = as; 
                    // if(state==DIRTY_STATE){
                    // q_addtail(dirtyqueue, (void*) i);                        
                    // }
                }
                coremap[start].npages = npages; 
                    
                return start;
            }
        } else {
            count = 0;
        }
    }
    rp("OM");
    evict();
    return get_pages(npages, as, state);

    return -1;
}
#undef printf







struct addrspace* as_create(void) {
  struct addrspace* as = kmalloc(sizeof(struct addrspace));
  if (as == NULL) { return NULL; }
  //////////////////////START//////////////////////////
  as->as_vbase1 = 0;
  as->as_pbase1 = 0;
  as->as_npages1 = 0;
  as->as_vbase2 = 0;
  // as->as_pbase2 = 0;
  unsigned int i=0; for(i=0; i<PT_LENGTH;i++){
    as->pagetable[i].va=0; 
    as->pagetable[i].pa=0; 
  }
  as->as_npages2 = 0;
  // as->as_stackpbase = 0;
  //////////////////////END//////////////////////////

  return as;
}
int as_copy(struct addrspace *old, struct addrspace **ret) {
  struct addrspace * new = as_create();
  if (new == NULL) { return ENOMEM; }
  //////////////////////START//////////////////////////

  new->as_vbase1 = old->as_vbase1;
  new->as_npages1 = old->as_npages1;
  new->as_vbase2 = old->as_vbase2;
  new->as_npages2 = old->as_npages2;

  if (as_prepare_load(new)) {
    as_destroy(new);
    return ENOMEM;
  }

  assert(new->as_pbase1 != 0);
  // assert(new->as_pbase2 != 0);
  // assert(new->as_stackpbase != 0);

  memmove((void *)PADDR_TO_KVADDR(new->as_pbase1),
          (const void *)PADDR_TO_KVADDR(old->as_pbase1),
          old->as_npages1 * PAGE_SIZE);

  // memmove((void *)PADDR_TO_KVADDR(new->as_pbase2),
  //         (const void *)PADDR_TO_KVADDR(old->as_pbase2),
  //         old->as_npages2 * PAGE_SIZE);
  //##untested##
  unsigned int i=0; for(i=0; i<PT_LENGTH;i++){
    if(old->pagetable[i].va!=0){
      assert(new->pagetable[i].va==0);
      new->pagetable[i].va =old->pagetable[i].va;
      new->pagetable[i].pa =getppages_vm(1, new, DIRTY_STATE);
      
      memmove((void *)PADDR_TO_KVADDR(new->pagetable[i].pa),
            (const void *)PADDR_TO_KVADDR(old->pagetable[i].pa),
            1 * PAGE_SIZE);
    }
  }

  // memmove((void *)PADDR_TO_KVADDR(new->as_stackpbase),
  //         (const void *)PADDR_TO_KVADDR(old->as_stackpbase),
  //         DUMBVM_STACKPAGES * PAGE_SIZE);
  //////////////////////END//////////////////////////
  *ret = new; return 0;
}

void as_destroy(struct addrspace *as) { 
  //////////////////////START//////////////////////////
  // rp("DESTORY");
  lock_acquire(&cmlock);
  unsigned int i; for (i = 0; i < COREMAP_SIZE; ++i) {
      if (coremap[i].as == curthread->t_vmspace) {
          coremap[i].state=FREE_STATE;
      }
  }
  // if(as->v!=NULL) vfs_close(as->v);
  lock_release(&cmlock);
  //kprintf("\n%d\n", sizeof(struct addrspace));
  //////////////////////END//////////////////////////
  kfree(as); 
  //struct addrspace* as2 = kmalloc(sizeof(struct addrspace));
  //(void) as2;   
}

void as_activate(struct addrspace *as) {
  (void)as;
  //////////////////////START//////////////////////////
  tlbclear();
  //////////////////////END//////////////////////////

}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz, int readable, int writeable, int executable) {
  //////////////////////START//////////////////////////
  rp("as_define_region"); kprintf("%x %u r%d w%d e%d\n", vaddr, sz,readable,writeable,executable);
  size_t npages;

  /* Align the region. First, the base... */
  sz += vaddr & ~(vaddr_t)PAGE_FRAME;
  vaddr &= PAGE_FRAME;

  /* ...and now the length. */
  sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

  npages = sz / PAGE_SIZE;

  /* We don't use these - all pages are read-write */
  (void)readable;
  (void)writeable;
  (void)executable;

  if (as->as_vbase1 == 0) {
    as->as_vbase1 = vaddr;
    as->as_npages1 = npages;
    as->heap_start = vaddr+ PAGE_SIZE*npages;
    as->heap_end = vaddr+ PAGE_SIZE*npages;
    
    return 0;
  }

  if (as->as_vbase2 == 0) {
    as->as_vbase2 = vaddr;
    as->as_npages2 = npages;
    return 0;
  }

  /*
   * Support for more than two regions is not available.
   */
  kprintf("dumbvm: Warning: too many regions\n");
  //////////////////////END//////////////////////////

  return EUNIMP;
}

int as_prepare_load(struct addrspace *as) {
  //////////////////////START//////////////////////////
  //gp("load\n");
  assert(as->as_pbase1 == 0);
  // assert(as->as_pbase2 == 0);
  // assert(as->as_stackpbase == 0);
  assert(as->as_npages1<20);
  as->as_pbase1 = getppages_vm(as->as_npages1, as, FIXED_STATE);
  if (as->as_pbase1 == 0) {
    return ENOMEM;
  }

  // as->as_pbase2 = getppages(as->as_npages2);
  // if (as->as_pbase2 == 0) {
  //   return ENOMEM;
  // }

  // as->as_stackpbase = getppages(DUMBVM_STACKPAGES);
  // if (as->as_stackpbase == 0) {
  //   return ENOMEM;
  // }
  //////////////////////END//////////////////////////
  //gp("doneload\n");
  
  return 0;
}

int as_complete_load(struct addrspace *as) {
  //////////////////////START//////////////////////////
  (void)as;
  //////////////////////END//////////////////////////
  return 0;
}

int as_define_stack(struct addrspace *as, vaddr_t *stackptr) {
  //////////////////////START//////////////////////////
  // assert(as->as_stackpbase != 0);
  //////////////////////END//////////////////////////
  (void) as;
  *stackptr = USERSTACK; /* Initial user-level stack pointer */
  return 0;
}

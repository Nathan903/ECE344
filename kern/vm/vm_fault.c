
int vm_fault(int faulttype, vaddr_t faultaddress) {
  vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
  paddr_t paddr;
  int i;
  u_int32_t ehi, elo;
  struct addrspace *as;
  int spl;

  spl = splhigh();

  faultaddress &= PAGE_FRAME; //..fff000

  //DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

  switch (faulttype) {
  case VM_FAULT_READONLY: panic("dumbvm: got VM_FAULT_READONLY\n"); /* We always create pages read-write, so we can't get this */
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
    //No address space set up. This is probably a kernel fault early in boot. Return EFAULT so as to panic instead of getting into an infinite faulting loop.
    DEBUG(DB_VM, "\x1b[31m\n[vmfault]EFAULT 0x%x\n\x1b[0m",faultaddress);
    return EFAULT;
  }
  /* Assert that the address space has been set up properly. */
  assert(as->as_vbase1 != 0);
  assert(as->as_pbase1 != 0);
  assert(as->as_npages1 != 0);
  assert(as->as_vbase2 != 0);
  // assert(as->as_pbase2 != 0);
  assert(as->as_npages2 != 0);
  // assert(as->as_stackpbase != 0);
  assert((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
  assert((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
  assert((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
  // assert((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
  // assert((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);



  vbase1 = as->as_vbase1;
  vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
  vbase2 = as->as_vbase2;
  vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
  stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
  stacktop = USERSTACK;
  //kprintf("0x%x 0x%x 0x%x 0x%x  0x%x 0x%x\n", vbase1, vtop1, vbase2, vtop2, stackbase, stacktop);
  if (faultaddress >= vbase1 && faultaddress < vtop1) {
    paddr = (faultaddress - vbase1) + as->as_pbase1;
  // } else if ( faultaddress >= stackbase && faultaddress < stacktop) {
  //   paddr = (faultaddress - stackbase) + as->as_stackpbase;
  } else if ( (faultaddress >= vbase2 && faultaddress < vtop2) || ( faultaddress >= stackbase && faultaddress < stacktop) || (faultaddress >= as->heap_start && faultaddress < as->heap_end) ) {
    unsigned int i=0; for(i=0; i<PT_LENGTH;i++){
      if(as->pagetable[i].va==faultaddress){break;}
    }
    if(i<PT_LENGTH){// found
      paddr = as->pagetable[i].pa;
      if(paddr>0 && paddr<SWAPSIZE){
        paddr = unevict(paddr);
        as->pagetable[i].pa=paddr;
        // rp("1\n");
      }
    } else{
      // kprintf("[vm] getting page ");
          paddr = getppages(1);
          unsigned int j=0; for(j=0; j<PT_LENGTH;j++){
            if(as->pagetable[j].va==0){
              as->pagetable[j].va=faultaddress;
              as->pagetable[j].pa=paddr;
              break;
            } if(j==PT_LENGTH){ rp("vmfault PT full\n");assert(1==2);}
          }
      if((faultaddress >= vbase2 && faultaddress < vtop2)){
        //kprintf("%x %x %x %x",vbase2, vtop2, as->heap_start, as->heap_end);gp("loadseg\n");
        			//result = load_segment(v, ph.p_offset, ph.p_vaddr, ph.p_memsz, ph.p_filesz, ph.p_flags & PF_X);
        
        
        
        loadseg(as->v, faultaddress,as->ph);
        
        
        
        //gp("loadsegd\n");
      }
    }
    // paddr = (faultaddress - vbase2) + as->as_pbase2;
  } else if (( faultaddress >= stackbase-PAGE_SIZE && faultaddress < stacktop)){
    rp("infiniterecursion...");
    splx(spl); return EFAULT;
  } else {
    splx(spl);
    rp("\x1b[31m\n[vmfault]EFAULT\x1b[0m");kprintf("0x%x\n",faultaddress);
    return EFAULT;
  }
  // rp("2\n");
  /* make sure it's page-aligned */
  assert((paddr & PAGE_FRAME) == paddr);
  // kprintf(" [vm] done\n");
  unsigned int p =  (unsigned int) (curthread->pid)<<6;
  assert((p & 0xFFFFFFC0) == p);
  faultaddress |=(p & 0xFFF); // add pid to 6 bits
  
  i = TLB_Probe(faultaddress,0);

  #define MAX(a, b) ((a) > (b) ? (a) : (b))
  for (i = MAX(0,i); i < NUM_TLB; i++) {
    TLB_Read(&ehi, &elo, i);
    if (elo & TLBLO_VALID && ehi!=faultaddress) {
      continue;
    }
    ehi = faultaddress;
    elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
    DEBUG(DB_VM, "\x1b[32mvm: %d 0x%x -> 0x%x [%d][%d]\x1b[0m\n", faulttype,faultaddress, paddr,i, free_page_size() );
    TLB_Write(ehi, elo, i);
    splx(spl);
    return 0;
  }
  kprintf(" [vm] not\n");

  rp("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
  splx(spl);
  return EFAULT;
}


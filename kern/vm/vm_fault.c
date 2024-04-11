
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
  //kprintf("0x%x 0x%x 0x%x 0x%x  0x%x 0x%x\n", vbase1, vtop1, vbase2, vtop2, stackbase, stacktop);
  if (faultaddress >= vbase1 && faultaddress < vtop1) {
    paddr = (faultaddress - vbase1) + as->as_pbase1;
  } else if (faultaddress >= vbase2 && faultaddress < vtop2) {
    paddr = (faultaddress - vbase2) + as->as_pbase2;
  } else if (faultaddress >= stackbase && faultaddress < stacktop) {
    paddr = (faultaddress - stackbase) + as->as_stackpbase;
  } else {
    splx(spl);
    //DEBUG(DB_VM, "\x1b[31m\n[vmfault]EFAULT 0x%x\n\x1b[0m",faultaddress);

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
    //DEBUG(DB_VM, "\x1b[32mvm: 0x%x -> 0x%x [%d]\x1b[0m\n", faultaddress, paddr,i);
    TLB_Write(ehi, elo, i);
    splx(spl);
    return 0;
  }

  rp("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
  splx(spl);
  return EFAULT;
}


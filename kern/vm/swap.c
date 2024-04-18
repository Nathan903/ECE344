#include "kern/unistd.h"
#include "vnode.h"
#include "kern/stat.h"
#include "kern/stat.h"
#include "uio.h"
struct vnode* swapvnode=NULL;

char bitmap[SWAPSIZE]; //1280
struct lock slock;
char swap_setupped=0;

void make_swap(){ swap_setupped=1;
	char p[]="lhd0raw:";
	if(vfs_open(p, O_RDWR, &swapvnode)){
		kprintf("file not created\n\n");
		assert(1==2);
	}
	// struct stat s;
	// VOP_STAT(swapvnode, &s);
	// kprintf("%d\n",s.st_size/4096);
	

	// #define SIZE 8
    // unsigned int buf=3;
    // unsigned int i; for(i=0;i<100;i++){
	//     struct uio u;
	// 	// mk_kuio(struct uio *uio, void *kbuf, size_t len, off_t pos, enum uio_rw rw)
	//     mk_kuio(&u,&buf,SIZE,SIZE*i,UIO_WRITE);
	//     VOP_WRITE(swapvnode, &u);
	// } 
	// for(i=0;i<100;i++){
	//     struct uio u;
	// 	// mk_kuio(struct uio *uio, void *kbuf, size_t len, off_t pos, enum uio_rw rw)
	//     mk_kuio(&u,&buf,SIZE,SIZE*i,UIO_READ);
	//     VOP_READ(swapvnode, &u);
	//     kprintf("%x ",buf);
	// } rp("\n");
}

void swrite(u_int32_t i, void* buf){
	lock_acquire(&slock);
    assert(swapvnode!=NULL);
    struct uio u;
	// mk_kuio(struct uio *uio, void *kbuf, size_t len, off_t pos, enum uio_rw rw)
    mk_kuio(&u,buf,(size_t)PAGE_SIZE,(off_t)PAGE_SIZE*i,UIO_WRITE);
	lock_release(&slock);
    if(VOP_WRITE(swapvnode, &u)){rp("swrite");kprintf("%d\n",i);assert(0); }

}
void sread(u_int32_t  i, void* buf){
	assert(swapvnode!=NULL);
	lock_acquire(&slock);
	//rp("\nsreada\n");
    struct uio u;
    mk_kuio(&u,buf,(size_t)PAGE_SIZE,(off_t)PAGE_SIZE*i,UIO_READ);
    //rp("sreadb\n");
    if(VOP_READ(swapvnode, &u)){rp("sread");kprintf("%d\n",i);assert(0);} 
	lock_release(&slock);
}
int ivictim=4;// <77

int evict(){ // make space, return swapi
rp("e");
	if(!swap_setupped){
		make_swap();
	}
	
	// rp("evict");kprintf("%d\n",free_page_size());
	if(swapi==SWAPSIZE){swapi=1; gp("OOOF\n");}
	assert(swapi<SWAPSIZE);
  	
  	
  	
  	/*////////////////////////////////
  	gp("EVICT\n");
  	unsigned int i;unsigned int ii;for (i=0; i < COREMAP_SIZE; ++i) {

        if(coremap[i].state==2){
        sr();kprintf("| %-6d| %-6d| %p| %x  |%x", i, coremap[i].state, coremap[i].as, I_TO_ADDR(i), PADDR_TO_KVADDR(I_TO_ADDR(i)));er();
        } else{
                kprintf("| %-6d| %-6d| %p| %x  |%x\n", i, coremap[i].state, coremap[i].as, I_TO_ADDR(i), PADDR_TO_KVADDR(I_TO_ADDR(i)));
        }
    }
    struct addrspace * as[2] = { (struct addrspace *)0x80037f00,(struct addrspace *)0x80037e00};
    for(i=0;i<2;i++){
        kprintf("PT%p\n", as[i]);
	    for(ii=0; ii<PT_LENGTH;ii++){
	    if (as[i]->pagetable[ii].va!=0 || 0!=as[i]->pagetable[ii].pa)
            kprintf("||%u|%x %x\n",ii,as[i]->pagetable[ii].va,as[i]->pagetable[ii].pa);
        }
    }
    /*//////////////////////////////////
  	
  	
  	ivictim=(ivictim+7)%COREMAP_SIZE;
  	int ja = 0;
	while(coremap[ivictim].state!=DIRTY_STATE){
	  	ivictim=(ivictim+3)%COREMAP_SIZE;
	  	ja++;
	  	if(ja==20000) print_core_map();
	}
	rp("e");
	//gp("victim is");kprintf("%d \n",ivictim);
	coremap_entry victim = coremap[ivictim];

	#define pt victim.as->pagetable
	unsigned int j=0; for(j=0; j<PT_LENGTH;j++){
		// kprintf("%u %u\n",pt[j].pa,I_TO_ADDR(ivictim));
		if(pt[j].pa==I_TO_ADDR(ivictim)){
		  pt[j].pa=swapi;
		  break;
		}
	} if(j==PT_LENGTH){ 
		rp(" PT not found\n");print_core_map();
			for(j=0; j<PT_LENGTH;j++){
			      if(pt[j].pa!=0)
					kprintf("%u %u\n",pt[j].pa,I_TO_ADDR(ivictim));	
			}
			
			
			assert(1==2);
	}
	coremap[ivictim].state = FREE_STATE;
	coremap[ivictim].as = NULL;
	tlbclear();
	swrite(swapi,(void*)PADDR_TO_KVADDR(I_TO_ADDR(ivictim)));
	swapi++;
	
	/*/////////////////////////////
	for (i=5; i < COREMAP_SIZE; ++i) {

        if(coremap[i].state==2){
        sr();kprintf("| %-6d| %-6d| %p| %x  |%x", i, coremap[i].state, coremap[i].as, I_TO_ADDR(i), PADDR_TO_KVADDR(I_TO_ADDR(i)));er();
        }else{
        kprintf("| %-6d| %-6d| %p| %x  |%x\n", i, coremap[i].state, coremap[i].as, I_TO_ADDR(i), PADDR_TO_KVADDR(I_TO_ADDR(i)));}
    }
    gp("evicted");kprintf("%d %x\n",ivictim,pt[j].va);
	
	*///////////////////////////////
	rp("a");
	return ivictim;
}

paddr_t unevict(u_int32_t swapii, paddr_t * pp){
    rp("u");


    //rp("UNEVICT");kprintf("%d\n",swapii);
	
	// find empty page
    //rp("hereswap\n");
          
	paddr_t pa = getppages(1);
    (*pp)=pa;
    //rp("UNEVICTA");
	sread(swapii,(void*)PADDR_TO_KVADDR(pa));
	// rp("3\n");
	//rp("inside done");

    rp("v");
	return pa;
}

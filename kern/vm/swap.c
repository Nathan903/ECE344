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
    struct uio u;
    mk_kuio(&u,buf,(size_t)PAGE_SIZE,(off_t)PAGE_SIZE*i,UIO_READ);
    if(VOP_READ(swapvnode, &u)){rp("sread");kprintf("%d\n",i);assert(0);} 
	lock_release(&slock);
}
int ivictim=4;// <77

int evict(){ // make space, return swapi
	if(!swap_setupped){
		make_swap();
	}
	// rp("evict");kprintf("%d\n",free_page_size());
	if(swapi==SWAPSIZE){swapi=1; gp("OOOF\n");}
	assert(swapi<SWAPSIZE);
  	ivictim=(ivictim+3)%COREMAP_SIZE;

	while(coremap[ivictim].state!=DIRTY_STATE){
	  	ivictim=(ivictim+1)%COREMAP_SIZE;
	}
	coremap_entry victim = coremap[ivictim];
	coremap[ivictim].state = FREE_STATE;
	#define pt victim.as->pagetable
	unsigned int j=0; for(j=0; j<PT_LENGTH;j++){
		// kprintf("%u %u\n",pt[j].pa,I_TO_ADDR(ivictim));
		if(pt[j].pa==I_TO_ADDR(ivictim)){
		  pt[j].pa=swapi;
		  break;
		}
	} if(j==PT_LENGTH){ 
		rp("PT not found\n");print_core_map();
			for(j=0; j<PT_LENGTH;j++){
					kprintf("%u %u\n",pt[j].pa,I_TO_ADDR(ivictim));	}
					assert(1==2);
	}
	tlbclear();
	swrite(swapi,(void*)PADDR_TO_KVADDR(I_TO_ADDR(ivictim)));
	swapi++;
	return ivictim;
}

paddr_t unevict(u_int32_t swapii){
	// find empty page
	paddr_t pa = getppages(1);
	// rp("unevict\n");
	sread(swapii,(void*)PADDR_TO_KVADDR(pa));
	// rp("3\n");
	return pa;
}

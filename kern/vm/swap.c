#include "kern/unistd.h"
#include "vnode.h"
#include "kern/stat.h"
#include "kern/stat.h"
#include "uio.h"
struct vnode* swapvnode=NULL;
char bitmap[700]; //1280
struct lock slock;
void make_swap(){
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

void swrite(int i, void* buf){
    assert(swapvnode!=NULL);
    struct uio u;
	// mk_kuio(struct uio *uio, void *kbuf, size_t len, off_t pos, enum uio_rw rw)
    mk_kuio(&u,&buf,(size_t)PAGE_SIZE,(off_t)PAGE_SIZE*i,UIO_WRITE);
    if(VOP_WRITE(swapvnode, &u)){rp("swrite");}
}
void sread(int i, void* buf){
	assert(swapvnode!=NULL);
    struct uio u;
    mk_kuio(&u,&buf,(size_t)PAGE_SIZE,(off_t)PAGE_SIZE*i,UIO_READ);
    if(VOP_READ(swapvnode, &u)){rp("sread");} 
}

void read_swap(){
	lock_acquire(&slock);
	lock_release(&slock);
}
// void write_swap(){

// }

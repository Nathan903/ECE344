/*
 * Test code for kmalloc.
 */
#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <test.h>
/*
 * Test kmalloc; allocate ITEMSIZE bytes NTRIES times, freeing
 * somewhat later.
 *
 * The total of ITEMSIZE * NTRIES is intended to exceed the size of
 * available memory.
 *
 * mallocstress does the same thing, but from NTHREADS different
 * threads at once.
 */

#define NTRIES   1200
#define ITEMSIZE  997
#define NTHREADS  8

static
void
mallocthread(void *sm, unsigned long num)
{
	struct semaphore *sem = sm;
	void *ptr;
	void *oldptr=NULL;
	void *oldptr2=NULL;
	int i;

	for (i=0; i<NTRIES; i++) {
		ptr = kmalloc(ITEMSIZE);
		if (ptr==NULL) {
			if (sem) {
				kprintf("thread %lu: kmalloc returned NULL\n",
					num);
				V(sem);
				return;
			}
			kprintf("kmalloc returned null; test failed.\n");
			return;
		}
		if (oldptr2) {
			kfree(oldptr2);
		}
		oldptr2 = oldptr;
		oldptr = ptr;
	}
	if (oldptr2) {
		kfree(oldptr2);
	}
	if (oldptr) {
		kfree(oldptr);
	}
	if (sem) {
		V(sem);
	}
}

int
malloctest(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	kprintf("Starting kmalloc test...\n");
	mallocthread(NULL, 0);
	kprintf("kmalloc test done\n");

	return 0;
}

int
mallocstress(int nargs, char **args)
{
	struct semaphore *sem;
	int i, result;

	(void)nargs;
	(void)args;

	sem = sem_create("mallocstress", 0);
	if (sem == NULL) {
		panic("mallocstress: sem_create failed\n");
	}

	kprintf("Starting kmalloc stress test...\n");

	for (i=0; i<NTHREADS; i++) {
		result = thread_fork("mallocstress", sem, i, mallocthread,
				     NULL);
		if (result) {
			panic("mallocstress: thread_fork failed: %s\n",
			      strerror(result));
		}
	}

	for (i=0; i<NTHREADS; i++) {
		P(sem);
	}

	sem_destroy(sem);
	kprintf("kmalloc stress test done\n");

	return 0;
}


////////////////////////////////////////////////////////////
// km3 https://github.com/ops-class/os161/blob/6edf0d19dc9b0b7c1f16c8b4be61a1eaeaecdcde/kern/test/kmalloctest.c#L478

/*
 * Larger kmalloc test. Or at least, potentially larger. The size is
 * an argument.
 *
 * The argument specifies the number of objects to allocate; the size
 * of each allocation rotates through sizes[]. (FUTURE: should there
 * be a mode that allocates random sizes?) In order to hold the
 * pointers returned by kmalloc we first allocate a two-level radix
 * tree whose lower tier is made up of blocks of size PAGE_SIZE/4.
 * (This is so they all go to the subpage allocator rather than being
 * whole-page allocations.)
 *
 * Since PAGE_SIZE is commonly 4096, each of these blocks holds 1024
 * pointers (on a 32-bit machine) or 512 (on a 64-bit machine) and so
 * we can store considerably more pointers than we have memory for
 * before the upper tier becomes a whole page or otherwise gets
 * uncomfortably large.
 *
 * Having set this up, the test just allocates and then frees all the
 * pointers in order, setting and checking the contents.
 */
#include <kern/errno.h>
#include <vm.h> /* for PAGE_SIZE */

int
kmalloctest3(int nargs, char **args)
{
#define NUM_KM3_SIZES 5
	static const unsigned sizes[NUM_KM3_SIZES] = { 32, 41, 109, 86, 9 };
	unsigned numptrs;
	size_t ptrspace;
	size_t blocksize;
	unsigned numptrblocks;
	void ***ptrblocks;
	unsigned curblock, curpos, cursizeindex, cursize;
	size_t totalsize;
	unsigned i, j;
	unsigned char *ptr;

	if (nargs != 2) {
		kprintf("kmalloctest3: usage: km3 numobjects\n");
		return EINVAL;
	}

	/* Figure out how many pointers we'll get and the space they need. */
	numptrs = atoi(args[1]);
	ptrspace = numptrs * sizeof(void *);

	/* Figure out how many blocks in the lower tier. */
	blocksize = PAGE_SIZE / 4;
	numptrblocks = DIVROUNDUP(ptrspace, blocksize);

	kprintf("kmalloctest3: %u objects, %u pointer blocks\n",
		numptrs, numptrblocks);

	/* Allocate the upper tier. */
	ptrblocks = kmalloc(numptrblocks * sizeof(ptrblocks[0]));
	if (ptrblocks == NULL) {
		panic("kmalloctest3: failed on pointer block array\n");
	}
	/* Allocate the lower tier. */
	for (i=0; i<numptrblocks; i++) {
		ptrblocks[i] = kmalloc(blocksize);
		if (ptrblocks[i] == NULL) {
			panic("kmalloctest3: failed on pointer block %u\n", i);
		}
	}

	/* Allocate the objects. */
	curblock = 0;
	curpos = 0;
	cursizeindex = 0;
	totalsize = 0;
	for (i=0; i<numptrs; i++) {
		cursize = sizes[cursizeindex];
		ptr = kmalloc(cursize);
		if (ptr == NULL) {
			kprintf("kmalloctest3: failed on object %u size %u\n",
				i, cursize);
			kprintf("kmalloctest3: pos %u in pointer block %u\n",
				curpos, curblock);
			kprintf("kmalloctest3: total so far %zu\n", totalsize);
			panic("kmalloctest3: failed.\n");
		}
		/* Fill the object with its number. */
		for (j=0; j<cursize; j++) {
			ptr[j] = (unsigned char) i;
		}
		/* Move to the next slot in the tree. */
		ptrblocks[curblock][curpos] = ptr;
		curpos++;
		if (curpos >= blocksize / sizeof(void *)) {
			curblock++;
			curpos = 0;
		}
		/* Update the running total, and rotate the size. */
		totalsize += cursize;
		cursizeindex = (cursizeindex + 1) % NUM_KM3_SIZES;
	}

	kprintf("kmalloctest3: %zu bytes allocated\n", totalsize);

	/* Free the objects. */
	curblock = 0;
	curpos = 0;
	cursizeindex = 0;
	for (i=0; i<numptrs; i++) {

		cursize = sizes[cursizeindex];
		ptr = ptrblocks[curblock][curpos];
		assert(ptr != NULL);
		for (j=0; j<cursize; j++) {
			if (ptr[j] == (unsigned char) i) {
				continue;
			}
			kprintf("kmalloctest3: failed on object %u size %u\n",
				i, cursize);
			kprintf("kmalloctest3: pos %u in pointer block %u\n",
				curpos, curblock);
			kprintf("kmalloctest3: at object offset %u\n", j);
			kprintf("kmalloctest3: expected 0x%x, found 0x%x\n",
				ptr[j], (unsigned char) i);
			panic("kmalloctest3: failed.\n");
		}
		kfree(ptr);
		curpos++;
		if (curpos >= blocksize / sizeof(void *)) {
			curblock++;
			curpos = 0;
		}
		assert(totalsize > 0);
		totalsize -= cursize;
		cursizeindex = (cursizeindex + 1) % NUM_KM3_SIZES;
	}
	assert(totalsize == 0);

	/* Free the lower tier. */
	for (i=0; i<numptrblocks; i++) {
		assert(ptrblocks[i] != NULL);
		kfree(ptrblocks[i]);
	}
	/* Free the upper tier. */
	kfree(ptrblocks);

	kprintf("KM3 SUCESS\n");

	return 0;
}


/*
 *我的malloc使用了显示空闲链表+首次适配，并对realloc进行了一定的优化，优化方案是：
 *若当前bp所指的块的下一个块是空闲块，并且二者相加后的大小满足asize，则不进行malloc，
 *直接合并后代替。
 *得分是87分。
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "5130379059",
    /* First member's full name */
    "JinJianian",
    /* First member's email address */
    "jinjianian03703@sjtu.edu.cn",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WSIZE 4 /* word size (bytes) */
#define DSIZE 8 /* double word size (bytes) */
#define CHUNKSIZE (1<<13) /* Extend heap by this amount (bytes) */
#define OVERHEAD 8
#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREV(p) ((p)?(void*)GET(PREV(p)):NULL)
#define GET_NEXT(p) ((p)?(void*)GET(NEXT(p)):NULL)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((void *)(bp) - WSIZE)
#define FTRP(bp) ((void *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define PREV(bp) ((void *)(bp))
#define NEXT(bp) ((void *)(bp) + WSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

int mm_init ();  
void *mm_malloc (size_t size);  
void mm_free (void *bp);  
void *mm_realloc (void *bp,size_t size); 

static void *coalesce (void *bp);  
static void *extend_heap (size_t size);  
static void place (void *ptr,size_t asize);  
static void delete_node (void *bp);  
static void add_node (void *bp);  
static void *find_fit (size_t asize);  

void * heap_listp = 0;
void * block_list = 0;

static void mm_check(){
	void *buf = heap_listp;
	while(buf)
	{
		printf("0x%x[%d/%d] ~ 0x%x[%d/%d]\n",HDRP(buf),GET_SIZE(HDRP(buf)),GET_ALLOC(HDRP(buf)),
				FTRP(buf),GET_SIZE(HDRP(buf)),GET_ALLOC(HDRP(buf)));
	}
	printf("\n");
}

static void *find_fit(size_t asize)
{
	void *bp = 0;
	void *blist = block_list;
        /* first fit search*/ 
        while(blist != 0){
 	    	if (!GET_ALLOC(HDRP(blist)) && (asize<=GET_SIZE(HDRP(blist)))) {
 				return blist;
 	   		}
			blist = GET_NEXT(blist);
        }
}
static void place(void *bp, size_t asize)
{
	size_t csize = GET_SIZE(HDRP(bp)) ;
 	delete_node(bp);
	void *temp = 0;
  	if ( (csize - asize) >= (DSIZE * 3) ) {
 		PUT(HDRP(bp), PACK(asize, 1)) ;
 		PUT(FTRP(bp), PACK(asize, 1)) ;
 		bp = NEXT_BLKP(bp) ;
 		PUT(HDRP(bp), PACK(csize-asize, 0)) ;
 		PUT(FTRP(bp), PACK(csize-asize, 0)) ;
		temp = coalesce(bp);
		add_node(temp);
 	} else {
 		PUT(HDRP(bp), PACK(csize, 1)) ;
 		PUT(FTRP(bp), PACK(csize, 1)) ;
 	}
}
static void *coalesce(void *bp)
{
	size_t prev_alloc;
	/*初始化时要对是否是表头进行特判*/
	if (bp != heap_listp)
		prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	else
		prev_alloc = 1;
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));
	void *next_block = NEXT_BLKP(bp);
	void *prev_block = PREV_BLKP(bp);
	if (prev_alloc && next_alloc) { /* Case 1 */
		return bp;
	}

	else if (prev_alloc && !next_alloc) { /* Case 2 */
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		delete_node(next_block);
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
		return bp;
	}
	else if (!prev_alloc && next_alloc) { /* Case 3 */
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		delete_node(prev_block);
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		return prev_block;
	}

	else { /* Case 4 */
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
		             GET_SIZE(FTRP(NEXT_BLKP(bp)));
		delete_node(next_block);
		delete_node(prev_block);
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		return prev_block;
	}
}

static void *extend_heap(size_t words)
{
	void *bp = 0;
	void *temp = 0;
	int size = 0;

	/* Allocate an even number of words to maintain alignment */
	size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
	if ((long)(bp = mem_sbrk(size))  == -1)
		return NULL;

	/* Initialize free block header/footer and the epilogue header */
	PUT(HDRP(bp), PACK(size, 0)); 		/* free block header */
	PUT(FTRP(bp), PACK(size, 0)); 		/* free block footer */
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   /* new epilogue header */

	/* Coalesce if the previous block was free */
	temp = coalesce(bp);
	add_node(temp);
	return temp;
}
int mm_init(void)
{
	/* create the initial empty heap */
	if ((heap_listp = mem_sbrk(6*WSIZE)) ==  (void *) -1)
		return -1;
	PUT(heap_listp, PACK(0, 1)); 		        /* alignment padding */
	PUT(heap_listp + (1*WSIZE), PACK(4*WSIZE, 0));  /* prologue header */
	PUT(heap_listp + (2*WSIZE), 0);
	PUT(heap_listp + (3*WSIZE), 0);
	PUT(heap_listp + (4*WSIZE), PACK(4*WSIZE, 0));  /* prologue footer */
	PUT(heap_listp + (5*WSIZE), PACK(0, 1));        /* epilogue header */
	heap_listp += (2*WSIZE);
	block_list = heap_listp;

	/* Extend the empty heap with a free block of CHUNKSIZE bytes */
	if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
		return -1;
	return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc (size_t size)
{
	size_t asize; /* adjusted block size */
	size_t extendsize; /* amount to extend heap if no fit */
	void *bp;

	/* Ignore spurious requests */
	if (size <= 0)
		return NULL;

	/* Adjust block size to include overhead and alignment reqs. */
	/*if (size <= DSIZE)
		asize = DSIZE * 2;
	else
		asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE); */
	asize = ((size + 8) <= 24)? 24:(ALIGN(size + 8));

	/* Search the free list for a fit */
	if ((bp = find_fit(asize)) != NULL) {
		place (bp, asize);
		return bp;
	}


	/* No fit found. Get more memory and place the block */
	extendsize = MAX (asize, CHUNKSIZE) ;
	if ((bp = extend_heap (extendsize/WSIZE)) == NULL)
		return NULL;
	place (bp, asize);
	return bp;
}

/*
 * mm_free - Freeing a block does nothing.after it works,add the new free block to the block_list table.
 */
void mm_free(void *bp)
{
	size_t size = GET_SIZE(HDRP(bp));

	void *temp = 0;
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	temp = coalesce(bp);
	add_node(temp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
	/*对realloc的优化方案是：若当前bp的后一个空闲块足够大，不进行malloc，直接合并这两块*/
	void *newptr = NEXT_BLKP(ptr);
	size_t copySize;

	if (size <= DSIZE)
		size = DSIZE + OVERHEAD;
	else
		size = DSIZE * ((size + (OVERHEAD) + (DSIZE-1)) / DSIZE);

	if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr)))) {
		if (GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(newptr)) > size + 2 * DSIZE) {
			int newSize = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr))) - size;
 			void *prev = GET_PREV(newptr);
			void *next = GET_NEXT(newptr);

			PUT(HDRP(ptr), PACK(size, 1));
			PUT(FTRP(ptr), PACK(size, 1));

			ptr = NEXT_BLKP(ptr);
			PUT(HDRP(ptr), PACK(newSize, 0));
			PUT(FTRP(ptr), PACK(newSize, 0));
			PUT(HDRP(ptr) + WSIZE, prev);
            PUT(HDRP(ptr) + DSIZE, next);

			if (prev) {
				PUT(HDRP(prev) + DSIZE, ptr);
			}
			else
				block_list = ptr;
			if (next) {
				PUT(HDRP(next) + WSIZE, ptr);
			}
			
			ptr = PREV_BLKP(ptr);
			return ptr;
		}	
    }  

    newptr = mm_malloc(size);

    if (newptr == NULL)
        return NULL;

    if (ptr == NULL)
        return newptr;

    if (size != 0) {
        copySize = GET_SIZE(HDRP(ptr));
        if (size < copySize)  copySize = size;
        memcpy(newptr, ptr, copySize);
    }
    else {
        newptr = 0;
    }

    mm_free(ptr);
   
    return newptr;
}
static void add_node(void *bp)
{	/*添加在表头，注释掉的部分为添加在表尾，不过调不对TAT*/
	/*if(block_list)
	{
		PUT(NEXT(block_list),bp);
	}
	PUT(PREV(bp),block_list);
	PUT(NEXT(bp),GET_NEXT(block_list));
	block_list = bp;
	return;*/
	if (!block_list) {
		PUT(PREV(bp), NULL);
		PUT(NEXT(bp), NULL);
		block_list = bp;
		return;
	}
	PUT(PREV(bp), NULL);
	PUT(NEXT(bp), block_list);
	PUT(PREV(block_list), bp);
	block_list = bp;
	return;
}
static void delete_node(void *bp)
{	/*采用的是一般的显式空闲链表*/
	/*if(PREV(bp))
	{
		PUT((char *)(HDRP(PREV(bp)) + DSIZE), NEXT(bp));
	}
	if(!NEXT(bp))
	{
		block_list = PREV(bp);
	}
	if(NEXT(bp)) PUT((char *)(HDRP(NEXT(bp)) + WSIZE),PREV(bp));
	return;*/
	void *p = GET_PREV(bp);
	void *n = GET_NEXT(bp);
	if (p) PUT(NEXT(p),n);
	if (n) PUT(PREV(n),p);
	if (block_list == bp)
		block_list = GET_NEXT(bp);
}















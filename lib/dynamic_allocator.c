/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk), is_free_block(blk)) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...
	 uint32* FBHeader = (uint32*)(daStart + sizeof(uint32));
	 uint32* FBFooter = (uint32*)(daStart + initSizeOfAllocatedSpace - 2 * sizeof(uint32));

	 *FBHeader = initSizeOfAllocatedSpace - 2 * sizeof(uint32);
	 *FBFooter = initSizeOfAllocatedSpace - 2 * sizeof(uint32);

	 struct BlockElement* Fblock = (struct BlockElement*)(daStart + 2*sizeof(uint32));

	 LIST_INIT(&freeBlocksList);
	 LIST_INSERT_HEAD(&freeBlocksList, Fblock);

	 uint32* BEG = (uint32*)daStart;
	 *BEG = 1;

	 uint32* END = (uint32*)(daStart + initSizeOfAllocatedSpace - sizeof(uint32));
	 *END = 1;
}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("set_block_data is not implemented yet");
	//Your Code is Here...
	uint32* header = (uint32*)va - 1;
	uint32* footer = (uint32*)(va + totalSize - 2*sizeof(uint32));
	uint32 flag;
	if(isAllocated == 1){
		flag = 1;
	}
	else if(isAllocated == 0){
		flag = 0;
	}
	*header = totalSize | flag;
	*footer = *header;
}


//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//==================================================================================
	//==================================================================================
		//DON'T CHANGE THESE LINES==========================================================
		//==================================================================================
		{
			if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
			if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
				size = DYN_ALLOC_MIN_BLOCK_SIZE ;
			if (!is_initialized)
			{
				uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
				uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
				uint32 da_break = (uint32)sbrk(0);
				initialize_dynamic_allocator(da_start, da_break - da_start);
			}
		}
		//==================================================================================
		//==================================================================================

		//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
		//COMMENT THE FOLLOWING LINE BEFORE START CODING
	    //	panic("alloc_block_FF is not implemented yet");
		//Your Code is Here...
		if(size== 0){
						return NULL ;
					}

		    uint32 header_size = sizeof(int);
		    uint32 footer_size = sizeof(int);
		    uint32 metadata_size = header_size + footer_size;
		    uint32 diff;
		    struct BlockElement *blk;
		    struct BlockElement *req= NULL;

		    LIST_FOREACH(blk, &freeBlocksList) {
		            uint32 block_size = get_block_size(blk);
		            if (is_free_block(blk) && block_size >= (size + metadata_size)) {
		                req = blk;
		                uint32 diff = block_size - (size + metadata_size);
		                if (diff >= sizeof(uint32)*4) {
		                    struct BlockElement *new_blk = (struct BlockElement *)((char*)req + size + metadata_size);
		                    set_block_data(req, size + metadata_size, 1);
		                    set_block_data(new_blk, diff, 0);
		                    LIST_INSERT_AFTER(&freeBlocksList, req, new_blk);
		                } else {

		                    set_block_data(req, block_size, 1);
		                }

		                LIST_REMOVE(&freeBlocksList, req);
		                return (void *)((char*)req );
		                break;
		            }
		        }




		    ////sbrrrkkkkkkkkkkk


		    uint32 pages_needed = ROUNDUP(size + metadata_size, PAGE_SIZE) / PAGE_SIZE;
		    struct BlockElement *new_block = (struct BlockElement *)sbrk(pages_needed);

		    if (new_block == (void *)-1) {
		    	//sbrk fail
		        return NULL;
		    }
		    //check prev block is freee
		  	    struct BlockElement *prev_blk = (struct BlockElement *)((char *)new_block - sizeof(uint32));
		  	    if (new_block!= NULL && is_free_block(prev_blk)) {

		  	        set_block_data(new_block, pages_needed * PAGE_SIZE, 1);

		  	      uint32* new_va = (uint32 *)((char *)new_block + sizeof(uint32) * 2);
		  	    	free_block(new_block);
		  	    } else {
//if prev not free
		  	        set_block_data(new_block, pages_needed * PAGE_SIZE, 0);
		  	        LIST_INSERT_TAIL(&freeBlocksList, new_block);
		  	    }

		  	    struct BlockElement *allocated_block = freeBlocksList.lh_last;
		  	    uint32 allocated_size = size + metadata_size;

		  	    if (get_block_size(freeBlocksList.lh_last) >= allocated_size + sizeof(uint32) * 4) {
		  	        struct BlockElement *new_free_block = (struct BlockElement *)((char *)allocated_block + allocated_size);
		  	        uint32 remaining_size = get_block_size(allocated_block) - allocated_size;

		  	        set_block_data(allocated_block, allocated_size, 1);
		  	        set_block_data(new_free_block, remaining_size, 0);

		  	        struct BlockElement *it;
		  	        //insert new free after splitting
		  	        LIST_FOREACH(it, &freeBlocksList) {
		  	            if (it > new_free_block) {
		  	                LIST_INSERT_BEFORE(&freeBlocksList, it, new_free_block);
		  	                break;
		  	            }
		  	        }
		  	        if (it == NULL) {
		  	            LIST_INSERT_TAIL(&freeBlocksList, new_free_block);
		  	        }

		  	        LIST_REMOVE(&freeBlocksList, allocated_block);
		  	    } else {
		  	        set_block_data(allocated_block, get_block_size(allocated_block), 1);
		  	    }

		  	    return (void *)((char *)allocated_block);
}
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size){
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE ;
		if (!is_initialized)
		{
			uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
			uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
			uint32 da_break = (uint32)sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...
	if(size== 0){
		return NULL ;
	}
	uint32 header_size = sizeof(int);
    uint32 footer_size = sizeof(int);
    uint32 metadata_size = header_size + footer_size;
    uint32 diff=0;
    struct BlockElement *blk;
    struct BlockElement *req= NULL;
    LIST_FOREACH(blk, &freeBlocksList){
	    uint32 block_size = get_block_size(blk);
        if (is_free_block(blk) && (block_size >= size + metadata_size)){
		     uint32 current_diff = block_size - (size + metadata_size);
		     if (req == NULL || current_diff < diff){
		          req = blk;
		          diff = current_diff;
	         }
		 }
    }
    if (req != NULL){
	     uint32 block_size = get_block_size(req);
		 diff = block_size - (size + metadata_size);
		 if (diff >= sizeof(uint32)*4){
		      struct BlockElement *new_blk = (struct BlockElement *)((char*)req + size + metadata_size);
		      set_block_data(req, size + metadata_size, 1);
		      set_block_data(new_blk, diff, 0);
		      struct BlockElement *it;
		      LIST_FOREACH(it, &freeBlocksList){
		          if (it > new_blk) {
		               LIST_INSERT_BEFORE(&freeBlocksList, it, new_blk);
		               break;
		          }
		      }
		      if (it == NULL){
		           LIST_INSERT_TAIL(&freeBlocksList, new_blk);
		      }
		  }
		  else{
		      set_block_data(req, block_size, 1);
		  }
		  LIST_REMOVE(&freeBlocksList, req);
		  return (void *)((char*)req);
    }
    void *new_block = sbrk(size + metadata_size);
    if (new_block == (void *)-1) {
		 return NULL;
	}
	set_block_data(new_block, size + metadata_size, 1);
	return (void *)((char*)new_block);
}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_block is not implemented yet");
	//Your Code is Here...
	if(va != NULL){
		int8 status = is_free_block(va);
        //if(status != 1){//allocated
    	    uint32 blockSize = get_block_size(va);
            int dd = 0;
    	    struct BlockElement* currBlk = (struct BlockElement*) va;
    	    struct BlockElement* last = (struct BlockElement*) (va + blockSize);
    	    struct BlockElement* first = (struct BlockElement*) (va - sizeof(uint32));
    	    if((get_block_size(last) == 0) && (is_free_block(last) == 0)){// end
    	    	struct BlockElement* prevBlk = (struct BlockElement*) (va - sizeof(uint32));
    	    	int8 prevState = is_free_block(prevBlk);
    	    	if(prevState == 1){// prev only
    	    		uint32 prevSize = get_block_size(prevBlk);
    	    		uint32 *prevHeader = (uint32*) (va - sizeof(uint32) - prevSize);
    	    		uint32 *prevFooter = (uint32*) (va + blockSize - 2*sizeof(uint32));
    	            *prevHeader = (prevSize + blockSize);
    	    	    *prevFooter = *prevHeader;
    	    	    dd = 1;
    	    	}
    	    }
    	    else if((get_block_size(first) == 0) && (is_free_block(first) == 0)){// begin
    	    	struct BlockElement* nextBlk = (struct BlockElement*) (va + blockSize);
    	    	int8 nextState = is_free_block(nextBlk);
    	    	if(nextState == 1){// next only
    	    		uint32 nextSize = get_block_size(nextBlk);
    	    		uint32 *nextHeader = (uint32*) (va - sizeof(uint32));
    	    		uint32 *nextFooter = (uint32*) (va + blockSize + nextSize - 2*sizeof(uint32));
    	    		*nextHeader = (blockSize + nextSize);
    	    		*nextFooter = (blockSize + nextSize);
    	    		LIST_REMOVE(&freeBlocksList, nextBlk);
    	    	    struct BlockElement* insertionPoint = (struct BlockElement*) ((char*)nextHeader + sizeof(uint32));
    	    	    struct BlockElement* it;
    	    	    LIST_FOREACH(it, &freeBlocksList){
    	    	        if (it > insertionPoint){
    	    		         LIST_INSERT_BEFORE(&freeBlocksList, it, insertionPoint);
    	    		         break;
    	    	        }
    	    	    }
    	            if (it == NULL){
    	                 LIST_INSERT_TAIL(&freeBlocksList, insertionPoint);
    	    	    }
    	    		dd = 1;
    	    	}
    	    }
    	    else{//prev and next
    	    	struct BlockElement* prevBlk = (struct BlockElement*) (va - sizeof(uint32));
    	    	struct BlockElement* nextBlk = (struct BlockElement*) (va + blockSize);
    	    	int8 prevState = is_free_block(prevBlk);
    	    	int8 nextState = is_free_block(nextBlk);
    	    	if(prevState == 1 && nextState == 0){//with prev
    	    		uint32 prevSize = get_block_size(prevBlk);
    	    		uint32 *prevHeader = (uint32*) (va - sizeof(uint32) - prevSize);
    	    		uint32 *prevFooter = (uint32*) (va + blockSize - 2*sizeof(uint32));
    	            *prevHeader = (prevSize + blockSize);
    	            *prevFooter = (prevSize + blockSize);
    	            dd = 1;
    	    	}
    	    	else if(prevState == 0 && nextState == 1){//with next
    	    		uint32 nextSize = get_block_size(nextBlk);
    	    		uint32 *nextFooter = (uint32*) (va + blockSize + nextSize - 2*sizeof(uint32));
    	    		uint32 *nextHeader = (uint32*) (va - sizeof(uint32));
    	    		*nextFooter = (blockSize + nextSize);
    	    		*nextHeader = (blockSize + nextSize);
    	    		LIST_REMOVE(&freeBlocksList, nextBlk);
    	    	    struct BlockElement* insertionPoint = (struct BlockElement*) ((char*)nextHeader + sizeof(uint32));
    	    	    struct BlockElement* it;
    	    	    LIST_FOREACH(it, &freeBlocksList){
    	    	        if (it > insertionPoint){
    	    		         LIST_INSERT_BEFORE(&freeBlocksList, it, insertionPoint);
    	    		         break;
    	    	        }
    	    	    }
    	            if (it == NULL){
    	                 LIST_INSERT_TAIL(&freeBlocksList, insertionPoint);
    	    	    }
    	    		dd = 1;
    	    	}
    	    	else if(prevState == 1 && nextState == 1){//prev and next
    	    		uint32 prevSize = get_block_size(prevBlk);
    	    		uint32 nextSize = get_block_size(nextBlk);
    	    		uint32 *prevHeader = (uint32*) (va - sizeof(uint32) - prevSize);
    	    		uint32 *prevFooter = (uint32*) (va + blockSize + nextSize - 2*sizeof(uint32));
    	    		*prevHeader = (blockSize + prevSize + nextSize);
    	    		*prevFooter = (blockSize + prevSize + nextSize);
                    LIST_REMOVE(&freeBlocksList, nextBlk);
    	    		dd = 1;
    	    	}

    	    }
    	    if (dd == 0){// No merge
    	         set_block_data(va, blockSize, 0);
    	         struct BlockElement* it;
    	         LIST_FOREACH(it, &freeBlocksList){
    	             if (it > currBlk){
    	                  LIST_INSERT_BEFORE(&freeBlocksList, it, currBlk);
    	                  return;
    	             }
    	         }
    	         LIST_INSERT_TAIL(&freeBlocksList, currBlk);
    	    }
        //}
	}
}

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
    //panic("realloc_block_FF is not implemented yet");
	//Your Code is Here...
	if(va == NULL && new_size!=0){
	    return alloc_block_FF(new_size);
	}
	if(new_size == 0 && va==NULL){
	    return NULL;
	}
	if(new_size== 0 && va!= NULL){
	    free_block(va);
	    return NULL;
	}
	uint32 current_size= get_block_size(va);
	if(new_size>current_size){
		struct BlockElement * next =(struct BlockElement *)(va+current_size);
		uint32 next_size = get_block_size(next);
		uint32 next_blk = is_free_block(next);
		if(next_blk == 1){
			uint32 diff = (new_size + 2*sizeof(uint32)) - current_size;
			uint32 size = 0;
			if(diff <= next_size){
				uint32 remain = next_size - diff;
				if(remain >= (4*sizeof(uint32))){
			        uint32 *Footer = (uint32*) (va + current_size - 2*sizeof(uint32)+ diff);
				    uint32 *Header = (uint32*) (va - sizeof(uint32));
				    uint32 *next_header = (uint32*) (va + current_size + diff - sizeof(uint32));
				    uint32 *next_footer = (uint32*) (va + current_size + next_size - 2*sizeof(uint32));
				    *Footer = (current_size + diff) | 1;
				    *Header = (current_size + diff) | 1;
				    return va;
				    *next_header = remain;
				    *next_footer = remain;
				    LIST_REMOVE(&freeBlocksList, next);
				    struct BlockElement* insertionPoint = (struct BlockElement*) ((char*)next_header + sizeof(uint32));
				    struct BlockElement* it;
				    LIST_FOREACH(it, &freeBlocksList){
				    	if (it > insertionPoint){
				    		LIST_INSERT_BEFORE(&freeBlocksList, it, insertionPoint);
				    		break;
				    	}
				    }
				    if (it == NULL){
				    	LIST_INSERT_TAIL(&freeBlocksList, insertionPoint);
				    }
			    }
				else{
					uint32 *Footer = (uint32*) (va + current_size - 2*sizeof(uint32)+ diff);
				    uint32 *Header = (uint32*) (va - sizeof(uint32));
				    *Footer = (current_size + diff) | 1;
				    *Header = (current_size + diff) | 1;
				    return va;
				}

			}
			else{
				struct BlockElement* it = (struct BlockElement*) ((char*)next + sizeof(uint32));
			    LIST_FOREACH(it, &freeBlocksList){
			    	uint32 free = is_free_block(it);
			    	size += get_block_size(it);
		    	    if(free == 1 && (diff <= size)){
		    	    	uint32 *Headerr = (uint32*) (va - sizeof(uint32));
						uint32 *Footerr = (uint32*) (va + current_size - 2*sizeof(uint32)+ diff);
		    	   		LIST_REMOVE(&freeBlocksList, next);
		    			*Headerr = (current_size + diff) | 1;
		    			*Footerr = (current_size + diff) | 1;
		    			return va;
		    			break;
		    	    }
			    }
			}
		}
		else{
			void *va = alloc_block_FF(new_size);
			return va;
		}
	}
	if((new_size + 2*sizeof(uint32))< current_size){
	        struct BlockElement *next =(struct BlockElement *)(va+current_size);
	        uint32 next_blk = is_free_block(next);
	        if(new_size != 0 && ((new_size + 2*sizeof(uint32)) < (4*sizeof(uint32)))){
	        	new_size = (4*sizeof(uint32));
	        }
	        if(next_blk == 0){
	            uint32 diff = current_size -(new_size + 2*sizeof(uint32));
	            if(diff >= 4*sizeof(uint32) ){
	                uint32* CFooter =(uint32*) (va + current_size -diff - 2*sizeof(uint32));
	                uint32* CHeader = (uint32*) (va - sizeof(uint32));
	                uint32* next_header = (uint32*)(va + current_size - diff - sizeof(uint32));
	                uint32* next_footer = (uint32*)(va + current_size - 2*sizeof(uint32));
	                *CFooter = (new_size + 2*sizeof(uint32)) | 1;
	                *CHeader = (new_size + 2*sizeof(uint32)) | 1;
	                *next_header = diff | 0;
	                *next_footer = diff | 0;
	                struct BlockElement* insertionPoint = (struct BlockElement*) ((char*)next_header + sizeof(uint32));
	                struct BlockElement* it;
	                LIST_FOREACH(it, &freeBlocksList){
	                    if (it > insertionPoint){
	                        LIST_INSERT_BEFORE(&freeBlocksList, it, insertionPoint);
	                        //return LIST_PREV(it);
	                        break;
	                    }
	                }
	                if (it == NULL){
	                    LIST_INSERT_TAIL(&freeBlocksList, insertionPoint);
	                }
	                return va;
	            }
	            else{// internal fragmentation
	            	return va;
	            }

	        }
	}
	return NULL;

 }
/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
uint32 count = 0;
struct store_data arr[1024];
uint32 va_map[1024*1024] = {0};
//uint32* kheap_start = NULL;
//uint32* kheap_break = NULL;
//uint32* kheap_limit = NULL;
//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
	// Write your code here, remove the panic and write your code
    // panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");

	    kheap_start = (uint32*)daStart;
	    kheap_break = kheap_start + initSizeToAllocate / sizeof(uint32);
	    kheap_limit = (uint32*)daLimit;
	    if (kheap_break > kheap_limit) {
	        panic("no enough memory or initial size exceed given limit !!");
	    }

	    for (uint32 *i = kheap_start; i < kheap_break; i = (uint32*)((char*)i + 4*kilo)) {
	        struct FrameInfo* new_frm;
	        allocate_frame(&new_frm);
	        ////for virtuaallllllllllllllllllllllll
			uint32 pa = to_physical_address(new_frm);
			uint32 aligned_pa = pa & ~(PAGE_SIZE - 1);
			uint32 index = aligned_pa / PAGE_SIZE;
			uint32 aligned_va = (uint32)i & ~(PAGE_SIZE - 1);
			va_map[index] = aligned_va;
	        map_frame(ptr_page_directory, new_frm, (uint32)i, PERM_WRITEABLE);
	    }

	    initialize_dynamic_allocator(daStart, initSizeToAllocate);

	    return 0;
}

void* sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
	 * 				you should allocate pages and map them into the kernel virtual address space,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//return (void*)-1 ;
	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
    // panic("sbrk() is not implemented yet...!!");
	uint32* old_brk = kheap_break;

	    if (numOfPages == 0) {
	        return kheap_break;
	   }
	    else if (numOfPages > 0) {
	        uint32* new_break = (uint32*)((char*)kheap_break + numOfPages *(4* kilo));

	        if (new_break > kheap_limit) {
	            return (void*)-1;
	        }
	        for (uint32* i = kheap_break; i < new_break; i = (uint32*)((char*)i + 4*kilo)) {
	            struct FrameInfo* new_frm;
	            int ret = allocate_frame(&new_frm);
	            if (ret == E_NO_MEM) {
	                return (void*)-1;
	            }
	            ////for virtuaallllllllllllllllllllllll
				uint32 pa = to_physical_address(new_frm);
				uint32 aligned_pa = pa & ~(PAGE_SIZE - 1);
				uint32 index = aligned_pa / PAGE_SIZE;
				uint32 aligned_va = (uint32)i & ~(PAGE_SIZE - 1);
				va_map[index] = aligned_va;
	            map_frame(ptr_page_directory, new_frm, (uint32)i, PERM_WRITEABLE);
	        }
	        kheap_break = new_break;
	        uint32* END = (uint32*)((char*)kheap_break - sizeof(uint32));
	        		*END = 1;
	    }
	    return old_brk;
}


//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
    uint32* heap_pointer = (uint32*)((char*)kheap_limit + PAGE_SIZE);
    uint32* ptr = NULL;
	if (!kheap_start || !kheap_limit || heap_pointer == NULL) {
	      panic("Kernel heap not initialized!");
	}
	uint32 available_heap_space = (uint32) ((uint32*)KERNEL_HEAP_MAX - (uint32*)heap_pointer);
	uint32 required_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32 ret_ptr;
	if (size >= available_heap_space) {
	     return NULL;  // Not enough space
	}
	if (size == 0) {
	     return NULL;
	}
    if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
	     struct BlockElement *ret = alloc_block(size, DA_FF);
	     if (ret != NULL) {
	          return ret; // Return address from block allocator
	     } else {
	          return NULL; // Allocation failed
	     }
	} else if (size > DYN_ALLOC_MAX_BLOCK_SIZE) {
         uint32 flag = 0;
	     for(uint32* va = heap_pointer; va < (uint32*)(KERNEL_HEAP_MAX); va = (uint32*)((char*)va + PAGE_SIZE)){
	          uint32 *ptr_page_table = NULL;
	    	  struct FrameInfo* frame = get_frame_info(ptr_page_directory, (uint32)va, &ptr_page_table);
	    	  if (frame == 0) { // Check if the page is not mapped
	    	       flag = flag + 1;
	    	       if(flag == required_pages){
	    	           ptr = (uint32*)((char*)va-((required_pages-1)*PAGE_SIZE));
	    	           break;
	    	       }
	    	  }
	    	  else{
	    	     flag = 0;
	    	  }


	     }
	     for (uint32 i = 0; i < required_pages; i++) {
	    	   if ((uint32)ptr + i * PAGE_SIZE >= KERNEL_HEAP_MAX) {
	    		     return NULL; // Exceeds memory boundary
	    	   }
	    	   struct FrameInfo* new_frm;
	    	   int result = allocate_frame(&new_frm);
	    	   if (result == E_NO_MEM) {
	    	        return NULL;
	    	   }
	    	   ////for virtuaaaaallllllllll
	    	   uint32 pa = to_physical_address(new_frm);
	    	        uint32 aligned_pa = pa & ~(PAGE_SIZE - 1);
	    	        uint32 index = aligned_pa / PAGE_SIZE;

	    	        if (index >= (1024*1024)) {
	    	            panic("Physical address out of bounds!");
	    	        }



	    	   uint32 address = (uint32)((uint32*)((char*)ptr + i * PAGE_SIZE));
	    	   uint32 aligned_va = address & ~(PAGE_SIZE - 1);
	    	   int res = map_frame(ptr_page_directory, new_frm, address, PERM_WRITEABLE);
	    	   if (res == E_NO_MEM) {
	    		    return NULL;
	           }
	    	   va_map[index] = aligned_va;
	      }
	      if(count < 1024){
	     	  arr[count].va = (void*)ptr;
	     	  arr[count].numOfPages = required_pages;
	     	  count++;
	      }
	}

	return (void*)ptr ;

}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	uint32 num = 0;

    if(virtual_address >= (void*)kheap_start && virtual_address <= (void*)kheap_limit){
        free_block(virtual_address);
    }
    else if(virtual_address >= (void*)((char*)kheap_limit + 4*kilo) && virtual_address <= (void*)KERNEL_HEAP_MAX){
    	uint32 *ptr_page_table = NULL;
    	struct FrameInfo* frame = get_frame_info(ptr_page_directory, (uint32)virtual_address, &ptr_page_table);
    	if (frame == 0) { // Check if the page is not mapped
    	     panic("frame is free");
        }
    	for(uint32 i = 0; i < 1024; i++){
    		if(arr[i].va == virtual_address){
    			num = arr[i].numOfPages;
    			break;
    		}
    	}//tst kheap FF kphysaddr
    	for(uint32 i = 0; i < num; i++){
    		uint32 *ptr_page_table = NULL;
    		uint32 address = (uint32)((uint32*)((char*)virtual_address + i * PAGE_SIZE));
    		struct FrameInfo* ptr = get_frame_info(ptr_page_directory, address, &ptr_page_table);
    		if (ptr == NULL) {
    		                panic("Frame not found for the address");
    		}
    		///////foooor virtuaaaaaalllllll
    		   uint32 pa = to_physical_address(ptr);
    		            uint32 index = pa / PAGE_SIZE;
    		            if (index < (1024*1024)) {
    		                va_map[index] = 0;  // Clear the reverse mapping entry
    		            }

    		free_frame(ptr);
    		unmap_frame(ptr_page_directory, address);

    	}
    	for (uint32 i = 0; i < 1024; i++) {
    	      if (arr[i].va == virtual_address) {
    	           arr[i].va = NULL;
    	           arr[i].numOfPages = 0;
    	           break;
    	      }
    	}
    }
    else{
    	panic("invalid address");
    }
	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
    //[PROJECT'24.MS2] [KERNEL HEAP] kheap_physical_address
    // Write your code here, remove the panic and write your code
        uint32 *ptr_page_table;
        uint32 page_directory_entry;
        uint32 page_table_entry;
        int ret = get_page_table(ptr_page_directory, virtual_address, &ptr_page_table);
        if (ret != TABLE_IN_MEMORY || ptr_page_table == NULL) {

            return 0;
        }

        page_table_entry = ptr_page_table[PTX(virtual_address)];
        if ((page_table_entry & ~0xFFF) == 0) {

            return 0;
        }

        unsigned int physical_address = EXTRACT_ADDRESS(page_table_entry) | (virtual_address & 0xFFF);
        return physical_address;
    }

unsigned int kheap_virtual_address(unsigned int physical_address) {

    uint32 index = physical_address / PAGE_SIZE;

    // Check if the index is out of bounds
    if (index >= (1024*1024)) {
        return 0;  // Physical address out of range
    }

    uint32 base_va = va_map[index];

    // Check if there is a valid mapping for the physical address
    if (base_va == 0) {
        return 0;  // No mapping found for this physical address
    }

    uint32 offset = physical_address & (PAGE_SIZE - 1);  // Get the offset within the page
    return base_va + offset;

}



	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================



//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}

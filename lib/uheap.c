#include <inc/lib.h>
uint32 count = 0;
uint32 count2 = 0;
struct store arr[10000];
struct store arr2[10000];
//static uint32 set = 0;
//uint32 array[660000] = {0};
//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
//	if(set == 0){
//		set = 1;
//		sys_zero();
//	}
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	uint32* uheap_pointer = (uint32*)((char*)myEnv->uheap_limit + PAGE_SIZE);
	uint32 required_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32 available_heap_space = (uint32) ((uint32*)USER_HEAP_MAX - (uint32*)uheap_pointer);
	uint32* ptr = NULL;
	if (size >= available_heap_space) {
	     return NULL;  // Not enough space
	}
    if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
	     struct BlockElement *ret = alloc_block(size, DA_FF);
	     if (ret != NULL) {
	          return ret; // Return address from block allocator
	     } else {
	          return NULL; // Allocation failed
	     }
	}else if(size > DYN_ALLOC_MAX_BLOCK_SIZE) {
		 uint32 flag = 0;
		 uint32 j = 1;
		 for(uint32* va = uheap_pointer; va < (uint32*)(USER_HEAP_MAX); va = (uint32*)((char*)va + PAGE_SIZE)){
//			 uint32 isMarked = check_mem((uint32)va);
//			 if(isMarked == 0){// unmarked
//				 flag++;
//				 if(flag == required_pages){
//			         ptr = (uint32*)((char*)va-((required_pages-1)*PAGE_SIZE));
//			    	 break;
//				 }
//			 }else{
//				 flag = 0;
//			 }
			 //-----------------------------------------------------
			 uint32 perm = sys_check_permissions((uint32)va);
			 if((perm & PERM_AVAILABLE) != PERM_AVAILABLE || perm == -1){
				 flag++;
				 if(flag == required_pages){
					 ptr = (uint32*)((char*)va-((required_pages-1)*PAGE_SIZE));
					 break;
				 }
			 }else{
				 flag = 0;
			 }

		 }
         if(((uint32)ptr + required_pages*PAGE_SIZE) >= USER_HEAP_MAX){
        	 return NULL;
         }
	     if(count < 1024 ){
		     arr[count].va = (void*)ptr;
		     arr[count].numOfPages = required_pages;
		     count++;
	     }
		 sys_allocate_user_mem((uint32)ptr, required_pages);
		 //allocate_mem((uint32)ptr, required_pages);

	}
	return (void*)ptr;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{

    //TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
    //Write your code here, remove the panic and write your code
	uint32 num = 0;
	if(virtual_address >= (void*)myEnv->uheap_start && virtual_address <= (void*)myEnv->uheap_limit){
		free_block(virtual_address);
	}
	else if(virtual_address >= (void*)((char*)myEnv->uheap_limit + PAGE_SIZE) && virtual_address <= (void*)USER_HEAP_MAX){
		for(uint32 i = 0; i < 1024; i++){
			if(arr[i].va == virtual_address){
				//uint32 indx = (uint32)virtual_address/PAGE_SIZE;
				//array[indx] = 0;
				num = arr[i].numOfPages;
				break;
			}
		}

		sys_free_user_mem((uint32)virtual_address, num);

	}
	else{
		panic("invalid address");
	}
}

//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
//	if(set == 0){
//		set = 1;
//		sys_zero();
//	}
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0){
		return NULL ;
	}
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	uint32* uheap_pointer = (uint32*)((char*)myEnv->uheap_limit + PAGE_SIZE);
	uint32 required_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32 available_heap_space = (uint32) ((uint32*)USER_HEAP_MAX - (uint32*)uheap_pointer);
	uint32* ptr = NULL;
	if (size >= available_heap_space) {
		 return NULL;  // Not enough space
	}
	uint32 flag = 0;
    for(uint32* va = uheap_pointer; va < (uint32*)(USER_HEAP_MAX); va = (uint32*)((char*)va + PAGE_SIZE)){
//	    uint32 isMarked = check_mem((uint32)va);
//	    if(isMarked == 0){// unmarked
//		    flag++;
//		    if(flag == required_pages){
//			    ptr = (uint32*)((char*)va-((required_pages-1)*PAGE_SIZE));
//			    break;
//		    }
//	    }else{
//		    flag = 0;
//	    }
	    uint32 perm = sys_check_permissions((uint32)va);
		 if((perm & PERM_AVAILABLE) != PERM_AVAILABLE || perm == -1){
			 flag++;
			 if(flag == required_pages){
				 ptr = (uint32*)((char*)va-((required_pages-1)*PAGE_SIZE));
				 break;
			 }
		 }else{
			 flag = 0;
		 }

    }
    if(((uint32)ptr + required_pages*PAGE_SIZE) >= USER_HEAP_MAX){
	    return NULL;
    }
    int status = sys_createSharedObject(sharedVarName,size,isWritable,ptr);
    if(status == E_SHARED_MEM_EXISTS || status == E_NO_SHARE || status == E_NO_MEM){
    	return NULL;
    }
    sys_allocate_user_mem((uint32)ptr, required_pages);
    //allocate_mem((uint32)ptr, required_pages);
    if(count2 < 1024 ){
	     arr2[count2].va = (void*)ptr;
	     arr2[count2].numOfPages = required_pages;
	     arr2[count2].sharedObjID = status;
	     count2++;
    }
    return (void*)ptr;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
//	if(set == 0){
//		set = 1;
//		sys_zero();
//	}
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
    //	panic("sget() is not implemented yet...!!");
	uint32 size = sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	uint32* uheap_pointer = (uint32*)((char*)myEnv->uheap_limit + PAGE_SIZE);
	uint32 required_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32 available_heap_space = (uint32) ((uint32*)USER_HEAP_MAX - (uint32*)uheap_pointer);
	uint32* ptr = NULL;
	if (size >= available_heap_space) {
		 return NULL;
	}
	 uint32 flag = 0;
	 for(uint32* va = uheap_pointer; va < (uint32*)(USER_HEAP_MAX); va = (uint32*)((char*)va + PAGE_SIZE)){
//		 uint32 isMarked = check_mem((uint32)va);
//		 if(isMarked == 0){// unmarked
//			 flag++;
//			 if(flag == required_pages){
//				 ptr = (uint32*)((char*)va-((required_pages-1)*PAGE_SIZE));
//				 break;
//			 }
//		 }else{
//			 flag = 0;
//		 }
		 uint32 perm = sys_check_permissions((uint32)va);
		 if((perm & PERM_AVAILABLE) != PERM_AVAILABLE || perm == -1){
			 flag++;
			 if(flag == required_pages){
				 ptr = (uint32*)((char*)va-((required_pages-1)*PAGE_SIZE));
				 break;
			 }
		 }else{
			 flag = 0;
		 }

	 }
	 if(((uint32)ptr + required_pages*PAGE_SIZE) >= USER_HEAP_MAX){
		 return NULL;
	 }
	 sys_allocate_user_mem((uint32)ptr, required_pages);
	 //allocate_mem((uint32)ptr, required_pages);
	 int sss = sys_getSharedObject(ownerEnvID,sharedVarName,ptr);
	 if(count2 < 1024 ){
		 arr2[count2].va = (void*)ptr;
		 arr2[count2].numOfPages = required_pages;
		 arr2[count2].sharedObjID = sss;
		 count2++;
	 }
	 return (void*)(ptr);
}



//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void *virtual_address) {
    if (virtual_address == NULL) {
        return;
    }
    int32 objID = -1;
    for (uint32 i = 0; i < count2; i++) {
        if (arr2[i].va == virtual_address) {
            objID = arr2[i].sharedObjID;
            arr2[i].va = NULL;
            arr2[i].sharedObjID = 0;
            break;
        }
    }
    if (objID == -1) {
        panic("Invalid address");
    }
    int result = sys_freeSharedObject(objID, virtual_address);
    if (result != 0) {
        panic("Failed to free");
    }
}



//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}

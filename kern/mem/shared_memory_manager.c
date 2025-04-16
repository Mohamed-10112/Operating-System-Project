#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	//Your Code is Here...
    if(numOfFrames <= 0){
    	return NULL;
    }
	struct FrameInfo** framesStorage = (struct FrameInfo**) kmalloc(numOfFrames * sizeof(struct FrameInfo*));
    if(framesStorage == NULL){
    	return NULL;
    }
	for(int i=0; i<numOfFrames; i++){

		  framesStorage[i] = NULL;
	}
	return framesStorage;
}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...
    uint32 required_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	struct Share* newShare = (struct Share*) kmalloc(sizeof(struct Share));
	if(newShare == NULL){
		return NULL;
	}
	newShare->references = 1;
	newShare->ID = (uint32)newShare & 0x7FFFFFFF;
	newShare->isWritable = isWritable;
	newShare->ownerID = ownerID;
	newShare->size = size;
	//strcpy(newShare->name, shareName);
	strncpy(newShare->name, shareName, sizeof(newShare->name));
	newShare->framesStorage = create_frames_storage(required_pages);

	if(newShare->framesStorage == NULL){
		kfree(newShare);
		return NULL;
	}

	return newShare;


}

//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_share is not implemented yet");
	//Your Code is Here...
	struct Share* current_share = AllShares.shares_list.lh_first;

	acquire_spinlock(&AllShares.shareslock);

	 while(current_share!= NULL){
		 if (current_share->ownerID == ownerID && strcmp(current_share->name, name) == 0) {
			release_spinlock(&AllShares.shareslock);
			return current_share;
		}
		 current_share = current_share->prev_next_info.le_next;

	 }

	 release_spinlock(&AllShares.shareslock);
	 return NULL;

}

//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...
	struct Env* myenv = get_cpu_proc(); //The calling environment

//	uint32 permissions = PERM_USER | PERM_PRESENT;
//		if(isWritable){
//			permissions |= PERM_WRITEABLE;
//		}

		struct Share* existingID=NULL;
		existingID= get_share(ownerID, shareName);
				if (existingID != NULL) {

					return E_SHARED_MEM_EXISTS;
				}


				acquire_spinlock(&AllShares.shareslock);

		struct Share* newShare=create_share(ownerID,shareName,size,isWritable);
		if (newShare == NULL) {
		        return E_NO_SHARE; // Error: Failed to create the shared object
		    }



			uint32 required_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;


		    LIST_INSERT_HEAD(&AllShares.shares_list, newShare);
			 release_spinlock(&AllShares.shareslock);
			for(uint32 i=0;i<required_pages;i++){
				 uint32 va = (uint32)virtual_address + (i * PAGE_SIZE);

				    struct FrameInfo* frame = NULL;
				    int result = allocate_frame(&frame);
				    if (result != E_NO_MEM && frame != NULL) {
				        newShare->framesStorage[i] = frame;
				        map_frame(myenv->env_page_directory, frame, va, PERM_WRITEABLE | PERM_USER);
				    } else {
				        release_spinlock(&AllShares.shareslock);
				        return E_NO_MEM;
				    }

			}
			if(newShare==NULL){
				return E_NO_SHARE ;
			}


		    return newShare->ID;

}


//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("getSharedObject is not implemented yet");
	//Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment

	struct Share* sharedObj = get_share(ownerID, shareName);
	if (sharedObj == NULL) {
		return E_SHARED_MEM_NOT_EXISTS;
	}

	uint32 required_pages = (sharedObj->size + PAGE_SIZE - 1) / PAGE_SIZE;
	acquire_spinlock(&AllShares.shareslock);

	struct FrameInfo** framesStorage = sharedObj->framesStorage;
	for (uint32 i = 0; i < required_pages; i++) {
		uint32 va = (uint32)((char*)virtual_address + i * PAGE_SIZE);

	   int permo=PERM_PRESENT|PERM_USER;
		 if(sharedObj->isWritable){
		   permo|=PERM_WRITEABLE;
		  }
		 int result = map_frame(myenv->env_page_directory, framesStorage[i], va,permo);


	}

	sharedObj->references++;
	release_spinlock(&AllShares.shareslock);
	return sharedObj->ID;

}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share *ptrShare) {

    LIST_REMOVE(&AllShares.shares_list, ptrShare);
    uint32 numFrames = (ptrShare->size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32 i = 0; i < numFrames; i++) {
        if (ptrShare->framesStorage[i] != NULL) {
            free_frame(ptrShare->framesStorage[i]);
        }
    }

    kfree(ptrShare->framesStorage);
    kfree(ptrShare);
}


//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA) { //100

    struct Share *sharedObj = NULL;
    acquire_spinlock(&AllShares.shareslock);
    struct Share *current = AllShares.shares_list.lh_first;
    while (current != NULL) {
        if (current->ID == sharedObjectID) {
            sharedObj = current;
            break;
        }
        current = current->prev_next_info.le_next;
    }
    if (sharedObj == NULL) {
    	//cprintf("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww\n");
        release_spinlock(&AllShares.shareslock);
        return E_SHARED_MEM_NOT_EXISTS;
    }

    sharedObj->references--;

    if (sharedObj->references == 0) {
    	free_share(sharedObj);
    }

    release_spinlock(&AllShares.shareslock);

    struct Env *myenv = get_cpu_proc();
    uint32 numFrames = (sharedObj->size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32 i = 0; i < numFrames; i++) {
        uint32 va = (uint32)startVA + (i * PAGE_SIZE);
        unmap_frame(myenv->env_page_directory, (uint32)va);
        uint32 *page_table = NULL;
        get_page_table(myenv->env_page_directory, (uint32)va, &page_table);
        if (page_table != NULL && table_empty(page_table)) {
            uint32 pd_index = PDX(va);
            myenv->env_page_directory[pd_index] = 0;
            kfree(page_table);
        }
    }

    tlbflush();

    return 0;
}

int table_empty(uint32 *page_table) {
    for (int i = 0; i < 1024; i++) {
        if (page_table[i] & PERM_PRESENT) {
            return 0;
        }
    }
    return 1;
}

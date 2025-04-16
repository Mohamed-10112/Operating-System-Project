// User-level Semaphore

#include "inc/lib.h"


struct semaphore create_semaphore(char *semaphoreName, uint32 value)// smalloc
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
	if (semaphoreName == NULL) {
		panic("Semaphore name is NULL");
	}
	struct __semdata* data = smalloc(semaphoreName,sizeof(struct __semdata) , 1);

	if (data == NULL) {
		panic("Failed to allocate memory for myqueue");
	}
	struct semaphore semaphore_ptr;
	semaphore_ptr.semdata = data;
	//sys_init_queue((void*)data);
	if(&(data->queue) != NULL)
	{
		LIST_INIT(&(data->queue));
	}
	strncpy(semaphore_ptr.semdata->name, semaphoreName, sizeof(semaphore_ptr.semdata->name));
	//strcpy(semaphore_ptr.semdata->name, semaphoreName);
	//semaphore_ptr.semdata->name[sizeof(semaphore_ptr.semdata->name)] = '\0';
	semaphore_ptr.semdata->count= value;
	semaphore_ptr.semdata->lock = 0;
	//cprintf("Semaphore '%s': Value = %d, Address = %p\n",semaphore_ptr.semdata->name,
			//semaphore_ptr.semdata->count,
            //semaphore_ptr.semdata);

	//cprintf("created semaphore/n");
	//cprintf("smallocADD %x\n", semaphore_ptr);
	return semaphore_ptr;

}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	if (semaphoreName == NULL) {
		        panic("name is NULL");
		    }
	 struct __semdata* data = sget(ownerEnvID,semaphoreName);
	 if (data == NULL) {
	   	  panic("not found");
	 }
	 struct semaphore semaphore_data;
	 semaphore_data.semdata = data;
	 //cprintf("sgetADD %x\n",semaphore_data);
	 return semaphore_data;
}

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
	    struct __semdata* semdata = sem.semdata;
	    if (sem.semdata == NULL) {
	        panic("data is NULL");
	    }
	    while (xchg(&(semdata->lock),1)!=0);
	    semdata->count--;
	    if (semdata->count < 0) {
	    	semdata->lock=0;
	    	sys_wait_semaphore((uint32)&semdata->queue, (uint32)semdata);
	    	//cprintf("finished wait");
	    }else{
	    	semdata->lock=0;
	    }
	    //sem.semdata->lock=0;
}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	    struct __semdata* semdata = sem.semdata;
	    if (semdata == NULL) {
	        panic("data is NULL");
	    }
	    while (xchg(&(sem.semdata->lock),1)!=0);
	    //cprintf("entered signal");
	    semdata->count++;
	    if (semdata->count <= 0) {
	    	sem.semdata->lock=0;
	    	sys_signal_semaphore((uint32)&semdata->queue);
	    }else{
	    	sem.semdata->lock=0;
	    }
	    //sem.semdata->lock=0;
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}

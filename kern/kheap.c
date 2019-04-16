#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
int heapPages[40960]={};
uint32 next=KERNEL_HEAP_START;
int alloc=1;
void* kmalloc(unsigned int size)
{

	//TODO: [PROJECT 2018 - MS1 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//panic("kmalloc() is not implemented yet...!!");
	int cnt=0;
	int numOfFrames=size/PAGE_SIZE+(size%PAGE_SIZE!=0);
	bool ok=0,l=(next==KERNEL_HEAP_START);
	//check if there is a consecutive free pages in a kernel Heap
	for(uint32 i=next;1;i+=PAGE_SIZE){
		uint32 t=(i-KERNEL_HEAP_START)/PAGE_SIZE;

		if(heapPages[t]){
			next=i+PAGE_SIZE;cnt=0; continue;
		}
		cnt++;
		if(cnt==numOfFrames){
			ok=1; break;
		}
		if(i>=KERNEL_HEAP_MAX-PAGE_SIZE&&!l)
				next=KERNEL_HEAP_START,i=KERNEL_HEAP_START,cnt=0, l=1;
		else if(i>=KERNEL_HEAP_MAX-PAGE_SIZE&&l)break;

	}
	if(!ok)return NULL;void* ret=(void*)next;
	int t=(next-KERNEL_HEAP_START)/PAGE_SIZE;
	for(int i=0;i<numOfFrames;i++){
		struct Frame_Info* newFrame=NULL;
		allocate_frame(&newFrame);
		map_frame(ptr_page_directory,newFrame,(void*)next,PERM_PRESENT|PERM_WRITEABLE);
		heapPages[t++]=alloc; next+=PAGE_SIZE;
	}
	alloc++;
	//NOTE: Allocation is based on NEXT FIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details


	//TODO: [PROJECT 2018 - BONUS1] Implement the BEST FIT strategy for Kernel allocation
	// Beside the NEXT FIT
	// use "isKHeapPlacementStrategyBESTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
	//cprintf("size = %d\ncnt = %d\nret = %x",size,cnt,ret);
	return ret;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2018 - MS1 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	uint32 va=(uint32) virtual_address;
	uint32 t=(va-KERNEL_HEAP_START)/PAGE_SIZE;
	int u=heapPages[t];
	for(int i=t;(u&&heapPages[i]==u);i++)
	{
		uint32* ptrPageTable=NULL;
		struct Frame_Info* frame=get_frame_info(ptr_page_directory,(void*)va,
				&ptrPageTable);
		ptrPageTable[PTX(va)]=0;
		unmap_frame(ptr_page_directory,(void*)va);
		free_frame(frame);
		frame->references=0;
		va+=PAGE_SIZE;
		heapPages[i]=0;
	}
	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2018 - MS1 - [1] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");
	for(uint32 i=KERNEL_HEAP_START;i<KERNEL_HEAP_MAX;i+=PAGE_SIZE){
		uint32* ptrPageTable=NULL;
		get_page_table(ptr_page_directory,(void*)i,&ptrPageTable);
		if((ptrPageTable[PTX(i)]&0xFFFFF000)==physical_address)
			return i;

	}


	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer

	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2018 - MS1 - [1] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");
	uint32* ptrPageTable=NULL;
	struct Frame_Info* frame=get_frame_info(ptr_page_directory,(void*)virtual_address,&ptrPageTable);
	uint32 pa=ptrPageTable[PTX(virtual_address)]&0xFFFFF000;
	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer

	return pa;
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2018 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code
	int numOfPages=(new_size+PAGE_SIZE-1)/PAGE_SIZE;

	if((uint32)virtual_address>=KERNEL_HEAP_START){
		int t= ((uint32)virtual_address-KERNEL_HEAP_START+PAGE_SIZE-1)/PAGE_SIZE;
		int u=heapPages[t],c=0;
		if(u){
			for(int i=t;(heapPages[i]==u);i++)
				c++;t++;
			if(c>numOfPages){
				uint32 tmp=((t-numOfPages-1)*PAGE_SIZE+KERNEL_HEAP_START);
				uint32* ptr=(uint32*)tmp;
				kfree(ptr);
				return virtual_address;
			}
			else if(c<numOfPages){ uint32 va=t*PAGE_SIZE+KERNEL_HEAP_START;

			for(int i=0;i<numOfPages-c;i++){
					heapPages[t++]=u;
					struct Frame_Info* newFrame=NULL;
					allocate_frame(&newFrame);
					map_frame(ptr_page_directory,newFrame,(void*)va,PERM_PRESENT);
					va+=PAGE_SIZE;
				}
				return virtual_address;
			}
			else return virtual_address;
		}
		else
			{uint32* va=virtual_address;
			for(int i=0;i<numOfPages;i++){
				int t=((uint32)va-KERNEL_HEAP_START+PAGE_SIZE-1)/PAGE_SIZE;
				heapPages[t]=alloc;
				struct Frame_Info* newFrame=NULL;
				allocate_frame(&newFrame);
				map_frame(ptr_page_directory,newFrame,(void*)va,PERM_PRESENT|PERM_WRITEABLE);
				va+=PAGE_SIZE;
			}
			alloc++;
			return virtual_address;
			}
	}

	uint32* start=kmalloc(new_size);
	if(start==NULL)return NULL;
	void* ret=(void*)start;

	uint32* va =ROUNDDOWN(virtual_address,PAGE_SIZE);
	for(uint32 i=0;i<numOfPages;i++){
		uint32* pt=NULL;
		struct Frame_Info* frame=get_frame_info(ptr_page_directory,(void*)start,&pt);
		uint32 pa=to_physical_address(frame);
		pt[PTX(start)] = CONSTRUCT_ENTRY(pa , PERM_PRESENT|PERM_WRITEABLE);
	}
	return ret;
	//panic("krealloc() is not implemented yet...!!");


}

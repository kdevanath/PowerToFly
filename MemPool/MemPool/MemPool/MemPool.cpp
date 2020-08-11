// MemPool.cpp : Defines the entry point for the console application.
//

//************ Please read this ****************//
// expandFreeList method should take care of expanding automatically
// if there is not enough memory in the pool.
// if you look at the MemoryPool.h file you can uncomment 
// runner->next_ = NULL;
// After I wrote this I looked up boost to see if they have something like this.
// They do have it and it is better to use it.
// 
#include "stdafx.h"
#include <vector>
#include <functional>
#include <algorithm>

#include "MemPool.h"
#include "DemoMemPool.h"

using namespace scm_mempool;

MemoryPool<DemoPool>* DemoPool::demoPool_ = 0;
int MemoryPool<DemoPool>::num_ = 0;
int MemoryPool<DemoPool>::poolSize_ = 0;

int _tmain(int argc, char* argv[])
{
	typedef std::vector<std::string> CMDLINE_OPTIONS;
	CMDLINE_OPTIONS args(argv,argv+argc);
	CMDLINE_OPTIONS::iterator itr=args.begin();
	int totalPoolSize=40;
	int numOfGetMemCalls=40;
	int numOfPutMemCalls=40;
	for(itr; itr != args.end(); ++itr) {
		if(*itr == "--help" || *itr == "-h") {
			std::cout << 
				"argv[0] -t <total pool size> -g <get mem calls> -p <put mem calls>"
				<< std::endl;
			::exit(-1);
		}
		if(*itr == "-t") {
			++itr;
			std::cout << " next arg " << (*itr).c_str() << std::endl;
			totalPoolSize = atoi((*itr).c_str());
		} else if(*itr == "-g") {
			++itr;
			std::cout << " next arg getmem " << (*itr).c_str() << std::endl;
			numOfGetMemCalls = atoi((*itr).c_str());
		}else if(*itr == "-p") {
			++itr;
			std::cout << " next arg putmem" << (*itr).c_str() << std::endl;
			numOfPutMemCalls = atoi((*itr).c_str());
		}

		//std::cout << " args " << argc << " " << *itr << std::endl;
	}
	numOfPutMemCalls = numOfPutMemCalls > numOfGetMemCalls ? numOfGetMemCalls : numOfPutMemCalls;
	typedef std::vector<DemoPool*> DemoPoolVector;
	DemoPoolVector vecs;

	//Allocate with a default size 32 * sizeof(DemoPool)
	DemoPool::newPool(totalPoolSize);
	std::cout << " Total pool size " << MemoryPool<DemoPool>::size() << std::endl;

	//Allocating <numOfGetMemCalls> times
	for(int i = 0; i < numOfGetMemCalls;i++)
	{
		DemoPool* tmp = new DemoPool(i,100.0+i,"test"); // will call getMem
		vecs.push_back(tmp);
	}
	std::cout << " Number of getMem calls " 
		<< MemoryPool<DemoPool>::num()
		<< std::endl;

	std::cout << " Total left in the pool after calling getMem " 
		<< MemoryPool<DemoPool>::size() - MemoryPool<DemoPool>::num() 
		<< std::endl;
	//for_each(vecs.begin(), vecs.end(),std::mem_fun(&DemoPool::print));	
	
	std::cout << " Size to be released to the pool " << numOfPutMemCalls << std::endl;
	// Freeing <numOfPutMemCalls> calls
	for(int i = 0; i < numOfPutMemCalls;i++)
	{
		delete vecs[i];//Calls putMem
	}
	int currentSize = MemoryPool<DemoPool>::size() - MemoryPool<DemoPool>::num();
	std::cout << " Total size of the pool after releasing to the pool " 
		<< currentSize
		<< std::endl;
	//std::cout << " Clear" << std::endl;
	vecs.clear();

	
	std::cout << " Allocating from the pool " << currentSize << std::endl;
	for(int i = 0; i < currentSize;i++)
	{
		DemoPool* tmp = new DemoPool(i,100.0+i,"test2");
		vecs.push_back(tmp);
	}
	int totalLeft = MemoryPool<DemoPool>::size() - MemoryPool<DemoPool>::num();
	std::cout << " Total left in the pool after calling getMem " 
		<< totalLeft
		<< std::endl;

	// It will fail as there is no more memory in the pool
	std::cout << " Will try to get " << totalLeft +10 
		<<  " more than that " << totalLeft << " exists in the pool " 
		<< std::endl;
	for(int i = 0; i < totalLeft+10;i++)
	{
		DemoPool* tmp = new DemoPool(i,100.0+i,"test3");
		vecs.push_back(tmp);
	}
	//DemoPool::deletePool();
	return 0;
}


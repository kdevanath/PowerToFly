#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__

#include <stdlib.h>
#include <iostream>

namespace scm_mempool {

	template <class T> 
	class MemoryPool {

	public:
		MemoryPool(unsigned size = 32)
		{
			std::cout << " Allocating pool of size " << size << std::endl;
			expandFreeList(size);
		}
		~MemoryPool()
		{
			MemoryPool<T> *tmpNext = next_;
			for (tmpNext = next_; tmpNext != NULL; tmpNext = next_) {
				next_ = next_->next_;
				delete [] tmpNext;
			}
		}

		inline void* getMem(unsigned num){

			//std::cout << " Get mem " << num << std::endl;
			if (next_ == NULL) {
				expandFreeList(num);
			}

			MemoryPool<T> *head = next_;
			next_ = head->next_;
			num_++;

			return head;
		}

		inline void putMem(void* which) {
			//std::cout << " Calling putmem " << std::endl;
			MemoryPool<T> *head = static_cast <MemoryPool<T> *> (which);
			head->next_ = next_;
			next_ = head;
			num_--;
		}

		static int num() { return num_;}
		static int size() { return poolSize_;}
	private:
		MemoryPool<T> *next_;
		static int num_;
		static int poolSize_;
		void expandFreeList(unsigned int num = 32) {
			
			size_t size = (sizeof(T) > sizeof(MemoryPool<T> *)) ? sizeof(T) : sizeof(MemoryPool<T> *);
			MemoryPool<T> *runner = static_cast <MemoryPool<T> *> ((void*) new char[size]);
			next_ = runner;
			for (int i=0; i<(int)num; i++) {
				runner->next_ = static_cast <MemoryPool<T> *> ((void*) new char[size]);
				runner = runner->next_;
			}
			//If you uncooment this the expand Free list will work. 
			//runner->next_ = NULL;
			poolSize_ += num;
			std::cout  << " Total pool size is " << poolSize_ << std::endl;
		}
	};
}
#endif
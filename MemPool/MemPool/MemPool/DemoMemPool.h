#ifndef __DEMO_MEM_POOL_H__
#define __DEMO_MEM_POOL_H__

#include <string>
// A class that I can use to demonstrate one use of MemoryPool. 
// Redefine operator new and delete so that it uses 
// Memory pools getMem and putMem seperately.
namespace scm_mempool {

	class DemoPool
	{
	public:
		DemoPool() :
			size_(-1),
			price_(0.0),
			name_("test")
		  {
		  }
			DemoPool(int s, double p, const std::string& n) :
			  size_(s),
			  price_(p),
			  name_(n)
		  {
		  }

		int size() const { return size_;}
		const std::string& name() { return name_;}
		double price() const { return price_;}

		void* operator new(unsigned size) { return demoPool_->getMem(size);}
		void operator delete(void* which) { demoPool_->putMem(which);}
		static void newPool(unsigned size = 32) {demoPool_ = new MemoryPool<DemoPool>(size);}
		static void deletePool() { delete demoPool_;}

		void print()
		{
			std::cout << " size " << size_
				<< " price " << price_
				<< " name " << name_ 
				<< std::endl;
		}

	private:
		static MemoryPool<DemoPool>* demoPool_; // Every new will be taken from the pool
		int size_;
		double price_;
		std::string name_;
	};
}
#endif
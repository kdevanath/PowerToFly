#include <IPCQueue.H>
#include <stdlib.h>

namespace Yammer {

  // for exception safe synchronization
  struct Guard {  
    pthread_mutex_t *lock_;

    Guard(pthread_mutex_t *lock) : lock_(lock) { pthread_mutex_lock(lock_); }
    ~Guard() { pthread_mutex_unlock(lock_); }
  };

  const size_t sizeOfInt = sizeof(size_t);

  IPCQueue::IPCQueue(const string &fileName, bool create,
    size_t queueSize) throw (SystemException) : fileName_(fileName),
    pageSize_(ACE_OS::sysconf(_SC_PAGE_SIZE))
  {
    int oflag = O_RDWR;
    if (create)
      oflag |= O_CREAT;

    if ((fd_ = ACE_OS::open(fileName_.c_str(), oflag, 0666)) == -1)
      throw SystemException("open");

    if (create) {
      fileSize_ = sizeof(Header) + queueSize;
      if (ACE_OS::lseek(fd_, fileSize_ - 1, SEEK_SET) == -1)  // extend the file
        throw SystemException("lseek");
      if (ACE_OS::write(fd_, "", 1) == -1)  // write a zero at the end of file
        throw SystemException("write");
    } else {
      ACE_stat stat;
      if (ACE_OS::fstat(fd_, &stat) == -1)
        throw SystemException("fstat");
      fileSize_  = stat.st_size;
    }

    void *addr;
    if ((addr = ACE_OS::mmap(0, fileSize_, PROT_READ | PROT_WRITE, MAP_SHARED,
      fd_, 0)) == MAP_FAILED)
      throw SystemException("mmap");

    header_ = reinterpret_cast<Header*>(addr);
    queue_  = reinterpret_cast<char*>(header_ + 1);

    if (create == false)
      return;

    header_->queueSize_ = queueSize;
    header_->readerOffset_ = 0;
    header_->writerOffset_ = 0;
    header_->full_ = false;

    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&header_->lock_, &mutexAttr);

    pthread_condattr_t condAttr;
    pthread_condattr_init(&condAttr);
    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&header_->cvReader_, &condAttr);
    pthread_cond_init(&header_->cvWriter_, &condAttr);

    sync(header_, sizeof(Header));
  }

  IPCQueue::~IPCQueue()
  {
    ACE_OS::munmap(header_, fileSize_);
    ACE_OS::close(fd_);
  }

  void IPCQueue::push(const void *msg, size_t size) throw (SystemException)
  {
    size_t totalSize = sizeOfInt + size;

    Guard guard(&header_->lock_);

    // block if full or if msg can't fit
    while (full(totalSize))
      pthread_cond_wait(&header_->cvWriter_, &header_->lock_);

    // there is space - do we wrap?
    char *pos = queue_ + header_->writerOffset_;
    size_t spaceUntilEnd = queue_ + header_->queueSize_ - pos;
    bool sizeWraps = spaceUntilEnd <= sizeOfInt,
         msgWraps  = spaceUntilEnd <= totalSize;

    if (sizeWraps) {
      memmove(pos, &size, spaceUntilEnd);  // copy all or part of size
      sync(pos, spaceUntilEnd);  // sync end of queue
      size_t rem = sizeOfInt - spaceUntilEnd;
      if (rem) {
        const char *p = reinterpret_cast<const char*>(&size) + spaceUntilEnd;
        memmove(queue_, p, rem);  // copy remainder
      }
      pos = queue_ + rem;
      memmove(pos, msg, size);  // copy msg
      sync(queue_, size + rem);  // sync head of queue
      pos += size;
    } else if (msgWraps) {
      size_t syncAmt = spaceUntilEnd;
      memmove(pos, &size, sizeOfInt);  // copy size
      spaceUntilEnd -= sizeOfInt;
      memmove(pos + sizeOfInt, msg, spaceUntilEnd);  // copy all or part of msg
      sync(pos, syncAmt);  // sync end of queue
      size_t rem = size - spaceUntilEnd;
      if (rem) {
        const char *p = reinterpret_cast<const char*>(msg) + spaceUntilEnd;
        memmove(queue_, p, rem);  // copy remainder 
        sync(queue_, rem);  // sync head of queue
      }
      pos = queue_ + rem;
    } else {  // no wrap
      memmove(pos, &size, sizeOfInt);
      memmove(pos + sizeOfInt, msg, size);
      sync(pos, totalSize);  // sync data
      pos += totalSize;
    }

    header_->writerOffset_ = pos - queue_;
    header_->full_ = header_->writerOffset_ == header_->readerOffset_; 

    sync(header_, sizeof(Header));  // sync header to disk

    pthread_cond_signal(&header_->cvReader_);
  }

  size_t IPCQueue::pop(void *msg, size_t size) throw (SystemException)
  {
    Guard guard(&header_->lock_);

    // block if empty
    while (empty())
      pthread_cond_wait(&header_->cvReader_, &header_->lock_);

    // there is a msg - does it wrap?
    char *pos = queue_ + header_->readerOffset_;
    size_t len, spaceUntilEnd = queue_ + header_->queueSize_ - pos;

    if (spaceUntilEnd <= sizeOfInt) {  // size wraps
      memmove(&len, pos, spaceUntilEnd);  // copy all or part of size
      size_t rem = sizeOfInt - spaceUntilEnd;
      if (rem) {
        char *p = reinterpret_cast<char*>(&len) + spaceUntilEnd;
        memmove(p, queue_, rem);  // copy remainder
      }
      pos = queue_ + rem;
      memmove(msg, pos, len < size ? len : size);  // copy msg
      pos += len;
    } else {  // msg may wrap
      memmove(&len, pos, sizeOfInt);  // copy size
      pos += sizeOfInt;
      spaceUntilEnd -= sizeOfInt;
      // size bytes requested, len bytes actual
      size_t n = len < size ? len : size;  // copy n bytes
      if (n < spaceUntilEnd) {  // no wrap
        memmove(msg, pos, n);  // copy msg
        pos += len;
      } else {
        memmove(msg, pos, spaceUntilEnd);  // copy part of msg
        size_t rem = n - spaceUntilEnd;
        if (rem) {
          char *p = reinterpret_cast<char*>(msg) + spaceUntilEnd;
          memmove(p, queue_, rem);  // copy remainder
        }
        pos = queue_ + len - spaceUntilEnd;
      }
    }

    header_->readerOffset_ = pos - queue_;
    header_->full_ = false;

    sync(header_, sizeof(Header));  // sync header to disk

    pthread_cond_signal(&header_->cvWriter_);

    return len;
  }

  void IPCQueue::sync(void *addr, size_t size) throw (SystemException)
  {
    // addr must be a multiple of the system page size 
    char *alignedAddr = (char*)((long)addr & ~(pageSize_ - 1));  // round down
    ptrdiff_t diff = (char*)addr - alignedAddr;
    if (ACE_OS::msync(alignedAddr, size + diff, MS_SYNC) == -1)
      throw SystemException("msync");
  }

}


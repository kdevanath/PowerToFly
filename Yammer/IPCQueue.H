#ifndef _YAMMER_IPC_QUEUE_H_
#define _YAMMER_IPC_QUEUE_H_

#include <string>
#include <sstream>
#include <pthread.h>
#include <ace/OS.h>

namespace Yammer {

  using std::string;
  using std::stringstream;

  // an exception class
  class SystemException {
  public:
    SystemException(const string &cmd) {
      int err = ACE_OS::last_error();
      stringstream ss;
      ss << "SystemException caused by " << cmd << ": (" << err << ") "
         << ACE_OS_String::strerror(err);
      msg_ = ss.str();
    }
    const char *what() const { return msg_.c_str(); }

  private:
    string msg_;
  };


  // an interprocess queue
  class IPCQueue {
  public:

    struct Header {
      size_t queueSize_;
      size_t readerOffset_;
      size_t writerOffset_;
      bool full_;
      pthread_mutex_t lock_;
      pthread_cond_t cvReader_;
      pthread_cond_t cvWriter_;
    };

    IPCQueue(const string &fileName, bool create, size_t queueSize = 0)
      throw (SystemException);

    ~IPCQueue();

    // blocks if full
    void push(const void *msg, size_t len) throw (SystemException);

    // blocks if empty, returns amount written 
    size_t pop(void *msg, size_t len) throw (SystemException);

    bool full(size_t size) {
      if (header_->full_)
        return true;

      if (header_->writerOffset_ == header_->readerOffset_)  // empty
        return header_->queueSize_ < size;  // block if queue too small

      // check remaining size in queue - block if can't fit
      size_t rem;
      if (header_->writerOffset_ < header_->readerOffset_)
        rem = header_->readerOffset_ - header_->writerOffset_;
      else
        rem = header_->queueSize_ - header_->writerOffset_ +
          header_->readerOffset_;
      
      return rem < size;
    }

    bool empty() {
      if (header_->full_)
        return false;

      return header_->writerOffset_ == header_->readerOffset_;
    }

    string fileName() const { return fileName_; }
    size_t fileSize() const { return fileSize_; }

  private:
    // synchronize memory with backing store
    void sync(void *addr, size_t size) throw (SystemException);

    Header *header_;
    char *queue_;
    string fileName_;
    size_t fileSize_;
    int fd_;  // file descriptor
    long pageSize_;  // system memory page size
  };

}

#endif


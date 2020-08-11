#include <IPCQueueStreamAdapter.H> 

namespace Yammer {

  IPCQueueStreamAdapter::IPCQueueStreamAdapter()
    : queue_(0) {}

  IPCQueueStreamAdapter::IPCQueueStreamAdapter(IPCQueue *queue)
    : queue_(queue) {}

  int IPCQueueStreamAdapter::read(void *buffer, size_t len)
    throw (NetworkError)
  {
    if (queue_ == 0)
      return 0;

    return queue_->pop(buffer, len);
  }

  int IPCQueueStreamAdapter::write(const void *buffer, size_t len)
    throw (NetworkError)
  {
    if (queue_ == 0)
      return 0;

    queue_->push(buffer, len);
    return len;
  }

  void IPCQueueStreamAdapter::close() {}

}


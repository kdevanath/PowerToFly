#ifndef _YAMMER_IPC_QUEUE_STREAM_ADAPTER_H_
#define _YAMMER_IPC_QUEUE_STREAM_ADAPTER_H_

#include <ace/SOCK_Dgram.h> 
#include <Stream.H> 
#include <IPCQueue.H> 

namespace Yammer {

  // This class adapts an IPCQueue to the Stream interface.

  class IPCQueueStreamAdapter : public Stream {  // Adapter pattern
  public:
    IPCQueueStreamAdapter();
    IPCQueueStreamAdapter(IPCQueue *queue);

    int read(void *buffer, size_t len) throw (NetworkError);
    int write(const void *buffer, size_t len) throw (NetworkError);

    void flush() {}
    void close();

    IPCQueue *getIPCQueue() { return queue_; }
    void setIPCQueue(IPCQueue *queue) { queue_ = queue; }

  private:
    IPCQueue *queue_;
  };

}

#endif


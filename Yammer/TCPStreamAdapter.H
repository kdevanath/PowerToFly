#ifndef _YAMMER_TCP_STREAM_ADAPTER_H_
#define _YAMMER_TCP_STREAM_ADAPTER_H_

#include <ace/SOCK_Stream.h> 
#include <Stream.H> 

namespace Yammer {

  // This class adapts an ACE TCP socket to the Stream interface.

  class TCPStreamAdapter : public Stream {  // Adapter pattern
  public:
    TCPStreamAdapter();
    TCPStreamAdapter(const ACE_SOCK_Stream &stream);

    int read(void *buffer, size_t len) throw (NetworkError);
    int write(const void *buffer, size_t len) throw (NetworkError);

    int readv(const iovec *vec, int len) throw (NetworkError);
    int writev(const iovec *vec, int len) throw (NetworkError);

    void flush() {}
    void close();
    int handle() { return stream_.get_handle(); }

    ACE_SOCK_Stream &getStream() { return stream_; }
    void setStream(const ACE_SOCK_Stream &stream) { stream_ = stream; }

  private:
    ACE_SOCK_Stream stream_;
  };

}

#endif


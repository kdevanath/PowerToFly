#ifndef _YAMMER_BUFFERED_STREAM_H_
#define _YAMMER_BUFFERED_STREAM_H_

#include <memory> 
#include <string> 
#include <vector> 
#include <Stream.H> 

namespace Yammer {

  using std::auto_ptr;
  using std::string;
  using std::vector;

  // This class provides buffered I/O for a stream.  Input or output can be
  // set to unbuffered by setting its buffer size to zero; output is
  // unbuffered by default.  Use of an input buffer can prevent multiple
  // system calls when multiple reads are needed.  The readline() method
  // returns one complete message based on a separator.  The stream should be
  // set in the constructor or before calling read() or write().  Copy and
  // assignment is not permitted as this class is only a decorator.  Further,
  // the class does not own the Stream resource.

  class BufferedStream : public Stream {  // Decorator pattern
  public:
    enum { RCV_BUF = 32 * 1024, SND_BUF = 0 };

    BufferedStream(size_t rcvBufSize = RCV_BUF, size_t sndBufSize = SND_BUF);
    BufferedStream(Stream *stream, size_t rcvBufSize = RCV_BUF,
      size_t sndBufSize = SND_BUF);

    // read up to and including separator
    // returns the amount copied into buffer
    int readline(string &buffer, char separator) throw (NetworkError);
    int readline(string &buffer, const string &separator) throw (NetworkError);

    int read(void *buffer, size_t len) throw (NetworkError);
    int write(const void *buffer, size_t len) throw (NetworkError);

    void flush();
    void close();
    int handle();

    Stream *getStream() { return stream_; }
    void setStream(Stream *stream) { stream_ = stream; }

  private:
    // prevent copy and assignment
    BufferedStream(const BufferedStream&);
    BufferedStream &operator=(const BufferedStream&);

    char readChar() throw (NetworkError);
    int check() throw (NetworkError);

    Stream *stream_;

    auto_ptr<char> readBuffer_;
    size_t readBufferSize_;
    size_t readOffset_;
    size_t readCount_;

    auto_ptr<char> writeBuffer_;
    size_t writeBufferSize_;
    size_t writeCount_;
  };

}

#endif


#ifndef _YAMMER_STREAM_H_
#define _YAMMER_STREAM_H_

#include <sys/uio.h>  // for struct iovec
#include <string>
#include <vector> 
#include <ByteOrder.H> 
#include <NetworkExceptions.H> 

namespace Yammer {

  using std::string;
  using std::vector;

  // Abstracts the behavior of a stream.  Underlying implementaion may be 
  // stream based or message oriented.  Methods should read/write up to len
  // bytes and return the amount read/written.  Blocking semantics are defined
  // by the subclass.

  class Stream {  // interface class 
  public:
    virtual ~Stream() {}

    virtual int read(void *buffer, size_t len) throw (NetworkError) = 0;
    virtual int write(const void *buffer, size_t len) throw (NetworkError) = 0;

    // scatter/gather I/O (optional)
    virtual int readv(const iovec *vec, int len) throw (NetworkError)
      { return 0; }
    virtual int writev(const iovec *vec, int len) throw (NetworkError)
      { return 0; }

    virtual void flush() = 0;

    virtual void close() = 0;

    // for TCP/IP based connections (optional)
    virtual int handle() { return -1; }
  };


  // Recommended Message I/O Convention
  // 
  // To write a message, put values into a vector<char> buffer using the 
  // stream output operators defined in MsgHelper.H.  Then send the message
  // in one call with
  //
  //   operator<<(Stream&, const vector<char>&)
  //
  // To read a message, use the following stream input operators with a
  // BufferedStream.  The stream output operators are provided for convenience,
  // however if the BufferedStream has output unbuffered (the default), their
  // use is discouraged, as it is less efficient (e.g. sending values
  // individually using multiple system calls vs sending one buffer containing
  // multiple values in one system call).  An alternative is to use a
  // BufferedStream with an output buffer and call flush when necessary.  In 
  // that case, the output buffer would need to be sized appropriately if 
  // only one system call was desired.
  //
  // Be sure to write and read values in the same order.  To transmit vectors,
  // send the size first and either have the reader call resize (more efficient)
  // or have the reader use push_back.


  // stream I/O operators that wrap read/write and perform network byte order
  // conversion

  inline Stream &operator<<(Stream&, bool) throw (NetworkError);
  inline Stream &operator>>(Stream&, bool&) throw (NetworkError);

  inline Stream &operator<<(Stream&, char) throw (NetworkError);
  inline Stream &operator>>(Stream&, char&) throw (NetworkError);

  inline Stream &operator<<(Stream&, int) throw (NetworkError);
  inline Stream &operator>>(Stream&, int&) throw (NetworkError);

  inline Stream &operator<<(Stream&, size_t) throw (NetworkError);
  inline Stream &operator>>(Stream&, size_t&) throw (NetworkError);

  inline Stream &operator<<(Stream&, double) throw (NetworkError);
  inline Stream &operator>>(Stream&, double&) throw (NetworkError);

  inline Stream &operator<<(Stream&, const string&) throw (NetworkError);
  inline Stream &operator>>(Stream&, string&) throw (NetworkError);

  // flush manipulator
  // e.g. stream << value << flush;
  inline Stream &operator<<(Stream&, Stream &(*fn)(Stream&)) throw (NetworkError);
  inline Stream &flush(Stream&) throw (NetworkError);

  // this does not write the size of the buffer due to the above message I/O
  // convention (sending using a buffer and receiving using BufferedStream)
  inline Stream &operator<<(Stream&, const vector<char>&) throw (NetworkError);

  // Temporary containers for use with the stream operators, for example
  // 
  //   char header[len];
  //   string payload;
  //   ...
  //   stream << OutPacket(header, len)
  //          << OutPacket(payload.data(), payload.size());
  //   vector<char> buffer(len);
  //   stream >> InPacket(&buffer[0], buffer.size());

  struct InPacket {  // for input
    InPacket(void *buf, size_t size) : buf_(buf), size_(size) {}

    void *buf_;
    size_t size_;
  };

  struct OutPacket {  // for output
    OutPacket(const void *buf, size_t size) : buf_(buf), size_(size) {}

    const void *buf_;
    size_t size_;
  };

  inline Stream &operator<<(Stream&, const OutPacket&) throw (NetworkError);
  inline Stream &operator>>(Stream&, const InPacket&) throw (NetworkError);

  // a vector of buffers
  struct IOVec {
    IOVec(const iovec *vec, int size) : iovec_(vec), size_(size) {}

    const iovec *iovec_;
    int size_;
  };

  // scatter/gather I/O 
  // read/write mulitple buffers in one system-level call; can prevent
  // complications resulting from interaction between Nagle's algorithm and
  // delayed ACKs in TCP
  inline Stream &operator<<(Stream&, const IOVec&) throw (NetworkError);
  inline Stream &operator>>(Stream&, const IOVec&) throw (NetworkError);


  // inline definitions


  inline Stream &operator<<(Stream &stream, bool val) throw (NetworkError)
  {
    stream.write(&val, byte);
    return stream;
  }

  inline Stream &operator>>(Stream &stream, bool &val) throw (NetworkError)
  {
    stream.read(&val, byte);
    return stream;
  }

  inline Stream &operator<<(Stream &stream, char val) throw (NetworkError)
  {
    stream.write(&val, byte);
    return stream;
  }

  inline Stream &operator>>(Stream &stream, char &val) throw (NetworkError)
  {
    stream.read(&val, byte);
    return stream;
  }

  inline Stream &operator<<(Stream &stream, int val) throw (NetworkError)
  {
    hton(val);
    stream.write(&val, word);
    return stream;
  }

  inline Stream &operator>>(Stream &stream, int &val) throw (NetworkError)
  {
    stream.read(&val, word);
    ntoh(val);
    return stream;
  }

  inline Stream &operator<<(Stream &stream, size_t val) throw (NetworkError)
  {
    hton(val);
    stream.write(&val, word);
    return stream;
  }

  inline Stream &operator>>(Stream &stream, size_t &val) throw (NetworkError)
  {
    stream.read(&val, word);
    ntoh(val);
    return stream;
  }

  inline Stream &operator<<(Stream &stream, double val) throw (NetworkError)
  {
    hton(val);
    stream.write(&val, dword);
    return stream;
  }

  inline Stream &operator>>(Stream &stream, double &val) throw (NetworkError)
  {
    stream.read(&val, dword);
    ntoh(val);
    return stream;
  }

  inline Stream &operator<<(Stream &stream, const string &val)
    throw (NetworkError)
  {
    stream << val.size();
    stream.write(val.data(), val.size());
    return stream;
  }

  inline Stream &operator>>(Stream &stream, string &val) throw (NetworkError)
  {
    size_t size;
    stream >> size;
    // put the string into a char buffer first, as std::string is not
    // guaranteed to use contiguous memory
    vector<char> buffer(size);
    stream.read(&buffer[0], buffer.size());
    val.assign(&buffer[0], buffer.size());
    return stream;
  }

  inline Stream &operator<<(Stream &stream, Stream &(*fn)(Stream&))
    throw (NetworkError)
  {
    return fn(stream);
  }

  inline Stream &flush(Stream &stream) throw (NetworkError)
  {
    stream.flush();
    return stream;
  }

  inline Stream &operator<<(Stream &stream, const vector<char> &buffer)
    throw (NetworkError)
  {
    // the standard guarantees vector uses contiguous memory
    stream.write(&buffer[0], buffer.size());
    return stream;
  }

  inline Stream &operator<<(Stream &stream, const OutPacket &packet)
    throw (NetworkError)
  {
    stream.write(packet.buf_, packet.size_);
    return stream;
  }

  inline Stream &operator>>(Stream &stream, const InPacket &packet)
    throw (NetworkError)
  {
    stream.read(packet.buf_, packet.size_);
    return stream;
  }

  inline Stream &operator<<(Stream &stream, const IOVec& vec)
    throw (NetworkError)
  {
    stream.writev(vec.iovec_, vec.size_);
    return stream;
  }

  inline Stream &operator>>(Stream &stream, const IOVec& vec)
    throw (NetworkError)
  {
    stream.readv(vec.iovec_, vec.size_);
    return stream;
  }

}

#endif


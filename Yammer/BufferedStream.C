#include <BufferedStream.H> 
#include <string.h>

namespace Yammer {

  BufferedStream::BufferedStream(size_t rcvBufSize, size_t sndBufSize)
    : stream_(0), readBuffer_(new char[rcvBufSize]),
    readBufferSize_(rcvBufSize), readOffset_(0), readCount_(0),
    writeBuffer_(new char[sndBufSize]), writeBufferSize_(sndBufSize),
    writeCount_(0) {}

  BufferedStream::BufferedStream(Stream *stream, size_t rcvBufSize,
    size_t sndBufSize) : stream_(stream), readBuffer_(new char[rcvBufSize]),
    readBufferSize_(rcvBufSize), readOffset_(0), readCount_(0),
    writeBuffer_(new char[sndBufSize]), writeBufferSize_(sndBufSize),
    writeCount_(0) {}

  int BufferedStream::readline(string &buffer, char separator)
    throw (NetworkError)
  {
    if (stream_ == 0)
      return 0;

    int cnt = 0;
    for (;;) {
      char c = readChar();
      buffer += c;
      cnt++;
      if (c == separator)  // end of message
        return cnt;
    }
  }

  int BufferedStream::readline(string &buffer, const string &separator)
    throw (NetworkError)
  {
    if (stream_ == 0)
      return 0;

    int cnt = 0;
    size_t bsz = buffer.size(), ssz = separator.size();
    for (;;) {
      buffer += readChar();
      cnt++;
      bsz++;
      // compare the last ssz chars of buffer
      if (bsz >= ssz && buffer.compare(bsz - ssz, ssz, separator) == 0)
        return cnt;
    }
  }

  int BufferedStream::read(void *buffer, size_t len) throw (NetworkError)
  {
    if (stream_ == 0)
      return 0;

    if (readBufferSize_ == 0)  // input not buffered
      return stream_->read(buffer, len);

    char *ptr = reinterpret_cast<char*>(buffer);
    int size = len;

    for (;;) {
      if (check() >= size)  // may read (and block)
        break;
      // copy all of the read buffer
      memmove(ptr, readBuffer_.get() + readOffset_, readCount_);
      ptr += readCount_;
      size -= readCount_;
      readCount_ = 0;  // will trigger a refill
    }

    // copy what's left of size
    memmove(ptr, readBuffer_.get() + readOffset_, size);
    readOffset_ += size;
    readCount_ -= size;

    return len;
  }

  int BufferedStream::write(const void *buffer, size_t len) throw (NetworkError)
  {
    if (stream_ == 0)
      return 0;

    if (writeBufferSize_ == 0)  // output not buffered
      return stream_->write(buffer, len);

    const char *p = reinterpret_cast<const char*>(buffer);
    bool shouldFlush = writeCount_ + len >= writeBufferSize_;

    if (shouldFlush) {
      size_t n = writeCount_ + len - writeBufferSize_;
      // copy n bytes, or len bytes if perfect fit
      memmove(writeBuffer_.get() + writeCount_, p, n ? n : len);  // fill
      flush();  // send 
      if (n)  // buffer remaining bytes
        memmove(writeBuffer_.get() + writeCount_, p + n, len - n); 
      writeCount_ = len - n;
    } else {  // buffer the output
      memmove(writeBuffer_.get() + writeCount_, p, len);
      writeCount_ += len;
    }

    return len;
  }

  void BufferedStream::flush()
  {
    if (stream_ == 0 || writeCount_ == 0)
      return;

    stream_->write(writeBuffer_.get(), writeCount_);
    writeCount_ = 0;
  }

  void BufferedStream::close()
  {
    if (stream_ == 0)
      return;

    stream_->close();
  }

  int BufferedStream::handle()
  {
    if (stream_ == 0)
      return -1;

    return stream_->handle();
  }

  char BufferedStream::readChar() throw (NetworkError)
  {
    if (readBufferSize_ == 0) {  // input not buffered
      char c;
      stream_->read(&c, 1);
      return c;
    }
    
    check();
    readCount_--;
    return readBuffer_.get()[readOffset_++];
  }

  // check if buffer needs to be refilled
  int BufferedStream::check() throw (NetworkError)
  {
    if (readCount_ <= 0) {  // buffer used up
      readOffset_ = 0;
      readCount_ = stream_->read(readBuffer_.get(), readBufferSize_);
    }
    return readCount_;
  }

}


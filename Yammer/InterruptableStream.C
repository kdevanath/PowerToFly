#include <InterruptableStream.H>
#include <ace/OS.h> 

namespace Yammer {

  using std::max;

  InterruptableStream::InterruptableStream(Stream *stream,
    long timeout) : stream_(stream), interrupted_(false), timeout_(timeout)
  {
    if (stream_ == 0)
      return;

    pipe_.open();

    ACE_HANDLE pipeFd = pipe_.read_handle(), streamFd = stream_->handle();
    maxFd_ = max(streamFd, pipeFd) + 1;

    FD_ZERO(&rset_);
    FD_SET(pipeFd, &rset_);
    FD_SET(streamFd, &rset_);
  }

  int InterruptableStream::read(void *buffer, size_t len) throw (NetworkError)
  {
    if (stream_ == 0)
      return 0;

    fd_set set = rset_;
    ACE_Time_Value tmp, *timeout = 0;  // use null to block
    if (timeout_ >= 0) {
      tmp.msec(timeout_);
      timeout = &tmp;
    }

    int ret;
    if ((ret = ACE_OS::select(maxFd_, &set, 0, 0, timeout)) == -1)
      throw SocketError();
    else if (ret == 0)  // timed out
      throw TimeoutException();

    bool pipeHasData = FD_ISSET(pipe_.read_handle(), &set);
    if (pipeHasData) {
      resetPipe();
      throw InterruptException();
    }

    return stream_->read(buffer, len);
  }

  int InterruptableStream::write(const void *buffer, size_t len)
    throw (NetworkError)
  { 
    if (stream_ == 0)
      return 0;

    return stream_->write(buffer, len);
  }


  void InterruptableStream::flush()
  {
    if (stream_ == 0)
      return;

    stream_->flush();
  }

  void InterruptableStream::close()
  {
    if (stream_ == 0)
      return;

    stream_->close();
  }

  int InterruptableStream::handle()
  {
    if (stream_ == 0)  
      return -1;
    
    return stream_->handle();
  }

  void InterruptableStream::interrupt() throw (SocketError)
  {
    {
      ACE_Guard<ACE_Thread_Mutex> lock(mutex_);
      if (interrupted_)  // no need to interrupt again
        return;

      interrupted_ = true;
    }

    if (ACE_OS::send(pipe_.write_handle(), "", 1) == -1)  // write a zero
      throw SocketError();
  }

  void InterruptableStream::resetPipe() throw (SocketError)
  {
    {
      ACE_Guard<ACE_Thread_Mutex> lock(mutex_);
      interrupted_ = false;
    }

    char c;
    if (ACE_OS::recv(pipe_.read_handle(), &c, 1) == -1)  // read a zero
      throw SocketError();

  }

}


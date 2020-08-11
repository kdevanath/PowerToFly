#ifndef _YAMMER_INTERRUPTABLE_STREAM_H_
#define _YAMMER_INTERRUPTABLE_STREAM_H_

#include <ace/Synch.h> 
#include <ace/Pipe.h> 
#include <Stream.H> 
#include <NetworkExceptions.H> 

namespace Yammer {

  // This class allows you to unblock, or interrupt, a thread blocked in a
  // socket recv() call.
  //
  // This class can be used as an alternative to inefficient polling using
  // small timeouts.  Instead, when there is no data to be read, the thread will
  // be put to sleep by the OS and will awaken only when necessary, for example,
  // due to a timeout or an outside event.
  //
  // Here's an example of inefficient polling using the select() system call.
  //
  //    // inefficient
  //    for (;;) {
  //      select(...);              // uses non-blocking sockets
  //      if (shouldDoSomething())  // check all the sockets
  //        doSomething();          // send a heartbeat, recv a msg, etc
  //      sleep(1);
  //    }
  //      
  // The main downside to polling this way is the process is working when it
  // doesn't need to (when the timer ends and there is nothing to do), and
  // loses its ability to work when it does need to (when there is I/O
  // activity and the process is blocked in a call to sleep).
  //
  // Moreover, when select() is used with multiple sockets (multiplexing),
  // other downsides are that it prevents concurrent I/O (it doesn't make use of
  // a multiprocessor machine) and it makes the code structure more complicated
  // (the code is simpler when there is one thread performing one function).
  //
  // Here's an example where the use of InterruptableStream is appropriate.  A
  // connection needs to both send and receive heartbeats periodically, and
  // send data when an event occurs, such as a fill or a quote.  The
  // connection can block in a recv() call and be awaken only when 
  //
  //   *  there is data to be read
  //   *  there is data to be sent (due to an outside event)
  //   *  a timeout occurs (a heartbeat should be sent)
  //
  // In this example, the outbound message can be put on a queue and the
  // sending thread can be notified by interrupting the stream.  After being
  // interrupted, the thread can read off of the queue and send the message. 
  // As in
  //
  //   for (;;) {
  //     try {
  //       stream >> msg;
  //       process(msg);
  //     } catch (InterruptException) {
  //       Msg event = readFromQueue();
  //       stream << event;
  //     } catch (TimeoutException) {
  //       stream << heartbeat;
  //     }
  //   }
  // 
  // Copy and assignment are not permitted as this class is only a decorator. 
  // Further, the class does not own the Stream resource.

  class InterruptableStream : public Stream {  // Decorator pattern
  public:
    InterruptableStream(Stream *stream, long timeout = -1);

    // read() can throw an InterruptException or TimeoutException
    int read(void *buffer, size_t len) throw (NetworkError);
    int write(const void *buffer, size_t len) throw (NetworkError);

    void interrupt() throw (SocketError);

    void flush();
    void close();
    int handle();

    // timeout values are in milliseconds, -1 blocks indefinitely, 0 returns
    // immediately
    long getTimeout() { return timeout_; }
    void setTimeout(long msecs) { timeout_ = msecs; }

    Stream *getStream() { return stream_; }

  private:
    // prevent copy and assignment
    InterruptableStream(const InterruptableStream&);
    InterruptableStream &operator=(const InterruptableStream&);

    void resetPipe() throw (SocketError);

    Stream *stream_;

    bool interrupted_;
    ACE_Thread_Mutex mutex_;

    // used by select()
    ACE_Pipe pipe_;  // used to interrupt 
    fd_set rset_;
    int maxFd_;
    long timeout_;
  };

}

#endif


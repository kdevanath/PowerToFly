#ifndef _REFERENCE_IMPLEMENTATION_EXAMPLE_LOCAL_SERVICE_H_
#define _REFERENCE_IMPLEMENTATION_EXAMPLE_LOCAL_SERVICE_H_

#include <string>
#include <Service.H>
#include <Task.H>
#include <Stream.H>

namespace ReferenceImplementation {

  using std::string;
  using Yammer::Service;
  using Yammer::Task;
  using Yammer::Stream;

  // A service which uses local interprocess queues for communication.

  // client event loop
  class ExampleLocalService : public Service {
  public: 
    ExampleLocalService();
    ~ExampleLocalService();

    ExampleLocalService *clone() const
      { return new ExampleLocalService(*this); } 

    void spawn(Stream *stream, Task *startTask) throw();

    static void *run(void *This);

    Stream *stream() { return stream_; }

    string getRequestQueueName() const { return requestQueueName_; }
    size_t getRequestQueueSize() const { return requestQueueSize_; }
    string getReplyQueueName() const { return replyQueueName_; }
    size_t getReplyQueueSize() const { return replyQueueSize_; }

  private:
    Stream *stream_;  // for intial connection from ServiceAccessPoint

    string requestQueueName_;
    size_t requestQueueSize_;
    string replyQueueName_;
    size_t replyQueueSize_;
  }; 
}

#endif


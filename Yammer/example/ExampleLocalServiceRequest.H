#ifndef _REFERENCE_IMPLEMENTATION_EXAMPLE_LOCAL_SERVICE_REQUEST_H_
#define _REFERENCE_IMPLEMENTATION_EXAMPLE_LOCAL_SERVICE_REQUEST_H_

#include <string> 
#include <Task.H> 
#include <ExampleTypeIds.H> 

namespace ReferenceImplementation {

  using std::string;
  using Yammer::Task;
  using Yammer::Stream;
  using Yammer::NetworkError;

  // Request used to initiate the ExampleLocalService which uses
  // local interprocess queues for communication.

  class ExampleLocalServiceRequest : public Task {
  public:
    ExampleLocalServiceRequest();
    ExampleLocalServiceRequest(const string &requestQueueName,
      size_t requestQueueSize, const string &replyQueueName,
      size_t replyQueueSize);

    ExampleLocalServiceRequest *clone() const
      { return new ExampleLocalServiceRequest(*this); }

    void run() {}

    int type() const { return ExampleLocalServiceRequestId; }

    string name() const { return "ExampleLocalServiceRequest"; }

    void toStream(Stream &stream) const throw (NetworkError);
    void fromStream(Stream &stream) throw (NetworkError);

    string getRequestQueueName() const { return requestQueueName_; }
    void setRequestQueueName(const string &name) { requestQueueName_ = name; }

    size_t getRequestQueueSize() const { return requestQueueSize_; }
    void setRequestQueueSize(size_t size) { requestQueueSize_ = size; }

    string getReplyQueueName() const { return replyQueueName_; }
    void setReplyQueueName(const string &name) { replyQueueName_ = name; }

    size_t getReplyQueueSize() const { return replyQueueSize_; }
    void setReplyQueueSize(size_t size) { replyQueueSize_ = size; }

  private:
    string requestQueueName_;
    size_t requestQueueSize_;
    string replyQueueName_;
    size_t replyQueueSize_;
  };
}

#endif


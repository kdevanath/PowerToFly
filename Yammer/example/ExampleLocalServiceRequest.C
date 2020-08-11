#include <ExampleLocalServiceRequest.H> 
#include <vector> 
#include <TaskFactory.H> 
#include <MsgHelper.H> 

namespace ReferenceImplementation {

  using std::vector;
  using Yammer::operator<<;

  ExampleLocalServiceRequest::ExampleLocalServiceRequest() 
    : requestQueueSize_(0), replyQueueSize_(0) {}

  ExampleLocalServiceRequest::ExampleLocalServiceRequest(const string
    &requestQueueName, size_t requestQueueSize, const string &replyQueueName,
    size_t replyQueueSize)
    : requestQueueName_(requestQueueName), requestQueueSize_(requestQueueSize),
      replyQueueName_(replyQueueName), replyQueueSize_(replyQueueSize) {}

  void ExampleLocalServiceRequest::toStream(Stream &stream) const
    throw (NetworkError)
  {
    // build the message using a vector<char> as the buffer
    // be sure to send the type first
    vector<char> buffer;
    buffer << type() 
           << requestQueueName_
           << requestQueueSize_
           << replyQueueName_
           << replyQueueSize_;

    // send the message
    stream << buffer;
  }

  void ExampleLocalServiceRequest::fromStream(Stream &stream)
    throw (NetworkError)
  {
    // read in values in the same order as sent excluding the type; the type
    // is read by the TaskFactory
    stream >> requestQueueName_
           >> requestQueueSize_
           >> replyQueueName_
           >> replyQueueSize_;
  }

  Yammer::TaskRegistrar<ExampleLocalServiceRequest>
    ExampleLocalServiceRequestPrototype;

}


#include <ExampleRequest.H> 
#include <TaskFactory.H> 
#include <MsgHelper.H> 
#include <ExampleReply.H> 
#include <Logger.H> 

namespace ReferenceImplementation {

  using std::auto_ptr;
  using std::vector;
  using Yammer::operator<<;

  ExampleRequest::ExampleRequest() : sync_(0), intData_(0), dblData_(0) {}

  void ExampleRequest::run() 
  {
    // execute request

    LOG("ExampleRequest::run");

    // create reply, if necessary
    auto_ptr<ExampleReply> reply(new ExampleReply);
    reply->setString(strData_);
    reply->setInt(intData_);
    reply->setDouble(dblData_);

    // if the TaskDispatcher thread pool is executing the run method, notify
    // the Service of the reply by using the notify method, as in
    //
    //   sync_->notify(reply.release());
    //
    // alternatively, if the Service is executing the run method, use the
    // setTask method to return the reply, as in
    //
    //   sync_->setTask(reply.release());

    if (sync_)
      sync_->notify(reply.release());
  }

  void ExampleRequest::toStream(Stream &stream) const
    throw (NetworkError) 
  {
    // build the message using a vector<char> as the buffer
    // be sure to send the type first
    vector<char> buffer;
    buffer << type() 
           << strData_
           << intData_
           << dblData_;

    // send the message
    stream << buffer;
  }

  void ExampleRequest::fromStream(Stream &stream) throw (NetworkError)
  {
    // read in values in the same order as sent excluding the type; the type
    // is read by the TaskFactory
    stream >> strData_
           >> intData_
           >> dblData_;
  }

  Yammer::TaskRegistrar<ExampleRequest> ExampleRequestPrototype;

}


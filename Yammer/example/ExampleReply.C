#include <ExampleReply.H> 
#include <TaskFactory.H> 
#include <MsgHelper.H> 

namespace ReferenceImplementation {

  using std::vector;
  using Yammer::operator<<;

  ExampleReply::ExampleReply() : intData_(0), dblData_(0) {}

  void ExampleReply::toStream(Stream &stream) const
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

  void ExampleReply::fromStream(Stream &stream) throw (NetworkError)
  {
    // read in values in the same order as sent excluding the type; the type
    // is read by the TaskFactory
    stream >> strData_
           >> intData_
           >> dblData_;
  }

  Yammer::TaskRegistrar<ExampleReply> ExampleReplyPrototype;

}


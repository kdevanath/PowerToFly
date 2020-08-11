#ifndef _REFERENCE_IMPLEMENTATION_EXAMPLE_REPLY_H_
#define _REFERENCE_IMPLEMENTATION_EXAMPLE_REPLY_H_

#include <string> 
#include <Task.H> 
#include <ExampleTypeIds.H> 

namespace ReferenceImplementation {

  using std::string;
  using Yammer::Task;
  using Yammer::Stream;
  using Yammer::NetworkError;

  class ExampleReply : public Task {
  public:
    ExampleReply();

    ExampleReply *clone() const { return new ExampleReply(*this); }

    void run() {}

    int type() const { return ExampleReplyId; }

    string name() const { return "ExampleReply"; }

    void toStream(Stream &stream) const throw (NetworkError);
    void fromStream(Stream &stream) throw (NetworkError);

    string getString() { return strData_; }
    void setString(const string &val) { strData_ = val; }

    int getInt() { return intData_; }
    void setInt(int val) { intData_ = val; }

    double getDouble() { return dblData_; }
    void setDouble(double val) { dblData_ = val; }

  private:
    string strData_;
    int intData_;
    double dblData_;
  };

}

#endif


#ifndef _REFERENCE_IMPLEMENTATION_EXAMPLE_REQUEST_H_
#define _REFERENCE_IMPLEMENTATION_EXAMPLE_REQUEST_H_

#include <string> 
#include <Task.H> 
#include <TaskSync.H> 
#include <ExampleTypeIds.H> 

namespace ReferenceImplementation {

  using std::string;
  using Yammer::Task;
  using Yammer::TaskSync;
  using Yammer::Stream;
  using Yammer::NetworkError;

  class ExampleRequest : public Task {
  public:
    ExampleRequest();
    ExampleRequest *clone() const { return new ExampleRequest(*this); }

    void run();

    int type() const { return ExampleRequestId; }

    string name() const { return "ExampleRequest"; }

    void toStream(Stream &stream) const throw (NetworkError);
    void fromStream(Stream &stream) throw (NetworkError);

    void sync(TaskSync *sync) { sync_ = sync; }
    bool needsReply() const { return true; }

    string getString() { return strData_; }
    void setString(const string &val) { strData_ = val; }

    int getInt() { return intData_; }
    void setInt(int val) { intData_ = val; }

    double getDouble() { return dblData_; }
    void setDouble(double val) { dblData_ = val; }

  private:
    TaskSync *sync_;
    string strData_;
    int intData_;
    double dblData_;
  };

}

#endif


#ifndef _YAMMER_IMPORT_TASK_H_
#define _YAMMER_IMPORT_TASK_H_

#include <string> 
#include <Task.H> 
#include <TypeIds.H> 

namespace Yammer {

  using std::string;

  // Dynamically loads a native library into the process space.  Used to change
  // the behavior of a running system.  Statically allocated objects in the
  // shared library can add new Tasks or Services or swap out existing ones.

  // A path that contains a '/' is considered absolute or relative to the 
  // current directory; otherwise LD_LIBRARY_PATH is used.

  class ImportTask : public Task {
  public:
    ImportTask() {}
    ImportTask(const string &library);

    ImportTask *clone() const { return new ImportTask(*this); }

    void run();

    int type() const { return ImportTaskId; }

    string name() const { return "ImportTask"; }

    void toStream(Stream &stream) const throw (NetworkError);
    void fromStream(Stream &stream) throw (NetworkError);

    string library() const { return library_; }
    void library(const string &library) { library_ = library; }
    
  private:
    string library_;
  };

}

#endif


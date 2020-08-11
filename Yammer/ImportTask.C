#include <ImportTask.H> 
#include <vector> 
#include <ace/OS.h> 
#include <Logger.H> 
#include <TaskFactory.H> 
#include <MsgHelper.H> 

namespace Yammer {

  using std::vector;

  ImportTask::ImportTask(const string &library) : library_(library) {}

  void ImportTask::run()
  {
    if (ACE_OS::dlopen(library_.c_str(), RTLD_NOW) == 0)
      LOG("ERROR: attempt to load native library " << library_.c_str() << ": "
          << ACE_OS::dlerror());
  }

  void ImportTask::toStream(Stream &stream) const throw (NetworkError)
  {
    vector<char> buffer;
    buffer << type() 
           << library_;
    stream << buffer;
  }

  void ImportTask::fromStream(Stream &stream) throw (NetworkError)
  {
    stream >> library_;
  }

  TaskRegistrar<ImportTask> ImportTaskPrototype;

}


#ifndef _YAMMER_TASK_FACTORY_H_
#define _YAMMER_TASK_FACTORY_H_

#include <ace/Singleton.h> 
#include <ace/Synch.h> 
#include <Factory.H> 
#include <Task.H> 
#include <UnknownTask.H> 

namespace Yammer {

  class TaskFactory : public Factory<int, Task> {
  public:
    Task *createTask(Stream &stream) throw (UnknownTask, NetworkError) {
      int type;
      stream >> type;
      try { return create(type); }
      catch (UnknownType) { throw UnknownTask(); }
    }
  };

  typedef ACE_Singleton<TaskFactory, ACE_Thread_Mutex> TaskFactorySingleton;

  // To automatically register a task with TaskFactory, create a global instance
  // of TaskRegistrar.  For example, the following line can be placed in the .C
  // file of the Task class.
  //
  //   TaskRegistrar<MyTask> MyTaskPrototype;

  template <typename TaskClass>
  struct TaskRegistrar {
    TaskRegistrar() {
      TaskClass *prototypicalInstance = new TaskClass;
      TaskFactorySingleton::instance()->
        registerPrototype(prototypicalInstance->type(), prototypicalInstance);
    }
  };

}

#endif


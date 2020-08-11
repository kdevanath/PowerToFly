#ifndef _YAMMER_SERVICE_FACTORY_H_
#define _YAMMER_SERVICE_FACTORY_H_

#include <ace/Singleton.h> 
#include <Factory.H> 
#include <Service.H> 
#include <Task.H> 
#include <UnknownServiceRequest.H> 

namespace Yammer {

  class ServiceFactory : public Factory<int, Service> {
  public:
    Service *createService(const Task *startTask) throw (UnknownServiceRequest)
    {
      try { return create(startTask->type()); }
      catch (UnknownType) { throw UnknownServiceRequest(); }  // transform
    }
  };

  typedef
    ACE_Singleton<ServiceFactory, ACE_Thread_Mutex> ServiceFactorySingleton;


  // To automatically register a service with ServiceFactory, create a global
  // instance of ServiceRegistrar.  For example, the following line can be 
  // placed in the .C file of the Service class.
  //
  //   ServiceRegistrar<MyService, MyStartTask> MyServicePrototype;

  template <typename ServiceClass, typename StartTaskClass>
  struct ServiceRegistrar {                                       
    ServiceRegistrar() {                                          
      ServiceClass *prototypicalInstance = new ServiceClass;      
      ServiceFactorySingleton::instance()->
        registerPrototype(StartTaskClass().type(), prototypicalInstance);    
    }                                                             
  };

}

#endif


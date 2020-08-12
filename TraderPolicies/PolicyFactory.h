#ifndef _FSB_POLICY_FACTORY_H_
#define _FSB_POLICY_FACTORY_H_

#include <ace/Singleton.h> 
#include <ace/Synch.h> 
#include <Factory/Factory.h>
#include "Policy.h" 
#include "UnknownPolicy.h" 
#include "TraderPolicies.h"
#include <iostream>

#pragma warning( disable : 4290 )

using namespace fsbfactory;

namespace fsb {

	class TRADERPOLICIES_API PolicyFactory : public Factory<std::string, Policy> {
  public:
	  Policy *createPolicy(std::string &type) throw (UnknownPolicy) {
		//std::cout << " Printing Policyfactory " << type << std::endl;
		//printprototypeFactory();
      try { return create(type); }
	  catch (fsbfactory::UnknownType) { throw UnknownPolicy(); }
    }
  };

  typedef ACE_Singleton<PolicyFactory, ACE_Thread_Mutex> PolicyFactorySingleton;

  // To automatically register a Policy with PolicyFactory, create a global instance
  // of PolicyRegistrar.  For example, the following line can be placed in the .C
  // file of the Policy class.
  //
  //   PolicyRegistrar<MyPolicy> MyPolicyPrototype;

  template <typename PolicyClass>
  struct PolicyRegistrar {
    PolicyRegistrar() {
      PolicyClass *prototypicalInstance = new PolicyClass;
	  //std::cout << " Name in PolicyRegistrar " << prototypicalInstance->name() << std::endl;
      PolicyFactorySingleton::instance()->
        registerPrototype(prototypicalInstance->name(), prototypicalInstance);
	  //std::cout << " Printing in PolicyRegistrar " << std::endl;
	  //PolicyFactorySingleton::instance()->printprototypeFactory();
    }
  };

  TRADERPOLICIES_API PolicyFactory* get_instance_pf();

#define REGISTER_Policy(PolicyClass)                                        \
namespace {                                                             \
  struct PlocyRegistrar {                                                \
    PolicyRegistrar() {                                                   \
      PolicyClass *prototypicalInstance = new PolicyClass;                  \
      typedef Yammer::PolicyFactorySingleton TF;            \
      TF::instance()->registerPrototype(prototypicalInstance->type(),   \
                                        prototypicalInstance);          \
    }                                                                   \
  } PolicyRegistrarInstance;                                              \
  } 

}

#endif
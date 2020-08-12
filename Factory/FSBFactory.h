#ifndef _FSB_POLICY_FACTORY_H_
#define _FSB_POLICY_FACTORY_H_

#include <ace/Singleton.h> 
#include <ace/Synch.h> 
#include "Factory.h"
#include "FSBPolicy.h" 
#include <iostream>

#pragma warning( disable : 4290 )

namespace fsbfactory {

	class UnknownPolicy : public FSBPolicy {
	public:
		UnknownPolicy *clone() const { return new UnknownPolicy(*this); }

		int type() const { return fsb::UnknownFSBPolicyId; }

		std::string name() const { return "UnknownPolicy"; }

	};

	class FACTORYPOLICIES_API PolicyFactory : public Factory<std::string, FSBPolicy> {
  public:
	  FSBPolicy *createPolicy(std::string &type) throw (UnknownPolicy) {
		std::cout << " Printing Policyfactory " << type << std::endl;
		printprototypeFactory();
      try { return create(type); }
	  catch (fsbfactory::UnknownType) { throw fsbfactory::UnknownPolicy(); }
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
	  std::cout << " Name in PolicyRegistrar " << prototypicalInstance->name() << std::endl;
	  get_instance_FSBpf()->registerPrototype(prototypicalInstance->name(), prototypicalInstance);
	  //std::cout << " Printing in PolicyRegistrar " << std::endl;
	  //PolicyFactorySingleton::instance()->printprototypeFactory();
	  //get_instance_FSBpf()->printprototypeFactory();
    }
  };

   FACTORYPOLICIES_API PolicyFactory* get_instance_FSBpf();

#define REGISTER_Policy(PolicyClass)                                        \
	namespace {                                                             \
   struct PolicyRegistrar {                                                \
   PolicyRegistrar() {                                                   \
   PolicyClass *prototypicalInstance = new PolicyClass;                  \
   typedef fsbfactory::PolicyFactorySingleton TF;            \
   TF::instance()->registerPrototype(prototypicalInstance->type(),   \
   prototypicalInstance);          \
   }                                                                   \
   } PolicyRegistrarInstance;                                              \
   } 

}

#endif
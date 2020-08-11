#ifndef _YAMMER_FACTORY_H_
#define _YAMMER_FACTORY_H_

#ifdef USE_STLPORT
#  include <hash_map>
#  define MY_MAP std::hash_map
#else
#  ifdef __GNUC__  // gcc
#    include <ext/hash_map>
#    define MY_MAP __gnu_cxx::hash_map
#  else
#    include <map>
#    define MY_MAP std::map
#  endif 
#endif

#include <ace/Synch.h> 

namespace Yammer {

  class UnknownType {};  // exception

  template <typename K, typename V>
  class Factory {
  public:

    typedef MY_MAP<K, V*> my_map;

    virtual ~Factory() {
      ACE_Guard<ACE_Thread_Mutex> guard(lock_);
      typedef typename my_map::iterator I;
      for (I i = prototypeMap_.begin(); i != prototypeMap_.end(); i++)
        delete i->second;
    }

    virtual V *create(K type) throw (UnknownType) {
      ACE_Guard<ACE_Thread_Mutex> guard(lock_);
      typename my_map::const_iterator i = prototypeMap_.find(type);
      if (i == prototypeMap_.end()) throw UnknownType();
      V *prototype = i->second;
      return prototype->clone();
    }

    void registerPrototype(K type, V *prototype) {
      if (prototype) {
        ACE_Guard<ACE_Thread_Mutex> guard(lock_);
        prototypeMap_[type] = prototype;
      }
    }

    void unregisterPrototype(K type) {
      ACE_Guard<ACE_Thread_Mutex> guard(lock_);
      delete prototypeMap_[type];
      prototypeMap_.erase(type);
    }
    
  private:
    my_map prototypeMap_;
    ACE_Thread_Mutex lock_;
  };

}

#undef MY_MAP

#endif


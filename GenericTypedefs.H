#ifndef __GENERIC_TYPEDEFS_H__
#define __GENERIC_TYPEDEFS_H__

#include <string>
#include <list>
#include <vector>
#include <map>
#include <sstream>
#include <datetime.h>
#if defined (__SUNPRO_CC)
#include <locale>
#endif

typedef std::list<std::string> StrList;
typedef StrList::iterator StrListItr;

typedef std::vector<std::string> StrVect;
typedef StrVect::iterator StrVectItr;

typedef std::map<std::string, std::string> StrToStrMap;
typedef StrToStrMap::iterator StrToStrMapItr;

typedef std::multimap<std::string, std::string> StrToStrMultiMap;
typedef StrToStrMultiMap::iterator StrToStrMultiMapItr;

typedef std::string::const_iterator strIter;

typedef std::map<std::string, StrList> StrToListMap;
typedef StrToListMap::iterator StrToListMapItr;
typedef StrToListMap::value_type StrToListMapValue;

typedef std::vector<double> DoubleVect;
typedef DoubleVect::iterator DoubleVectItr;

//class specialization for sorting pointers as well
//  NOTE: cannot be used on std::lists as they do not support
//template member functions in the forte 6.2 compiler

template <class T>
struct isEqual : std::binary_function<T*, T*, bool>
{
public:
  bool operator () (const T* t1, const T* t2) const { return *t1 == *t2;}
};

template <class C, typename T>
class SortBy
{
public:
     typedef T (C::*accessor)() const;

     SortBy(accessor a) : a_(a) { }

     bool operator()(const C& p, const C& q)
     {
         return (p.*a_)() < (q.*a_)();
     }

private:
     accessor a_;
};

template <class C, typename T>
class SortBy <C*,T>
{
public:
     typedef T (C::*accessor)() const;

     SortBy(accessor a) : a_(a) { }

     bool operator()(const C* p, const C* q)
     {
         return (p->*a_)() < (q->*a_)();
     }

private:
     accessor a_;
};

template <class C, typename T>
class EqualTo
{
public:
     typedef T (C::*accessor)() const;

     EqualTo(accessor a, C elem) : a_(a), elem_(elem) { }

     bool operator()(const C& p)
     {
         return (p.*a_)() == (elem_.*a_)();
     }

private:
     accessor a_;
     const C elem_;
};

template <class C, typename T>
class EqualTo <C*,T>
{
public:
     typedef T (C::*accessor)() const;

     EqualTo(accessor a, C* elem) : a_(a), elem_(elem) { }

     bool operator()(const C* p) 
     {
         return (p->*a_)() == (elem_->*a_)();
     }

private:
     accessor a_;
     const C* elem_;
};


//class to delete self for use when clearing arrays of ptrs
template <class T>
struct DeletePtr : public std::unary_function<T*,void>
{
  void operator() (T* t)
  {
    if (t != static_cast<T*>(0))
      delete t;
  }
};

//string hash func
struct StringHash
{
  unsigned long operator() (const std::string& x) const
  {
#if 0
    std::locale loc;
    const std::collate<char>& col = std::use_facet(loc, (std::collate<char>*)0);
    return col.hash(x.data(), x.data()+x.size());
#else
    unsigned long h = 0;
    for (int i = 0; i < x.length(); i++)
      h = 31 * h + x[i];
    return h;
#endif
  }
};



#endif

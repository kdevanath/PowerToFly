#ifndef _FSB_Policy_H_
#define _FSB_Policy_H_

#include <string>

namespace fsb {

  using std::string;

  // A serializable Policy interface
  class Policy {
  public:
    virtual ~Policy() {}

    virtual Policy *clone() const = 0;  // Prototype pattern

    virtual int type() const = 0;

    virtual string name() const = 0;
  };


  // for ordering in a hashed container, or run-time type identification
  inline bool operator==(const Policy &lhs, const Policy &rhs)
    { return lhs.type() == rhs.type(); }

  inline bool operator!=(const Policy &lhs, const Policy &rhs)
    { return !(lhs == rhs); }

  // for ordering in a std (tree-based) container
  inline bool operator<(const Policy &lhs, const Policy &rhs)
    { return lhs.type() < rhs.type(); }

}

#endif
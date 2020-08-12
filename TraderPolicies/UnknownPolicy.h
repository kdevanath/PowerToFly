#ifndef _FSB_UNKNOWN_Policy_H_
#define _FSB_UNKNOWN_Policy_H_

#include "Policy.h"
#include "PolicyIds.h"

namespace fsb {

  class UnknownPolicy : public Policy {
  public:
    UnknownPolicy *clone() const { return new UnknownPolicy(*this); }

	int type() const { return fsb::UnknownPolicyId; }

    string name() const { return "UnknownPolicy"; }

	virtual double calc() { return 0.0;}

  };
}

#endif
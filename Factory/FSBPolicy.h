#ifndef __FSB_POLICY_H_FACTORY_
#define __FSB_POLICY_H_FACTORY_

#include <Utilities/FSBMsgsId.h>
#include "FactoryExport.h"
#include <string>

namespace fsbfactory {

	class FACTORYPOLICIES_API FSBPolicy
	{
	public:
		virtual ~FSBPolicy() {}

		virtual FSBPolicy *clone() const = 0;  // Prototype pattern

		virtual int type() const = 0;

		virtual std::string name() const = 0;
	};


	// for ordering in a hashed container, or run-time type identification
	inline bool operator==(const FSBPolicy &lhs, const FSBPolicy &rhs)
	{ return lhs.type() == rhs.type(); }

	inline bool operator!=(const FSBPolicy &lhs, const FSBPolicy &rhs)
	{ return !(lhs == rhs); }

	// for ordering in a std (tree-based) container
	inline bool operator<(const FSBPolicy &lhs, const FSBPolicy &rhs)
	{ return lhs.type() < rhs.type(); }
}

#endif
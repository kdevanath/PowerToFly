#include "stdafx.h"
#include "PolicyFactory.h"

namespace fsb {
PolicyFactory* get_instance_pf()
{ 
	return PolicyFactorySingleton::instance();
}
}
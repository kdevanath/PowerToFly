#include "stdafx.h"
#include "FSBFactory.h"

namespace fsbfactory {

	PolicyFactory* get_instance_FSBpf()
	{ 
		return PolicyFactorySingleton::instance();
	}

}


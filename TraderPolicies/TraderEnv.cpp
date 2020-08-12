#include "stdafx.h"
#include "TraderEnv.h"

namespace fsb {
	TraderEnv* get_trader_env_instance()
	{
		return TraderEnvSingleton::instance();
	}
}
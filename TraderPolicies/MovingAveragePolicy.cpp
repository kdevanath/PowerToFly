#include "stdafx.h"
#include "PolicyFactory.h"
#include "MovingAveragePolicy.h"
#include "TraderEnv.h"
namespace fsb { 

	DPRatio::DPRatio(const std::string& ptype)
	{
		_priceType = dynamic_cast<PriceTypePolicy*>(fsb::PolicyFactorySingleton::instance()->create(ptype));
	}

	DPSingle::DPSingle(const std::string& ptype)
	{
		priceType = dynamic_cast<PriceTypePolicy*>(fsb::PolicyFactorySingleton::instance()->create(ptype));
	}

	DPMultiply::DPMultiply(const std::string& ptype)
	{
		_priceType = dynamic_cast<PriceTypePolicy*>(fsb::PolicyFactorySingleton::instance()->create(ptype));
	}

	DPReturn::DPReturn(const std::string& ptype)
	{
		_priceType = dynamic_cast<PriceTypePolicy*>(fsb::PolicyFactorySingleton::instance()->create(ptype));
	}

	double CEFma::operator () (const double& b, const double& h)
	{
		float ratio = TraderEnvSingleton::instance()->_params.mixRatio();
		ratio = ratio <= 0.0 ? 1.0 : ratio;
		return (b - (h* ratio));
	}
}

fsb::PolicyRegistrar<fsb::MAMultiply> MAMultiplyPrototype;
fsb::PolicyRegistrar<fsb::MARatio> MARatioPrototype;
fsb::PolicyRegistrar<fsb::MAReturn> MAReturnPrototype;
fsb::PolicyRegistrar<fsb::CEFma> MACEFPrototype;
fsb::PolicyRegistrar<fsb::BidPricePolicy> BidPricePolicyPrototype;
fsb::PolicyRegistrar<fsb::AskPricePolicy> AskPricePolicyPrototype;
fsb::PolicyRegistrar<fsb::LastPricePolicy> LastPricePolicyPrototype;
fsb::PolicyRegistrar<fsb::MidPricePolicy> MidPricePolicyPrototype;

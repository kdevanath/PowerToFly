#include "stdafx.h"
#include "OrderPricePolicy.h"
#include <Utilities\genericfns.h>
#include "PolicyFactory.h"

namespace fsb {

double JoinPricePolicy::calcPrice(Instrument* i,bool buy, double theoPrice)
{
	double current =  buy ? i->bid() : i->ask();
	if( current <=0.0 ) return 0.0;
	if( buy )
		return theoPrice >= current ?  current : theoPrice;
	else
		return theoPrice <= current ?  current : theoPrice;
}

double PlusPricePolicy::calcPrice(Instrument* i,bool buy, double theoPrice)
{
	double current = 0.0;
	if( (i->ask() - i->bid()) >= 2* i->ticksize() )
		current =  buy ? (i->bid() + i->ticksize()) : (i->ask() - i->ticksize());
	else
		current =  buy ? i->bid() : i->ask();

	if( current <= 0.0 ) return 0.0;
	if( buy )
		return theoPrice >= current ?  current : theoPrice;
	else
		return theoPrice <= current ?  current : theoPrice;
		
}

double CrossPricePolicy::calcPrice(Instrument* i,bool buy, double theoPrice)
{
	double current =  buy ? i->ask() : i->bid();
	if( current <= 0.0 ) return 0.0;
	if( theoPrice <= 0.0) return current;
	if( buy )
		return theoPrice >= current ?  current : theoPrice;
	else
		return theoPrice <= current ?  current : theoPrice;
	
}

double AntiSqzPricePolicy::calcPrice(Instrument* i,bool buy, double theoPrice)
{
	double current =  buy ? (i->ask() - i->ticksize()) : (i->bid() + i->ticksize());
	if( current <= 0.0 ) return 0.0;
	if( buy )
		return theoPrice >= current ?  current : theoPrice;
	else
		return theoPrice <= current ?  current : theoPrice;
		
}

double RoundNearestPolicy::round(const double& fsbticksize,bool buy, const double& theoPrice)
{
	if( !fsbticksize) return 0.0;
	if( buy)
		return fsbutils::RoundingFunctions::RoundUpPrice(fsbticksize,theoPrice);
	else
		return fsbutils::RoundingFunctions::RoundDownPrice(fsbticksize,theoPrice);
}

double RoundAwayPolicy::round(const double& fsbticksize,bool buy, const double& theoPrice)
{
	if( !fsbticksize) return 0.0;
	if( buy)
		return fsbutils::RoundingFunctions::RoundDownPrice(fsbticksize,theoPrice);
	else
		return fsbutils::RoundingFunctions::RoundUpPrice(fsbticksize,theoPrice);
}

}
fsb::PolicyRegistrar<fsb::JoinPricePolicy> JoinPricePolicyPrototype; 
fsb::PolicyRegistrar<fsb::PlusPricePolicy> PlusPricePolicyPrototype; 
fsb::PolicyRegistrar<fsb::RoundNearestPolicy> RoundNearestPolicyPrototype; 
fsb::PolicyRegistrar<fsb::RoundAwayPolicy> RoundAwayPolicyPrototype;
fsb::PolicyRegistrar<fsb::CrossPricePolicy> CrossPricePolicyPrototype;

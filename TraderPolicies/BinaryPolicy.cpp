#include "stdafx.h"

#include "PolicyFactory.h"
#include "BinaryPolicy.h"
#include "TraderEnv.h"

namespace fsb {

	bool MidThreshold::BuySignal(const double& baseAsk, const double& fvBid)
	{
		return baseAsk <= fvBid ? true : false;
	}

	bool MidThreshold::SellSignal(const double& baseBid, const double& fvAsk)
	{
		return baseBid >= fvAsk ? true : false;
	}

	int MidThreshold::buySellCondition(const Instrument& i, const double& bidFv, const double& askFv)
	{
		double mid = (i.askNoLock() + i.bidNoLock())/2.0;

		if( mid >= askFv ) return SELL;

		if( mid <= bidFv) return BUY;

		return NO_CHANGE;
	}

	bool CrossThreshold::BuySignal(const double& baseAsk, const double& fvBid)
	{
		return baseAsk <= fvBid ? true : false;
	}

	bool CrossThreshold::SellSignal(const double& baseBid, const double& fvAsk)
	{
		return baseBid >= fvAsk ? true : false;
	}

	int CrossThreshold::buySellCondition(const Instrument& i, const double& bidFv, const double& askFv)
	{
		if( i.askNoLock() <= bidFv ) return BUY;

		if( i.bidNoLock() >= askFv ) return SELL;
		return NO_CHANGE;
	}

	bool JoinThreshold::BuySignal(const double& baseBid, const double& fvBid)
	{
		return baseBid <= fvBid ? true : false;
	}

	bool JoinThreshold::SellSignal(const double& baseAsk, const double& fvAsk)
	{
		return baseAsk >= fvAsk ? true : false;
	}

	int JoinThreshold::buySellCondition(const Instrument& i, const double& bidFv, const double& askFv)
	{

		if( i.bidNoLock() && i.bidNoLock() <= bidFv) return BUY;

		if( i.askNoLock() && i.askNoLock() >= askFv ) return SELL;

		return NO_CHANGE;
	}

	int CompareThreshold::buySellCondition(const Instrument& i, const double& bidFv, const double& askFv)
	{
		if( (askFv - i.askNoLock()) < (i.bidNoLock() - bidFv ) )
			return SELL;
		else
			return BUY;
	}

	int MMKRBSFlag::buySellCondition(const fsb::Instrument &i, const double &bidFv, const double &askFv)
	{
		int bsflag=BinaryPolicy::NONE;
		double deltathresh = TraderEnvSingleton::instance()->_params.deltaThreshold();
		if( deltathresh <= 0 ) {
			FSB_LOG(" Algo. does not know delta threshold number");
			return bsflag;
		}

		double totaldelta = TraderEnvSingleton::instance()->_totalPos._delta;
		
		if( totaldelta < deltathresh)
			bsflag |= BinaryPolicy::BUY;
		if(totaldelta > (deltathresh*-1))
			bsflag |= BinaryPolicy::SELL;
		FSB_LOG(" Total: " << totaldelta << " thresh: " << deltathresh << " " << bsflag );
		return bsflag;
	}
}
fsb::PolicyRegistrar<fsb::JoinThreshold> JoinThresholdPolicyPrototype;
fsb::PolicyRegistrar<fsb::CrossThreshold> CrossThresholdPolicyPrototype;
fsb::PolicyRegistrar<fsb::MidThreshold> MidThresholdPolicyPrototype;
fsb::PolicyRegistrar<fsb::CompareThreshold> CompareThresholdPolicyPrototype;
fsb::PolicyRegistrar<fsb::NoBSThreshold> NoBSThresholdPolicyPrototype;

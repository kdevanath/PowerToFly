#ifndef __BINARY_POLICY_H__
#define __BINARY_POLICY_H__

#include "Policy.h"
#include "TraderPolicyExports.h"
#include "PolicyIds.h"
#include <Data/Instruments.h>

namespace fsb {

	class BinaryPolicy : public Policy
	{
	public:
		enum BS_FLAG { ZERO_QUOTES=0,BUY=1,SELL=2,NO_CHANGE=4,BOTH=8,NONE=16};
		virtual bool BuySignal(const double& baseBid, const double& fv) = 0;
		virtual bool SellSignal(const double& baseAsk, const double& fv) = 0;
		virtual int  buySellCondition(const Instrument& i, const double& bidFv, const double& askFV)=0;
	};

	class MidThreshold : public BinaryPolicy
	{
	public:
		MidThreshold() {}

		virtual ~MidThreshold() {}

		int type() const { return fsb::MidThresholdId;}
		std::string name() const { return "MidThreshold";}

		MidThreshold *clone() const { return new MidThreshold(*this);}

		bool BuySignal(const double& baseAsk, const double& fvBid); //{ return baseAsk <= fvBid ? true : false;}
		bool SellSignal(const double& baseBid, const double& fvAsk);// { return baseBid >= fvAsk ? true : false;} 
		virtual int  buySellCondition(const Instrument& i, const double& bidFv, const double& askFV);
	};

	class CrossThreshold : public BinaryPolicy
	{
	public:
		CrossThreshold() {}

		virtual ~CrossThreshold() {}

		int type() const { return fsb::CrossThresholdId;}
		std::string name() const { return "CrossThreshold";}

		CrossThreshold *clone() const { return new CrossThreshold(*this);}

		bool BuySignal(const double& baseAsk, const double& fvBid); //{ return baseAsk <= fvBid ? true : false;}

		bool SellSignal(const double& baseBid, const double& fvAsk);// { return baseBid >= fvAsk ? true : false;} 
		virtual int  buySellCondition( const Instrument& i, const double& bidFv, const double& askFV);
	};

	class  JoinThreshold : public BinaryPolicy
	{
	public:
		JoinThreshold() {}

		virtual ~JoinThreshold() {}

		int type() const { return fsb::JoinThresholdId;}
		std::string name() const { return "JoinThreshold";}

		JoinThreshold *clone() const { return new JoinThreshold(*this);}

		bool BuySignal(const double& baseBid, const double& fvBid); /*{ return baseBid <= fvBid ? true : false; } */
		bool SellSignal(const double& baseAsk, const double& fvAsk); /*{ return baseAsk >= fvAsk ? true : false; }*/ 
		virtual int  buySellCondition(const Instrument& i, const double& bidFv, const double& askFV);
	};

	class  CompareThreshold : public BinaryPolicy
	{
	public:
		CompareThreshold() {}

		virtual ~CompareThreshold() {}

		int type() const { return fsb::CompareThresholdId;}
		std::string name() const { return "CompareThreshold";}

		CompareThreshold *clone() const { return new CompareThreshold(*this);}

		bool BuySignal(const double& baseBid, const double& fvBid) { return false; }
		bool SellSignal(const double& baseAsk, const double& fvAsk) { return false; }
		virtual int  buySellCondition(const Instrument& i, const double& bidFv, const double& askFV);
	};

	class  NoBSThreshold : public BinaryPolicy
	{
	public:
		NoBSThreshold() {}

		virtual ~NoBSThreshold() {}

		int type() const { return fsb::NoBSThresholdId;}
		std::string name() const { return "NoBSThreshold";}

		NoBSThreshold *clone() const { return new NoBSThreshold(*this);}

		bool BuySignal(const double& baseBid, const double& fvBid) { return false; }
		bool SellSignal(const double& baseAsk, const double& fvAsk) { return false; }
		virtual int  buySellCondition(const Instrument& i, const double& bidFv, const double& askFV){ return BOTH;}
	};

	class EmptyBSFlag
	{
		int buySellCondition(const Instrument& i, const double& bidFv, const double& askFv) { return BinaryPolicy::NONE;}
	};
	class MMKRBSFlag
	{
	public:
		int buySellCondition(const Instrument& i, const double& bidFv, const double& askFv);

	};
}
#endif
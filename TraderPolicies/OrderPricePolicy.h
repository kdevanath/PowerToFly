#ifndef _ORDER_PRICE_POLICY
#define _ORDER_PRICE_POLICY

#include "Policy.h"
#include "PolicyIds.h"
#include <Data/Instruments.h>
#include "TraderPolicyExports.h"

namespace fsb {
	class  OrderPricePolicy : public Policy
	{
	public:
		virtual double calcPrice(Instrument*, bool,double theoPrice)=0;
	};
	class RoundingPolicy : public Policy
	{
	public:
		virtual double round(const double& ticksize, bool,const double& theoPrice)=0;
	};
	//Abstract class in charge of getting a price for a given symbol
	class TRADERPOLICIES_API JoinPricePolicy : public OrderPricePolicy
	{
	public:
		JoinPricePolicy() {}

		virtual ~JoinPricePolicy() {}

		int type() const { return fsb::JoinPricePolicyId;}
		std::string name() const { return "JOIN";}

		JoinPricePolicy *clone() const { return new JoinPricePolicy(*this);}

		virtual double calcPrice(Instrument*, bool,double theoPrice);

	};

	class TRADERPOLICIES_API PlusPricePolicy : public OrderPricePolicy
	{
	public:
		PlusPricePolicy() {}

		virtual ~PlusPricePolicy() {}

		int type() const { return fsb::PlusPricePolicyId;}
		std::string name() const { return "SQZ";}

		PlusPricePolicy *clone() const { return new PlusPricePolicy(*this);}

		double calcPrice(Instrument*, bool,double theoPrice);

	};

	class TRADERPOLICIES_API CrossPricePolicy : public OrderPricePolicy
	{
	public:
		CrossPricePolicy() {}

		virtual ~CrossPricePolicy() {}

		int type() const { return fsb::CrossPricePolicyId;}
		std::string name() const { return "CROSS";}

		CrossPricePolicy *clone() const { return new CrossPricePolicy(*this);}

		double calcPrice(Instrument*, bool,double theoPrice);

	};

	class TRADERPOLICIES_API AntiSqzPricePolicy : public OrderPricePolicy
	{
	public:
		AntiSqzPricePolicy() {}

		virtual ~AntiSqzPricePolicy() {}

		int type() const { return fsb::AntiSqzPricePolicyId;}
		std::string name() const { return "ASQZ";}

		AntiSqzPricePolicy *clone() const { return new AntiSqzPricePolicy(*this);}

		double calcPrice(Instrument*, bool,double theoPrice);

	};

	class TRADERPOLICIES_API RoundNearestPolicy : public RoundingPolicy
	{
	public:
		RoundNearestPolicy() {}

		virtual ~RoundNearestPolicy() {}

		int type() const { return fsb::RoundNearestPolicyId;}
		std::string name() const { return "RoundNearestPolicy";}

		RoundNearestPolicy *clone() const { return new RoundNearestPolicy(*this);}

		double round(const double&, bool,const double& theoPrice);

	};

	class TRADERPOLICIES_API RoundAwayPolicy : public RoundingPolicy
	{
	public:
		RoundAwayPolicy() {}

		virtual ~RoundAwayPolicy() {}

		int type() const { return fsb::RoundAwayPolicyId;}
		std::string name() const { return "RoundAwayPolicy";}

		RoundAwayPolicy *clone() const { return new RoundAwayPolicy(*this);}

		double round(const double&, bool,const double& theoPrice);

	};

}
#endif
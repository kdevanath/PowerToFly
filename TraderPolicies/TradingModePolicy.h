#ifndef __LIQUIDITY_POLICY_H__
#define __LIQUIDITY_POLICY_H__

#include "OrderPricePolicy.h"

namespace fsb {

	class HedgePriceMode
	{
	public:
		HedgePriceMode();
	public:
		void calcFV(Instrument& instr,double& theoBid, double& theoAsk){ return;}
		//bool calcPrice(Instrument& instr, bool buyFlag,double& fv,double& sendPrice);
		double calcPrice(const Instrument& instr,double&,double&,bool);
	private:
		RoundingPolicy* _roundingPolicy;
	};

	class TakeLiqPriceMode
	{
	public:
		TakeLiqPriceMode();
	protected:	
		void calcFV(Instrument& instr,double& theoBid, double& theoAsk){ return;}
		bool calcPrice(Instrument& instr, bool buyFlag,double& fv,double& sendPrice);
		
	private:
		OrderPricePolicy* _pricePolicy;
	};

	class MMKRWorkLiqPriceMode
	{
	public:
		MMKRWorkLiqPriceMode();
	protected:
		void calcFV(Instrument& instr,double& theoBid, double& theoAsk){ return;}
		bool calcPrice(Instrument& instr, bool buyFlag,double& fv,double& sendPrice);
	private:
		OrderPricePolicy* _pricePolicy;
		RoundingPolicy* _roundPolicy;
	};

	class SpiderWorkLiqPriceMode
	{
	public:
		SpiderWorkLiqPriceMode();
	protected:
		void calcFV(Instrument& instr,double& theoBid, double& theoAsk){ return;}
		bool calcPrice(Instrument& instr, bool buyFlag,double& fv,double& sendPrice);
	private:
		OrderPricePolicy* _pricePolicy;
		RoundingPolicy* _roundPolicy;
	};

	class CloseOnlyPriceMode
	{
	public:
		CloseOnlyPriceMode() { }
	public:
		void calcFV(Instrument& instr,double& theoBid, double& theoAsk);
	};

	class WorkLiqPriceMode
	{
	public:
		WorkLiqPriceMode();
	protected:
		void calcFV(Instrument& instr,double& theoBid, double& theoAsk){ return;}
		bool calcPrice(Instrument& instr, bool buyFlag,double& fv,double& sendPrice);
	private:
		OrderPricePolicy* _pricePolicy;
		RoundingPolicy* _roundPolicy;
	};

	class ProspectPriceMode
	{
	public:
		ProspectPriceMode();
	protected:
		void calcFV(Instrument& instr,double& theoBid, double& theoAsk){ return;}
		bool calcPrice(Instrument& instr, bool buyFlag,double& fv,double& sendPrice);
	private:
	};

	class CrossPriceMode
	{
	public:
		CrossPriceMode(){}
	protected:
		void calcFV(Instrument& instr,double& theoBid, double& theoAsk){ return;}
		bool calcPrice(Instrument& instr, bool buyFlag,double& fv,double& sendPrice);
	private:
	};
}
#endif
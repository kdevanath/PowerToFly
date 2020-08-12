#ifndef __MOVING_AVERAGE_POLICY__
#define __MOVING_AVERAGE_POLICY__

#include <Data/Instruments.h>
#include "Policy.h"
#include "PolicyIds.h"

namespace fsb {

	class PriceTypePolicy : public Policy
	{
	public:
		virtual double operator() (Instrument::VectorOfInstrPtrs&)=0;
		virtual double operator() (const Instrument& i)=0;
	};

	class BidPricePolicy : public PriceTypePolicy
	{
	public:
		int type() const { return fsb::BidPriceId;}
		std::string name() const { return "BidPrice";}

		BidPricePolicy *clone() const { return new BidPricePolicy(*this);}

		double operator() (Instrument::VectorOfInstrPtrs& instrs) 
		{ 
			double bid = 0.0;
			Instrument::InstrPtrsIterator itr = instrs.begin();
			for(itr; itr != instrs.end(); ++itr) {
				if( (*itr)->bid() <= 0 ) return bid;
				bid += ( (*itr)->bid() * (*itr)->factor());
			}
			return bid;
		}

		double operator() (const Instrument& i) {return const_cast<Instrument&>(i).bid(); }
	};

	class AskPricePolicy : public PriceTypePolicy
	{
	public:

		int type() const { return fsb::AskPriceId;}
		std::string name() const { return "AskPrice";}

		AskPricePolicy *clone() const { return new AskPricePolicy(*this);}

		double operator() (const Instrument& i) {return const_cast<Instrument&>(i).ask(); }

		double operator() (Instrument::VectorOfInstrPtrs& instrs) 
		{ 
			double ask = 0.0;
			Instrument::InstrPtrsIterator itr = instrs.begin();
			for(itr; itr != instrs.end(); ++itr) {
				if( (*itr)->ask() <= 0 ) return ask;
				ask += ( (*itr)->ask() * (*itr)->factor());
			}
			return ask;
		}
	};
	class LastPricePolicy : public PriceTypePolicy
	{
	public:

		int type() const { return fsb::LastPriceId;}
		std::string name() const { return "LastPrice";}

		LastPricePolicy *clone() const { return new LastPricePolicy(*this);}

		double operator() (const Instrument& i) { return const_cast<Instrument&>(i).last();}
		double operator() (Instrument::VectorOfInstrPtrs& instrs) 
		{ 
			double last = 0.0;
			Instrument::InstrPtrsIterator itr = instrs.begin();
			for(itr; itr != instrs.end(); ++itr) {
				if( (*itr)->last() <= 0 ) return last;
				last += ( (*itr)->last() * (*itr)->factor());
			}
			return last;
		}
	};
	class MidPricePolicy : public PriceTypePolicy
	{
	public:

		int type() const { return fsb::MidPriceId;}
		std::string name() const { return "MidPrice";}

		MidPricePolicy *clone() const { return new MidPricePolicy(*this);}

		double operator() (const Instrument& i) {return const_cast<Instrument&>(i).mid();}
		double operator() (Instrument::VectorOfInstrPtrs& instrs) 
		{ 
			double bid = 0.0, ask = 0.0;
			Instrument::InstrPtrsIterator itr = instrs.begin();
			for(itr; itr != instrs.end(); ++itr)
			{
				if( (*itr)->bid() <= 0 || (*itr)->ask() <= 0 ) return 0.0;

				bid += ( (*itr)->bid()* (*itr)->factor());
				ask += ( (*itr)->ask()* (*itr)->factor());
			}

			if( bid > 0.0 && ask > 0.0)
				return ((bid + ask)/2.0);

			return 0.0;
		}

	};

	class DPSingle
	{
	public:
		DPSingle(const std::string& ptype);
		double dp0(const Instrument& i) { return ((*priceType)(i));}
	protected:
		PriceTypePolicy* priceType;
	};

	class DPRatio
	{
	public:
		DPRatio(const std::string& ptype);
		double dp0(const Instrument& i1,const Instrument& i2) { 
			return( ((*_priceType)(i1))/((*_priceType)(i2)) );
		}
	protected:
		PriceTypePolicy* _priceType;
	};

	class DPReturn
	{
	public:
		DPReturn(const std::string& ptype);
		double dp0(const Instrument& i1,const Instrument& i2) { 
			return( ((*_priceType)(i1)) - ((*_priceType)(i2)) );
		}
	protected:
		PriceTypePolicy* _priceType;
	};

	class DPMultiply
	{
	public:
		DPMultiply(const std::string& ptype);
		double dp0(const Instrument& i1,const Instrument& i2) { 
			return( ((*_priceType)(i1)) * ((*_priceType)(i2)) );
		}
	protected:
		PriceTypePolicy* _priceType;
	};


	class MATypePolicy : public Policy
	{
	public:
		virtual double operator () (const double& b, const double& h)=0;
	protected:
	};

	class MARatio : public MATypePolicy
	{
	public:

		int type() const { return fsb::MARatioId;}
		std::string name() const { return "MARatio";}

		MARatio *clone() const { return new MARatio(*this);}

		virtual double operator ()(const double& b, const double& h){ return h>0.0 ? b / h : 0.0;}
	};

	class CEFma : public MATypePolicy
	{
	public:
		int type() const { return fsb::CEFmaId;}
		std::string name() const { return "CEF";}

		CEFma *clone() const { return new CEFma(*this);}

		virtual double operator ()(const double& b, const double& h);
	};

	class MAReturn : public MATypePolicy
	{
	public:
		int type() const { return fsb::MAReturnId;}
		std::string name() const { return "MAReturn";}

		MAReturn *clone() const { return new MAReturn(*this);}

		virtual double operator ()(const double& b, const double& h){ return (b - h);}
	};

	class MAMultiply : public MATypePolicy
	{
	public:
		int type() const { return fsb::MAMultiplyId;}
		std::string name() const { return "MAMultiply";}

		MAMultiply *clone() const { return new MAMultiply(*this);}

		virtual double operator ()(const double& b, const double& h) { return ( b * h ); }
	}; 

	template <class dppolicy>
	class DP : private dppolicy
	{
	public:
		DP(const std::string& ptype)
			:dppolicy(ptype)
		{
		}
		double calculate(const Instrument& b, const Instrument& h){ return this->dp0(b,h); }
	};

	template <>
	class DP<DPSingle> : private DPSingle
	{
	public:
		DP(const std::string& ptype)
			:DPSingle(ptype)
		{
		}
		double calculate(const Instrument& i){ return this->dp0(i); };
	};

	class MAPolicy
	{
	public:
		MAPolicy()
		{
		}
		virtual double calculate(int,int,time_t)=0;
		virtual double datapoint0()=0;
		virtual double ma()=0;
		virtual double ma(const Instrument&) { return 0.0;}
		virtual double previous() const { return 0.0;}
		virtual void previous(const double&)=0;
		virtual void print() { }
	protected:	
	};
}
#endif
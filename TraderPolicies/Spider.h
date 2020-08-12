#ifndef __FSB_SPIDER_H_
#define __FSB_SPIDER_H_

#include <TraderPolicies/TraderEnv.h>
#include <Utilities/genericfns.h>
#include "TraderPolicyExports.h"
#include <Utilities/FSBLogger.h>
#include <Utilities/genericfns.h>
#include "PolicyFactory.h"

namespace fsb {

	class SpiderModel 
	{
	public:
		SpiderModel()
			:_basketMid(0.),
			_basketAsk(0.),
			_basketBid(0.),
			_basis(0.),
			_ma(0.)
			{ }

		void calc(double& basketbid,
			double& basketAsk,
			const Instrument::VectorOfInstrs& bases,
			const Instrument::VectorOfInstrs& hedges)
		{
			/*_basketAsk = _basketBid = 0.0;
			for(Instrument::VectorOfInstrs::const_iterator i = bases.begin(); i!= bases.end(); ++i) {
			std::cout << i->symbol() << " " << i->bidNoLock() << "X" << i->askNoLock() << " " <<  i->weight() << std::endl;
			_basketBid += (i->bidNoLock() * (double)i->weight());
			_basketAsk += (i->askNoLock() * (double)i->weight());
			}		
			std::cout << " " << _basketBid << "X" << _basketAsk << std::endl;*/
			basketbid = _basketBid = for_each(bases.begin(),bases.end(),fsbutils::Sum<Instrument,double>(&Instrument::weightedBidNoLock,0.0));
			basketAsk = _basketAsk = for_each(bases.begin(),bases.end(),fsbutils::Sum<Instrument,double>(&Instrument::weightedAskNoLock,0.0));
			_basketMid = (_basketBid + _basketAsk)/2.0;
			_basis = hedges[0].midNoLock() - _basketMid;
			TraderEnvSingleton::instance()->_basket.ask(_basketAsk);
			TraderEnvSingleton::instance()->_basket.bid(_basketBid);
			TraderEnvSingleton::instance()->_basket.basis(_basis);	
			/*time_t ts; 
			time(&ts);
			TraderEnvSingleton::instance()->_basket.timestamp(ts);*/
			_width = (TraderEnvSingleton::instance()->_params.width()/2.0)/10000.0;
			_ma = TraderEnvSingleton::instance()->_longTermMA->ma();
			TraderEnvSingleton::instance()->_basket.ma1(_ma);
			TraderEnvSingleton::instance()->_basket.ma2(_ma);
			TraderEnvSingleton::instance()->_basket.dp0(_basis);
			FSB_LOG(" " << _basketBid
				<< "X" << _basketAsk
				<< " " << hedges[0].midNoLock()
				<< " " << _basis);
		}
		void calc(double& theoBid,
			double& theoAsk,
			const Instrument& instr1,
			const Instrument& instr2)
		{		
			if(_ma == 0.0) {
				return;
			}
			double ticksize = instr1.ticksize();
			int bsize = instr1.bidSizeNoLock();
			int asize = instr1.askSizeNoLock();
			double bid = instr1.bidNoLock();
			double ask = instr1.askNoLock();
			//double mid = ((bid * asize) + (ask * bsize))/(bsize + asize);
			double mid = (bid + ask)/2.0;
			double hedgeBid = instr2.bidNoLock();
			double hedgeAsk = instr2.askNoLock();
			double hedgeMid = instr2.midNoLock();
			double pennyDelta = mid/hedgeMid;
			int index = instr1.index();
			int ourBidSize = (*TraderEnvSingleton::instance()->_instrObjItrs[index])->_orders.qtyPendingBuys();
			int ourAskSize = (*TraderEnvSingleton::instance()->_instrObjItrs[index])->_orders.qtyPendingSells();
			double bidfader = abs(bsize - ourBidSize) > 5000 ? 0.0 : 0.005;
			double askfader = abs(asize - ourAskSize) > 5000 ? 0.0 : 0.005;
			double fvBid = mid + (((hedgeBid - _basketMid) - _ma)*pennyDelta ) - 0.0015 - _width ;
			fvBid -= bidfader;
			double fvAsk = mid + (((hedgeAsk - _basketMid) - _ma)*pennyDelta) + 0.0015 + _width + askfader;
			fvAsk += askfader;
			theoBid = fsbutils::RoundingFunctions::RoundDownPrice(ticksize,fvBid);
			theoAsk = fsbutils::RoundingFunctions::RoundUpPrice(ticksize,fvAsk);
			FSB_LOG( " " << instr1.symbol()
				<< " " << bsize
				<< " " << bid
				<< "X" << ask
				<< " " << asize
				<< "Mid " << mid
				<< " H " << hedgeBid
				<< "X" << hedgeAsk
				<< " B " << _basketBid
				<< "X" << _basketAsk
				<< " " << bidfader
				<< " " << ourBidSize
				<< " " << fvBid
				<< "X" << fvAsk
				<< " " << ourAskSize
				<< " " << askfader
				<< " RFV " << theoBid
				<< "X" << theoAsk
				<< " Ma " << _ma
				<< " Pd " << pennyDelta
				<< " W " << _width);
		}
	private:
		double _ma,_width,_basketBid,_basketAsk,_basis,_basketMid;
	};
}
#endif
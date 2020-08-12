#ifndef __CEF__FV_H__
#define __CEF__FV_H__

#include "FVPolicy.h"

namespace fsb {

	class CEFBaseFV : public PTBaseFV
	{
	public:
		void calc(double& theoBid,
			double& theoAsk,
			const Instrument& base, 
			const Instrument& hedge)
		{
			//double skewInBps = TraderEnvSingleton::instance()->_params.skew();
			//double widthInBps = TraderEnvSingleton::instance()->_params.width()/2.0;
			theoBid = theoAsk = 0.0;
			double sizeAdj = TraderEnvSingleton::instance()->_params.sizeAdjustment();
			int minHdgSizeReq = TraderEnvSingleton::instance()->_params.minHedgeSize();
			//float bump = TraderEnvSingleton::instance()->_params.bump();
			//double fsbticksz = base.fsbticksize();

			int hedgeBidSize = hedge.bidSizeNoLock();
			int  hedgeAskSize = hedge.askSizeNoLock();
			double baseBid = base.bidNoLock();
			double baseAsk = base.askNoLock();
			double hedgeBid = hedge.bidNoLock();
			double hedgeAsk = hedge.askNoLock();
			double hedgeMid = hedge.midNoLock();
			
			 double bidSizeAdj = hedgeBidSize < minHdgSizeReq ? sizeAdj : 0.0; 
			 double askSizeAdj = hedgeAskSize < minHdgSizeReq ? sizeAdj : 0.0;

			double skewAsk = 0.0, skewBid = 0.0;

			if( !calcSKEW_WIDTH(skewBid,skewAsk)) {
				TraderEnvSingleton::instance()->_basket.ask(theoAsk);
				TraderEnvSingleton::instance()->_basket.bid(theoBid);
				TraderEnvSingleton::instance()->_basket.ma1(0.0);
				TraderEnvSingleton::instance()->_basket.ma2(0.0);
				TraderEnvSingleton::instance()->_basket.dp0(0.0);
					return;
			}

			double ma1 = TraderEnvSingleton::instance()->_longTermMA->result();
			double ma2 = TraderEnvSingleton::instance()->_shortTermMA->result();
			float ratio = TraderEnvSingleton::instance()->_params.mixRatio();
			double dp0 = TraderEnvSingleton::instance()->_longTermMA->datapoint0();
			double maxMa = std::max<double>(ma1,ma2);
			double minMa = std::min<double>(ma1,ma2);
			theoAsk = (ratio * hedgeAsk) * skewAsk + askSizeAdj + maxMa ;
			theoBid = (ratio * hedgeBid) * skewBid - bidSizeAdj + minMa;
			TraderEnvSingleton::instance()->_basket.ma1(ma1);
			TraderEnvSingleton::instance()->_basket.ma2(ma2);
			TraderEnvSingleton::instance()->_basket.dp0(dp0);

			FSB_LOG( " theoBid " << theoBid 
				<< " theoAsk " << theoAsk
				<< " base bid " << baseBid
				<< " base ask " << baseAsk
				<< " Bid " << hedgeBid
				<< " Ask " << hedgeAsk
				<< " hedgeBidSize " << hedgeBidSize
				<< " hedgeAskSize " << hedgeAskSize
				<< " MA1(lonG) " << ma1
				<< " MA2(short) " << ma2
				<< " ratio " << ratio
				<< " " << bidSizeAdj
				<< " " << askSizeAdj);
		}
	};
}
#endif
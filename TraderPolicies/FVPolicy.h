#ifndef __FV_POLICY_H__
#define __FV_POLICY_H__

#include <TraderPolicies/TraderEnv.h>
#include <Utilities/genericfns.h>
#include "BinaryPolicy.h"
#include "TraderPolicyExports.h"
#include <Utilities/FSBLogger.h>
#include "PolicyFactory.h"

namespace fsb {

	class ETFSpreadModel
	{
		
	private:
		bool _checkedInverse;
		bool _inverse; // = false indicating inverse etfs
	public:
		ETFSpreadModel()
			:_checkedInverse(false),
			_inverse(false)
		{
		}

		bool checkForInverse(double baseLevRatio, double hdgLevRatio)
		{
			if(!_checkedInverse ) {
				FSB_LOG(" baseLevRatio " << baseLevRatio
					<< " hdgLevRatio " << hdgLevRatio);
				unsigned int baseSignFld = fsbutils::Sign::GetSignBit(baseLevRatio);
				unsigned int hdgSignFld = fsbutils::Sign::GetSignBit(hdgLevRatio);
				_inverse = baseSignFld != hdgSignFld ? true : false;
				FSB_LOG(" baseLevRatio " << baseLevRatio
					<< " hdgLevRatio " << hdgLevRatio
					<< " baseSignFld " << baseSignFld
					<< " hdgSignFld " << hdgSignFld
					<< " " << ( _inverse ? "Inverse " : " Not Inverse") );
				_checkedInverse = true;
			}
			return _inverse;
		}

		void calc(double& theoBid,
			double& theoAsk,
			const Instrument::VectorOfInstrs& bases,
			const Instrument::VectorOfInstrs& hedges)
		{
			return;
		}
		void calc(double& theoBid,
			double& theoAsk,
			const Instrument& base,
			const Instrument& hedge)
		{
			double skewInBps = TraderEnvSingleton::instance()->_params.skew();
			double widthInBps = (TraderEnvSingleton::instance()->_params.width()/2.0)/10000;
			float skewUnit = TraderEnvSingleton::instance()->_params.skewUnit();
			double skewInBpsPer100K = skewInBps/(10000 * skewUnit);
			double basema1 = TraderEnvSingleton::instance()->_longTermMA->result(base);
			double hedgema1 = TraderEnvSingleton::instance()->_longTermMA->result(hedge);
			float bump = TraderEnvSingleton::instance()->_params.bump();
			
			if(!_checkedInverse)
				checkForInverse(base.levRatio(), hedge.levRatio());
			int net = (*TraderEnvSingleton::instance()->_instrObjItrs[base.index()])->_orders.net();
			double baseNotional = (base.bidNoLock() + base.askNoLock())/2.0 * net;
			double fvBid= 0.0,fvAsk = 0.0,bid=0.0,ask=0.0;

			!_inverse ?  (bid = hedge.bidNoLock(),ask = hedge.askNoLock()) : (bid = hedge.askNoLock(),ask = hedge.bidNoLock()) ;
			fvBid = (((bid/hedgema1 - 1) * (base.levRatio()/hedge.levRatio())) + 1) * basema1;
			fvAsk = (((ask/hedgema1 - 1) * (base.levRatio()/hedge.levRatio())) + 1) * basema1;
			theoAsk = fvAsk * ( 1+ widthInBps - (skewInBpsPer100K * baseNotional)) + bump;
			theoBid = fvBid * (1 - widthInBps - (skewInBpsPer100K * baseNotional)) + bump;

			FSB_LOG(" theoBid " << theoBid
				<< " theoAsk " << theoAsk
				<< " fvBid " << fvBid
				<< " fvAsk " << fvAsk
				<< " bid  " << bid
				<< " ask " << ask
				<< " B: " << base.bidNoLock()
				<< " X " << base.askNoLock()
				<< " H: " << hedge.bidNoLock()
				<< " X " << hedge.askNoLock() 
				<< " Skew " << skewInBpsPer100K			
				<< " Width " << widthInBps
				<< " Net " << net
				<< " Notional " << baseNotional
				<< " Base ma1 " << basema1
				<< " hedge ma1 " << hedgema1 
				<< " bump " << bump );
		}
	};


	class MMKR1by1FV
	{
	public:
		void calc(double& theoBid,
			double& theoAsk,
			const Instrument::VectorOfInstrs& bases,
			const Instrument::VectorOfInstrs& hedges)
		{
			return;
		}
		void calc(double& theoBid,
			double& theoAsk,
			const Instrument& instr1,
			const Instrument& instr2)
		{
			double skewInBps = TraderEnvSingleton::instance()->_params.skew();
			double widthInBps = TraderEnvSingleton::instance()->_params.width()/2.0;
			double fx = TraderEnvSingleton::instance()->_params.fxMult();
			float mixratio = TraderEnvSingleton::instance()->_params.mixRatio();
			float bump = TraderEnvSingleton::instance()->_params.bump();

			double ma1 = TraderEnvSingleton::instance()->_longTermMA->result(instr1);
			double ma2 = TraderEnvSingleton::instance()->_shortTermMA->result(instr1);
			double ratio = TraderEnvSingleton::instance()->_longTermMA->datapoint0();
			double maxMa = std::max<double>(ma1,ma2);
			double minMa = std::min<double>(ma1,ma2);

			double fvMin=0.0,fvMax=0.0;

			double basemid = instr1.midNoLock();
			double hedgemid = instr2.midNoLock();

			fvMin = hedgemid * minMa;
			fvMax = hedgemid * maxMa;

			InstrumentKey key(instr1.symbol(),instr1.exchange());
			InstrumentOrdersPtrItr itr;
			int net = 0;

			if( (itr = TraderEnvSingleton::instance()->_instrOrders.find(key)) != TraderEnvSingleton::instance()->_instrOrders.end())
				net = itr->second->net();
			else {
				FSB_LOG("Could not find the net for " << key._symbol << " " << key._exchange);			
			}		
			double baseDelta = (net * basemid * instr1.multiplier() * fx)/1000.0;
			double deltaothers = TraderEnvSingleton::instance()->_totalPos.delta();

			theoBid = fvMin * (1 - (widthInBps/10000.0) + bump - (skewInBps * (baseDelta + ((deltaothers - baseDelta) * mixratio)))/10000.0);
			theoAsk = fvMax * (1 + (widthInBps/10000.0) + bump - (skewInBps * (baseDelta + ((deltaothers - baseDelta)*mixratio)))/10000.0);

			FSB_LOG(" " << instr1.symbol()
				<< " TB " << theoBid
				<< " TA " << theoAsk
				<< " " << instr1.bidNoLock()
				<< " X " << instr1.askNoLock()
				<< " mid " << basemid
				<< " net  " << net
				<< " W " << widthInBps
				<< " S " << skewInBps
				<< " Mix " << mixratio
				<< " MA1 " << ma1
				<< " MA2 " << ma2
				<< " FM " << fvMin
				<< " FM " << fvMax			
				<< " delta " << baseDelta
				<< " deltaOthers " << deltaothers
				);
		}
	};

	class PTBaseFV
	{
	private:

	public:

		PTBaseFV()
		{			
		}

		void calc(double& theoBid,
			double& theoAsk,
			const Instrument::VectorOfInstrs& bases,
			const Instrument::VectorOfInstrs& hedges)
		{
			double basketHedgeBid = 0.0;
			double basketHedgeAsk = 0.0;

			double basketBaseBid = 0.0;
			double basketBaseAsk = 0.0;

			double bidSizeAdj = 0.0, askSizeAdj = 0.0;
			double baseNotionalBuy = 0.0, baseNotionalSell = 0.0;

			calcBasketBase(bases,
						basketBaseBid,
						basketBaseAsk,
						baseNotionalBuy,
						baseNotionalSell);
			calcBasketHedge(hedges,
				basketHedgeBid,
				basketHedgeAsk);
			calcSizeAdj(hedges,baseNotionalBuy,baseNotionalSell,bidSizeAdj,askSizeAdj);

			double bidSkew = 0.0, askSkew = 0.0;
			if( !calcSKEW_WIDTH(bidSkew,askSkew) )
				return;

			double ma1 = TraderEnvSingleton::instance()->_longTermMA->result();
			double ma2 = TraderEnvSingleton::instance()->_shortTermMA->result();
			double ratio = TraderEnvSingleton::instance()->_longTermMA->datapoint0();
			double maxMa = std::max<double>(ma1,ma2);
			double minMa = std::min<double>(ma1,ma2);

			theoBid = minMa * basketHedgeBid * bidSkew - bidSizeAdj;
			theoAsk = maxMa * basketHedgeAsk * askSkew + askSizeAdj;

			FSB_LOG( " theoBid " << theoBid 
				<< " theoAsk " << theoAsk
				<< " base " << basketBaseBid
				<< "  X " << basketBaseAsk
				<< " hedge " << basketHedgeBid
				<< " X " << basketHedgeAsk
				<< " MA1(lonG) " << ma1
				<< " MA2(short) " << ma2
				<< " ratio " << ratio);
		}

		void calcBasketBase(const Instrument::VectorOfInstrs& bases,
						double& basketBaseBid,
						double& basketBaseAsk,
						double& baseNotionalBuy,
						double& baseNotionalSell)
		{
			int worksize = TraderEnvSingleton::instance()->_params.baseWorkSize();
			for( Instrument::VectorOfInstrs::const_iterator i = bases.begin(); i != bases.end(); ++i)
			{
				
				double bid = i->bidNoLock();
				double ask = i->askNoLock();
				basketBaseBid += (bid * i->factor());

				baseNotionalBuy = worksize * basketBaseBid * i->multiplier();
				
				basketBaseAsk += (i->askNoLock() * i->factor());
				
				baseNotionalSell = worksize * basketBaseAsk * i->multiplier(); 
			}
			//FSB_LOG(" Done cal base " << basketBaseAsk << " " << basketBaseBid);
		}

		void calcBasketHedge(const Instrument::VectorOfInstrs& hedges,
			double& basketHedgeBid, 
			double& basketHedgeAsk)

		{
			for( Instrument::VectorOfInstrs::const_iterator i = hedges.begin(); i != hedges.end(); ++i)
			{
				basketHedgeBid += ( i->bidNoLock() * i->factor());
				basketHedgeAsk += ( i->askNoLock() * i->factor());
			}
		}

		void calcSizeAdj(const Instrument::VectorOfInstrs& hedges,
			double& bidSizeAdj, 
			double& askSizeAdj,
			const double& baseNotionalBuy,
			const double& baseNotionalSell)
		{
			double sizeAdj = TraderEnvSingleton::instance()->_params.sizeAdjustment();
			double fx = TraderEnvSingleton::instance()->_params.fxMult();
			double totalWeightAsk = 0.0, totalWeightBid = 0.0;
			for( Instrument::VectorOfInstrs::const_iterator i = hedges.begin(); i != hedges.end(); ++i)
			{
				int bsize = i->bidSizeNoLock();
				double bid = i->bidNoLock();
				double hedgeContractVal = bid * i->multiplier() * fx;
				if( bsize >= (baseNotionalBuy * i->weight())/hedgeContractVal )
					totalWeightBid += i->weight();

				int asize = i->askSizeNoLock();
				double ask = i->askNoLock();
				hedgeContractVal = ask * i->multiplier() * fx;
				if( asize >= baseNotionalSell * i->weight()/hedgeContractVal )
					totalWeightAsk += i->weight();
				FSB_LOG(" symbol " << i->symbol()
					<< " wt. " << i->weight()
					<< " mult. " << i->multiplier()
					<< " " << hedgeContractVal
					<< " ASZ " << asize
					<< " Tot. Wt. " << totalWeightAsk
					<< " BSZ " << bsize
					<< " Tot Wt. " << totalWeightBid);
			}
			askSizeAdj = sizeAdj * (1- totalWeightAsk);
			bidSizeAdj = sizeAdj * (1-totalWeightBid);

			FSB_LOG(" " << bidSizeAdj
				<< " " << askSizeAdj
				<< " " << sizeAdj);
		}

		bool calcSKEW_WIDTH(double& bidSkew,double& askSkew)
		{
			double skewInBps = TraderEnvSingleton::instance()->_params.skew();
			float skewUnit = TraderEnvSingleton::instance()->_params.skewUnit();
			double widthInBps = TraderEnvSingleton::instance()->_params.width()/2.0;
			/*double bidsizeAdj = TraderEnvSingleton::instance()->_params._sizeAdjustment;
			double asksizeAdj = TraderEnvSingleton::instance()->_params._sizeAdjustment;*/
			//int minHdgSizeReq = TraderEnvSingleton::instance()->_params._minHedgeSizeRequired;
			double fsbticksz = TraderEnvSingleton::instance()->_base[0]->_instr.fsbticksize();
			int net = TraderEnvSingleton::instance()->_base[0]->_orders.net();
			double mid = TraderEnvSingleton::instance()->_base[0]->_instr.mid();
			double multiplier = TraderEnvSingleton::instance()->_base[0]->_instr.multiplier();
			double skew = 0.0;
			if( skewUnit != -1.0 ){			
				double breakevenNet = (widthInBps * skewUnit)/(skewInBps * multiplier * mid);
				if( breakevenNet < 1000.0 ) {
					FSB_LOG(" Error: Skew value is wrong  breakeven net = " << breakevenNet << " will not calc theo. vals.");
					return false;
				}
				skew = (1 - ((skewInBps * net * mid * multiplier)/(10000.0* skewUnit)));			
			} else {
				skew = (1 - (skewInBps * net)/10000.0);
			}
			 //theoAsk = maxMa* hedgeAsk * (1+widthInBps/10000.0) * (1 - (skewInBps * net)/10000.0) + askSizeAdj;
			 //theoBid = minMa* hedgeBid * (1-widthInBps/10000.0) * (1 - (skewInBps * net)/10000.0) - bidSizeAdj;
			//double skew = (1 - ((skewInBps * net * mid * multiplier)/(10000.0* skewUnit)));
			double width = (1+widthInBps/10000.0);
			askSkew = (1+widthInBps/10000.0) * skew ;
			bidSkew = (1-widthInBps/10000.0) * skew;

			FSB_LOG( " bidSkew " << bidSkew 
				<< " askSkew " << askSkew
				<< " net " << net
				<< " skew in Bps " << skewInBps << " " << skew
				<< " width in Bps " << widthInBps
				<< " skew Unit " << skewUnit
				<< " fsb tick " << fsbticksz
				<< " mid " << mid);
			return true;

		}

		void calcInverse(double& theoBid,
			double& theoAsk,
			const Instrument& base, 
			const Instrument& hedge)
		{
			double sideAdj = TraderEnvSingleton::instance()->_params.sizeAdjustment();
			int minHdgSizeReq = TraderEnvSingleton::instance()->_params.minHedgeSize();
			float bump = TraderEnvSingleton::instance()->_params.bump();
			int hedgeBidSize = hedge.bidSizeNoLock();
			int  hedgeAskSize = hedge.askSizeNoLock();
			double baseBid = base.bidNoLock();
			double baseAsk = base.askNoLock();
			double hedgeBid = hedge.bidNoLock();
			double hedgeAsk = hedge.askNoLock();
			if(baseBid <= 0.0 ||
				baseAsk <= 0.0 ||
				hedgeBid <= 0.0 ||
				hedgeAsk <= 0.0) {
					FSB_LOG("Zero prices " << baseBid << " X " << baseAsk
						<< " " << hedgeBid << " X " << hedgeAsk);
					return;
			}


			double bidSizeAdj = hedgeBidSize < minHdgSizeReq ? sideAdj : 0.0; 
			double askSizeAdj = hedgeAskSize < minHdgSizeReq ? sideAdj : 0.0;

			double skewAsk = 0.0, skewBid = 0.0;

			if( !calcSKEW_WIDTH(skewBid,skewAsk))
				return;

			double ma1 = TraderEnvSingleton::instance()->_longTermMA->result();
			double ma2 = TraderEnvSingleton::instance()->_shortTermMA->result();
			double ratio = TraderEnvSingleton::instance()->_longTermMA->datapoint0();
			double maxMa = std::max<double>(ma1,ma2);
			double minMa = std::min<double>(ma1,ma2);
			theoAsk = (maxMa / hedgeBid) * skewAsk + askSizeAdj+bump;//(1+widthInBps/10000.0) * (1 - (skewInBps * net)/10000.0) + askSizeAdj;
			theoBid = (minMa / hedgeAsk) * skewBid - bidSizeAdj+ bump;//(1-widthInBps/10000.0) * (1 - (skewInBps * net)/10000.0) - bidSizeAdj;

			FSB_LOG( " theoBid " << theoBid 
				<< " theoAsk " << theoAsk
				<< " base  " << baseBid
				<< " X " << baseAsk
				<< "  " << hedgeBidSize
				<< " hedge " << hedgeBid
				<< " X " << hedgeAsk			
				<< " " << hedgeAskSize
				<< " MA1(lonG) " << ma1
				<< " MA2(short) " << ma2
				<< " bump " << bump
				<< " ratio " << ratio);
		}

		void calc(double& theoBid,double& theoAsk,const Instrument& base, const Instrument& hedge)
		{
			//double skewInBps = TraderEnvSingleton::instance()->_params.skew();
			//double widthInBps = TraderEnvSingleton::instance()->_params.width()/2.0;
			theoBid = theoAsk = 0.0;
			double sizeAdj = TraderEnvSingleton::instance()->_params.sizeAdjustment();
			int minHdgSizeReq = TraderEnvSingleton::instance()->_params.minHedgeSize();
			float bump = TraderEnvSingleton::instance()->_params.bump();
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
			double ratio = TraderEnvSingleton::instance()->_longTermMA->datapoint0();
			double maxMa = std::max<double>(ma1,ma2);
			double minMa = std::min<double>(ma1,ma2);

			//int totBuys = TraderEnvSingleton::instance()->_base[0]->_orders.totalBuys();
			//int totSells = TraderEnvSingleton::instance()->_base[0]->_orders.totalSells();
			//int net = totBuys - totSells;

			theoAsk = maxMa * hedgeAsk * skewAsk + askSizeAdj +bump ;//(1+widthInBps/10000.0) * (1 - (skewInBps * net)/10000.0) + askSizeAdj;
			theoBid = minMa * hedgeBid * skewBid - bidSizeAdj + bump;//(1-widthInBps/10000.0) * (1 - (skewInBps * net)/10000.0) - bidSizeAdj;
			TraderEnvSingleton::instance()->_basket.ma1(ma1);
			TraderEnvSingleton::instance()->_basket.ma2(ma2);
			TraderEnvSingleton::instance()->_basket.dp0(ratio);

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
				<< " " << askSizeAdj
				<< " bump " << bump);
		}
	};

	class PTBaseCloseFv : public PTBaseFV
	{
	public:
		PTBaseCloseFv()
		{
		}

		void calc(double& theoBid, double& theoAsk,
			Instrument::VectorOfInstrs& bases,
			Instrument::VectorOfInstrs& hedges)
		{
			theoBid = theoAsk = 0.0;
			PTBaseFV::calc(theoBid,theoAsk,bases,hedges);

			double bid = theoBid;
			double ask = theoAsk;

			double ma1 = TraderEnvSingleton::instance()->_longTermMA->result();
			double ma2 = TraderEnvSingleton::instance()->_shortTermMA->result();

			double avgMa = (ma1+ma2)/2.0;

			double hedgeAsk = 0.0,hedgeBid=0.0; 
			calcBasketHedge(hedges,hedgeBid,hedgeAsk);

			double closeBaseAsk = avgMa * hedgeAsk;
			double closeBaseBid = avgMa * hedgeBid;

			theoBid	= std::max<double>(closeBaseBid,theoBid);
			theoAsk = std::min<double>(closeBaseAsk,theoAsk);

			FSB_LOG( " Close only mode " 
				<< " orig. bid fv " << bid
				<< " orig. ask fv " << ask
				<< " close fv bid " << closeBaseBid
				<< " close fv ask " << closeBaseAsk
				<< " theo ask " << theoAsk
				<< " theo bid " << theoBid);
		}

		void calcInverse(double& theoBid, double& theoAsk,
			const Instrument& base,const Instrument& hedge)
		{
			PTBaseFV::calcInverse(theoBid,theoAsk,base,hedge);

			double bid = theoBid;
			double ask = theoAsk;

			double ma1 = TraderEnvSingleton::instance()->_longTermMA->result();
			double ma2 = TraderEnvSingleton::instance()->_shortTermMA->result();

			double avgMa = (ma1+ma2)/2.0;

			double hedgeAsk = hedge.askNoLock();
			double hedgeBid = hedge.bidNoLock();

			double closeBaseAsk = avgMa / hedgeBid;
			double closeBaseBid = avgMa / hedgeAsk;

			theoBid	= std::max<double>(closeBaseBid,theoBid);
			theoAsk = std::min<double>(closeBaseAsk,theoAsk);

			FSB_LOG( " Close only mode " 
				<< " orig. bid fv " << bid
				<< " orig. ask fv " << ask
				<< " close fv bid " << closeBaseBid
				<< " close fv ask " << closeBaseAsk
				<< " theo ask " << theoAsk
				<< " theo bid " << theoBid);

		}
		void calc(double& theoBid, double& theoAsk,
			const Instrument& base,const Instrument& hedge)
		{
			PTBaseFV::calc(theoBid,theoAsk,base,hedge);

			double bid = theoBid;
			double ask = theoAsk;

			double ma1 = TraderEnvSingleton::instance()->_longTermMA->result();
			double ma2 = TraderEnvSingleton::instance()->_shortTermMA->result();

			double avgMa = (ma1+ma2)/2.0;

			double hedgeAsk = hedge.askNoLock();
			double hedgeBid = hedge.bidNoLock();

			double closeBaseAsk = avgMa * hedgeAsk;
			double closeBaseBid = avgMa * hedgeBid;

			theoBid	= std::max<double>(closeBaseBid,theoBid);
			theoAsk = std::min<double>(closeBaseAsk,theoAsk);

			FSB_LOG( " Close only mode " 
				<< " orig. bid fv " << bid
				<< " orig. ask fv " << ask
				<< " close fv bid " << closeBaseBid
				<< " close fv ask " << closeBaseAsk
				<< " theo ask " << theoAsk
				<< " theo bid " << theoBid);

		}
	};

	template<class FVCalculator,class BSSignal>
	class TRADERPOLICIES_API FVPolicy1 : private FVCalculator,private BSSignal
	{
	public:
		FVPolicy1(const std::string& indicator)		 
		{
			_bsIndicator = (BinaryPolicy* ) PolicyFactorySingleton::instance()->create(indicator); 
		}

		int bsIndicator(Instrument& instr, const double& theoBid, const double& theoAsk)
		{
			int type = BSSignal::buySellCondition(instr,theoBid,theoAsk);
			FSB_LOG(" Signal: " << ( ((type & BinaryPolicy::BUY) && (type & BinaryPolicy::SELL)) ? "BOTH" : type & BinaryPolicy::SELL ? "SELL" : type & BinaryPolicy::BUY ? "BUY" : "NONE"));
			return type;
		}

		int fv(double& theoBid,
			double& theoAsk,
			Instrument::VectorOfInstrs& bases,
			Instrument::VectorOfInstrs& hedges)
		{
			for(Instrument::VectorOfInstrs::const_iterator instrObj = bases.begin(); instrObj != bases.end(); ++instrObj) {
				if( zeroQuotes(*instrObj) )
					return BinaryPolicy::ZERO_QUOTES;
				if( crossedQuotes(*instrObj) )
					return BinaryPolicy::ZERO_QUOTES;
			}
			for(Instrument::VectorOfInstrs::const_iterator instrObj = hedges.begin(); instrObj != hedges.end(); ++instrObj) {
				if( zeroQuotes(*instrObj))
					return BinaryPolicy::ZERO_QUOTES;
				if( crossedQuotes(*instrObj)) 
					return BinaryPolicy::ZERO_QUOTES;
			}

			calc(theoBid,theoAsk,bases,hedges);

//			TraderEnvSingleton::instance()->_toDisplay.update(theoBid,theoAsk);
			TraderEnvSingleton::instance()->_basket.ask(theoAsk);
			TraderEnvSingleton::instance()->_basket.bid(theoBid);

			int bsFlag =  _bsIndicator->buySellCondition(bases[0],theoBid,theoAsk);

			FSB_LOG(" Flag " << (bsFlag == BinaryPolicy::BUY ? "BUY" : bsFlag == BinaryPolicy::SELL ? "SELL" : bsFlag == BinaryPolicy::BOTH ? "BOTH" : "NO CHANGE"));
		
			return bsFlag;

		}

		int fvInverse(double& theoBid,
			double& theoAsk,
			const Instrument& base, 
			const Instrument& hedge)
		{
			//using FVCalculator::calc;

			if( zeroQuotes(base) ||
				zeroQuotes(hedge) )
			{
				return BinaryPolicy::ZERO_QUOTES;
			}

			if( crossedQuotes(base) ||
				crossedQuotes(hedge) )
			{
				return BinaryPolicy::ZERO_QUOTES;
			}

			calcInverse(theoBid,theoAsk,base,hedge);

			//TraderEnvSingleton::instance()->_toDisplay.update(theoBid,theoAsk);
			TraderEnvSingleton::instance()->_basket.ask(theoAsk);
			TraderEnvSingleton::instance()->_basket.bid(theoBid);
			int bsFlag =  _bsIndicator->buySellCondition(base,theoBid,theoAsk);

			FSB_LOG(" Flag " << (bsFlag == BinaryPolicy::BUY ? "BUY" : bsFlag == BinaryPolicy::SELL ? "SELL" : bsFlag == BinaryPolicy::BOTH ? "BOTH" : "NO CHANGE"));
			
			return bsFlag;
		}

		int fv(double& theoBid,
			double& theoAsk,
			const Instrument& base, 
			const Instrument& hedge)
		{
			//using FVCalculator::calc;

			if( zeroQuotes(base) ||
				zeroQuotes(hedge) )
			{
				return BinaryPolicy::ZERO_QUOTES;
			}

			if( crossedQuotes(base) ||
				crossedQuotes(hedge) )
			{
				return BinaryPolicy::ZERO_QUOTES;
			}

			calc(theoBid,theoAsk,base,hedge);
			TraderEnvSingleton::instance()->_basket.ask(theoAsk);
			TraderEnvSingleton::instance()->_basket.bid(theoBid);

			int bsFlag =  _bsIndicator->buySellCondition(base,theoBid,theoAsk);

			FSB_LOG(" Flag " << (bsFlag == BinaryPolicy::BUY ? "BUY" : bsFlag == BinaryPolicy::SELL ? "SELL" : bsFlag == BinaryPolicy::BOTH ? "BOTH" : "NO CHANGE"));
			
			return bsFlag;
		}

		bool zeroQuotes(const Instrument& instr)
		{
			if( instr.bidNoLock() <= 0 || instr.askNoLock() <= 0) {
				FSB_LOG("Zero quotes " << instr.symbol()
					     << instr.bidNoLock() << "X" << instr.askNoLock() );
				return true;
			}
			return false;
		}
		bool crossedQuotes( const Instrument& instr)
		{
			if( instr.bidNoLock() > instr.askNoLock() ) {
				FSB_LOG(" Crossed quotes " << instr.symbol()
					     << instr.bidNoLock() << "X" << instr.askNoLock());
				return true;
			}
			return false;
		}

	private:
		BinaryPolicy* _bsIndicator;
	};
}
#endif
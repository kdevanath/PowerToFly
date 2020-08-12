#include "stdafx.h"
#include "SizePolicy.h"
#include "PolicyFactory.h"
#include "TraderEnv.h"
#include <Utilities/genericfns.h>
#include <Utilities/FSBLogger.h>

namespace fsb {

	void SpiderBase1SizePolicy::calcQuantity(const Instrument& instr,int& buy, int& sell)
	{
		if(instr.index() != 0) {
			FSB_LOG(" The is for base 1 only may be xlf/xlk not " << instr.symbol());
			return;
		}
		
		int position = TraderEnvSingleton::instance()->_base[instr.index()]->_orders.net();
		int maxpos = TraderEnvSingleton::instance()->_params.maxPosition();
		int lotsize = instr.lotsize();
		buy = 100;//(int)maxpos/10;
		sell = 100;//(int)maxpos/10;

		int residual = position - ( (position/lotsize) * lotsize);
		
		residual > 0 ? buy += (lotsize - residual) : sell += residual;
		FSB_LOG(" Net " << position
			<< " buy " << buy
			<< " sell " << sell
			<< " residual " << residual
			<< " max " << maxpos);

	}

	void SpiderBaseNSizePolicy::calcQuantity(const Instrument& instr,int& buy, int& sell)
	{
		buy=sell=0;
		bool closeOnly = TraderEnvSingleton::instance()->_params.closeOnly();
		int positionBase1 = TraderEnvSingleton::instance()->_base[0]->_orders.net();
		float base1sph = TraderEnvSingleton::instance()->_base[0]->_instr.weight();
		int positionBaseN = TraderEnvSingleton::instance()->_base[instr.index()]->_orders.net();
		//ALERT DO IT DIFFERENTLY
		int targetPos = closeOnly ? 0 : (int) positionBase1 * (instr.weight()/base1sph);
		int roundedTarget = fsbutils::RoundingFunctions::Round(instr.lotsize(),targetPos);
		/*int minTarget=0,maxTarget = 0;
		roundedTarget < 0 ? minTarget = roundedTarget : maxTarget = roundedTarget;*/
		if(positionBaseN < roundedTarget ) buy = roundedTarget - positionBaseN;
		else if(positionBaseN > roundedTarget) sell = positionBaseN - roundedTarget;
		/*if( positionBaseN < maxTarget)
			maxTarget == 0 ? buy = 100 : buy = maxTarget - positionBaseN;
		if( positionBaseN > minTarget )
			minTarget == 0 ? sell = 100 : sell = positionBaseN - minTarget;*/
		if(buy && buy < 100 ) {
			FSB_LOG(" setting buy 0, " << buy << " < 0 ");
			buy = 0;
		}
		if( sell && sell < 100 ) {
			FSB_LOG(" setting sell 0, " << sell << " < 0 ");
			sell = 0;
		}

		FSB_LOG(" positionBase1 " << positionBase1
			<< " positionBaseN " << positionBaseN
			<< " targetPos " << targetPos
			<< " roundedTarget " << roundedTarget
			//<< " min " << minTarget
			//<< " max " << maxTarget
			<< " buy " << buy
			<< " sell " << sell);
	}

	SpiderHedgeSizePolicy::SpiderHedgeSizePolicy()
	{
	}
	void SpiderHedgeSizePolicy::calcQuantity(const Instrument& instr,
		const OrderReplyInfo& reply, 
		int& buys, int& sells)
	{
		/*double baseNotional = std::for_each(TraderEnvSingleton::instance()->_base.begin(),
				TraderEnvSingleton::instance()->_base.end(),calc_notional());*/
		double baseNotional = TraderEnvSingleton::instance()->_basePos.delta();
		double mid = TraderEnvSingleton::instance()->_hedge[0]->_instr.mid();
		int lotsize = TraderEnvSingleton::instance()->_hedge[0]->_instr.lotsize();
		int net = TraderEnvSingleton::instance()->_hedge[0]->_orders.net() + instr.incomingInv();
		int pendingBuy = TraderEnvSingleton::instance()->_hedge[0]->_orders.qtyPendingBuys();
		int pendingSell = TraderEnvSingleton::instance()->_hedge[0]->_orders.qtyPendingSells();
		buys = (int)(baseNotional/mid) + net + pendingBuy;
		sells = (int)(baseNotional/mid) + net - pendingSell;
		//buys = sells = (int)(baseNotional/mid) + net; 
		buys = buys < 0 ? abs(buys) : 0;
		sells = sells < 0 ? 0 : sells;
		FSB_LOG(" Notional " << baseNotional
			<< " HN: " << (net * mid)
			<< " Mid " << mid
			<< " Net " << net
			<< " PB " << pendingBuy
			<< " PS " << pendingSell
			<< " " << buys
			<< " " << sells);
	}

	void GenericSizePolicy::calcQuantity(const Instrument& instr,int& buy, int& sell)
	{
		buy=sell=1;
	}

	void ETFWorkLiqSizePolicy::calcQuantity(const Instrument& instr,int& buy, int& sell)
	{
		int ls = (*TraderEnvSingleton::instance()->_instrObjItrs[instr.refIndex()])->_instr.lotsize();
		double hmp = (*TraderEnvSingleton::instance()->_instrObjItrs[instr.refIndex()])->_instr.mid();
		double lev = (*TraderEnvSingleton::instance()->_instrObjItrs[instr.refIndex()])->_instr.levRatio();

		double worksize  = fabs((ls * lev * hmp)/(instr.midNoLock() * instr.levRatio()));
		buy = sell = (int)fsbutils::RoundingFunctions::Round(instr.lotsize(),worksize);
		
		FSB_LOG(" lot " << ls
			<< " mid (H) "<< hmp
			<< " leb " << lev
			<< " base lev " << instr.levRatio()
			<< " ws " << worksize
			<< " " << buy
			<< " " << sell);

		if( buy < 100 ) 
			buy = sell = 100;

	}

	WorkLiqSizePolicy::WorkLiqSizePolicy()
	{
	}
	void WorkLiqSizePolicy::calcQuantity(const Instrument& instr,int& buy, int& sell)
	{
		buy = 0; sell = 0;
		if( TraderEnvSingleton::instance()->_params.randomize() )
			_randomize.calcQuantity(buy,sell);
		else
			buy = sell = TraderEnvSingleton::instance()->_params.baseWorkSize();

		FSB_LOG( " Work Liq. "
			<< " buy " << buy
			<< " sell " <<  sell
			<< " work size " << TraderEnvSingleton::instance()->_params.baseWorkSize()
			);
	}

	void TakeLiqSizePolicy::calcQuantity(const Instrument& instr,int &buy, int &sell)
	{
		buy = 0; sell = 0;
		int baseSell = 0, hedgeSell = 0,baseBuy=0,hedgeBuy=0;

		int baseTakeSize = TraderEnvSingleton::instance()->_params.baseTakeSize();
		int hdgTakeSize = TraderEnvSingleton::instance()->_params.hedgeTakeSize();

		InstrumentObj::VectPtrsItr itr = TraderEnvSingleton::instance()->_base.begin();

		int baseAskSize,baseBidSize,hdgAskSize,hdgBidSize;

		for( itr; itr != TraderEnvSingleton::instance()->_base.end();++itr) {
			baseAskSize = (*itr)->_instr.askSize();
			baseBuy = std::min<int>(baseTakeSize,baseAskSize);
			baseBidSize = (*itr)->_instr.bidSize();
			baseSell = std::min<int>(baseTakeSize,baseBidSize);
		}

		itr =  TraderEnvSingleton::instance()->_hedge.begin();
		for( itr; itr != TraderEnvSingleton::instance()->_hedge.end();++itr) {
			hdgAskSize = (*itr)->_instr.askSize();
			hedgeSell = std::min<int>(hdgTakeSize,hdgAskSize);
			hdgBidSize = (*itr)->_instr.bidSize();
			hedgeBuy = std::min<int>(hdgTakeSize,hdgBidSize);
		}
		buy = baseBuy >= hedgeBuy ? baseBuy : hedgeBuy;
		sell = baseSell >= hedgeSell ? baseSell : hedgeSell;

		FSB_LOG( " Take Liq. "
			<< " buy " << buy
			<< " sell " <<  sell
			<< " base take size " << baseTakeSize
			<< " hdg take size " << hdgTakeSize
			<< " base " << baseBidSize
			<< " X " << baseAskSize
			<< " hdg " << hdgBidSize
			<< " X " << hdgAskSize
			);
	}

	/***********************************************************************/
	/****************************************************/

	void InverseDeltaCloseSizePolicy::calcQuantity(const OrderReplyInfo& reply,int& buy, int& sell)
	{
		
		double fx = TraderEnvSingleton::instance()->_params.fxMult();
		double beta = TraderEnvSingleton::instance()->_params.betaMult();

		double basecs = TraderEnvSingleton::instance()->_base[0]->_instr.multiplier();
		int basePos = TraderEnvSingleton::instance()->_base[0]->_orders.net();
		double baseMid = TraderEnvSingleton::instance()->_base[0]->_instr.mid();
		double  baseNotional = basePos * baseMid * basecs * fx * beta;

		int hedgeBuyPos = TraderEnvSingleton::instance()->_hedge[0]->_orders.totalBuys();
		int hedgeSellPos = TraderEnvSingleton::instance()->_hedge[0]->_orders.totalSells();
		double hedgecs = TraderEnvSingleton::instance()->_hedge[0]->_instr.multiplier();
		double hedgeMid = TraderEnvSingleton::instance()->_hedge[0]->_instr.mid();
		double hedgeSingleContract = hedgeMid *  hedgecs * fx * beta;

		int pendingSells = TraderEnvSingleton::instance()->_hedge[0]->_orders.qtyPendingSells();
		int pendingBuys = TraderEnvSingleton::instance()->_hedge[0]->_orders.qtyPendingBuys();

		int buyHedge = (int)fsbutils::RoundingFunctions::Round(1.0,baseNotional/ hedgeSingleContract) - hedgeBuyPos + hedgeSellPos;
		buy = buyHedge > 0 ? buyHedge : 0;

		int sellHedge = (int)fsbutils::RoundingFunctions::Round(1.0,baseNotional/hedgeSingleContract) - hedgeBuyPos + hedgeSellPos;
		sell = sellHedge < 0 ? abs(sellHedge) : 0;
		
		FSB_LOG( " sell " << sell
			<< " buy " << buy
			<< " buyHedge " << buyHedge
			<< " sellHedge " << sellHedge
			<<  " baseNotional " <<  baseNotional
			<< " hedgeSingleContractValue " << hedgeSingleContract
			<< " basePos " << basePos
			<< " hedgeBuyPos " << hedgeBuyPos
			<< " hedgeSellPos " << hedgeSellPos
			<< " fx " << fx
			<< " beta " << beta
			<< " hedge cs " << hedgecs
			<< " base cs " << basecs );
	}

	void DeltaCloseUsingLevRatio::calcQuantity(const Instrument& instr,
		const OrderReplyInfo& reply,
		int& buy, int& sell)
	{
		double fx = TraderEnvSingleton::instance()->_params.fxMult();
		double beta = TraderEnvSingleton::instance()->_params.betaMult();

		int index = instr.index();
		int refIndex = instr.refIndex();
		double hedgeLevratio = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_instr.levRatio();
		int lotSize = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_instr.lotsize(); 
		int hedgePos = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_orders.net();
		int pendingSells = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_orders.qtyPendingSells();
		int pendingBuys = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_orders.qtyPendingBuys();
		double hedgecs = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_instr.multiplier();
		double hedgeMid = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_instr.mid();
		double hedgeSingleContract = hedgeMid *  hedgecs * fx * beta * fabs(hedgeLevratio);

		double basecs = instr.multiplier();
		double baseLevRatio = instr.levRatio();
		int basePos = (*TraderEnvSingleton::instance()->_instrObjItrs[index])->_orders.net();
		double baseMid = instr.midNoLock();
		double  baseNotional = basePos * baseMid * basecs * fx * beta * baseLevRatio;
		int tradeSize = (int)fsbutils::RoundingFunctions::Round(lotSize,(baseNotional/hedgeSingleContract)) + hedgePos;
		buy = tradeSize + pendingBuys;
		sell = tradeSize - pendingSells;
		
		FSB_LOG( " sell " << sell
			<< " buy " << buy
			<<  " baseNotional buy " <<  baseNotional
			<< " hedgeSingleContractValue " << hedgeSingleContract
			<< " basePos " << basePos
			<< " hedgePos " << hedgePos
			<< " trade size " << tradeSize
			<< " Pend B " << pendingBuys
			<< " Pending S " << pendingSells
			<< " fx " << fx
			<< " beta " << beta
			<< " hedge cs " << hedgecs
			<< " base cs " << basecs );
			
	}
	void DeltaCloseSizePolicy::calcQuantity(const OrderReplyInfo&,int& buy, int& sell)
	{
		double basecs,hedgecs,baseBid,hedgeBid,baseask,hedgeask;
		int hedgePos = TraderEnvSingleton::instance()->_hedge[0]->_orders.net();
		double fx = TraderEnvSingleton::instance()->_params.fxMult();
		double beta = TraderEnvSingleton::instance()->_params.betaMult();

		basecs = TraderEnvSingleton::instance()->_base[0]->_instr.multiplier();
		int basePos = TraderEnvSingleton::instance()->_base[0]->_orders.net();
		baseBid = TraderEnvSingleton::instance()->_base[0]->_instr.bid();
		double  baseNotionalBuy = basePos * baseBid * basecs * fx * beta;

		int pendingSells = TraderEnvSingleton::instance()->_hedge[0]->_orders.qtyPendingSells();
		hedgeBid = TraderEnvSingleton::instance()->_hedge[0]->_instr.bid();
		hedgecs = TraderEnvSingleton::instance()->_hedge[0]->_instr.multiplier();

		double hedgeSingleContract = hedgeBid *  hedgecs * fx * beta;
		sell = (int)fsbutils::RoundingFunctions::Round(1.0,baseNotionalBuy/hedgeSingleContract) + hedgePos - pendingSells;
		sell > 0 ? sell = sell : sell = 0;

		baseask = TraderEnvSingleton::instance()->_base[0]->_instr.ask();
		hedgeask = TraderEnvSingleton::instance()->_hedge[0]->_instr.ask();
		int pendingBuys = TraderEnvSingleton::instance()->_hedge[0]->_orders.qtyPendingBuys();
		double baseNotionalSell = basePos * baseask * basecs * fx * beta;
		hedgeSingleContract = hedgeask *  hedgecs * fx * beta;
		buy = (int)fsbutils::RoundingFunctions::Round(1.0,baseNotionalSell / hedgeSingleContract) + hedgePos + pendingBuys;
		buy < 0 ? buy = abs(buy): buy = 0;

		FSB_LOG( " sell " << sell
			<< " buy " << buy
			<<  " baseNotional buy " <<  baseNotionalBuy
			<< " baseNotional sell " << baseNotionalSell
			<< " hedgeSingleContractValue " << hedgeSingleContract
			<< " basePos " << basePos
			<< " hedgePos " << hedgePos
			<< " Pending S " << pendingSells
			<< " Pending B " << pendingBuys
			<< " fx " << fx
			<< " beta " << beta
			<< " hedge cs " << hedgecs
			<< " base cs " << basecs );
	}
	/****************************************************/

	void TakeHedgeSizePolicy::calcQuantity(const Instrument& instr,int& buy, int& sell)
	{
		int hedgebidsz = TraderEnvSingleton::instance()->_hedge[0]->_instr.bidSize();
		int hedgeasksz = TraderEnvSingleton::instance()->_hedge[0]->_instr.askSize();

		int hedgetakesz = TraderEnvSingleton::instance()->_params.hedgeTakeSize();
		hedgeasksz >= hedgetakesz ? sell = hedgetakesz : sell = 0;
		hedgebidsz >= hedgetakesz ? buy = hedgetakesz  : buy = 0;
		FSB_LOG( " TAKE LIQ "
			<< " bid sz " << hedgebidsz
			<< " ask sz " << hedgeasksz
			<< " hdg. take sz " << hedgetakesz
			<< " sell " << sell
			<< " buy " << buy );
		return;
	}

	void RandomizeSizePolicy::calcQuantity(int& buy, int& sell)
	{
		double randSize = rand()/(double)RAND_MAX;
		int size = TraderEnvSingleton::instance()->_params.baseWorkSize();
		buy = sell =  (int) fsbutils::RoundingFunctions::Round(1.0,(randSize * size * 0.6) + (size * 0.7));
		FSB_LOG(" Randomize " << randSize
			<< " buy " << buy << " sell " << sell << " size " << size);
	}

	void MultipleHedgesSizePolicy::calcQuantity(const Instrument& hedge,
		const int& hedgePos,
		int& buy,
		int& sell)
	{
		buy = sell = 0;
		double fx = TraderEnvSingleton::instance()->_params.fxMult();
		double beta = TraderEnvSingleton::instance()->_params.betaMult();
		double basemid = TraderEnvSingleton::instance()->_base[0]->_instr.mid();
		double basecs = TraderEnvSingleton::instance()->_base[0]->_instr.multiplier();
		int basePosition = TraderEnvSingleton::instance()->_base[0]->_orders.net();
		double baseNotional = basePosition * basemid * basecs* fx;
		double weight = hedge.weight();
		double hedgecs = hedge.multiplier();
		double hedgeContractVal = hedge.midNoLock() * hedgecs * fx;
		int hedgeTarget = (int)fsbutils::RoundingFunctions::Round(1.0,(baseNotional * weight)/hedgeContractVal);
		int tradeSize = hedgePos - hedgeTarget;
		tradeSize >= 0 ? buy = tradeSize: sell = abs(tradeSize);
		if(!_residue)
			_residue = baseNotional;
		if(!hedge.useForDelta())
			_residue -= (hedgeTarget*hedgeContractVal);
		FSB_LOG(hedge.symbol() 
			<< " " << buy
			<< " " << sell
			<< " Targte " << hedgeTarget
			<< " tradeSize " << tradeSize
			<< " hedgePos " << hedgePos
			<< " baseNotional " << baseNotional
			<< " _residue " << _residue);
	}

	void MultipleHedgesSizePolicy::calcResidue(const Instrument& hedge,
		const int& hedgePos,
		bool isBuy,
		int& buy, 
		int& sell)
	{
		buy=sell=0;
		double fx = TraderEnvSingleton::instance()->_params.fxMult();
		double hedgeContractVal = hedge.midNoLock() * hedge.multiplier() * fx;
		int hedgeTarget = (int)fsbutils::RoundingFunctions::Round(1.0,_residue/hedgeContractVal);
		int tradeSize = hedgePos - hedgeTarget;
		if(isBuy && tradeSize > 0 )
			tradeSize = 0;
		else if( !isBuy && tradeSize < 0 )
			tradeSize = 0;	
		tradeSize < 0 ? sell = abs(tradeSize) : buy = tradeSize;
		FSB_LOG(" Residue " << _residue
			<< " CVal " << hedgeContractVal
			<< " TradeSize " << tradeSize
			<< " Target " << hedgeTarget
			<< " Pos " << hedgePos);
		_residue = 0.0;
		
	}

	void MultipleHedgesDeltaCloseSizePolicy::calcQuantity(const fsb::Instrument &hedge, int &buy, int &sell)
	{
		buy = sell = 0;
	}

	void HedgeByBaseFills::calcQuantity(const OrderReplyInfo& reply, int &buy, int& sell)
	{
		buy = sell = 0;
		reply._qtyFilled > 0 ? sell = reply._qtyFilled : buy = abs(reply._qtyFilled);
	}

	void MMKRWorkSizePolicy::calcQuantity(const Instrument& instr, int& buy, int& sell)
	{
		double mid = instr.midNoLock();
		int worksize = TraderEnvSingleton::instance()->_params.baseWorkSize();

		sell = buy = (int)fsbutils::RoundingFunctions::Round(instr.lotsize(),worksize/mid);
		FSB_LOG(" " << instr.symbol()
			<< " " << instr.lotsize()
			<< " " << worksize
			<< " " << mid
			<< " " << buy
			<< " " << sell);
	}

	int DefaultMaxPositionPolicy::calcMaxPosition(const Instrument& instr,int net)
	{
		int maxPos = 0;
		if( TraderEnvSingleton::instance()->_params.closeOnly() ) {
			FSB_LOG("Close only mode is on");
		} else
			maxPos = TraderEnvSingleton::instance()->_params.maxPosition();
		return maxPos;
	}

	int MaxPositionBaseNPolicy::calcMaxPosition(const Instrument& instr,int net)
	{
		int maxPos = 0;
		if( TraderEnvSingleton::instance()->_params.closeOnly() ) {
			FSB_LOG("Close only mode is on " );
		} else {
			maxPos = (int) (( fabs(instr.weight()) * TraderEnvSingleton::instance()->_params.maxPosition())/instr.midNoLock());
		}
		return maxPos;
	}

	void ETFPaperTraderHedgeSizePolicy::calcQuantity(const Instrument&,const OrderReplyInfo& reply,int& buy,int& sell)
	{
		/*int hedgebidsz = TraderEnvSingleton::instance()->_hedge[0]->_instr.bidSize();
		int hedgeasksz = TraderEnvSingleton::instance()->_hedge[0]->_instr.askSize();*/

		int hedgetakesz = TraderEnvSingleton::instance()->_params.hedgeTakeSize();
		buy = sell = reply._qtyFilled/hedgetakesz;
		reply._isBuy ? buy = 0,sell = sell : sell = 0,buy = abs(buy);
		FSB_LOG( " "
			<< " Size filled " << reply._qtyFilled
			<< " hdg. take sz " << hedgetakesz
			<< " sell " << sell
			<< " buy " << buy );
		return;
	}

	void ProspectSizePolicy::calcQuantity(const Instrument& instr, int& buy, int& sell)
	{
		int index = instr.index();
		int net = (*TraderEnvSingleton::instance()->_instrObjItrs[index])->_orders.net();
		net > 0 ? buy = 0,sell = 100 : net < 0 ? sell = 0,buy = 100 : sell = 0,buy =0;
		FSB_LOG(" net " << net
			<< " buy " << buy
			<< " sell " << sell);
	}

	void WeightsSizePolicy::calcQuantity(const fsb::Instrument &instr, int &buy, int &sell)
	{
		//All this so there is no shortsale for now.
		int index = instr.index();
		float wt = instr.weight();
		double pflDlrs = (double) TraderEnvSingleton::instance()->_params.maxPosition();
		int target = 0;
		if(instr.midNoLock() > 0.0) 
			target = (int)(( wt * pflDlrs)/instr.midNoLock());
		else {
			FSB_LOG("Problem with prices " << instr.symbol());
			return;
		}
		int incoming = instr.incomingInv();
		int net = (*TraderEnvSingleton::instance()->_instrObjItrs[index])->_orders.net() + incoming;
		if(wt <= 0.0 ) {
			if( target < 0) target = 0;
			buy=0;
			int pending = (*TraderEnvSingleton::instance()->_instrObjItrs[index])->_orders.qtyPendingSells();		
			//sell = target > 0.0 ? std::min(target, (net - pending)) : (net-pending);
			sell = target - (net - pending);
			sell = sell < 0 ? abs(sell) : 0;
		} else {
			buy = target;
			sell=0;
		}
		FSB_LOG(instr.symbol() 
			<< " buys " << buy 
			<< " sells " << sell
			<< " weight " << wt
			<< " target " << target
			<< " incoming " << incoming
			<< " net " << net);
	}

	int MaxPosUsingCloseTarget::calcMaxPosition(const Instrument& instr,int net)
	{
		int maxPos = TraderEnvSingleton::instance()->_params.maxPosition();
		if( TraderEnvSingleton::instance()->_params.closeOnly() ) {
			int closeShares = TraderEnvSingleton::instance()->_params.closeOnlyTarget();
			
			maxPos = (closeShares > 0 && closeShares <= maxPos) ? closeShares*-1 : 0;
			FSB_LOG("Close only mode is on, close shares " << closeShares << " " << maxPos);
			if( abs(net) == abs(maxPos) ) {
				FSB_LOG(" Reached target Pos " << maxPos << " " << net);
				throw ReachedTargetShares();
			}		
		} 
		return maxPos;
	}
}

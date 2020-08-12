#ifndef __ETFTRADER_POLICY_H__
#define __ETFTRADER_POLICY_H__

#include "Mode.h"
#include "Spider.h"

namespace fsb {

	class SPDRNby1MktUpdatePolicy
	{
	public:
		SPDRNby1MktUpdatePolicy()
			:spdrFv("NoBSThreshold"),
			_workLiqBase1("WORK"),
			_workLiqBaseN("WORK")
		{
			_bases.reserve(20);
			_hedges.reserve(20);
		}

		void callMarketUpdate(Orders::OrdersVector& neworders,
			Orders::OrdersVector& cancels)
		{
			std::for_each(TraderEnvSingleton::instance()->_base.begin(),
				TraderEnvSingleton::instance()->_base.end(), copy_instrument(_bases));
			std::for_each(TraderEnvSingleton::instance()->_hedge.begin(),
				TraderEnvSingleton::instance()->_hedge.end(), copy_instrument(_hedges));
			//std::cout << " There are " << _bases.size() << " " << _hedges.size() << std::endl;
			double basketBid = 0.0, basketAsk=0.0;
			int ret = spdrFv.fv(basketBid,basketAsk,_bases,_hedges);
			if(  ret == BinaryPolicy::ZERO_QUOTES )
			{
				_bases.clear();
				_hedges.clear();
				return;
			}
			//std::cout << " " << basketBid << " " << basketAsk << std::endl;
			_base = _bases[0];
			_hedge = _hedges[0];
			int itrnum = 0;
			for(Instrument::VectorOfInstrs::iterator i = _bases.begin(); i!= _bases.end(); ++i,itrnum++) {
				double fvBid=0.0,fvAsk=0.0;
				spdrFv.fv(fvBid,fvAsk,*i,_hedge);//by instrument
				if(itrnum == 0) {
					/*FSB_LOG("Evaluating first " << i->symbol()
						<< " " << TraderEnvSingleton::instance()->_base[itrnum]->_instr.symbol());*/
					_workLiqBase1.evaluate(*i,
						TraderEnvSingleton::instance()->_base[itrnum]->_orders,
						fvBid,fvAsk,true,neworders,cancels);
					_workLiqBase1.evaluate(*i,
						TraderEnvSingleton::instance()->_base[itrnum]->_orders,
						fvBid,fvAsk,false,neworders,cancels);
				} else {
					/*FSB_LOG("Evaluating next " << i->symbol()
						<< " " << TraderEnvSingleton::instance()->_base[itrnum]->_instr.symbol());*/
					_workLiqBaseN.evaluate(*i,
						TraderEnvSingleton::instance()->_base[itrnum]->_orders,
						fvBid,fvAsk,true,neworders,cancels);
					_workLiqBaseN.evaluate(*i,
						TraderEnvSingleton::instance()->_base[itrnum]->_orders,
						fvBid,fvAsk,false,neworders,cancels);
				}
				TraderEnvSingleton::instance()->_base[itrnum]->_pricers.updateFv(fvBid,fvAsk);
			}
			if(!_bases.empty())
				_bases.clear();
			if(!_hedges.empty())
				_hedges.clear();
		}	

	private:
		FVPolicy1<SpiderModel,EmptyBSFlag> spdrFv;
		Mode<SpiderWorkLiqPriceMode,SpiderBase1SizePolicy,EquityOrderSizePolicy,SpiderCancelPolicy,true> _workLiqBase1;
		Mode<SpiderWorkLiqPriceMode,SpiderBaseNSizePolicy,EquityOrderSizePolicy,SpiderCancelPolicy,true,MaxPositionBaseNPolicy> _workLiqBaseN;
		Instrument::VectorOfInstrs _bases;
		Instrument::VectorOfInstrs _hedges;
		Instrument _base;
		Instrument _hedge;
	};

	class SPDRNby1HedgingPolicy
	{
	public:
		SPDRNby1HedgingPolicy() 
			:_hedgeTrader("HDG")
		{
			
		}

		bool evaluateCXLD(Orders::OrdersVector& orders,
				Orders::OrdersVector& cancels,
				OrderReplyInfo& reply)
		{
			//ALERT WILL NOT WORK if INVERSE...
			reply._qtyFilled = reply._isBuy ? -1 : 1;
			_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
			return _hedgeTrader.evaluateReply(orders,cancels,reply,_hedge);
		}
		bool evaluateFILL(Orders::OrdersVector& orders,
				Orders::OrdersVector& cancels,
				OrderReplyInfo& reply)
		{
			
			setDelta();
			if( reply._orderSource == WORK ||
				reply._orderSource == TAKE ||
				reply._orderSource == CLOSE_TAKE ||
				reply._orderSource == CLOSE_WORK ||
				reply._orderSource == CLOSE ) {
				_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
				_hedgeTrader.evaluateReply(orders,cancels,reply,_hedge);
			}
			return true;
		}
	private:
		void setDelta() {
			_notionalFuncObj.reset();
			std::for_each(TraderEnvSingleton::instance()->_base.begin(),
				TraderEnvSingleton::instance()->_base.end(),_notionalFuncObj);
			double baseNotional = _notionalFuncObj.getNotional();
			int bbuys = _notionalFuncObj.getTotalBuys();
			int bsells = _notionalFuncObj.getTotalSells();
			TraderEnvSingleton::instance()->_basePos.setDelta(baseNotional);
			TraderEnvSingleton::instance()->_basePos.setBuys(bbuys);
			TraderEnvSingleton::instance()->_basePos.setSells(bsells);

			_notionalFuncObj.reset();
			std::for_each(TraderEnvSingleton::instance()->_hedge.begin(),
				TraderEnvSingleton::instance()->_hedge.end(),_notionalFuncObj);
			double hedgeNotional = _notionalFuncObj.getNotional();
			TraderEnvSingleton::instance()->_hedgePos.setDelta(hedgeNotional);
			int hbuys = _notionalFuncObj.getTotalBuys();
			TraderEnvSingleton::instance()->_hedgePos.setBuys(hbuys);
			int hsells = _notionalFuncObj.getTotalSells();
			TraderEnvSingleton::instance()->_hedgePos.setSells(hsells);

			TraderEnvSingleton::instance()->_totalPos.setBuys( (bbuys + hbuys));
			TraderEnvSingleton::instance()->_totalPos.setSells(bsells+hsells);
			TraderEnvSingleton::instance()->_totalPos.setDelta(hedgeNotional+baseNotional);
		}
		Instrument _hedge;
		InstrumentObj_func_obj _notionalFuncObj;
		Mode<HedgePriceMode,SpiderHedgeSizePolicy,HedgeOrderSizePolicy,SpiderHedgeCancelPolicy,true,NoMaxPosPolicy> _hedgeTrader;
	};

	class ETF1by1MktUpdatePolicy
	{
	public:
		ETF1by1MktUpdatePolicy()
			:_workLiq("WORK","DAY","LMT",true),
			_takeLiq("TAKE","DAY","IOC"),
			_noChangeMode("NCHG"),
			_paperTraderFV(TraderEnvSingleton::instance()->_params.buySellFlagPolicy()),
			_closeModePTFV(TraderEnvSingleton::instance()->_params.buySellFlagPolicy())
		{
		}
		
		void callMarketUpdate(Orders::OrdersVector& neworders,
			Orders::OrdersVector& cancels)
		{
			_base = TraderEnvSingleton::instance()->_base[0]->_instr;
			_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
			double theoBid=0.0,theoAsk=0.0;
			int bsSignal = 0;
			if( TraderEnvSingleton::instance()->_params.closeOnly() )
				bsSignal ^= _closeModePTFV.fv(theoBid,theoAsk,_base,_hedge);
			else 
				bsSignal ^= _paperTraderFV.fv(theoBid,theoAsk,_base,_hedge);
			TraderEnvSingleton::instance()->_base[0]->_pricers.updateFv(theoBid,theoAsk);
			if( theoBid == 0.0 || theoAsk == 0.0) {
				FSB_LOG("Error do not send orders, something changed, theoBid = " << theoBid
					<< " theoAsk= " << theoAsk);
				TraderEnvSingleton::instance()->_base[0]->_orders.getAllOrders(cancels);
				if(!cancels.empty()) {
					FSB_LOG("Will cancel all orders " << cancels.size());
				}
				return;
			}
			if(!TraderEnvSingleton::instance()->_params.bsFlag() ) {
				bsSignal = 0;
				bsSignal ^= BinaryPolicy::BUY;
				bsSignal ^= BinaryPolicy::SELL;
				FSB_LOG(" bs flag off "
					<< ( (bsSignal&BinaryPolicy::SELL && bsSignal & BinaryPolicy::BUY) ? "BOTH" : "PROBLEM") );
			}
			//TraderEnvSingleton::instance()->_base[0]->_pricers.updateFv(theoBid,theoAsk);
			//TraderEnvSingleton::instance()->_hedge[0]->_pricers.updateFv(0.0,0.0);
			if(bsSignal & BinaryPolicy::NO_CHANGE) {
				double buyPrice = 0.0, sellPrice=0.0;
				buyPrice = _workLiq.price(_base,theoBid,theoAsk,true);
				_noChangeMode.evaluateCancel(cancels,true,theoBid,buyPrice);
				sellPrice = _workLiq.price(_base,theoBid,theoAsk,false);
				_noChangeMode.evaluateCancel(cancels,false,theoAsk,sellPrice);
				return;
			}		
			if(bsSignal & BinaryPolicy::SELL )
				evaluateBS(neworders,cancels,theoBid,theoAsk,false);
			if(bsSignal & BinaryPolicy::BUY )
				evaluateBS(neworders,cancels,theoBid,theoAsk,true);		
		}

		void evaluateBS(Orders::OrdersVector& neworders,
			Orders::OrdersVector& cancels,
			const double& theoBid, 
			const double& theoAsk,
			bool isBuy)
		{
			if( TraderEnvSingleton::instance()->_params.takeLiq() ) {
				if( _takeLiq.evaluate(_base,
					TraderEnvSingleton::instance()->_base[0]->_orders,
					theoBid,theoAsk,
					isBuy,
					neworders,
					cancels) )
					return;
			}

			if( TraderEnvSingleton::instance()->_params.workLiq() ) {
				_workLiq.evaluate(_base,
					TraderEnvSingleton::instance()->_base[0]->_orders,
					theoBid,theoAsk,
					isBuy,
					neworders,
					cancels);
			}
		}

	private:
		Instrument _base;
		Instrument _hedge;
		FVPolicy1<PTBaseFV,EmptyBSFlag> _paperTraderFV;
		FVPolicy1<PTBaseCloseFv,EmptyBSFlag> _closeModePTFV;
		//Mode<CloseOnlyPriceMode,EmptySizePolicy,EquityOrderSizePolicy,CloseOnlyModeCancel,true> _closeOnlyMode;	
		Mode<TakeLiqPriceMode,TakeLiqSizePolicy,EquityOrderSizePolicy,MMKRTakeCancelPolicy,true,MaxPosUsingCloseTarget> _takeLiq; 
		Mode<WorkLiqPriceMode,WorkLiqSizePolicy,EquityOrderSizePolicy,MMKRWorkCancelPolicy,true,MaxPosUsingCloseTarget> _workLiq;
		Mode<WorkLiqPriceMode,EmptySizePolicy,EquityOrderSizePolicy,CancelUpdateThreshold,false> _noChangeMode;
		//Mode<WorkLiqPriceMode,ETFWorkLiqSizePolicy,EquityOrderSizePolicy,MMKRWorkCancelPolicy,true> _workLiq;
	};

	
	class ETFTrader1by1HedgePolicy
	{
	public:
		ETFTrader1by1HedgePolicy() 
			:_hedgeTrader("HDG"),
			_firstTime(true)
		{
			
		}
		bool evaluateFILL(Orders::OrdersVector& orders,
				Orders::OrdersVector& cancels,
				OrderReplyInfo& reply)
		{
			if( _firstTime ) {
			 _baseLevRatio = TraderEnvSingleton::instance()->_base[0]->_instr.levRatio();
			 _hedgeLevRatio = TraderEnvSingleton::instance()->_hedge[0]->_instr.levRatio();
			unsigned int baseSignFld = fsbutils::Sign::GetSignBit(_baseLevRatio);
			unsigned int hdgSignFld = fsbutils::Sign::GetSignBit(_hedgeLevRatio);
			bool inverse = baseSignFld != hdgSignFld ? true : false;
			_hedgeTrader.setInverse(inverse);
			FSB_LOG(" Setting inverse " << inverse);
			_firstTime = false;
			}
			double hedgeNotional = TraderEnvSingleton::instance()->_hedge[0]->_instr.midNoLock() *  TraderEnvSingleton::instance()->_hedge[0]->_orders.net() * _hedgeLevRatio * TraderEnvSingleton::instance()->_hedge[0]->_instr.multiplier();
			double baseNotional = TraderEnvSingleton::instance()->_base[0]->_instr.midNoLock() *  TraderEnvSingleton::instance()->_base[0]->_orders.net() * _baseLevRatio * TraderEnvSingleton::instance()->_base[0]->_instr.multiplier();
			TraderEnvSingleton::instance()->_totalPos.setDelta(baseNotional + hedgeNotional);
			if( reply._orderSource == WORK ||
				reply._orderSource == TAKE ||
				reply._orderSource == CLOSE_TAKE ||
				reply._orderSource == CLOSE_WORK ||
				reply._orderSource == CLOSE ) {
				_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
				_hedgeTrader.evaluateReply(orders,cancels,reply,_hedge);
			}
			return true;
		}
	private:
		bool _firstTime;
		double _baseLevRatio;
		double _hedgeLevRatio;
		Instrument _hedge;
		Mode<HedgePriceMode,DeltaCloseUsingLevRatio,HedgeOrderSizePolicy,SellshortCancel,true,NoMaxPosPolicy> _hedgeTrader;
	};

	class ETFPaperTrader1by1HedgePolicy
	{
	public:
		ETFPaperTrader1by1HedgePolicy() 
			:_hedgeTrader("HDG")
		{
			
		}
		bool evaluateCXLD(Orders::OrdersVector& orders,
				Orders::OrdersVector& cancels,
				OrderReplyInfo& reply)
		{
			//ALERT WILL NOT WORK if INVERSE...
			reply._qtyFilled = reply._isBuy ? -1 : 1;
			_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
			return _hedgeTrader.evaluateReply(orders,cancels,reply,_hedge);
		}
		bool evaluateFILL(Orders::OrdersVector& orders,
				Orders::OrdersVector& cancels,
				OrderReplyInfo& reply)
		{
			/*double hedgeNotional = TraderEnvSingleton::instance()->_hedge[0]->_instr.midNoLock() *  TraderEnvSingleton::instance()->_hedge[0]->_orders.net() * TraderEnvSingleton::instance()->_hedge[0]->_instr.multiplier();
			double baseNotional = TraderEnvSingleton::instance()->_base[0]->_instr.midNoLock() *  TraderEnvSingleton::instance()->_base[0]->_orders.net() * TraderEnvSingleton::instance()->_base[0]->_instr.multiplier();
			TraderEnvSingleton::instance()->_totalPos.setDelta(baseNotional + hedgeNotional);*/
			if( reply._orderSource == WORK ||
				reply._orderSource == TAKE ||
				reply._orderSource == CLOSE_TAKE ||
				reply._orderSource == CLOSE_WORK ||
				reply._orderSource == CLOSE ) {
				_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
				_hedgeTrader.evaluateReply(orders,cancels,reply,_hedge);
			}
			return true;
		}
	private:
		Instrument _hedge;
		//Mode<HedgePriceMode,ETFPaperTraderHedgeSizePolicy,HedgeOrderSizePolicy,EmptyCancelPolicy2,true,NoMaxPosPolicy> _hedgeTrader;
		Mode<HedgePriceMode,SpiderHedgeSizePolicy,HedgeOrderSizePolicy,EmptyCancelPolicy2,true,NoMaxPosPolicy> _hedgeTrader;
	};

	
}
#endif

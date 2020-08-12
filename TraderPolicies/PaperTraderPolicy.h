#ifndef __PAPER_TRADER_POLICY_H
#define __PAPER_TRADER_POLICY_H


#include "Mode.h"

namespace fsb {

	class PaperTrader1By1MktUpdatePolicy
	{
	public:
		PaperTrader1By1MktUpdatePolicy()
			:_workLiq("WORK","DAY","LMT"),
			_takeLiq("TAKE","IOC","LMT"),
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
		Mode<TakeLiqPriceMode,TakeLiqSizePolicy,OrderSizePolicy,MMKRTakeCancelPolicy> _takeLiq; 
		Mode<WorkLiqPriceMode,WorkLiqSizePolicy,OrderSizePolicy,MMKRWorkCancelPolicy> _workLiq;
		Mode<WorkLiqPriceMode,EmptySizePolicy,OrderSizePolicy,CancelUpdateThreshold> _noChangeMode;
	};

	class PaperTrader1by1HedgePolicy
	{
	public:
		PaperTrader1by1HedgePolicy() 
			:_hedgeTrader("HDG"),
			_deltaTrader("DLTA")
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
			double hedgeNotional = TraderEnvSingleton::instance()->_hedge[0]->_instr.midNoLock() *  TraderEnvSingleton::instance()->_hedge[0]->_orders.net() * TraderEnvSingleton::instance()->_hedge[0]->_instr.multiplier();
			double baseNotional = TraderEnvSingleton::instance()->_base[0]->_instr.midNoLock() *  TraderEnvSingleton::instance()->_base[0]->_orders.net() * TraderEnvSingleton::instance()->_base[0]->_instr.multiplier();
			TraderEnvSingleton::instance()->_totalPos.setDelta(baseNotional + hedgeNotional);

			if( reply._orderSource == WORK ||
				reply._orderSource == TAKE ||
				reply._orderSource == CLOSE_TAKE ||
				reply._orderSource == CLOSE_WORK ||
				reply._orderSource == CLOSE ) {
				_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
				if( TraderEnvSingleton::instance()->_params.deltaClose() ) {
					return _deltaTrader.evaluateReply(orders,cancels,reply,_hedge);
				} else {
					return _hedgeTrader.evaluateReply(orders,cancels,reply,_hedge);
				}
			}
			return false;
		}
	private:
		Instrument _hedge;
		Mode<HedgePriceMode,HedgeByBaseFills,HedgeOrderSizePolicy,EmptyCancelPolicy,false,NoMaxPosPolicy > _hedgeTrader;
		Mode<HedgePriceMode,DeltaCloseSizePolicy,HedgeOrderSizePolicy,EmptyCancelPolicy,false,NoMaxPosPolicy > _deltaTrader;
	};

}
#endif
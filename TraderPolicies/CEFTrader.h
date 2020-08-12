#ifndef __CEFTRADER_H___
#define __CEFTRADER_H___

#include "Mode.h"
#include "CEFfv.h"

namespace fsb {

	class CEF1by1MktUpdatePolicy
	{
	public:
		CEF1by1MktUpdatePolicy()
			:_workLiq("WORK","DAY","LMT",true),
			_takeLiq("TAKE","DAY","IOC"),
			_noChangeMode("NCHG"),
			_cefFV(TraderEnvSingleton::instance()->_params.buySellFlagPolicy())
			//_closeModePTFV(TraderEnvSingleton::instance()->_params.buySellFlagPolicy())
		{
		}
		
		void callMarketUpdate(Orders::OrdersVector& neworders,
			Orders::OrdersVector& cancels)
		{
			_base = TraderEnvSingleton::instance()->_base[0]->_instr;
			_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
			double theoBid=0.0,theoAsk=0.0;
			int bsSignal = 0;
			//if( TraderEnvSingleton::instance()->_params.closeOnly() )
			bsSignal ^= _cefFV.fv(theoBid,theoAsk,_base,_hedge);

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
		FVPolicy1<CEFBaseFV,EmptyBSFlag> _cefFV;
		//FVPolicy1<PTBaseCloseFv,EmptyBSFlag> _closeModePTFV;
		//Mode<CloseOnlyPriceMode,EmptySizePolicy,EquityOrderSizePolicy,CloseOnlyModeCancel,true> _closeOnlyMode;	
		Mode<TakeLiqPriceMode,TakeLiqSizePolicy,EquityOrderSizePolicy,MMKRTakeCancelPolicy,true,MaxPosUsingCloseTarget> _takeLiq; 
		Mode<WorkLiqPriceMode,WorkLiqSizePolicy,EquityOrderSizePolicy,MMKRWorkCancelPolicy,true,MaxPosUsingCloseTarget> _workLiq;
		Mode<WorkLiqPriceMode,EmptySizePolicy,EquityOrderSizePolicy,CancelUpdateThreshold,false> _noChangeMode;
	};
}
#endif
#ifndef __TRADER_POLICY_H__
#define __TRADER_POLICY_H__

#include "Mode.h"

namespace fsb {

	class EmptyOrderUpdate
	{
	public:
		bool evaluateFILL(Orders::OrdersVector& orders,
					Orders::OrdersVector& cancels,
					OrderReplyInfo& reply) { return false;}
	};
	class EmptyMktUpdate
	{
	public:
		bool evaluateMARKET(Orders::OrdersVector& orders,
					Orders::OrdersVector& cancels) { return false;}
	};

	
	class PTBase1by1MktUpdate
	{
	public:
		PTBase1by1MktUpdate();

		void calcFV(bool closeOnly,int& buySellSignal);
		void calcDelta();

		bool evaluateMARKET(Orders::OrdersVector&,
							Orders::OrdersVector&,int,bool);
		bool evaluate(Orders::OrdersVector&,Orders::OrdersVector&,int,bool);
		bool evaluateBSFlag(Orders::OrdersVector&,Orders::OrdersVector&, int,bool);

		template<typename T>
		void GetDataToLog(T& output)
		{
			output._hedgeSymbol = _hedge.symbol();
			output._baseSymbol = _base.symbol();
			output._fvBid = _theoBid;
			output._fvAsk = _theoAsk;
			output._hedgeLast = _hedge.lastNoLock();
			output._hedgeAsk = _hedge.askNoLock();
			output._hedgeBid = _hedge.bidNoLock();
			output._hedgeVolume = _hedge.lastSizeNoLock();
			output._baseAsk = _base.askNoLock();
			output._baseBid = _base.bidNoLock();
			output._baseLast = _base.lastNoLock();
			output._baseVolume = _base.lastSizeNoLock();
		}
	protected:
		Mode<CloseOnlyPriceMode,EmptySizePolicy,OrderSizePolicy,CloseOnlyModeCancel> _closeOnlyMode;	
		Mode<TakeLiqPriceMode,TakeLiqSizePolicy,OrderSizePolicy,TakeLiqCancel> _takeLiq; 
		Mode<WorkLiqPriceMode,WorkLiqSizePolicy,OrderSizePolicy,CancelUpdateThreshold> _workLiq;

		FVPolicy1<PTBaseFV,EmptyBSFlag> *_paperTraderFV;
		FVPolicy1<PTBaseCloseFv,EmptyBSFlag> *_closeModePTFV;

		Instrument _base;
		Instrument _hedge;

		int _buySellSignal;
		double _theoBid,_theoAsk;

		SetupOrder _setupOrders;
	};	

	class PTHedge1by1
	{
	public:
		PTHedge1by1();
		bool evaluateFILL(Orders::OrdersVector& orders,
				Orders::OrdersVector& cancels,
				OrderReplyInfo& reply);
	private:
		Instrument _hedge;
		SetupOrder _orderSetup;
		Mode<HedgePriceMode,HedgeByBaseFills,HedgeOrderSizePolicy,EmptyCancelPolicy,false,NoMaxPosPolicy > _hedgeTrader;
		Mode<HedgePriceMode,DeltaCloseSizePolicy,HedgeOrderSizePolicy,EmptyCancelPolicy,false,NoMaxPosPolicy > _hedgeDelta;
	};

	class PTHedge1byN
	{
	public:
		PTHedge1byN();
		bool evaluateFILL(Orders::OrdersVector& orders,
				Orders::OrdersVector& cancels,
				OrderReplyInfo& reply);
	private:
		Instrument _hedge;
		SetupOrder _orderSetup;
		Mode<HedgePriceMode,MultipleHedgesSizePolicy,HedgeOrderSizePolicy,EmptyCancelPolicy,false,NoMaxPosPolicy> _multiplehedgeTrader;
	};
	
	class PTBase1byNMktUpdate : public PTBase1by1MktUpdate
	{
	public:
		PTBase1byNMktUpdate();

		void calcFV(bool closeOnly,int& buySellSignal);
		void calcDelta();

	protected:
		Instrument::VectorOfInstrs _baseInstrs;
		Instrument::VectorOfInstrs _hedgeInstrs;
		double _baseNotional,_hedgeNotional;

	};

	
/*****************************************************************/
	class PTBaseCrncyMktUpdate : public PTBase1by1MktUpdate
	{
		public:
		PTBaseCrncyMktUpdate();

		void calcFV(bool closeOnly,int& buySellSignal);
		void calcDelta();

		protected:
	};
	class PTInverseHedge1by1
	{
	public:
		PTInverseHedge1by1();
		bool evaluateFILL(Orders::OrdersVector& orders,
				Orders::OrdersVector& cancels,
				OrderReplyInfo& reply);
	private:
		Instrument _hedge;
		SetupOrder _orderSetup;
		Mode<HedgePriceMode,InverseHedgeByBaseFills2,HedgeOrderSizePolicy,EmptyCancelPolicy,false,NoMaxPosPolicy > _hedgeTrader;
		Mode<HedgePriceMode,InverseDeltaCloseSizePolicy,HedgeOrderSizePolicy,EmptyCancelPolicy,false,NoMaxPosPolicy> _hedgeDelta;
	};

	class MMKR1by1MktUpdate
	{
	public:
		MMKR1by1MktUpdate();

		void calcFV(bool closeOnly,int& buySellSignal){ }
		bool evaluateMARKET(Orders::OrdersVector&,
							Orders::OrdersVector&,int,bool);
		void calcDelta() { }
		//************ (TO DO) BETTER LOGGING OF VALUES***************
		template<typename T>
		void GetDataToLog(T& output)
		{
			output._hedgeSymbol = _hedge.symbol();
			output._baseSymbol = _base.symbol();
			output._fvBid = _theoBid;
			output._fvAsk = _theoAsk;
			output._hedgeLast = _hedge.lastNoLock();
			output._hedgeAsk = _hedge.askNoLock();
			output._hedgeBid = _hedge.bidNoLock();
			output._hedgeVolume = _hedge.lastSizeNoLock();
			output._baseAsk = _base.askNoLock();
			output._baseBid = _base.bidNoLock();
			output._baseLast = _base.lastNoLock();
			output._baseVolume = _base.lastSizeNoLock();
		}

	protected:
		Instrument::VectorOfInstrs _baseInstrs;
		Instrument::VectorOfInstrs _hedgeInstrs;
		Instrument _base;
		Instrument _hedge;
		double _theoBid,_theoAsk;
		FVPolicy1<MMKR1by1FV,MMKRBSFlag> _fvPolicy;
		SetupOrder _orderSetup;
		Mode<TakeLiqPriceMode,TakeLiqSizePolicy,OrderSizePolicy,MMKRTakeCancelPolicy> _takeLiq; 
		Mode<WorkLiqPriceMode,WorkLiqSizePolicy,OrderSizePolicy,MMKRWorkCancelPolicy> _workLiq;
	};

	class MMKrHedging
	{
	public:
		MMKrHedging();
		bool evaluateFILL(Orders::OrdersVector& orders,
				Orders::OrdersVector& cancels,
				OrderReplyInfo& reply);
	private:
		Instrument _base;
		Instrument _hedge;
		void updateDelta(OrderReplyInfo& reply);

	};

	template<class MktUpdatePolicy, class OrderUpdatePolicy>
	class PaperTrader : private MktUpdatePolicy, private OrderUpdatePolicy
	{
	public:
		PaperTrader()
		{
			srand( (unsigned)time( NULL ) );
		}
		void callMarketUpdate(Orders::OrdersVector& orders,
							Orders::OrdersVector& cancels)
		{
			int buySellSignal;
			bool closeOnly = TraderEnvSingleton::instance()->_params.closeOnly();
		
			MktUpdatePolicy::calcFV(closeOnly,buySellSignal);
			//FSB_LOG("evaluateMARKET");

			MktUpdatePolicy::calcDelta();

			MktUpdatePolicy::evaluateMARKET(orders,cancels,buySellSignal,closeOnly);
			//FSB_LOG("evaluateMARKET");
		}

		void callOrderResponseFILL(Orders::OrdersVector& orders,
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply)
		{
			if( reply._orderSource == WORK ||
				reply._orderSource == TAKE ||
				reply._orderSource == CLOSE_TAKE ||
				reply._orderSource == CLOSE_WORK ||
				reply._orderSource == CLOSE ) {
					OrderUpdatePolicy::evaluateFILL(orders,cancels,reply);
			}

		}

		template<typename T>
		void GetOutput(T& o)
		{
			MktUpdatePolicy::GetDataToLog(o);
		}

	};

	/*template<class FVCalcPolicy,
			class BSPolicy >
	class TraderPolicies
	{
	public:	
		TraderPolicies()
			:_fvPolicy("MidThreshold")
		{
		}
		void evaluateMarketUpdates(Orders::OrdersVector& orders,
								Orders::OrdersVector& cancels)
		{

			_fvPolicy.fv(
		}
		private:
		FVPolicy1<FVCalcPolicy,BSPolicy> _fvPolicy;
	};*/

	EXPIMP_TRADERPOLICY_TEMPLATE template class TRADERPOLICIES_API PaperTrader<fsb::PTBase1by1MktUpdate,fsb::PTHedge1by1>;
	EXPIMP_TRADERPOLICY_TEMPLATE template class TRADERPOLICIES_API PaperTrader<fsb::MMKR1by1MktUpdate,fsb::MMKrHedging>;
}
#endif
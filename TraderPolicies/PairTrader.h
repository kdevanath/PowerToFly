#ifndef __PAIR_TRADER_H__
#define __PAIR_TRADER_H__

#include <fstream>
#include "TraderPolicy.h"
#include "PaperTraderPolicy.h"
#include "ETFTraderPolicy.h"
#include "GrantTraderPolicy.h"
#include "CEFTrader.h"

namespace fsb {

	class Trader : public Policy
	{
	public:
		class TradesFileProblem{};
		Trader()
			:_start(false)
		{
		}
		virtual ~Trader();
		virtual void rcvdUserInput(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,UserEventType eventType)
		{
		}
		virtual void rcvdMarketUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels) = 0;
		virtual void rcvdOrderUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply,bool& recorStats) = 0;
		virtual void GetOutput(LogData& data){ }

		void CancelAll(Orders::OrdersVector& cancels)
		{
			InstrumentOrdersPtrItr citr = TraderEnvSingleton::instance()->_instrOrders.begin();
			for(citr; citr != TraderEnvSingleton::instance()->_instrOrders.end(); ++citr) {
				FSB_LOG("Cancelling " << citr->first._symbol << " " << citr->first._exchange );
				citr->second->getAllOrders(cancels);
			}
		}
		void OpenTradesFile(const std::string& filename);
		void WriteTradesToFile();
		
	protected:
		InstrumentObj_func_obj _notionalFuncObj;
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
		
	protected:
		bool _start;
		ofstream* _tradesStream;
		std::string _tradesFilename;
	};

	
	class ETFPaperTrader : public Trader
	{
	public:
		ETFPaperTrader(){}
		int type() const { return fsb::ETFPaperTraderId;}
		std::string name() const { return "PaperETF";}

		ETFPaperTrader *clone() const { return new ETFPaperTrader(*this);}

		virtual void rcvdUserInput(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,UserEventType type)
		{
			FSB_LOG(" Rcvd. User input of type " << USER_EVENT_NAMES[type]);
			if( type == fsb::STOP)
				WriteTradesToFile();
		}

		virtual void rcvdMarketUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels)
		{
			_etfPaperTraderPolicy.callMarketUpdate(neworders,cancels);
		}

		virtual void rcvdOrderUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply,bool& recorStats)
		{
			
			if( reply._orderStatus == OrderReplyInfo::FILL ||
				reply._orderStatus == OrderReplyInfo::PARTIAL ) {
					recorStats = true;
					setDelta();
					_etfPaperHedgePolicy.evaluateFILL(neworders,cancels,reply);
					return;
			}

			if(reply._orderStatus == OrderReplyInfo::CXLD &&
				reply._orderSource == HEDGE) {
					FSB_LOG(" Rcvd. a cancel on hedge order " << reply._odId 
						<< " " << reply._symbol
						<< " " << (reply._isBuy ? "BUY" : "SELL"));
					_etfPaperHedgePolicy.evaluateCXLD(neworders,cancels,reply);
			}
		}

		virtual void GetOutput(LogData& l) { }
	private:
		ETF1by1MktUpdatePolicy _etfPaperTraderPolicy;
		ETFPaperTrader1by1HedgePolicy _etfPaperHedgePolicy;
	};

	class ETFTrader : public Trader
	{
	public:
		ETFTrader(){}
		int type() const { return fsb::ETFTraderId;}
		std::string name() const { return "ETF";}

		ETFTrader *clone() const { return new ETFTrader(*this);}

		virtual void rcvdMarketUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels)
		{
			_etfTraderPolicy.callMarketUpdate(neworders,cancels);
		}

		virtual void rcvdOrderUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply,bool& recorStats)
		{
			
			if( reply._orderStatus == OrderReplyInfo::FILL ||
				reply._orderStatus == OrderReplyInfo::PARTIAL ) {
					recorStats = true;
					_etfHedgerPolicy.evaluateFILL(neworders,cancels,reply);
					return;
			}
			if(reply._orderStatus == OrderReplyInfo::CXLD &&
				reply._orderSource == HEDGE) {
					FSB_LOG(" Rcvd. a cancel on hedge order " << reply._odId 
						<< " " << reply._symbol
						<< " " << (reply._isBuy ? "BUY" : "SELL"));
					_etfHedgerPolicy.evaluateCXLD(neworders,cancels,reply);
			}

		}

		virtual void GetOutput(LogData& l) { }
	private:
		SPDRNby1MktUpdatePolicy _etfTraderPolicy;
		SPDRNby1HedgingPolicy _etfHedgerPolicy;

	};

	class CurrencyPair : public Trader
	{
	public:
		CurrencyPair(){}
		int type() const { return fsb::CurrencyPairId;}
		std::string name() const { return "CRNCY";}

		CurrencyPair *clone() const { return new CurrencyPair(*this);}

		virtual void rcvdMarketUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels)
		{
			_crncyTrader.callMarketUpdate(neworders,cancels);
		}

		virtual void rcvdOrderUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply,bool& recorStats)
		{
			if( reply._orderStatus == OrderReplyInfo::FILL ||
				reply._orderStatus == OrderReplyInfo::PARTIAL ) {
					recorStats = true;
					_crncyTrader.callOrderResponseFILL(neworders,cancels,reply);
			}

		}

		virtual void GetOutput(LogData& l) { _crncyTrader.GetOutput(l);}

	PaperTrader<PTBaseCrncyMktUpdate,PTInverseHedge1by1> _crncyTrader;
	};

	class PairTrader1by1 : public Trader
	{
	public:
		PairTrader1by1(){}
		int type() const { return fsb::OnebyOneId;}
		std::string name() const { return "1by1";}

		PairTrader1by1 *clone() const { return new PairTrader1by1(*this);}

		virtual void rcvdMarketUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels)
		{
			_mktUpdatePolicy.callMarketUpdate(neworders,cancels);
			//_oneByOneTrader.callMarketUpdate(neworders,cancels);
		}

		virtual void rcvdOrderUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply,bool& recorStats)
		{
			if( reply._orderStatus == OrderReplyInfo::FILL ||
				reply._orderStatus == OrderReplyInfo::PARTIAL ) {
					setDelta();
					_hedgePolicy.evaluateFILL(neworders,cancels,reply);
					//_oneByOneTrader.callOrderResponseFILL(neworders,cancels,reply);
			}

		}

		void rcvdUserInput(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,UserEventType eventType)
		{
			if( eventType == UserEventType::STOP) {
				FSB_LOG("Rcvd. a STOP Event will cancel all");
				CancelAll(cancels);
			}
		}

		//virtual void GetOutput(LogData& l) { _oneByOneTrader.GetOutput(l);}

	//PaperTrader<PTBase1by1MktUpdate,PTHedge1by1> _oneByOneTrader;
	PaperTrader1By1MktUpdatePolicy _mktUpdatePolicy;
	PaperTrader1by1HedgePolicy _hedgePolicy;
	};

	class PairTrader1byN : public Trader
	{
	public:
		PairTrader1byN(){}
		int type() const { return fsb::OnebyNId;}
		std::string name() const { return "1byN";}

		PairTrader1byN *clone() const { return new PairTrader1byN(*this);}

		virtual void rcvdMarketUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels)
		{
			_oneByNTrader.callMarketUpdate(neworders,cancels);
		}

		virtual void rcvdOrderUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply,bool& recorStats)
		{
			if( reply._orderStatus == OrderReplyInfo::FILL ||
				reply._orderStatus == OrderReplyInfo::PARTIAL ) {

					recorStats = true;
					if( reply._orderSource == HEDGE ||
						reply._orderSource == CLOSE_DELTA ) {
							FSB_LOG(" It is an order from the hedge side " << reply._symbol)
								return;
					}
				_oneByNTrader.callOrderResponseFILL(neworders,cancels,reply);
			}
		}
	PaperTrader<PTBase1byNMktUpdate,PTHedge1byN> _oneByNTrader;
	};

	class MarketMaker : public Trader
	{
	public:
		MarketMaker(){}
		int type() const { return fsb::MMKRId;}
		std::string name() const { return "MMKR";}

		MarketMaker *clone() const { return new MarketMaker(*this);}

		virtual void rcvdMarketUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels)
		{
			_marketMaker.callMarketUpdate(neworders,cancels);
		}

		virtual void rcvdOrderUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply,bool& recorStats)
		{
			if( reply._orderStatus == OrderReplyInfo::FILL ||
				reply._orderStatus == OrderReplyInfo::PARTIAL ) {
					recorStats = true;
					_marketMaker.callOrderResponseFILL(neworders,cancels,reply);
			}

		}

		virtual void GetOutput(LogData& l) { _marketMaker.GetOutput(l);}

	PaperTrader<MMKR1by1MktUpdate,MMKrHedging> _marketMaker;
	};

	class GTrader : public Trader
	{
	public:
		GTrader(){}
		int type() const { return fsb::GTraderId;}
		std::string name() const { return "GTrader";}

		GTrader *clone() const { return new GTrader(*this);}

		virtual void rcvdUserInput(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,UserEventType type)
		{
			FSB_LOG(" Rcvd. User input of type " << USER_EVENT_NAMES[type]);
			switch( type ) 
			{
			case fsb::START:
				_start = true;
				_grantTrader.evalUserInput(neworders,cancels);
				break;
			case fsb::STOP:
				_start = false;
				_grantTrader.writeTradesFile();
				break;
			default:
				break;
			}

			/*if(_start)
			_grantTrader.evalUserInput(neworders,cancels);
			if(!_start) {
			FSB_LOG(" Write trades to file ");
			_grantTrader.writeTradesFile();
			}*/
		}
		virtual void rcvdMarketUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels)
		{
		}

		virtual void rcvdOrderUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply,bool& recorStats)
		{

			if( reply._orderStatus == OrderReplyInfo::FILL ||
				reply._orderStatus == OrderReplyInfo::PARTIAL ) {
					recorStats = true;
					setDelta();
					return;
			}

			if(reply._orderStatus == OrderReplyInfo::CXLD &&
				reply._orderSource == "EOD") {
					FSB_LOG(" Rcvd. a cancel on base order " << reply._odId 
						<< " " << reply._symbol
						<< " " << (reply._isBuy ? "BUY" : "SELL"));
					if(_start)
						_grantTrader.evalOrderCXLD(neworders,cancels,reply);
					else 
						FSB_LOG(" STOP has been pressed, no replacing cxld orders");
			}
		}

		virtual void GetOutput(LogData& l) { }
	private:
		GrantTrader _grantTrader;
	};

	class CEFTrader : public Trader
	{
		public:
		CEFTrader()
		{}
		int type() const { return fsb::CEFTraderId;}
		std::string name() const { return "CEFTrader";}

		CEFTrader *clone() const { return new CEFTrader(*this);}

		virtual void rcvdUserInput(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,UserEventType type)
		{
			FSB_LOG(" Rcvd. User input of type " << USER_EVENT_NAMES[type]);
			if( type == fsb::STOP)
				WriteTradesToFile();
		}

		virtual void rcvdMarketUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels)
		{
			_cefMktUpdatePolicy.callMarketUpdate(neworders,cancels);
		}

		virtual void rcvdOrderUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply,bool& recorStats)
		{
			if( reply._orderStatus == OrderReplyInfo::FILL ||
				reply._orderStatus == OrderReplyInfo::PARTIAL ) {
					recorStats = true;
					setDelta();
					_cefHedgePolicy.evaluateFILL(neworders,cancels,reply);
					return;
			}

			if(reply._orderStatus == OrderReplyInfo::CXLD &&
				reply._orderSource == HEDGE) {
					FSB_LOG(" Rcvd. a cancel on hedge order " << reply._odId 
						<< " " << reply._symbol
						<< " " << (reply._isBuy ? "BUY" : "SELL"));
					_cefHedgePolicy.evaluateCXLD(neworders,cancels,reply);
			}
		}
	private:	
		CEF1by1MktUpdatePolicy _cefMktUpdatePolicy;
		ETFPaperTrader1by1HedgePolicy _cefHedgePolicy;
	};

	template <class TraderPolicyT>
	class PairTrader
	{
	public:
		PairTrader(TraderPolicyT* trader)
			:_trader(trader)
		{}
		void rcvdMarketUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels)
		{
			std::cout << " I am in PairTrader::rcvdMarketUpdate " << std::endl;
		}
		void rcvdOrderUpdate(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,
			OrderReplyInfo& reply,bool& recorStats)
		{
			
		}
		void rcvdUserInput(Orders::OrdersVector& neworders, 
			Orders::OrdersVector& cancels,UserEventType type)
		{
		}
	private:
		TraderPolicyT* _trader;
	};

}
#endif



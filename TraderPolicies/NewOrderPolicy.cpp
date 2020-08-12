#include "stdafx.h"
#include <Utilities/GenerateOdids.h>
#include "NewOrderPolicy.h"
#include "PolicyFactory.h"
#include <TraderPolicies/TraderEnv.h>
#include <TraderPolicies/UnKnownPolicy.h>
#include <Utilities/FSBLogger.h>
#include <Data/LogData.h>

namespace fsb {

	NewOrderPolicy::NewOrderPolicy(const std::string& name,
		const std::string& algName,
		const std::string& tradesfile)
		:_spreadName(name)
	{
		try
		{
			_trader = dynamic_cast<Trader*>(PolicyFactorySingleton::instance()->create(algName));
		} catch(UnknownPolicy p)
		{
			FSB_LOG("Unknown policy,Exiting....(PTBaseMode/PTHedgeMode)");
			::exit(-1);

		}
		if(!tradesfile.empty()) {
			try {
			FSB_LOG(" Will be writing trades to file " << tradesfile);
			_trader->OpenTradesFile(tradesfile);
			} catch(Trader::TradesFileProblem()) {
				FSB_LOG("Could not open the trades file");
			}
		}
		//_pairTrader = new PairTrader<Trader>(_trader);
	}

	void NewOrderPolicy::respondToUserInput(Orders::OrdersVector& orders,
			Orders::OrdersVector& cancels,UserEventType type)
	{
		if(!orders.empty()) orders.clear();
		if(!cancels.empty()) cancels.clear();	
		_trader->rcvdUserInput(orders,cancels,type);	
		if(type == fsb::START) {
			size_t numOrders = orders.size();
			for(size_t i = 0; i < numOrders;i++) {
				std::string orderid;
				Order& anOrder = orders[i];
				BuildNewOrder(anOrder,orderid);
			}
		} else {
			if(!orders.empty()) orders.clear();
		}
	}

	void NewOrderPolicy::insideOrder(Orders::OrdersVector& orders, 
		Orders::OrdersVector& cancels,
		bool start)
	{
		if(!orders.empty()) orders.clear();
		if(!cancels.empty()) cancels.clear();

		_trader->rcvdMarketUpdate(orders,cancels);
		//_pairTrader->rcvdMarketUpdate(orders,cancels);

		if(start) {
			size_t numOrders = orders.size();
			for(size_t i = 0; i < numOrders;i++) {
				std::string orderid;
				Order& anOrder = orders[i];
				BuildNewOrder(anOrder,orderid);
			}
		} else {
			if(!orders.empty()) orders.clear();
		}
	}

	void NewOrderPolicy::respondToOrderUpdates(OrderReplyInfo& reply,
		Orders::OrdersVector& orders,
		Orders::OrdersVector& cancels,
		bool& recordStats)
	{
		if( !orders.empty() )
			orders.clear();
		if(!cancels.empty()) cancels.clear();

		_trader->rcvdOrderUpdate(orders,cancels, reply,recordStats);
		BuildNewOrder(orders,reply._odId);
	}

	void NewOrderPolicy::GetStats(LogData& data, const OrderReplyInfo& reply)
	{
		//FSB_LOG(" Trying to get output");
		char timebuf[10];
		_trader->GetOutput(data);
		_strtime_s(timebuf, sizeof(timebuf)); 
		data._enumType = LogData::FILL;
		data._timestamp = timebuf;
		data._odId = reply._odId;
		//FSB_LOG(" done Trying to get output");
	}

	void NewOrderPolicy::BuildNewOrder(Order& anOrder,
		std::string& orderId)
	{
		anOrder._name = _spreadName;
		//generateClientId(anOrder._guid);
		fsbutils::OdIdFns::generateClientId(_spreadName.c_str(),_guid,sizeof(_guid));
		anOrder._guid = _guid;
		FSB_LOG(" Generating clientid" << anOrder._guid << " " << anOrder._orderSource);
		if( orderId.empty() ) {
			orderId = anOrder._orderId = anOrder._guid;
		} else
			anOrder._orderId = orderId;
	}

	void NewOrderPolicy::BuildNewOrder(Orders::OrdersVector& orders,
		std::string& orderId)
	{
		int numOrders = orders.size();
		for( int i=0;i<numOrders;i++) {
			Order& anOrder = orders[i];
			anOrder._name = _spreadName;
			fsbutils::OdIdFns::generateClientId(_spreadName.c_str(),_guid,sizeof(_guid));
			anOrder._guid = _guid;
			FSB_LOG(" Generating clientid" << anOrder._guid << " " << anOrder._orderSource);
			if( orderId.empty() ) {
				orderId = anOrder._orderId = anOrder._guid;
			} else
				anOrder._orderId = orderId;
		}
	}
	
}
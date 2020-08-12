#include "stdafx.h"
#include <math.h>
#include <algorithm>
#include "OrderCancellationPolicy.h"
#include <TraderPolicies/TraderEnv.h>
#include <Utilities/FSBMsgsId.h>

namespace fsb {

	bool orderSourceFunc(const Order& o, const std::string& type)
	{
		if(o._orderSource == type)
			return true;
		return false;
	}

	bool takeOrder(const Order& o)
	{
		if(o._orderSource == TAKE)
			return true;
		return false;
	}

	int CancelAllOrders::cancel(Orders::OrdersVector& cancels,const OrderReplyInfo& reply,const double& sendprice)
	{
		int state = OrderCancellationPolicy::SEND_NEW;
		for(int i =0; i<TraderEnvSingleton::instance()->_base.size();++i) {
			if( reply._isBuy) {			
				TraderEnvSingleton::instance()->_base[i]->_orders.getBuyOrders(cancels);
			} else {
				TraderEnvSingleton::instance()->_base[i]->_orders.getSellOrders(cancels);
			}
		}

		if( !cancels.empty())
			state = OrderCancellationPolicy::CANCEL_REPLACE;

		return state;
	}

	int CancelInsideOrders::cancel(Orders::OrdersVector& cancels,bool isBuy,double theoPrice,double sendPrice)
	{
		double cxlThresh = TraderEnvSingleton::instance()->_params.cancelThreshold();
		Orders::OrdersVector potentialCancels;
		for(int i =0; i<TraderEnvSingleton::instance()->_base.size();++i) {
			if(isBuy) {
				TraderEnvSingleton::instance()->_base[i]->_orders.insideBid(potentialCancels);
				Order& order = potentialCancels[0];
				cout << " Looking to cxl buy: theo " << theoPrice 
					<< "Our bid " << order._price
					<< " cxl threshold " << cxlThresh << endl;
				if(order._price >= (theoPrice * (1 + cxlThresh/10000.0))) {
					cancels.push_back(order);
					return CANCEL_REPLACE;
				} else if( order._price < (theoPrice * (1 - cxlThresh/10000.0))) {
					cancels.push_back(order);
					return CANCEL_REPLACE;
				}
			} else {
				TraderEnvSingleton::instance()->_base[i]->_orders.insideOffer(potentialCancels);
				Order& order = potentialCancels[0];
				cout << " Looking to cxl sell: theo " << theoPrice 
					<< "Our offfer " << order._price
					<< " cxl threshold " << cxlThresh << endl;
				if( order._price >= ( theoPrice * (1 + cxlThresh/10000.0) )) {
					cancels.push_back(order);
					return CANCEL_REPLACE;
				} else if( order._price < ( theoPrice * (1 - cxlThresh/10000.0) )) {
					cancels.push_back(order);
					return CANCEL_REPLACE;
				}
			}
		}
		if( !potentialCancels.empty() )potentialCancels.clear();
		return CANCEL;
	}

	int CancelOutsideOrders::cancel(Orders::OrdersVector& cancels,bool isBuy, double fv,double sendPrice)
	{
		Orders::OrdersVector potentialCancels;
		for(int i =0; i<TraderEnvSingleton::instance()->_base.size();++i) {
			TraderEnvSingleton::instance()->_base[i]->_orders.getCancels(isBuy,potentialCancels);
			if( isBuy) {
				int activeBuys = TraderEnvSingleton::instance()->_base[i]->_orders.numPendingBuys();
				maxNumOfOrders(potentialCancels,activeBuys);
				double bestBid = TraderEnvSingleton::instance()->_base[i]->_instr.bid();
				maxPriceDistance(potentialCancels,bestBid);
			} else {
				int activesells = TraderEnvSingleton::instance()->_base[i]->_orders.numPendingSells();
				maxNumOfOrders(potentialCancels,activesells);
				double bestOffer = TraderEnvSingleton::instance()->_base[i]->_instr.ask();
				maxPriceDistance(potentialCancels,bestOffer);
			}
		}
		Orders::OrdersVector::iterator itr;
		for(itr = potentialCancels.begin(); itr != potentialCancels.end(); ++itr) {
			if(itr->_enumStatus != Order::CANCEL ) {
				cancels.push_back(*itr);
				cout << " Cancelling " << endl;
				(*itr).dump();
			}
		}

		return CANCEL;	
	}
	void CancelOutsideOrders::maxNumOfOrders(Orders::OrdersVector& cancels,int activeOrders)
	{
		int maxOrders = TraderEnvSingleton::instance()->_params.maxPending();
		Orders::OrdersVector::iterator itr = cancels.end();
		//Keep cxling until we the maxNumOf orders rule is not breached
		while(activeOrders > maxOrders) {
			Order& co = *(--itr);
			if( co._enumStatus != co.CANCEL) { // the order has already been set to cancel
				co._enumStatus = co.CANCEL;
				cancels.push_back(co);
			}
			--activeOrders;
			FSB_LOG( " Max Number of orders violated for buys "
				<< " active " << activeOrders << " max " << maxOrders 
				<< " Cxling " << co._guid
				<< " " << (co._isBuy ? "BUY":"SELL")
				<< co._symbol
				);		
		}
	}

	void CancelOutsideOrders::maxQtyPending(Orders::OrdersVector& cancels,int pending)
	{
		int maxPending = TraderEnvSingleton::instance()->_params.maxQuantityPending();
		Orders::OrdersVector::iterator itr = cancels.end();
		//Keep cxling until we the maxNumOf orders rule is not breached
		while(pending > maxPending) {
			Order& co = *(--itr);
			if( co._enumStatus != co.CANCEL) { // the order has already been set to cancel
				co._enumStatus = co.CANCEL;
				cancels.push_back(co);
			}
			pending -= (co._quantity - co._quantityFilled);
			FSB_LOG( " Max Quantity Pending rule violated for "
				<< " pending " << pending << " max " <<  maxPending
				<< " Cxling " << co._guid
				<< " " << (co._isBuy ? "BUY":"SELL")
				<< co._symbol
				);
		}
					
	}

	void CancelOutsideOrders::maxPriceDistance(Orders::OrdersVector& cancels,double bestPrice)
	{
		double maxDistance = TraderEnvSingleton::instance()->_params.maxDistance();
		Orders::OrdersVector::reverse_iterator ritr;
		for( ritr = cancels.rbegin(); ritr != cancels.rend(); ++ritr) {
			Order& co = *(ritr);
			if( co._enumStatus != Order::CANCEL &&
				fabs(bestPrice - co._price) >= maxDistance) {
					co._enumStatus = Order::CANCEL;
					FSB_LOG( " Max Price Distance rule violated for "
				<< " price diff. " << bestPrice - co._price << " max " <<  maxDistance
				<< " Cxling " << co._guid
				<< " " << (co._isBuy ? "BUY":"SELL")
				<< co._symbol
				<< " Best " << bestPrice
				<< " " << co._price
				);
			}
		}
	}

	int TakeLiqCancel::cancel(fsb::Orders::OrdersVector &cancels, 
		bool isBuy, 
		double fv,
		double sendPrice)
	{
		if(!_potentialCancels.empty())
			_potentialCancels.clear();

		int state = NONE;
		if(!cancels.empty()) {
			FSB_LOG(" There are cancels already");
			state = CANCEL;
		}
		//if there are orders at this price don't do anything..
		if( TraderEnvSingleton::instance()->_base[0]->_orders.exists(isBuy,sendPrice)) {
			FSB_LOG(" There are orders at " << sendPrice
				<< " " << (isBuy ? "BUY" : "SELL") );
			return state;
		} else
		{
			FSB_LOG(" There are no orders at " << sendPrice
				<< " Will be sending " << (isBuy ? "BUY" : "SELL") );
			return SEND_NEW;
		}

		TraderEnvSingleton::instance()->_base[0]->_orders.getCancels(isBuy,_potentialCancels);
		//if there are then CANCEL_REPLACE
		if( !_potentialCancels.empty() ) {
			FSB_LOG("There are cancels for price  " << sendPrice );
			state = CANCEL_REPLACE;
			copy(_potentialCancels.begin(),_potentialCancels.end(),back_inserter(cancels));
		}  
		if(cancels.empty() )
			state = SEND_NEW;
		return state;
	}
	int CancelUpdateThreshold::cancel(fsb::Orders::OrdersVector &cancels, 
		bool isBuy, 
		double fv,
		double sendPrice)
	{
		int updateThresh = TraderEnvSingleton::instance()->_params.updateThreshold();
		int cxlThreshBetter = TraderEnvSingleton::instance()->_params.cxlThresholdBetter();
		int cxlThreshWorse = TraderEnvSingleton::instance()->_params.cxlThresholdWorse();
		Orders::OrdersVector potentialCancels;
		int state = NONE;
		if( isBuy)
		{
			TraderEnvSingleton::instance()->_base[0]->_orders.insideBid(potentialCancels);
			if(potentialCancels.empty() ) {
				state = SEND_NEW;
				return state;
			}
			int num = potentialCancels.size();
			for(int i=0;i<num;i++ ) {
				Order& order = potentialCancels[i];	
				if(order._extOrderId.empty()) {
					FSB_LOG(" _extOrderId does not exist for " << order._guid);
					continue;
				}
				if( order._enumStatus == Order::CANCEL) {
					FSB_LOG(" Already in Cancel mode " << order._guid);
					continue;
				}
				double baseBid = TraderEnvSingleton::instance()->_base[0]->_instr.bid();

				FSB_LOG("(Buy) Base mkt bid " << baseBid
					<< " fv " << fv
					<< " final send price " << sendPrice
					<< " working bid " << order._price
					<< " with update thresh " << (baseBid * (1 - updateThresh/10000.0))
					<< " with cxl thresh worse " << (fv * (1 + cxlThreshWorse/10000.0))
					<< " with cxl thresh better " << (fv * (1 - cxlThreshBetter/10000.0))
					);

				double priceWithThresh = (baseBid * (1 - updateThresh/10000.0));
				//Check for Market within update threshold
				// (change) if( (fv >= (baseBid * (1 - updateThresh/10000.0))) ||
				if( fv >= priceWithThresh || 
					(order._price >= priceWithThresh) ) 
				{
					FSB_LOG("Buy: Mkt is withing my update threshold, will  cxl/replace");
					if( order._price >= (fv * (1 + cxlThreshWorse/10000.0)) &&
						order._price != sendPrice ) {
						FSB_LOG(" Cxl Thresh Worse was not satisfied");
							order._enumStatus = Order::CANCEL;
							cancels.push_back(order);
							FSB_LOG("Cancelling, existing buy worse than FV, new order will be lower " << order._guid);
							state =  CANCEL;
							//TraderEnvSingleton::instance()->_base[0]->_orders.update(order);
					} else if( order._price < (fv * (1 - cxlThreshBetter/10000.0)) &&
							order._price != sendPrice ) {
							order._enumStatus = Order::CANCEL;
							cancels.push_back(order);
							FSB_LOG("Cancelling, existing buy better than FV, new order will be higher " << order._guid);
							state =  CANCEL;
							//TraderEnvSingleton::instance()->_base._orders.update(order);
					} else {
						FSB_LOG(" Cxl Thresh Better was not satisfied");
					}
				} else
				{
					FSB_LOG("Beyond Update threshold ");
					continue;
				}
			}
		} else {
			TraderEnvSingleton::instance()->_base[0]->_orders.insideOffer(potentialCancels);
			if(potentialCancels.empty() ) {
				state = SEND_NEW;
				return state;
			}
			double baseAsk = TraderEnvSingleton::instance()->_base[0]->_instr.ask();
			double askpriceWThresh = (baseAsk * (1 + updateThresh/10000.0));
			int num = potentialCancels.size();
			for(int i=0;i<num;i++ ) {
				Order& order = potentialCancels[i];
				if( order._enumStatus == Order::CANCEL) {
					FSB_LOG(" Already in Cancel mode " << order._guid);
					continue;
				}
				
				FSB_LOG(" (Sell) Base mkt ask " << baseAsk
					<< " ask fv " << fv
					<< " working ask " << order._price
					<< " final send price " << sendPrice
					<< " with update thresh " << askpriceWThresh
					<< " with cxl thresh worse " << (sendPrice * (1 - cxlThreshWorse/10000.0))
					<< " with cxl thresh better " << (sendPrice * (1 + cxlThreshBetter/10000.0))
					);
				//if(  (fv <= (baseAsk * (1 + updateThresh/10000.0))) 
				if( fv <= askpriceWThresh ||
					( order._price <= askpriceWThresh ) ) {
					FSB_LOG("Sell: Mkt is withing my update threshold, will  cxl/replace");
					if( order._price <= (fv * (1 - cxlThreshWorse/10000.0)) &&
						order._price != sendPrice ) {
							order._enumStatus = Order::CANCEL;
							cancels.push_back(order);
							FSB_LOG("Cancelling, existing sell worse than FV, new order will be lower " << order._guid);
							state =  CANCEL;
							//TraderEnvSingleton::instance()->_base._orders.update(order);
					} else if(order._price > (fv * (1 + cxlThreshBetter/10000.0)) &&
						order._price != sendPrice ) {
						FSB_LOG(" Cxl Thresh Worse was not satisfied");
							order._enumStatus = Order::CANCEL;
							cancels.push_back(order);
							FSB_LOG("Cancelling, existing sell better than FV, new order will be higher " << order._guid);
							state =  CANCEL;
							//TraderEnvSingleton::instance()->_base[0]->_orders.update(order);				
					}
				} else {
						FSB_LOG(" Market beyond my range ");
						continue;
				}
			}
		}
		if( !potentialCancels.empty() )
			potentialCancels.clear();

		return state;

	}

	int CloseOnlyModeCancel::cancel(fsb::Orders::OrdersVector &cancels, 
		bool isBuy,
		double fv,
		double sendPrice)
	{
		int state = OrderCancellationPolicy::NONE;
		if(!cancels.empty()) {
			FSB_LOG(" There are cancels already");
			state = OrderCancellationPolicy::CANCEL;
		}
		int net = TraderEnvSingleton::instance()->_base[0]->_orders.net();
		if( net == 0 )
		{
			FSB_LOG("net is 0, no need to look for cancels");
			return state;
		}
		if( abs(net) > 1 )
		{
			FSB_LOG("In close only mode, net = " << net << " > 1, go the usual way");
			return state;
		}
		// buy mode, we are short and there is a pending buy already
		int pendingBuy = TraderEnvSingleton::instance()->_base[0]->_orders.numPendingBuys();
		if( isBuy && pendingBuy ) {
			TraderEnvSingleton::instance()->_base[0]->_orders.getBuyOrders(cancels);
			return OrderCancellationPolicy::CANCEL;
		}

		int pendingSell = TraderEnvSingleton::instance()->_base[0]->_orders.numPendingSells();
		if(!isBuy && pendingSell) {
			TraderEnvSingleton::instance()->_base[0]->_orders.getSellOrders(cancels);
			return OrderCancellationPolicy::CANCEL;
		}
		return state;
	}

	int MMKRTakeCancelPolicy::cancel(fsb::Orders::OrdersVector &cancels,
		const Instrument& instr,
		Orders& orders,
		bool isBuy, 
		double fv,
		double sendPrice)
	{
		Orders::OrdersVector potentialCancels;
		
		int state = OrderCancellationPolicy::NONE;
		if(!cancels.empty()) {
			FSB_LOG(" There are cancels already");
			state = OrderCancellationPolicy::CANCEL;
		}

		if(isBuy)
			orders.insideBid(potentialCancels);
		else
			orders.insideOffer(potentialCancels);

		int num = potentialCancels.size();
		bool takeOrders = false;
		Orders::OrdersVector::iterator result = std::find_if(potentialCancels.begin(),potentialCancels.end(),&takeOrder);
		if( result == potentialCancels.end() ) {
			FSB_LOG("There are no take orders will send new orders ");
			state = OrderCancellationPolicy::SEND_NEW;
		} else {
			FSB_LOG("There are take orders will not send new orders ");
		}

		if(!potentialCancels.empty())
			potentialCancels.clear();
		return state;
	}

	int MMKRWorkCancelPolicy::cancel(fsb::Orders::OrdersVector &cancels,
		const Instrument& instr,
		Orders& orders,
		bool isBuy, 
		double fv,
		double sendPrice)
	{
		int updateThresh = TraderEnvSingleton::instance()->_params.updateThreshold();
		int cxlThreshBetter = TraderEnvSingleton::instance()->_params.cxlThresholdBetter();
		int cxlThreshWorse = TraderEnvSingleton::instance()->_params.cxlThresholdWorse();
		int maxPending = TraderEnvSingleton::instance()->_params.maxPending();
		Orders::OrdersVector potentialCancels;
		int state = OrderCancellationPolicy::NONE;
		if( isBuy)
		{
			orders.getBuyOrders(potentialCancels);
			if(potentialCancels.empty() ) {
				state = OrderCancellationPolicy::SEND_NEW;
				return state;
			}
			int num = potentialCancels.size();
			FSB_LOG(" There are " << num << " buys ");
			for(int i=0;i<num;i++ ) {
				Order& order = potentialCancels[i];	
				if(order._extOrderId.empty()) {
					FSB_LOG("The external order id does not exist, have not rcvd. active yet");
					continue;
				}
				FSB_LOG("Order status = " << order._enumStatus);
				if( order._enumStatus == Order::CANCEL) {
					FSB_LOG(" Already in Cancel mode " << order._guid);
					continue;
				}
				double baseBid = instr.bidNoLock();
				double updThresh = (baseBid * (1 - updateThresh/10000.0));
				FSB_LOG("(Buy) Base mkt bid " << baseBid
					<< " fv " << fv
					<< " final send price " << sendPrice
					<< " working bid " << order._price
					<< " with update thresh " << updThresh
					<< " with cxl thresh worse " << (sendPrice * (1 + cxlThreshWorse/10000.0))
					<< " with cxl thresh better " << (sendPrice * (1 - cxlThreshBetter/10000.0))
					);

				//Check for Market within update threshold
				if( (sendPrice >= updThresh) || //change**
					(order._price > updThresh) ) 
				{
					FSB_LOG("Buy: Mkt is withing my update threshold, will  cxl/replace");
					if( order._price > (sendPrice * (1 + cxlThreshWorse/10000.0)) ) {
							order._enumStatus = Order::CANCEL;
							cancels.push_back(order);
							FSB_LOG("Cancelling, existing buy worse than FV, new order will be lower " << order._guid);
							state =  OrderCancellationPolicy::CANCEL;
							if(maxPending > 1) {
								state = OrderCancellationPolicy::CANCEL_REPLACE;
								FSB_LOG("Will cancel replace");
							}
					} 
					if( order._price < (sendPrice * (1 - cxlThreshBetter/10000.0)) ) {
							order._enumStatus = Order::CANCEL;
							cancels.push_back(order);
							FSB_LOG("Cancelling, existing buy better than FV, new order will be higher " << order._guid);
							state =  OrderCancellationPolicy::CANCEL;
							if(maxPending > 1) {
								state = OrderCancellationPolicy::CANCEL_REPLACE;
								FSB_LOG("Will cancel replace");
							}
					}
				} else {
					FSB_LOG("Beyond Update threshold ");
					continue;
				}
			}
		} else {
			//TraderEnvSingleton::instance()->_base[0]->_orders.insideOffer(potentialCancels);
			orders.getSellOrders(potentialCancels);
			if(potentialCancels.empty() ) {
				state = OrderCancellationPolicy::SEND_NEW;
				return state;
			}
			//double baseAsk = TraderEnvSingleton::instance()->_base[0]->_instr.ask();
			double baseAsk = instr.askNoLock();
			double updThresh = (baseAsk * (1 + updateThresh/10000.0));
			int num = potentialCancels.size();
			FSB_LOG(" There are " << num << " sells ");
			for(int i=0;i<num;i++ ) {
				Order& order = potentialCancels[i];
				if(order._extOrderId.empty()) {
					FSB_LOG("The external order id does not exist, have not rcvd. active yet");
					continue;
				}
				FSB_LOG("Order status = " << order._enumStatus);
				if( order._enumStatus == Order::CANCEL) {
					FSB_LOG(" Already in Cancel mode " << order._guid);
					continue;
				}			
				FSB_LOG(" (Sell) Base mkt ask " << baseAsk
					<< " ask fv " << fv
					<< " working ask " << order._price
					<< " final send price " << sendPrice
					<< " with update thresh " << updThresh
					<< " with cxl thresh worse " << (sendPrice * (1 - cxlThreshWorse/10000.0))
					<< " with cxl thresh better " << (sendPrice * (1 + cxlThreshBetter/10000.0))
					<< " " << order._guid
					);
				if(  (sendPrice <= updThresh) ||
					 (order._price <= updThresh) ) 
				{
					FSB_LOG("Sell: Mkt is withing my update threshold, will  cxl/replace");
					if( order._price < (sendPrice * (1 - cxlThreshWorse/10000.0))) { // &&
						//order._price != sendPrice ) {
							order._enumStatus = Order::CANCEL;
							cancels.push_back(order);
							FSB_LOG("Cancelling, existing sell worse than FV, new order will be lower " << order._guid);
							state =  OrderCancellationPolicy::CANCEL;
							if(maxPending > 1) {
								state = OrderCancellationPolicy::CANCEL_REPLACE;
								FSB_LOG("Will cancel replace");
							}
							//TraderEnvSingleton::instance()->_base._orders.update(order);
					} 
					if(order._price > (sendPrice * (1 + cxlThreshBetter/10000.0))) { //&&
						//order._price != sendPrice) {
							order._enumStatus = Order::CANCEL;
							cancels.push_back(order);
							FSB_LOG("Cancelling, existing sell better than FV, new order will be higher " << order._guid);
							state =  OrderCancellationPolicy::CANCEL;
							if(maxPending > 1) {
								state = OrderCancellationPolicy::CANCEL_REPLACE;
								FSB_LOG("Will cancel replace");
							}
							//TraderEnvSingleton::instance()->_base[0]->_orders.update(order);

					}
				} else {
					FSB_LOG(" Market beyond my range ");
				}
			}
		}
		if( !potentialCancels.empty() )
			potentialCancels.clear();
		return state;
	}

	int SellshortCancel::cancel(fsb::Orders::OrdersVector &cancels,
		const OrderReplyInfo& reply,const double& sendprice)
	{
		int state = OrderCancellationPolicy::NONE;
		if( reply._qtyFilled < 0 ) return state;
		fsb::InstrumentPtrMap::iterator itr = TraderEnvSingleton::instance()->_instruments.find(InstrumentKey(reply._symbol, reply._exchange));
		if( itr == TraderEnvSingleton::instance()->_instruments.end() ) {
			FSB_LOG("Can't find instrument " << reply._symbol << ":" << reply._exchange);
			return state;
		}
		
		int index = itr->second->index();		
		(*TraderEnvSingleton::instance()->_instrObjItrs[index])->_orders.getSellShortOrders(potentialCancels);
		if(potentialCancels.empty() ) {
			return OrderCancellationPolicy::SEND_NEW;
		}
		int net = (*TraderEnvSingleton::instance()->_instrObjItrs[index])->_orders.net();
		FSB_LOG(" There are sell short orders ane net = " << net);
		if( net <0 ) {
			FSB_LOG(" I will leave the orders as is, as net < 0");
			return OrderCancellationPolicy::NONE;
		}
		(*TraderEnvSingleton::instance()->_instrObjItrs[index])->_orders.getSellOrders(cancels);
		int num = cancels.size();
		FSB_LOG("There are " << num << " sells to cxl" ); 
		for(int i =0; i<num;i++) {
			Order& order = cancels[i];
			if( order._enumStatus == Order::CANCEL) {
				FSB_LOG(" Already in Cancel mode " << order._guid);
				continue;
			}
			state = OrderCancellationPolicy::CANCEL;
			order._enumStatus = Order::CANCEL;
		}
		if(!potentialCancels.empty())
			potentialCancels.clear();
		return state;
	}	

	int SpiderCancelPolicy::cancel(fsb::Orders::OrdersVector & cancels, 
		const fsb::Instrument & instr, fsb::Orders & orders, bool isBuy, 
		double fv, double sendPrice)
	{
		Orders::OrdersVector potentialCancels;
		int state = OrderCancellationPolicy::NONE;
		if( isBuy)
			orders.getBuyOrders(potentialCancels);
		else
			orders.getSellOrders(potentialCancels);

		if(potentialCancels.empty() ) {
			return OrderCancellationPolicy::SEND_NEW;
		}
		int num = potentialCancels.size();
		FSB_LOG(" There are " << num << " orders to cancel ");
		for(int i=0;i<num;i++ ) {
			Order& order = potentialCancels[i];	
			if(order._extOrderId.empty()) {
				FSB_LOG("The external order id does not exist, have not rcvd. active yet");
				continue;
			}
			if( order._enumStatus == Order::CANCEL) {
				FSB_LOG(" Already in Cancel mode " << order._guid);
				continue;
			} 
			if( order._priceType == PT_MARKET) {
				order._enumStatus = Order::CANCEL;
				cancels.push_back(order);
				FSB_LOG("Will cancel " << (isBuy ? "buy" : "sell") << " " << order._guid << " " << order._extOrderId << " " << order._priceType);
				state = OrderCancellationPolicy::CANCEL;
			} else if( order._price != sendPrice) {
				order._enumStatus = Order::CANCEL;
				cancels.push_back(order);
				FSB_LOG("Will cancel " << (isBuy ? "buy" : "sell") << " working=" << order._price << " send=" << sendPrice << " " << order._guid << " " << order._extOrderId);
				state = OrderCancellationPolicy::CANCEL;
			}
		}
		if(!potentialCancels.empty())
			potentialCancels.clear();
		return state;
	}

	int SpiderHedgeCancelPolicy::cancel(fsb::Orders::OrdersVector & cancels, 
		const OrderReplyInfo& reply,
		const double& sendprice)
	{
		Orders::OrdersVector potentialCancels;
		int state = OrderCancellationPolicy::NONE;
		Orders& orders = TraderEnvSingleton::instance()->_hedge[0]->_orders;
		if( reply._qtyFilled < 0 ) //The hedge order will be a buy
			orders.getBuyOrders(potentialCancels);
		else // the hedge order will be a sell, so look for existing orders to cancel.
			orders.getSellOrders(potentialCancels);

		if(potentialCancels.empty() ) {
			return OrderCancellationPolicy::SEND_NEW;
		}

		FSB_LOG("There are orders to cancel");
		int num = potentialCancels.size();
		FSB_LOG(" There are " << num << " orders ");
		for(int i=0;i<num;i++ ) {
			Order& order = potentialCancels[i];	
			if(order._extOrderId.empty()) {
				FSB_LOG("The external order id does not exist, have not rcvd. active yet " << order._guid);
				continue;
			}
			if( order._enumStatus == Order::CANCEL) {
				FSB_LOG(" Already in Cancel mode " << order._guid);
				continue;
			}
			if(order._price == sendprice) {
				FSB_LOG(" no need to cxl, there is an order already at same price " << order._guid);
				continue;
			}
			FSB_LOG(" Will cancel the existing hedge order " << order._guid << " " << sendprice);
			order._enumStatus = Order::CANCEL;
			cancels.push_back(order);
			state = OrderCancellationPolicy::CANCEL;
		}
		if(!potentialCancels.empty())
			potentialCancels.clear();
		return state;
	}
}
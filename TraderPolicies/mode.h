#ifndef __MODE_H__
#define __MODE_H__

#include "FVPolicy.h"
#include "Policy.h"
#include "PolicyIds.h"
#include "BinaryPolicy.h"
#include "TradingModePolicy.h"
#include "OrderSizePolicy.h"
#include "SizePolicy.h"
#include "OrderCancellationPolicy.h"
#include <Data/Order.h>
#include <Data/OrderReplyInfo.h>

namespace fsb {

	class SetupOrder
	{
	public:
		void setup(Orders::OrdersVector& orders,
			const std::string& symbol,
			const std::string& exchange,
			char type,
			int quantity,
			double sendPrice,
			bool isBuy,
			const std::string& tif,
			const std::string& priceType,
			const std::string& source,
			int index = -1,
			bool sellshort=false,
			int minQty = 0,
			bool postOnly = false,
			bool htb = false )
		{
			Order anOrder;
			anOrder._symbol = symbol;
			anOrder._exchange = exchange;
			anOrder._isBuy = isBuy;
			anOrder._marketType = tif;
			anOrder._price = sendPrice;
			anOrder._priceType = priceType;
			anOrder._quantity = quantity;
			anOrder._instrType = type;			
			anOrder._orderSource = source;
			anOrder._enumStatus = Order::NEW;
			anOrder._sellShort = sellshort;
			anOrder._minQty = minQty;
			anOrder._htb = postOnly;
			anOrder._index = index;
			orders.push_back( anOrder);
		}
	};

	template<typename PricePolicy,
		typename SizePolicy,
		template <class SizePolicy,class MaxPosPolicy> class OrderSize,
		typename CancelPolicy,
		bool ShortSale=false,
		class MaxPosPolicy = DefaultMaxPositionPolicy>
	class Mode : public PricePolicy, public OrderSize<SizePolicy,MaxPosPolicy>,public CancelPolicy
	{
	public:
		Mode(const std::string& type,
			const std::string& tif="DAY",
			const std::string& priceType="LMT",
			bool postOnly = false)
			:_type(type),
			_tif(tif),
			_priceType(priceType),
			_postOnly(postOnly)
		{
			std::cout << " type: " << _type << " tif " << _tif << " ptype " << _priceType 
				<< " post only " << _postOnly
				<< std::endl;
		}

		std::string& type() { return _type;}

		void priceType(const std::string& p) { _priceType = p;}
		void tif(const std::string& t) { _tif = t;}

		bool evaluateReply(Orders::OrdersVector& newOrders,
			Orders::OrdersVector& cancels,
			const OrderReplyInfo& reply,
			const Instrument& instr)
		{
			return evaluateReply(newOrders,cancels,reply,instr,fsbutils::Int2Type<ShortSale>());
		}

		bool evaluateReply(Orders::OrdersVector& newOrders,
			Orders::OrdersVector& cancels,
			const OrderReplyInfo& reply,
			const Instrument& instr,
			fsbutils::Int2Type<true> )
		{
			//ALERT: HTB in Order structure HARDCODED TO false , using HTB Flag for now!!!!
			double buyPrice=0.0, sellPrice=0.0;
			double sendPrice = calcPrice(instr,buyPrice,sellPrice,true);
			 int state = cancel(cancels,reply,sendPrice);
			 if(state != OrderCancellationPolicy::SEND_NEW )
				 return false;			 
			 int buys=0,sells=0,sellshort=0;			 
			 int qty = calcQuantity(reply,buys,sells,sellshort);
			 int minQty = instr.minQty();
			 if(buys > 0 && buyPrice) {
				 _orderSetup.setup(newOrders,instr.symbol(),instr.exchange(),instr.type(),buys,buyPrice,true,_tif,_priceType,type(),instr.index(),false,minQty,_postOnly);
			 } 
			 if(sells > 0 && sellPrice) {
				 _orderSetup.setup(newOrders,instr.symbol(),instr.exchange(),instr.type(),sells,sellPrice,false,_tif,_priceType,type(),instr.index(),false,minQty,_postOnly);
			 }
			 if(sellshort > 0 && sellPrice) {
				 _orderSetup.setup(newOrders,instr.symbol(),instr.exchange(),instr.type(),sellshort,sellPrice,false,_tif,_priceType,type(),instr.index(),true,minQty,_postOnly);
			 }
			 return true;
		}

		bool evaluateReply(Orders::OrdersVector& newOrders,
			Orders::OrdersVector& cancels,
			const OrderReplyInfo& reply,
			const Instrument& instr,
			fsbutils::Int2Type<false> )
		{
			//FSB_LOG("NOT IMPLEMENTED");
			double buyPrice = 0.0, sellPrice = 0.0;
			int buys = 0,sells=0;
			int quantity = calcQuantity(reply,buys,sells);
			bool isBuy = buys > 0 ? true : false;
			double price = calcPrice(instr,buyPrice,sellPrice,isBuy);
			
			if( quantity == 0 ||
				price == 0.0 ) {
					FSB_LOG(" Problem, Hmm... hedgepolicy returned " 
						<< quantity
						<< " " <<  price);
					return false;
			}
			_orderSetup.setup(newOrders,instr.symbol(),instr.exchange(),instr.type(),
				quantity,price,isBuy,_tif,_priceType,type(),instr.index());
			 return true;
		}

//************************************************************************************
		bool evaluate(Instrument& instr,
			Orders& orders,
			const double& theoBid,
			const double& theoAsk,
			bool isBuy,
			Orders::OrdersVector& newOrders,
			Orders::OrdersVector& cancels)
		{
			double fv = isBuy ? theoBid : theoAsk;
			/*FSB_LOG(" " << isBuy
				<< " " << orders.totalBuysInclPending()
				<< " " << orders.totalSells()
				<< " " << orders.totalBuys()
				<< " " << orders.totalSellsInclPending()
				<< " " << instr.incomingInv());*/
			int net = isBuy ? (orders.totalBuysInclPending() - orders.totalSells()) : (orders.totalBuys() - orders.totalSellsInclPending());
			net += instr.incomingInv();
			double sendPrice = price(instr,theoBid,theoAsk,isBuy);
			return evaluate(instr,orders,fv,sendPrice,net,isBuy,newOrders,cancels,fsbutils::Int2Type<ShortSale>());
		}

		bool evaluate(Instrument& instr,
			Orders& orders,
			const double& fv,
			const double& sendPrice,
			const int& net,
			bool isBuy,
			Orders::OrdersVector& newOrders,
			Orders::OrdersVector& cancels,
			fsbutils::Int2Type<true>)
		{
			
			int sellshort = 0,buys=0,sells=0;
			int state;		
			int	qtyToSend = quantity(instr,net,buys,sells,sellshort,isBuy);
			int minQty = instr.minQty();
			//ALERT:HTB hard coded to post only in Order structure!!!
			if( sendPrice) {
				bool retVal = false;
				double priceMulti = (double)instr.priceMultiplier();
				FSB_LOG(instr.symbol() << " " << priceMulti);
				state = evaluateCancelByInstr(cancels,instr,orders,isBuy,fv,sendPrice);
				if(state == OrderCancellationPolicy::CANCEL_REPLACE ||
					state == OrderCancellationPolicy::SEND_NEW) {
						if( qtyToSend >0 ) { // could be a buy or sell long
							bool po = (qtyToSend < 100 && _postOnly) ? false : _postOnly; 							
							_orderSetup.setup(newOrders,instr.symbol(),instr.exchange(),
								instr.type(),qtyToSend,sendPrice,isBuy,_tif,
								_priceType,type(),instr.index(),
								false,minQty,po);
							retVal = true;
						}
						if( sellshort > 0) {
							bool po = (sellshort < 100 && _postOnly) ? false : _postOnly;
							_orderSetup.setup(newOrders,instr.symbol(),instr.exchange(),instr.type(),sellshort,sendPrice,isBuy,_tif,
								_priceType,type(),instr.index(),
								true,minQty,po);
							retVal = true;
						}
						return retVal;
				}
			} else {
				FSB_LOG("ERROR: Send price " << sendPrice << " Quantity " << qtyToSend);			
			}
			return false;
		}

		bool evaluate(Instrument& instr,
			Orders& orders,
			const double& fv,
			const double& sendPrice,
			const int& net,
			bool isBuy,
			Orders::OrdersVector& newOrders,
			Orders::OrdersVector& cancels,
			fsbutils::Int2Type<false>)

		{
			int buys=0,sells=0;
			int state;
			//double fv = isBuy ? theoBid : theoAsk;
			//int net = isBuy ? (orders.totalBuysInclPending() - orders.totalSells()) : (orders.totalBuys() - orders.totalSellsInclPending());
			//double sendPrice = price(instr,theoBid,theoAsk,isBuy);
			int	qtyToSend = quantity(instr,net,buys,sells,isBuy);
			if( sendPrice)
				state = evaluateCancelByInstr(cancels,instr,orders,isBuy,fv,sendPrice);
			if(sendPrice && qtyToSend > 0) {
				if(state == OrderCancellationPolicy::CANCEL_REPLACE ||
					state == OrderCancellationPolicy::SEND_NEW) {
						_orderSetup.setup(newOrders,instr.symbol(),instr.exchange(),
							instr.type(),qtyToSend,sendPrice,isBuy,_tif,
							_priceType,type(),instr.index(),
							false);
						return true;
				}
			} else {
				FSB_LOG("ERROR: Send price " << sendPrice << " Quantity " << qtyToSend);			
			}
			return false;
		}
		double price(Instrument& instr,const double& theoBid,const double& theoAsk,bool isBuy)
		{
			double sendPrice = 0.0;
			double fv = isBuy ? theoBid : theoAsk;

			if( calcPrice(instr,isBuy,fv,sendPrice) )
					return sendPrice;
			return 0.0;
		}

		int quantity(const Instrument& instr, int net, int& buys, int& sells, int& sellshort,bool isBuy)
		{
			calcQuantity(instr,net,buys,sells,sellshort);
			int qty = 0;
			isBuy ? (sells = sellshort =0,qty = buys) : (qty = sells,buys = 0);
			/*FSB_LOG(" " << (isBuy ? "BUY " : " SELL ")
				<< buys << " " << sells << " " << sellshort);*/
			return qty;
		}
		int quantity(const Instrument& instr, int net, int& buys, int& sells, bool isBuy)
		{
			calcQuantity(instr,net,buys,sells);
			if( isBuy )
				return buys;
			else
				return sells;
		}

		int evaluateCancelByInstr(Orders::OrdersVector& cancels,
			const Instrument& instr,
			Orders& orders,
			bool isBuy,
			double fv,
			double sendPrice)
		{
			return cancel(cancels,instr,orders,isBuy,fv,sendPrice);
		}

		int evaluateCancel(Orders::OrdersVector& cancels,
			bool isBuy,
			double fv,
			double sendPrice)
		{
			return cancel(cancels,isBuy,fv,sendPrice);
		}

		 std::string _type;
		 std::string _priceType;
		 std::string _tif;
		 bool _postOnly;
		 SetupOrder _orderSetup;
	};	
}
#endif

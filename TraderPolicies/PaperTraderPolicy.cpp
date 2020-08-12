#include "stdafx.h"
#include "PaperTraderPolicy.h"
#include "TraderEnv.h"
#include <stdlib.h>


namespace fsb {

//PaperTraderPolicy::PaperTraderPolicy(const std::string& pricePolicyName,
//									const std::string& roundingPolicyName,
//									const std::string& sizePolicyname,
//									const std::string& name)
//									:_pairName(name),
//									TraderPolicy(name)
//									//_hedgePolicy(name)
//{
//	srand( (unsigned)time( NULL ) );
//
//}
//
//PaperTraderPolicy::~PaperTraderPolicy()
//{
//}
//
//void PaperTraderPolicy::evaluateOrderResponse(Orders::OrdersVector& neworders,
//											  Orders::OrdersVector& cancels,
//											  OrderReplyInfo& reply,
//											  bool& recordStats)
//{
//	recordStats = false;
//	if( reply._orderStatus == OrderReplyInfo::FILL ||
//		reply._orderStatus == OrderReplyInfo::PARTIAL ) {
//			if( _paperTrader.evaluateFILL(neworders,cancels,reply) ) {
//				BuildNewOrder(neworders,reply._odId);
//			}
//		recordStats = true;
//	}
//	else if( reply._orderStatus == OrderReplyInfo::CXLD ) {
//	}
//}
//
//void PaperTraderPolicy::evaluateXXX(Orders::OrdersVector& neworders,
//								 Orders::OrdersVector& cancels,
//								 bool start)
//{
//	_theoAsk = 0.0;
//	_theoBid = 0.0;
//
//	ma1 = TraderEnvSingleton::instance()->_longTermMA->ma();
//	ma2 = TraderEnvSingleton::instance()->_shortTermMA->ma();
//	
//	if( _paperTrader.evaluateMARKET(neworders,cancels,_theoBid,_theoAsk)) {
//		if ( start ) {
//			std::string orderId;
//			BuildNewOrder(neworders,orderId);
//		}
//	}
//}
//
////*******************************************************************************
//
////PaperTraderHedgingPolicy::PaperTraderHedgingPolicy(const std::string& name)
////:_pairName(name),
////TraderPolicy(name),
////_hedge(HEDGE)
////{
////	_roundingPolicy = (RoundingPolicy*) PolicyFactorySingleton::instance()->create("RoundNearestPolicy");
////	_sizePolicy = (SizePolicy*) PolicyFactorySingleton::instance()->create("HedgeSizePolicy");
////}
////void PaperTraderHedgingPolicy::calcFv(double& sendBidPrice, double& sendAskPrice)
////{
////	Instrument& hedge = TraderEnvSingleton::instance()->_hedge._instr;
////	int hedgePrem = TraderEnvSingleton::instance()->_params.hedgePremium();
////	double sellprice = hedge.bid() * (1 - (hedgePrem/10000.0));
////	sendAskPrice = _roundingPolicy->round(hedge.fsbticksize(),false,sellprice);
////	double buy =  hedge.ask() *  (1 + (hedgePrem/10000.0));
////	sendBidPrice = _roundingPolicy->round(hedge.fsbticksize(),true,buy);
////
////	FSB_LOG(" Hedging w/ Premium: " << sendBidPrice << "X " << sendAskPrice
////		<< " Premium " << hedgePrem
////		<< " w/ Prem " << hedge.bid() << "X " << hedge.ask() );
////}
////
////void PaperTraderHedgingPolicy::calcQuantity(int& buy, int& sell)
////{
////	_sizePolicy->calcQuantity(buy,sell);
////}
////
////void PaperTraderHedgingPolicy::evaluateOrderResponseCancels(OrderReplyInfo& reply,
////															Orders::OrdersVector& cancels)
////{
////}
//
////void PaperTraderHedgingPolicy::evaluateOrderResponseFills(OrderReplyInfo& reply,
////														  Orders::OrdersVector& neworders)
////{
////	/*if ( reply._orderSource == TAKE ) {
////		FSB_LOG("No need to hedge since the order is from TAKE LIQ");
////		return;
////	}*/
////
////	if( reply._orderSource == HEDGE ) {
////		FSB_LOG(" It is an order from the hedge side " << reply._symbol)
////		return;
////	}
////	
////	/*if( TraderEnvSingleton::instance()->_hedge._instr.symbol() == reply._symbol) {
////		FSB_LOG(" It is an order from the hedge side " << reply._odId)
////			return;
////	}*/
////
////	if( reply._orderSource == WORK ||
////		reply._orderSource == TAKE ||
////		reply._orderSource == CLOSE_TAKE ||
////		reply._orderSource == CLOSE_WORK ||
////		reply._orderSource == CLOSE ) {
////
////	FSB_LOG(" Received a base trade " << reply._symbol
////		<< " " << reply._exchange
////		<< " " << reply._orderSource
////		<< " " << reply._qtyFilled
////		<< " " << reply._fillPrice
////		<< " " << (reply._qtyFilled > 0 ? "BUY" : "SELL")
////		);
////	Instrument& hedge = TraderEnvSingleton::instance()->_hedge._instr;
////	//InstrumentKey key(reply._symbol, reply._exchange);
////	int buy=0,sell=0;
////	double bidprice,offerprice=0.0;
////	int quantity=0;
////	bool isBuy;
////	//For now will Ignore delta
////	//calcQuantity(buy,sell);
////	calcFv(bidprice,offerprice);
////	double price = 0.0;
////	//if the original is a buy then the hedge order is a sell
////	if( reply._qtyFilled > 0 ) {
////		price = offerprice;
////		quantity = reply._qtyFilled;
////		isBuy = false;
////	} else {
////		price = bidprice;
////		quantity = abs(reply._qtyFilled);
////		isBuy = true;
////	}
////
////	if( quantity == 0 ||
////		price == 0.0 )
////		return;
////
////	FSB_LOG(" Sending a hedge trade " << hedge.symbol()
////		<< " " << hedge.exchange()
////		<< " " << quantity
////		<< " " << price
////		<< " " << (isBuy ? "BUY" : "SELL")
////		);
////	
////	BuildNewOrder(hedge.symbol(),hedge.exchange(),hedge.type(),quantity,price,isBuy,"LMT",reply._odId,HEDGE,neworders);
////	}
////	
////	return;	
////}
}

//fsb::PolicyRegistrar<fsb::PaperTraderPolicy> PaperTraderPolicyPrototype;
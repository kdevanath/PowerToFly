#include "stdafx.h"

#include "Mode.h"

namespace fsb
{
	//PTMode::PTMode()
	//	:_takeLiq(TAKE),
	//	_workLiq(WORK),
	//	_closeOnlyMode(CLOSE),
	//	_hedgeTrader(HEDGE)
	//{
	//	std::string bsFlagPolicy =  TraderEnvSingleton::instance()->_params.buySellFlagPolicy();
	//	FSB_LOG(" Binary Policy " << bsFlagPolicy);
	//	_paperTraderFV = new FVPolicy1<PTBaseFV>(bsFlagPolicy);
	//	_closeModePTFV = new FVPolicy1<PTBaseCloseFv>(bsFlagPolicy);
	//}

	//bool PTMode::evaluateMARKET(Orders::OrdersVector& orders,
	//						Orders::OrdersVector& cancels,
	//						double& theoBid,
	//						double& theoAsk)
	//{
	//	_base = TraderEnvSingleton::instance()->_base[0]->_instr;
	//	_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;

	//	for(InstrumentObj::VectPtrsItr i= TraderEnvSingleton::instance()->_base.begin(); i != TraderEnvSingleton::instance()->_base.end(); ++i)
	//	{
	//		_baseInstrs.push_back( (*i)->_instr);
	//		//_baseInstrs.push_back(_base);
	//	}

	//	for(InstrumentObj::VectPtrsItr i= TraderEnvSingleton::instance()->_hedge.begin(); i != TraderEnvSingleton::instance()->_hedge.end(); ++i)
	//	{
	//		_hedgeInstrs.push_back( (*i)->_instr);
	//	}

	//	bool closeOnly = TraderEnvSingleton::instance()->_params.closeOnly();
	//	int tradeType=0;
	//	if( closeOnly ) {
	//		tradeType = PTMode::CLOSE_ONLY;
	//		_buySellSignal = _closeModePTFV->fv(theoBid,theoAsk,_base,_hedge);
	//	} else {
	//		tradeType = PTMode::REGULAR;
	//		_buySellSignal = _paperTraderFV->fv(theoBid,theoAsk,_base,_hedge);
	//	}

	//	if(!TraderEnvSingleton::instance()->_params.bsFlag())
	//	{
	//		evaluate(orders,cancels,_base,_hedge,tradeType,theoBid,theoAsk,BinaryPolicy::BUY);
	//		evaluate(orders,cancels,_base,_hedge,tradeType,theoBid,theoAsk,BinaryPolicy::SELL);
	//		return true;
	//	} else {
	//		if( _buySellSignal == BinaryPolicy::BUY) //it is a buy, Cancel all sells
	//			TraderEnvSingleton::instance()->_base[0]->_orders.getCancels(false,cancels);
	//		else if( _buySellSignal == BinaryPolicy::SELL) //it is a sell Cancel all buys
	//			TraderEnvSingleton::instance()->_base[0]->_orders.getCancels(true,cancels);

	//		if( !cancels.empty()) {
	//			FSB_LOG("There are "
	//				<< ( _buySellSignal == BinaryPolicy::SELL? "BUYS" : "SELLS")
	//				<< "to cancel");
	//		}
	//		if( evaluate(orders,cancels,_base,_hedge,tradeType,theoBid,theoAsk,_buySellSignal)) {
	//			return true;
	//		}
	//	}

	//	return false;
	//}

	//bool PTMode::evaluate(Orders::OrdersVector& orders,
	//			Orders::OrdersVector& cancels,
	//			Instrument& instr,
	//			Instrument& instr2,
	//			int tradeType,
	//			double& theoBid,
	//			double& theoAsk,
	//			int buySellSignal)
	//{
	//	bool closeOnly = tradeType & PTMode::CLOSE_ONLY;

	//	if( closeOnly )
	//		FSB_LOG("CLOSE ONLY MODE IS ON");

	//	if( buySellSignal == BinaryPolicy::NO_CHANGE) {
	//		double buyPrice = 0.0, sellPrice=0.0;
	//		buyPrice = _workLiq.price(instr,theoBid,theoAsk,true);
	//		_workLiq.evaluateCancel(cancels,true,theoBid,buyPrice);
	//		sellPrice = _workLiq.price(instr,theoAsk,theoAsk,false);
	//		_workLiq.evaluateCancel(cancels,false,theoAsk,sellPrice);
	//		return false;
	//	} 

	//	int buys = 0, sells = 0;
	//	double sendPrice = 0.0;
	//	int quantity = 0;
	//	int state;
	//	std::string source;

	//	bool isBuy = true;
	//	double fv = theoBid;
	//	if( buySellSignal == BinaryPolicy::SELL) {
	//		isBuy = false;
	//		fv = theoAsk;
	//	}

	//	if( TraderEnvSingleton::instance()->_params.takeLiq() ) {
	//		sendPrice = _takeLiq.price(instr,theoBid,theoAsk,isBuy);
	//		quantity = _takeLiq.quantity(instr,net,buys,sells,isBuy);
	//		if(closeOnly) {
	//			quantity = _closeOnlyMode.calcQuantity(buys,sells,isBuy);
	//		}
	//		if( sendPrice && quantity ) {				
	//			state = _takeLiq.evaluateCancel(cancels,isBuy,fv,sendPrice);
	//			if( (state == OrderCancellationPolicy::CANCEL_REPLACE ||
	//						  state == OrderCancellationPolicy::SEND_NEW) ) {
	//				setup(instr.symbol(),instr.exchange(),instr.type(),quantity,sendPrice,isBuy,"IOC","LMT",_takeLiq.type(),orders);
	//				return true;
	//			}
	//		}
	//	}
	//	if( TraderEnvSingleton::instance()->_params.workLiq() ) {
	//		sendPrice = _workLiq.price(instr,theoBid,theoAsk,isBuy);
	//		quantity = _workLiq.quantity(buys,sells,isBuy);
	//		if(closeOnly) {
	//			quantity = _closeOnlyMode.calcQuantity(buys,sells,isBuy);
	//		}
	//		if( sendPrice)
	//			state = _workLiq.evaluateCancel(cancels,isBuy,fv,sendPrice);
	//		if( sendPrice && quantity ) {
	//			if( state == OrderCancellationPolicy::CANCEL_REPLACE ||
	//				state == OrderCancellationPolicy::SEND_NEW ) {
	//					setup(instr.symbol(),instr.exchange(),instr.type(),quantity,sendPrice,isBuy,"DAY","LMT",_workLiq.type(),orders);
	//					return true;
	//			}
	//		}
	//	}
	//	return false;
	//}

	//bool PTMode::evaluateFILL(Orders::OrdersVector& orders,
	//	Orders::OrdersVector& cancels,
	//	OrderReplyInfo& reply)
	//{
	//	if( reply._orderSource == HEDGE ) {
	//		FSB_LOG(" It is an order from the hedge side " << reply._symbol)
	//			return false;
	//	}

	//	if( reply._orderSource == WORK ||
	//		reply._orderSource == TAKE ||
	//		reply._orderSource == CLOSE_TAKE ||
	//		reply._orderSource == CLOSE_WORK ||
	//		reply._orderSource == CLOSE ) {

	//			//Instrument& hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;

	//			FSB_LOG(" Received a base trade " << reply._symbol
	//				<< " " << reply._exchange
	//				<< " " << reply._orderSource
	//				<< " " << reply._qtyFilled
	//				<< " " << reply._fillPrice
	//				<< " " << (reply._qtyFilled > 0 ? "BUY" : "SELL"));

	//			InstrumentObj::VectPtrsItr itr = TraderEnvSingleton::instance()->_hedge.begin();

	//			for(itr; itr != TraderEnvSingleton::instance()->_hedge.end();++itr) {
	//				
	//				Instrument& hedge = (*itr)->_instr;
	//				int pos = (*itr)->_orders.net();

	//				double buyPrice = 0.0, sellPrice = 0.0, price = 0.0;
	//				int quantity = 0,buys = 0,sells=0;
	//				bool isBuy;

	//				if( reply._qtyFilled > 0 )
	//					isBuy = false;
	//				else
	//					isBuy = true;

	//				price = _hedgeTrader.calcPrice(hedge,buyPrice,sellPrice,isBuy);
	//				quantity = _hedgeTrader.calcQuantity(hedge,pos,buys,sells,isBuy);

	//				if( quantity == 0 ||
	//					price == 0.0 ) {
	//						FSB_LOG(" Problem, Hmm... hedgepolicy returned " 
	//							<< quantity
	//							<< " " <<  price);
	//						continue;
	//				}
	//				setup(hedge.symbol(),hedge.exchange(),hedge.type(),quantity,price,isBuy,"DAY","LMT",_hedgeTrader.type(),orders);
	//				FSB_LOG(" Sending a hedge trade " << hedge.symbol()
	//					<< " " << hedge.exchange()
	//					<< " " << quantity
	//					<< " " << price
	//					<< " " << (isBuy ? "BUY" : "SELL")
	//					);
	//			}
	//	}
	//	return true;
	//}
}
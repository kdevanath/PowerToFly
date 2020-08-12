#include "stdafx.h"
#include "objbase.h"
#include "TraderPolicy.h"

namespace fsb {

	PTBase1by1MktUpdate::PTBase1by1MktUpdate()
		:_takeLiq("TAKE"),
		_workLiq("WORK"),
		_closeOnlyMode("CLOSE")
	{
		std::string bsFlagPolicy =  TraderEnvSingleton::instance()->_params.buySellFlagPolicy();
		FSB_LOG(" Binary Policy " << bsFlagPolicy << " " << TAKE << " " << _takeLiq.type());
		_paperTraderFV = new FVPolicy1<PTBaseFV,EmptyBSFlag>(bsFlagPolicy);
		_closeModePTFV = new FVPolicy1<PTBaseCloseFv,EmptyBSFlag>(bsFlagPolicy);
		FSB_LOG(" Binary Policy " << bsFlagPolicy<< " " << WORK << " " << _workLiq.type());

		FSB_LOG("TAKE: " << _takeLiq.type() << "WORK: " << _workLiq.type() << "CLOSE: " << _closeOnlyMode.type());
	}

	void PTBase1by1MktUpdate::calcFV(bool closeOnly,int& buySellSignal)
	{
		_base = TraderEnvSingleton::instance()->_base[0]->_instr;
		_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;

		FSB_LOG(" _hedge " << _hedge.bidNoLock() << " " << _hedge.askNoLock());
		_theoBid = _theoAsk = 0.0;

		if( closeOnly ) {			
			buySellSignal = _closeModePTFV->fv(_theoBid,_theoAsk,_base,_hedge);
		} else {		
			buySellSignal = _paperTraderFV->fv(_theoBid,_theoAsk,_base,_hedge);
		}
		TraderEnvSingleton::instance()->_base[0]->_pricers.updateFv(_theoBid,_theoAsk);
	}
	bool PTBase1by1MktUpdate::evaluateMARKET(Orders::OrdersVector& orders,
							Orders::OrdersVector& cancels,
							int buySellSignal, bool closeOnly)
	{
		//_base = TraderEnvSingleton::instance()->_base[0]->_instr;
		//_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
		if(_theoBid == 0.0 ||
			_theoAsk == 0.0) {
				FSB_LOG("theoBid = " <<  _theoBid
					<< " theoAsk = " << _theoAsk 
					<< " returned a zero will not send orders ");
				return false;
		}
		return evaluateBSFlag(orders,cancels,buySellSignal,closeOnly);
	}

	bool PTBase1by1MktUpdate::evaluateBSFlag(Orders::OrdersVector& orders,
											Orders::OrdersVector& cancels,
											int buySellSignal,
											bool closeOnly)
	{
		if(!TraderEnvSingleton::instance()->_params.bsFlag())
		{
			evaluate(orders,cancels,BinaryPolicy::BUY,closeOnly);
			evaluate(orders,cancels,BinaryPolicy::SELL,closeOnly);
			return true;
		} else {
			if( buySellSignal == BinaryPolicy::BUY) //it is a buy, Cancel all sells
				TraderEnvSingleton::instance()->_base[0]->_orders.getCancels(false,cancels);
			else if( buySellSignal == BinaryPolicy::SELL) //it is a sell Cancel all buys
				TraderEnvSingleton::instance()->_base[0]->_orders.getCancels(true,cancels);

			if( !cancels.empty()) {
				FSB_LOG("There are "
					<< ( buySellSignal == BinaryPolicy::SELL? "BUYS" : "SELLS")
					<< "to cancel");
			}
			if( evaluate(orders,cancels,buySellSignal,closeOnly)) {
				return true;
			}
		}
		return false;
	}

	bool PTBase1by1MktUpdate::evaluate(Orders::OrdersVector& orders,
										Orders::OrdersVector& cancels,
										int buySellSignal,
										bool closeOnlyOn)
	{

		if( closeOnlyOn )
			FSB_LOG("CLOSE ONLY MODE IS ON");

		if( buySellSignal == BinaryPolicy::NO_CHANGE) {
			double buyPrice = 0.0, sellPrice=0.0;
			buyPrice = _workLiq.price(_base,_theoBid,_theoAsk,true);
			_workLiq.evaluateCancel(cancels,true,_theoBid,buyPrice);
			sellPrice = _workLiq.price(_base,_theoBid,_theoAsk,false);
			_workLiq.evaluateCancel(cancels,false,_theoAsk,sellPrice);
			return false;
		} 

		int buys = 0, sells = 0;
		double sendPrice = 0.0;
		int quantity = 0;
		int state;
		std::string source;

		bool isBuy = true;
		double fv = _theoBid;
		int totalbuys = TraderEnvSingleton::instance()->_base[0]->_orders.totalBuysInclPending();
		int totalsells = TraderEnvSingleton::instance()->_base[0]->_orders.totalSells();

		if( buySellSignal == BinaryPolicy::SELL) {
			isBuy = false;
			fv = _theoAsk;
			totalbuys = TraderEnvSingleton::instance()->_base[0]->_orders.totalBuys();
			totalsells = TraderEnvSingleton::instance()->_base[0]->_orders.totalSellsInclPending();
		}
		int net = totalbuys - totalsells;
		if( TraderEnvSingleton::instance()->_params.takeLiq() ) {
			sendPrice = _takeLiq.price(_base,_theoBid,_theoAsk,isBuy);
			quantity = _takeLiq.quantity(_base,net,buys,sells,isBuy);
			if(closeOnlyOn) {
				//quantity = _closeOnlyMode.calcQuantity(_base,net,buys,sells);
				if( sendPrice && !quantity) {
					FSB_LOG(" Take set up, but quantity returned zero, checking to cxl/replace");
					_closeOnlyMode.evaluateCancel(cancels,isBuy,fv,sendPrice);
				}
			}
			if( sendPrice ) 		
				state = _takeLiq.evaluateCancel(cancels,isBuy,fv,sendPrice);
			if( sendPrice && quantity ) {		
				if( (state == OrderCancellationPolicy::CANCEL_REPLACE ||
							  state == OrderCancellationPolicy::SEND_NEW) ) {
								  _setupOrders.setup(orders,_base.symbol(),_base.exchange(),_base.type(),
									  quantity,sendPrice,isBuy,"IOC","LMT",
									  _takeLiq.type(),_base.index());
					return true;
				}
			}
		}
		
		if( TraderEnvSingleton::instance()->_params.workLiq() ) {
			sendPrice = _workLiq.price(_base,_theoBid,_theoAsk,isBuy);
			quantity = _workLiq.quantity(_base,net,buys,sells,isBuy);
			//if(closeOnlyOn) {
			//	//quantity = _closeOnlyMode.calcQuantity(_base,net,buys,sells);
			//}
			if( sendPrice)
				state = _workLiq.evaluateCancel(cancels,isBuy,fv,sendPrice);
			if( sendPrice && quantity ) {
				if( state == OrderCancellationPolicy::CANCEL_REPLACE ||
					state == OrderCancellationPolicy::SEND_NEW ) {
						_setupOrders.setup(orders,_base.symbol(),_base.exchange(),_base.type(),quantity,sendPrice,isBuy,"DAY","LMT",_workLiq.type(),_base.index());
						return true;
				}
			}
		}
		return false;
	}

	void PTBase1by1MktUpdate::calcDelta()
	{
		double baseNotional = TraderEnvSingleton::instance()->_base[0]->_orders.net() * _base.lastNoLock() * _base.multiplier() * _base.fx();
		double hedgeNotional = TraderEnvSingleton::instance()->_hedge[0]->_orders.net() * _hedge.lastNoLock() * _hedge.multiplier();
		TraderEnvSingleton::instance()->_totalPos.setDelta(baseNotional + hedgeNotional);
	}

	PTBase1byNMktUpdate::PTBase1byNMktUpdate()
		:PTBase1by1MktUpdate(),
		_baseNotional(0.0),
		_hedgeNotional(0.0)
	{
	}

	void PTBase1byNMktUpdate::calcFV(bool closeOnly,int& buySellSignal)
	{
		_theoBid = _theoAsk = 0.0;

		if(!_baseInstrs.empty())
			_baseInstrs.clear();
		if(!_hedgeInstrs.empty())
			_hedgeInstrs.clear();

		 _baseNotional = 0.0;
		 _hedgeNotional = 0.0;
		for(InstrumentObj::VectPtrsItr i= TraderEnvSingleton::instance()->_base.begin(); i != TraderEnvSingleton::instance()->_base.end(); ++i)
		{
			_baseInstrs.push_back( (*i)->_instr);
			
			_baseNotional +=   ( (*i)->_instr.mid() * (*i)->_instr.multiplier() * (*i)->_orders.net());
		}

		for(InstrumentObj::VectPtrsItr i= TraderEnvSingleton::instance()->_hedge.begin(); i != TraderEnvSingleton::instance()->_hedge.end(); ++i)
		{
			_hedgeInstrs.push_back( (*i)->_instr);
			_hedgeNotional +=   ( (*i)->_instr.mid() * (*i)->_instr.multiplier() * (*i)->_orders.net());
		}
		if( closeOnly ) {
			buySellSignal = _closeModePTFV->fv(_theoBid,_theoAsk,_baseInstrs,_hedgeInstrs);
		} else {
			buySellSignal = _paperTraderFV->fv(_theoBid,_theoAsk,_baseInstrs,_hedgeInstrs);
		}
	}

	void PTBase1byNMktUpdate::calcDelta()
	{
		TraderEnvSingleton::instance()->_totalPos.setDelta(_baseNotional + _hedgeNotional);
	}

	PTHedge1by1::PTHedge1by1()
		:_hedgeTrader("HDG"),
		_hedgeDelta("DLTA")
	{
	}

	bool PTHedge1by1::evaluateFILL(
		fsb::Orders::OrdersVector &orders, 
		fsb::Orders::OrdersVector &cancels, 
		fsb::OrderReplyInfo &reply)
	{
		_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
		int pos = TraderEnvSingleton::instance()->_hedge[0]->_orders.net();
		double buyPrice = 0.0, sellPrice = 0.0, price = 0.0;
		int quantity = 0,buys = 0,sells=0;
		bool isBuy;

		if( reply._qtyFilled > 0 )
			isBuy = false;
		else
			isBuy = true;

		std::string type(_hedgeTrader.type());
		if( TraderEnvSingleton::instance()->_params.deltaClose() ) {
			FSB_LOG("Delta closing mode");
			price = _hedgeDelta.calcPrice(_hedge,buyPrice,sellPrice,isBuy);
//			quantity = quantity = _hedgeDelta.calcQuantity(hedge,reply,pos,buys,sells,isBuy);
			quantity = _hedgeDelta.calcQuantity(reply,buys,sells);
			isBuy = buys > 0 ? true : sells > 0 ? false : false;
			type = _hedgeDelta.type();
		} else {
			price = _hedgeTrader.calcPrice(_hedge,buyPrice,sellPrice,isBuy);
			quantity = _hedgeTrader.calcQuantity(reply,buys,sells);
		}
		if( quantity == 0 ||
			price == 0.0 ) {
				FSB_LOG(" Problem, Hmm... hedgepolicy returned " 
					<< quantity
					<< " " <<  price);
				return false;
		}
		_orderSetup.setup(orders,_hedge.symbol(),_hedge.exchange(),_hedge.type(),quantity,price,isBuy,"DAY","LMT",type,_hedge.index());
		FSB_LOG(" Sending a hedge trade " << _hedge.symbol()
			<< " " << _hedge.exchange()
			<< " " << quantity
			<< " " << price
			<< " " << (isBuy ? "BUY" : "SELL")
			<< " " << type
			);

		return true;
	}

	//*********************************************************************************
	PTInverseHedge1by1::PTInverseHedge1by1()
		:_hedgeTrader("HDG"),
		_hedgeDelta("DLTA")
	{
	}

	bool PTInverseHedge1by1::evaluateFILL(
		fsb::Orders::OrdersVector &orders, 
		fsb::Orders::OrdersVector &cancels, 
		fsb::OrderReplyInfo &reply)
	{
		_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;
		double buyPrice = 0.0, sellPrice = 0.0, price = 0.0;
		int quantity = 0,buys = 0,sells=0;
		bool isBuy;

		if( reply._qtyFilled > 0 )
			isBuy = true;
		else
			isBuy = false;

		std::string type(_hedgeTrader.type());
		if( TraderEnvSingleton::instance()->_params.deltaClose() ) {
			FSB_LOG("Delta closing mode");
			price = _hedgeDelta.calcPrice(_hedge,buyPrice,sellPrice,isBuy);
			quantity = _hedgeDelta.calcQuantity(reply,buys,sells);
			type = _hedgeDelta.type();
		} else {
			price = _hedgeTrader.calcPrice(_hedge,buyPrice,sellPrice,isBuy);
			quantity = _hedgeTrader.calcQuantity(reply,buys,sells);
		}
		quantity = isBuy ? buys : sells;
		if( quantity == 0 ||
			price == 0.0 ) {
				FSB_LOG(" Problem, Hmm... hedgepolicy returned " 
					<< quantity
					<< " " <<  price);
				return false;
		}
		_orderSetup.setup(orders,_hedge.symbol(),_hedge.exchange(),_hedge.type(),quantity,price,isBuy,"DAY","LMT",type,_hedge.index());
		FSB_LOG(" Sending a hedge trade " << _hedge.symbol()
			<< " " << _hedge.exchange()
			<< " " << quantity
			<< " " << price
			<< " " << (isBuy ? "BUY" : "SELL")
			<< " " << type
			);

		return true;
	}
	//*********************************************************************************

	PTHedge1byN::PTHedge1byN()
		:_multiplehedgeTrader("HDG")
	{
	}

	bool PTHedge1byN::evaluateFILL(
		fsb::Orders::OrdersVector &orders, 
		fsb::Orders::OrdersVector &cancels, 
		fsb::OrderReplyInfo &reply)
	{
		bool calcResidue = false;
		Instrument deltaHedge;
		int posInDeltaHedge = 0;
		bool isBuy;
		if( reply._qtyFilled > 0 )
			isBuy = false;
		else
			isBuy = true;
		int quantity = 0,buys = 0,sells=0;
		double buyPrice = 0.0, sellPrice = 0.0, price = 0.0;

		InstrumentObj::VectPtrsItr itr = TraderEnvSingleton::instance()->_hedge.begin();

		for(itr; itr != TraderEnvSingleton::instance()->_hedge.end();++itr) {

			Instrument hedge = (*itr)->_instr;
			int pos = (*itr)->_orders.net();
			quantity = 0,buys = 0,sells=0;
			buyPrice = 0.0, sellPrice = 0.0, price = 0.0;
			bool bsFlag;
			price = _multiplehedgeTrader.calcPrice(hedge,buyPrice,sellPrice,isBuy);
			quantity = _multiplehedgeTrader.calcQuantity(hedge,pos,buys,sells,bsFlag);
			if(hedge.useForDelta() ) {
				calcResidue = true;
				deltaHedge = hedge;
				posInDeltaHedge = pos;
			}
			if( quantity == 0 ||
				price == 0.0 ) {
					FSB_LOG(" Problem, Hmm... hedgepolicy returned " 
						<< quantity
						<< " " <<  price
						<< " " << " BSFlag " << (bsFlag ? "BUY" : "SELL") );
					continue;
			}
			_orderSetup.setup(orders,hedge.symbol(),hedge.exchange(),hedge.type(),quantity,price,isBuy,"DAY","LMT",_multiplehedgeTrader.type(),hedge.index());
			FSB_LOG(" Sending a hedge trade " << hedge.symbol()
				<< " " << hedge.exchange()
				<< " " << quantity
				<< " " << price
				<< " Orig: " << (isBuy ? "BUY" : "SELL")
				<< " BSFlag " << (bsFlag ? "BUY" : "SELL")
				<< " " << _multiplehedgeTrader.type()
				);
		}
		if( calcResidue) {
			buys = sells = 0;			
			buyPrice = sellPrice = 0.0;
			price = _multiplehedgeTrader.calcPrice(deltaHedge,buyPrice,sellPrice,isBuy);
			quantity = _multiplehedgeTrader.getResidue(deltaHedge,posInDeltaHedge,isBuy,buys,sells);
			_multiplehedgeTrader.reset();
			if( quantity == 0 ||
				price == 0.0 ) {
					FSB_LOG(" Problem, Hmm... hedgepolicy returned " 
						<< quantity
						<< " " <<  price);
					return false;
			}
			_orderSetup.setup(orders,deltaHedge.symbol(),deltaHedge.exchange(),deltaHedge.type(),quantity,price,isBuy,"DAY","LMT",CLOSE_DELTA,deltaHedge.index());
			FSB_LOG(" delta calc. " << deltaHedge.symbol()
				<< " buy " << buys
				<< " sell " << sells );

		}
		return true;
	}

	PTBaseCrncyMktUpdate::PTBaseCrncyMktUpdate()
		:PTBase1by1MktUpdate()
	{
	}

	void PTBaseCrncyMktUpdate::calcFV(bool closeOnly,int& buySellSignal)
	{
		_base = TraderEnvSingleton::instance()->_base[0]->_instr;
		_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;

		_theoBid = _theoAsk = 0.0;

		if( closeOnly ) {			
			buySellSignal = _closeModePTFV->fvInverse(_theoBid,_theoAsk,_base,_hedge);
		} else {		
			buySellSignal = _paperTraderFV->fvInverse(_theoBid,_theoAsk,_base,_hedge);
		}
		TraderEnvSingleton::instance()->_base[0]->_pricers.updateFv(_theoBid,_theoAsk);
	}

	void PTBaseCrncyMktUpdate::calcDelta()
	{
		double baseNotional = TraderEnvSingleton::instance()->_base[0]->_orders.net() * _base.lastNoLock() * _base.multiplier();
		double hedgeNotional = TraderEnvSingleton::instance()->_hedge[0]->_orders.net() * _hedge.lastNoLock() * _hedge.multiplier();
		TraderEnvSingleton::instance()->_totalPos.setDelta(baseNotional + hedgeNotional);
	}

}
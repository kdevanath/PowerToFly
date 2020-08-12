#include "stdafx.h"
#include "TraderPolicy.h"

namespace fsb {

	MMKR1by1MktUpdate::MMKR1by1MktUpdate()
		:_takeLiq("TAKE","IOC"),
		_workLiq("WORK"),
		_fvPolicy(TraderEnvSingleton::instance()->_params.buySellFlagPolicy())
	{
		_baseInstrs.reserve(TraderEnvSingleton::instance()->_base.size() + 1);
		_hedgeInstrs.reserve(TraderEnvSingleton::instance()->_hedge.size() + 1);
	}

	bool MMKR1by1MktUpdate::evaluateMARKET(Orders::OrdersVector& orders,
		Orders::OrdersVector& cancels,int buySellFlag,bool closeOnly)
	{
		//************ (TO DO) BETTER LOGGING OF VALUES***************
		_base = TraderEnvSingleton::instance()->_base[0]->_instr;
		_hedge = TraderEnvSingleton::instance()->_hedge[0]->_instr;

		InstrumentObj::VectItrs::iterator itr = TraderEnvSingleton::instance()->_instrObjItrs.begin();
		InstrumentObj::VectItrs::iterator enditr = TraderEnvSingleton::instance()->_instrObjItrs.end();
		InstrumentItr refitr;

		for( itr; itr != enditr; ++itr) {
			if( !(*(*itr))->_instr.tradable() ) {
				FSB_LOG(" " << (*(*itr))->_instr.symbol() << " " << (*(*itr))->_instr.exchange()
					<< " is set to not tradable " );
				continue;
			}
			Instrument instr1 = (*(*itr))->_instr;
			InstrumentKey ref = instr1.refKey();
			int net=0;
			_theoAsk=0.0;_theoBid=0.0;
			FSB_LOG(" Evaluating " << instr1.symbol() << " ref " << ref._symbol);
			if( (refitr = TraderEnvSingleton::instance()->_instruments.find(ref)) != TraderEnvSingleton::instance()->_instruments.end()) {

				_fvPolicy.fv(_theoBid,_theoAsk,instr1,*(refitr->second) );
				int bsSignal = _fvPolicy.bsIndicator(instr1,_theoBid,_theoAsk);
				FSB_LOG(" Updating fv");
				(*(*itr))->_pricers.updateFv(_theoBid,_theoAsk);

				if( bsSignal & BinaryPolicy::BUY ) {
					//FSB_LOG(" Working buy " << _theoBid << "x" << _theoAsk);
					if( TraderEnvSingleton::instance()->_params.takeLiq() ) {
						if(_takeLiq.evaluate(instr1,(*(*itr))->_orders,_theoBid,_theoAsk,true,orders,cancels)) {
							continue;
						}
					}
					if( TraderEnvSingleton::instance()->_params.workLiq() ) {
						//FSB_LOG(" workliq buy");
						_workLiq.evaluate(instr1,(*(*itr))->_orders,_theoBid,_theoAsk,true,orders,cancels);
					}
				}

				//FSB_LOG(" There are " << orders.size());
				if( bsSignal & BinaryPolicy::SELL ) {
					//FSB_LOG(" Working SELL"<< _theoBid << "x" << _theoAsk);
					if( TraderEnvSingleton::instance()->_params.takeLiq() ) {
						if(_takeLiq.evaluate(instr1,(*(*itr))->_orders,_theoBid,_theoAsk,false,orders,cancels)) 
							continue;
					}
					if( TraderEnvSingleton::instance()->_params.workLiq()) {
						//FSB_LOG(" workliq sell");
							_workLiq.evaluate(instr1,(*(*itr))->_orders,_theoBid,_theoAsk,false,orders,cancels);
					}
				}
				//FSB_LOG(" There are " << orders.size());
			}
		}
		return true;
	}

	MMKrHedging::MMKrHedging()
	{
	}

	bool MMKrHedging::evaluateFILL(Orders::OrdersVector& orders,
									Orders::OrdersVector& cancels,
									OrderReplyInfo& reply)
	{
		updateDelta(reply);
		return true;
	}
	void MMKrHedging::updateDelta(OrderReplyInfo& reply)
	{
		InstrumentKey key(reply._symbol,reply._exchange);
		InstrumentItr itr;
		if( (itr=TraderEnvSingleton::instance()->_instruments.find(key)) != TraderEnvSingleton::instance()->_instruments.end())
		{
			double delta = (itr->second->mid() * itr->second->multiplier() * reply._qtyFilled * itr->second->fx())/1000.0;
			TraderEnvSingleton::instance()->_totalPos.delta(delta);
			FSB_LOG(" symbol : " << reply._symbol << " " << delta << " " << TraderEnvSingleton::instance()->_totalPos.delta());
		} else
		{
			FSB_LOG("Could not find the symbol " << reply._symbol);
		}
	}
}
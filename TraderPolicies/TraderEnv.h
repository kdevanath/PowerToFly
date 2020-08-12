#ifndef __TRADER_ENV_H__
#define __TRADER_ENV_H__

#include <Data/TraderParams.h>
#include <Data/lockedmap.h>
#include <boost/shared_ptr.hpp>
#include "MovingAverage.h"
#include "OrderPricePolicy.h"

namespace fsb {

	class TRADERPOLICIES_API TraderEnv
	{
	public:
		TraderEnv() :_tradingStatus(true) { }
		~TraderEnv() { delete _longTermMA; delete _shortTermMA;}
		TraderParams _params;

		InstrumentObj::VectPtrs _base;
		InstrumentObj::VectPtrs _hedge;
		InstrumentObj::VectItrs _instrObjItrs;

		Basket _basket;

		MovingAverage* _longTermMA;
		MovingAverage* _shortTermMA;

		InstrumentPtrMap _instruments;
		InstrumentOrdersPtrMap _instrOrders;
		Pricers::PtrMap _pricers;

		Position _totalPos;
		Position _hedgePos;
		Position _basePos;

		bool _tradingStatus;
		std::string _broadcastMessage;
		ACE_Recursive_Thread_Mutex traderEnvMutex_;

	};

	typedef ACE_Singleton<TraderEnv,ACE_Recursive_Thread_Mutex> TraderEnvSingleton;

	TRADERPOLICIES_API TraderEnv* get_trader_env_instance();

	class InstrumentObj_func_obj : public std::unary_function< InstrumentObj,void>
	{
		class InstrumentObj_impl
		{
			int _totalBuys,_totalSells;
			double _notional;
		public:
			InstrumentObj_impl():_totalBuys(0),_totalSells(0),_notional(0.0){}
			void calc_notional(InstrumentObj* iobj) {
				_notional += iobj->_instr.mid() * (iobj->_orders.net() + iobj->_instr.incomingInv()) * iobj->_instr.multiplier() * iobj->_instr.levRatio();
				_totalBuys+= iobj->_orders.totalBuys();
				_totalSells += iobj->_orders.totalSells();
			}
			void reset() {
				_totalBuys = _totalSells = 0;
				_notional = 0.0;
			}

			void cancelAll(InstrumentObj* iobj,Orders::OrdersVector& cancels) {
				iobj->_orders.getAllOrders(cancels);
			}
			int totalBuys() const { return _totalBuys;}
			int totalSells() const { return _totalSells;}
			double notional() const { return _notional;}
		};
		boost::shared_ptr<InstrumentObj_impl> _instrObjImpl;
	public:
		InstrumentObj_func_obj() :_instrObjImpl(new InstrumentObj_impl()) {}

		void operator () (InstrumentObj* iobj) {
			_instrObjImpl->calc_notional(iobj);
		}

		void operator () (InstrumentObj* iobj,Orders::OrdersVector& cancels) {
			_instrObjImpl->cancelAll(iobj,cancels);
		}

		int getTotalBuys() const 
		{
			return _instrObjImpl->totalBuys();
		}
		int getTotalSells() const 
		{
			return _instrObjImpl->totalSells();
		}
		double getNotional() const 
		{
			return _instrObjImpl->notional();
		}

		void reset()
		{
			_instrObjImpl->reset();
		}
	};

	struct calc_notional
	{
		calc_notional()
			:_notional(0.0)
		{
		}
		void operator() (InstrumentObj* iobj) {
			_notional += iobj->_instr.mid() * iobj->_orders.net() * iobj->_instr.multiplier() * iobj->_instr.levRatio();
		}
		operator double() { return _notional;}
		double _notional;
	};

};
#endif
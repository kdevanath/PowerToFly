#ifndef __GrantTRADER_POLICY_H__
#define __GrantTRADER_POLICY_H__

#include "Mode.h"

namespace fsb {

	class GrantTrader
	{
	public:
		class CrazyEightProblem{};
		GrantTrader()
			:_eodTradingPolicy("EOD","MOC","MKT")
		{
			_bases.reserve(50);
		}
		void evalUserInput(Orders::OrdersVector& neworders,
			Orders::OrdersVector& cancels);

		void evalOrderCXLD(Orders::OrdersVector& neworders,
			Orders::OrdersVector& cancels,
			const OrderReplyInfo& reply);
		void writeTradesFile();
		
	private:
		void readCrazyEightList();		
		Mode<CrossPriceMode,WeightsSizePolicy,EquityOrderSizePolicy,SpiderCancelPolicy,true,MaxPositionBaseNPolicy> _eodTradingPolicy;
		Instrument::VectorOfInstrs _bases;
	};

}

#endif


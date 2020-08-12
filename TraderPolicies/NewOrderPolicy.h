#ifndef __NEW_ORDER_POLICY_H__
#define __NEW_ORDER_POLICY_H__

#include "PairTrader.h"

namespace fsb {

	class TRADERPOLICIES_API NewOrderPolicy
	{
	public:
		NewOrderPolicy(const std::string&,
			const std::string& algName,
			const std::string& tradesfile=""); 
		void respondToUserInput(Orders::OrdersVector& orders,
			Orders::OrdersVector& cancels,UserEventType);
		void insideOrder(Orders::OrdersVector& orders,
			Orders::OrdersVector& cancels,bool start);

		void respondToOrderUpdates(OrderReplyInfo& reply,
								Orders::OrdersVector& orders,
								Orders::OrdersVector& cancels,
								bool& recordStats);
		void fillBook(Orders::OrdersVector& orders) { }
		

		void GetStats(LogData&, const OrderReplyInfo&);

		void generateClientId(std::string&);
		void BuildNewOrder(Order& , std::string& );
		void BuildNewOrder(Orders::OrdersVector& orders,
						std::string& orderId);

		 void BuildNewOrder(const std::string& symbol,
			  const std::string& exchange,
			  char type,
			  int quantity,
			  double price,
			  bool isBuy,
			  const std::string& priceType,
			  std::string& orderId,
			  const std::string& orderSource,
			  Orders::OrdersVector& orders);

	private:
		Trader* _trader;
		//PairTrader<Trader> *_pairTrader;
		//PairTrader<TradeMode*,TradeMode*>* _trader;
		
		std::string _spreadName;	
		char _orderId[100];
		char _guid[255];
	};
	//EXPIMP_TRADERPOLICY_TEMPLATE template class TRADERPOLICIES_API NewOrderPolicy<PaperTraderPolicy>;
	//EXPIMP_TRADERPOLICY_TEMPLATE template class TRADERPOLICIES_API NewOrderPolicy<PaperTraderPolicy*>;
}
#endif
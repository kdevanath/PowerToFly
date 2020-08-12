#ifndef __ORDER_CANCELLATION_POLICY_H__
#define __ORDER_CANCELLATION_POLICY_H__
#include "Policy.h"
#include "PolicyIds.h"
#include <Data/TraderParams.h>
#include "TraderPolicyExports.h"
#include <Data/OrderReplyInfo.h>

namespace fsb {

	class OrderCancellationPolicy : public Policy
	{
	public:
		enum { NONE=0,CANCEL,CANCEL_REPLACE,SEND_NEW};
		virtual int cancel(Orders::OrdersVector& cancels, bool isBuy, double fv, double sendPrice) = 0;
	protected:
		Orders::OrdersVector _potentialCancels;
	};

	class EmptyCancelPolicy
	{
	public:
		EmptyCancelPolicy() {}
		int cancel(Orders::OrdersVector& cancels,bool,double,double) {
			return OrderCancellationPolicy::SEND_NEW;
		}
	};

	

	class CancelUpdateThreshold : public OrderCancellationPolicy
	{
	public:
		CancelUpdateThreshold() {}

		virtual ~CancelUpdateThreshold() {}

		int type() const { return fsb::CancelUpdateThresholdId;}
		std::string name() const { return "CancelUpdateThreshold";}

		CancelUpdateThreshold *clone() const { return new CancelUpdateThreshold(*this);}

		virtual int cancel(Orders::OrdersVector& cancels,bool,double,double);
	};

	class CancelInsideOrders : public OrderCancellationPolicy
	{
	public:
		CancelInsideOrders() {}

		virtual ~CancelInsideOrders() {}

		int type() const { return fsb::CancelInsideOrdersId;}
		std::string name() const { return "CancelInsideOrders";}

		CancelInsideOrders *clone() const { return new CancelInsideOrders(*this);}

		virtual int cancel(Orders::OrdersVector& cancels,bool,double,double);
	};

	class CancelOutsideOrders : public OrderCancellationPolicy
	{
	public:
		CancelOutsideOrders() {}

		virtual ~CancelOutsideOrders() {}

		int type() const { return fsb::CancelOutsideOrdersId;}
		std::string name() const { return "CancelOutsideOrders";}

		CancelOutsideOrders *clone() const { return new CancelOutsideOrders(*this);}

		virtual int cancel(Orders::OrdersVector& cancels,bool,double,double);
		void maxNumOfOrders(Orders::OrdersVector& cancels,int);
		void maxQtyPending(Orders::OrdersVector& cancels,int pending);
		void maxPriceDistance(Orders::OrdersVector& cancels,double bestPrice);
	};

	class CancelAllOrders
	{
	public:
		CancelAllOrders() {}
		int cancel(Orders::OrdersVector& cancels,const OrderReplyInfo&,const double& sendprice = 0.0);
	protected:
		~CancelAllOrders() {}
	};

	class TakeLiqCancel : public OrderCancellationPolicy
	{
	public:
		TakeLiqCancel() {}

		virtual ~TakeLiqCancel() {}

		int type() const { return fsb::TakeLiqCancelId;}
		std::string name() const { return "TakeLiqCancel";}

		TakeLiqCancel *clone() const { return new TakeLiqCancel(*this);}

		virtual int cancel(Orders::OrdersVector& cancels,bool,double,double);
	};

	class SellshortCancel
	{
	public:
		int cancel(Orders::OrdersVector& cancels,const OrderReplyInfo&,const double& sendprice = 0.0);
	private:
		Orders::OrdersVector potentialCancels;
	};
	class CloseOnlyModeCancel
	{
	public:
		int cancel(Orders::OrdersVector& cancels,bool,double,double);
	};

	class MMKRWorkCancelPolicy
	{
		public:
			int cancel(Orders::OrdersVector&,
				const Instrument&,
				Orders&,
				bool,double,double); 
	};

	class MMKRTakeCancelPolicy
	{
		public:
			int cancel(Orders::OrdersVector&,
				const Instrument&,
				Orders&,
				bool,double,double); 
	};

	class SpiderCancelPolicy
	{
		public:
			int cancel(Orders::OrdersVector&,
				const Instrument&,
				Orders&,
				bool,double,double); 
	};

	class SpiderHedgeCancelPolicy {
	public:
		int cancel(fsb::Orders::OrdersVector & , 
			const OrderReplyInfo&, const double& sendprice);
	};

	class EmptyCancelPolicy2
	{
	public:
		EmptyCancelPolicy2() {}
		int cancel(Orders::OrdersVector& cancels,const OrderReplyInfo&, const double& sendprice) {
			return OrderCancellationPolicy::SEND_NEW;
		}
	};
}
#endif
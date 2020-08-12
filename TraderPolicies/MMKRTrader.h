#ifndef __MMKR_TRADER_H__
#define __MMKR_TRADER_H__

#include "TraderPolicy.h"

namespace fsb {

	class MMKR1by1Policy
	{
	public:
		MMKR1by1Policy();
	private:
		SetupOrder _setupOrders;
		FVPolicy1<MarketMakerFV> _mmkrFv;
		Mode<TakeLiqPriceMode,TakeLiqSizePolicy,OrderSizePolicy,TakeLiqCancel> _takeLiq; 
		Mode<WorkLiqPriceMode,MMKRWorkSizePolicy,OrderSizePolicy,CancelUpdateThreshold> _workLiq;
	};
}
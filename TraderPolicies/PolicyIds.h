#ifndef __POLICY_IDS_H__
#define __POLICY_IDS_H__

#include <Utilities/FSBMsgsId.h>

namespace fsb
{
	//needs cleanup badly

	enum {
		UnknownPolicyId = fsb::NextAvailableId,
		JoinPricePolicyId,
		PlusPricePolicyId,
		CrossPricePolicyId,
		AntiSqzPricePolicyId,
		RoundNearestPolicyId,
		RoundAwayPolicyId,
		CancelInsideOrdersId,
		CancelOutsideOrdersId,
		CancelUpdateThresholdId,
		OrderSizePolicyId,
		GenericSizePolicyId,
		HedgeSizePolicyId,
		CrossThresholdId,
		JoinThresholdId,
		MidThresholdId,
		RandomizeSizePolicyId,
		PaperTraderLiqId,
		BaseSizePolicyId,
		TakeLiqSizePolicyId,
		WorkLiqSizePolicyId,
		DeltaCloseSizePolicyId,
		CancelAllOrdersId,
		CompareThresholdId,
		TakeLiqCancelId,
		TakeHedgeSizePolicyId,
		PTModeId,
		PTCloseOnlyModeId,
		EmptySizePolicyId,
		NoBSThresholdId,
		PaperTraderMAId,
		CurrencyPairId,
		OnebyOneId,
		OnebyNId,
		NbyNId,
		MMKRId,
		PaperTraderPolicyId,
		PTBaseModeId,
		PTHedgeModeId,
		PTMultipleHedgeModeId,
		MARatioId,
		MAReturnId,
		MAMultiplyId,
		CEFmaId,
		BidPriceId,
		AskPriceId,
		LastPriceId,
		MidPriceId,
		ETFTraderId,
		ETFPaperTraderId,
		GTraderId,
		CEFTraderId
	};
}
#endif
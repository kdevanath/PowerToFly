#ifndef __ORDER_SIZE_POLICY_H
#define __ORDER_SIZE_POLICY_H

#include <Utilities/FSBLogger.h>
//#include <TraderPolicies/TraderEnv.h>
#include "SizePolicy.h"
#include "TraderPolicyExports.h"

namespace fsb  {

	template<class SizePolicyT, class MaxPosPolicyT>
	class TRADERPOLICIES_API HedgeOrderSizePolicy : public SizePolicyT,private MaxPosPolicyT
	{
	public:
		HedgeOrderSizePolicy()
		{
		}
		int calcQuantity(const OrderReplyInfo& reply,int& buys,int& sells)
		{
			FSB_LOG(" IS NOT IMPLEMENTED");
			return 0;
		}
		int calcQuantity(const OrderReplyInfo& reply,int& buys,int& sells,int& sellshort)
		{
			FSB_LOG(" IS NOT IMPLEMENTED");
			return 0;
		}
	private:
	};

	template<class SizePolicyT>
	class TRADERPOLICIES_API HedgeOrderSizePolicy<SizePolicyT,NoMaxPosPolicy> : public SizePolicyT,private NoMaxPosPolicy
	{
	public:
		HedgeOrderSizePolicy()
			:_inverse(false)
		{
		}

		void setInverse(bool i) { _inverse  = i;}
		
		int calcQuantity(const OrderReplyInfo& reply,int& buys,int& sells)
		{
			SizePolicyT::calcQuantity(reply,buys,sells);
			FSB_LOG(" HedgeOrderSizePolicy  mode " 
				<< " buys " << buys
				<< " sells " << sells
				);
			if(buys) 
				return buys;
			else if(sells)
				return sells;
			else
				return 0;
		}	

		int calcQuantity(const OrderReplyInfo& reply,int& buys,int& sells,int& sellshort)
		{
			// This is how it should be but for faster access I will shortcut it.
			/*fsb::InstrumentPtrMap::iterator itr = TraderEnvSingleton::instance()->_instruments.find(InstrumentKey(reply._symbol, reply._exchange));
			if( itr == TraderEnvSingleton::instance()->_instruments.end() ) {
				FSB_LOG("Can't find instrument " << reply._symbol << ":" << reply._exchange);
				return;
			}
			int refIndex = itr->second->refIndex();
			int hedgePos = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_orders.net();*/
			Instrument hinstr = TraderEnvSingleton::instance()->_hedge[0]->_instr;
			int hedgePos = TraderEnvSingleton::instance()->_hedge[0]->_orders.net() + hinstr.incomingInv();
			
			SizePolicyT::calcQuantity(hinstr,reply,buys,sells);
			sellshort=0;
			if( reply._qtyFilled > 0 ) { // Base is a buy
				if( _inverse ) {
					sells = 0;sellshort = 0;
					buys < 0 ?  buys = abs(buys) : buys = 0;
				} else if(!_inverse ) {
					buys = 0;
					int pendingSells = TraderEnvSingleton::instance()->_hedge[0]->_orders.qtyPendingSells();
					if(pendingSells && sells > 0 ) {
						FSB_LOG(" There are pending sells " << pendingSells << " to sell " << sells << " net " << hedgePos);
						if(pendingSells > hedgePos ) {
							sellshort = sells;sells=0;
						} else {
							(pendingSells + sells) >= hedgePos ? (sellshort = pendingSells + sells - hedgePos,sells-= sellshort) : (sellshort=0);
						}
					} else if(sells > 0 && sells > hedgePos ) {
						hedgePos > 0 ? (sellshort = sells - hedgePos,sells = hedgePos) : (sellshort = sells,sells=0);
					} else if(sells < 0 )
						sells = 0;
				}
				FSB_LOG(" Base is a BUY " << buys << " " << sells << " " << sellshort);
			} else if( reply._qtyFilled < 0 ) { // Base is a Sell
				if( !_inverse) {
					sells = sellshort = 0;
					buys = buys < 0 ? 0 : buys;
					//buys < 0 ? buys = abs(buys) : buys = 0;
				} else if( _inverse ) {
					buys = 0;
					int pendingSells = TraderEnvSingleton::instance()->_hedge[0]->_orders.qtyPendingSells();
					if(pendingSells && sells > 0 ) {
						FSB_LOG(" There are pending sells " << pendingSells << " to sell " << sells << " net " << hedgePos);
						pendingSells > hedgePos ? (sellshort = sells,sells=0): (sellshort = sells - (hedgePos-pendingSells), sells=hedgePos-pendingSells);
					} else  if(sells > 0 && sells > hedgePos ) {
						hedgePos > 0 ? (sellshort = sells - hedgePos,sells = hedgePos) : (sellshort = sells,sells=0);
					} else if( sells < 0 )
						sells = 0;
				}
				FSB_LOG(" Base is a SELL " << buys << " " << sells << " " << sellshort);
			}
			int lotsize = hinstr.lotsize();
			
			if( buys)
				buys = fsbutils::RoundingFunctions::Round(lotsize,buys);
			else if (sells)
				sells = fsbutils::RoundingFunctions::Round(lotsize,sells);
			if (sellshort)
				sellshort = fsbutils::RoundingFunctions::Round(lotsize,sellshort);

			FSB_LOG(" HedgeOrderSizePolicy  mode " 
				<< " buys " << buys
				<< " sells " << sells
				<< " hedgePos " << hedgePos
				<< " sellshort " << sellshort
				<< " lots " << lotsize
				<< " inverse " << (_inverse ? "YES" : "NO")
				<< " Base " << (reply._qtyFilled > 0 ? "BUY" : "SELL")
				);
			return 0;
		}
	private:
		bool _inverse;
	};

	template<>
	class TRADERPOLICIES_API HedgeOrderSizePolicy<MultipleHedgesSizePolicy,NoMaxPosPolicy> : public MultipleHedgesSizePolicy,private NoMaxPosPolicy
	{
	public:
		int calcQuantity(const Instrument& hedge,
						const int& pos,
						int& buys, 
						int& sells,
						bool& buyFlag)
		{
			buys=sells=0;
			MultipleHedgesSizePolicy::calcQuantity(hedge,pos,buys,sells);			
			int qty=0;
			buys ?  buyFlag = true,qty = buys : buyFlag = false,qty=sells;
			FSB_LOG(" MultipleHedgesSizePolicy  mode " 
				<< " buys " << buys
				<< " sells " << sells 
				<< " " << (buyFlag ? "BUY" : "SELL"));
			return qty;

		}

		int getResidue(const Instrument& hedge, const int& pos,bool isBuy,int& buys, int& sells)
		{
			calcResidue(hedge,pos,isBuy,buys,sells);
			return (buys ? buys : sells);
		}

		void reset()
		{
			resetResidue();
		}
	};

	template<class SizePolicyT>
	class TRADERPOLICIES_API CloseOrderSizePolicy : private SizePolicyT
	{
	public:
		CloseOrderSizePolicy()
		{
		}

		int calcQuantity(int& buys, int& sells,bool isBuy)
		{
			int maxPos = 0;
			int totalbuys,totalsells;

			if( isBuy ) {
			  totalbuys = TraderEnvSingleton::instance()->_base[0]->_orders.totalBuysInclPending();
			  totalsells = TraderEnvSingleton::instance()->_base[0]->_orders.totalSells();
			} else {
				totalbuys = TraderEnvSingleton::instance()->_base[0]->_orders.totalBuys();
			    totalsells = TraderEnvSingleton::instance()->_base[0]->_orders.totalSellsInclPending();
			}
			int net = totalbuys - totalsells;
			if ( net == maxPos ) {
				buys=sells=0;
				FSB_LOG(" net is 0 ");
				return 0;
			}
			SizePolicyT::calcQuantity(buys,sells);

			net > 0 ? buys = 0 : sells = 0;

			FSB_LOG(" Close only mode " 
				<< " buys " << buys
				<< " sells " << sells
				<< " net " << net );

			int quantity = isBuy ? buys : sells;
			return quantity;
		}
	};

	template<class SizePolicyT,class MaxPosPolicyT>
	class TRADERPOLICIES_API OrderSizePolicy : private SizePolicyT,private MaxPosPolicyT
	{
	public:
		OrderSizePolicy()
		{
		}
		int calcQuantity(const Instrument& instr,int net,int& buys, int& sells)
		{
			int maxPos = MaxPosPolicyT::calcMaxPosition(instr);		
			SizePolicyT::calcQuantity(instr,buys,sells);
			int validBuy = maxPos - net ;
			int validSell = maxPos + net;

			net <= (maxPos * -1) ? sells = 0 : sells = std::min<int>(validSell,sells);
			net >= maxPos ? buys = 0 : buys = std::min<int>(validBuy,buys);

			FSB_LOG( " maxPos " << maxPos
				<< " net " << net
				<< " validBuy " << validBuy
				<< " validSell " << validSell
				<< " buys " << buys
				<< " sells " << sells
				);

			return (buys ? buys : sells);
		}
	};

	template<class SizePolicyT, class MaxPosPolicyT>
	class TRADERPOLICIES_API EquityOrderSizePolicy : private SizePolicyT,private MaxPosPolicyT
	{
	public:
		EquityOrderSizePolicy()
		{
		}
		void calcQuantity(const Instrument& instr,int net,int& buys, int& sells,int& sellshort)
		{
			sellshort=0;
			
			try {
			int maxPos = MaxPosPolicyT::calcMaxPosition(instr,net);
			SizePolicyT::calcQuantity(instr,buys,sells);
			int validBuy = maxPos - net ;
			int validSell = maxPos + net;

			net <= (maxPos * -1) ? sells = 0 : sells = std::min<int>(validSell,sells);
			net >= maxPos ? buys = 0 : buys = std::min<int>(validBuy,buys);
			//FSB_LOG("sells " << sells << " buys " << buys);
			if( sells && sells > net ) {
				net > 0 ? (sellshort = sells - net,sells = net) : (sellshort = sells,sells=0);
			}
			bool baseHTB = TraderEnvSingleton::instance()->_params.baseHTB();
			bool hedgeHTB = TraderEnvSingleton::instance()->_params.hedgeHTB();

			if( baseHTB ) {
				FSB_LOG(" Base is HTB sell short " << sellshort);
				sellshort = 0;
			}
			if( hedgeHTB) {
				FSB_LOG(" Hedge is HTB buys " << buys);
				int refIndex = instr.refIndex();
				int hedgePos = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_orders.net();					
				hedgePos += (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_instr.incomingInv();
				FSB_LOG( " It is a buy " << buys << " hedge pos including incoming " << hedgePos);
				buys = hedgePos <= 0 ? 0 : buys;
				//buys = 0;
			}
			/*if(!baseHTB && !hedgeHTB) {
			}else if(baseHTB && !hedgeHTB) {
				FSB_LOG(" Base is HTB and Hedge is not HTB, sellshort= " << sellshort << " will be set to 0 ");
				sellshort = 0;				
			} else if( baseHTB && hedgeHTB) {
				FSB_LOG(" Base & Hedge is HTB");
				if( sellshort) {
					FSB_LOG( "The order is a sell sellshort " << sellshort << " will be set to 0 ");
					sellshort = 0;
				}else if( buys) {
					int refIndex = instr.refIndex();
					int hedgePos = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_orders.net();					
					hedgePos += (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_instr.incomingInv();
					FSB_LOG( " It is a buy " << buys << " hedge pos including incoming " << hedgePos);
					buys = hedgePos <= 0 ? 0 : buys;
				}
			} else if( !baseHTB && hedgeHTB) {
				FSB_LOG(" Base is not HTB & Hedge is HTB");
				if( buys) {
					int refIndex = instr.refIndex();
					int hedgePos = (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_orders.net();					
					hedgePos += (*TraderEnvSingleton::instance()->_instrObjItrs[refIndex])->_instr.incomingInv();
					FSB_LOG( " It is a buy " << buys << " hedge pos including incoming " << hedgePos);
					buys = hedgePos <= 0 ? 0 : buys;
				}
				}*/
			FSB_LOG( " maxPos " << maxPos
				<< " net " << net
				<< " validBuy " << validBuy
				<< " validSell " << validSell
				<< " sshort " << sellshort
				<< " buys " << buys
				<< " sells " << sells
				);
			} catch(ReachedTargetShares) {
				sells = 0;buys=0;
			}
		}
	};

}
#endif
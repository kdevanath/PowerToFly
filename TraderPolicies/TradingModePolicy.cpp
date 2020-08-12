#include "stdafx.h"
#include "TradingModePolicy.h"
#include "PolicyFactory.h"
#include "TraderEnv.h"

namespace fsb {

	HedgePriceMode::HedgePriceMode()
	{
		_roundingPolicy = (RoundingPolicy*) PolicyFactorySingleton::instance()->create("RoundNearestPolicy");
	}

	double HedgePriceMode::calcPrice(const Instrument& hedge,double& sendBidPrice, double& sendAskPrice, bool isBuy)
	{
		int hedgePrem = TraderEnvSingleton::instance()->_params.hedgePremium();
		double sellprice = hedge.bidNoLock() * (1 - (hedgePrem/10000.0));
		sendAskPrice = _roundingPolicy->round(hedge.fsbticksize(),false,sellprice);
		double buy =  hedge.askNoLock() *  (1 + (hedgePrem/10000.0));
		sendBidPrice = _roundingPolicy->round(hedge.fsbticksize(),true,buy);

		FSB_LOG(" Hedging w/ Premium: " << sendBidPrice << "X " << sendAskPrice
			<< " Premium " << hedgePrem
			<< " w/ Prem " << hedge.bidNoLock() << "X " << hedge.askNoLock() );
		if( isBuy)
			return sendBidPrice;
		else
			return sendAskPrice;
	}

	TakeLiqPriceMode::TakeLiqPriceMode()
	{
		_pricePolicy = (OrderPricePolicy*)PolicyFactorySingleton::instance()->create("CROSS");
		
	}

	bool TakeLiqPriceMode::calcPrice(Instrument& instr,bool buyFlag,double& fv, double& sendPrice)
	{
		
		int tradeThreshold = TraderEnvSingleton::instance()->_params.tradeThreshold();
		if( !buyFlag ) {
			double baseMktBid = instr.bidNoLock();
			if( baseMktBid >= ( fv * ( 1+ tradeThreshold/10000.0))) {
				sendPrice = baseMktBid;
				return true;
			}
		} else {
			double baseMktAsk = instr.askNoLock();
			if( baseMktAsk <= ( fv * ( 1 - tradeThreshold/10000.0))) {
				sendPrice = baseMktAsk; 
				return true;
			}
		}
		FSB_LOG(" take liq: Did not meet trade threshold criteria "
			<< " " << fv << " " << tradeThreshold
			<< " " << (buyFlag ? "BUY" : "SELL") );

		return false;

	}

	WorkLiqPriceMode::WorkLiqPriceMode()
	{
		_roundPolicy = (RoundingPolicy*)PolicyFactorySingleton::instance()->create("RoundAwayPolicy");
		_pricePolicy = (OrderPricePolicy*)PolicyFactorySingleton::instance()->create("SQZ");
		
	}
	bool WorkLiqPriceMode::calcPrice(Instrument& instr,bool buyFlag,double& fv, double& sendPrice)
	{
		if( TraderEnvSingleton::instance()->_params.pricePolicyChanged() ) {
			std::string ppname(TraderEnvSingleton::instance()->_params.pricePolicy());
			FSB_LOG(" Price policy has changed from : " << _pricePolicy->name()
				<< " to : " << ppname );
			TraderEnvSingleton::instance()->_params.pricePolicyChanged(false);
			_pricePolicy = (OrderPricePolicy*)PolicyFactorySingleton::instance()->create(ppname);
		}
		if( buyFlag ) {
			double roundbid = _roundPolicy->round(instr.fsbticksize(),true,fv);
			sendPrice = _pricePolicy->calcPrice(&instr,true,roundbid);
		} else {
			double roundask = _roundPolicy->round(instr.fsbticksize(),false,fv);
			sendPrice = _pricePolicy->calcPrice(&instr,false,roundask);
		}		
		return true;
	}

	void CloseOnlyPriceMode::calcFV(Instrument& instr,double& theoBid, double& theoAsk)
	{
		double ma1 = TraderEnvSingleton::instance()->_longTermMA->result();
		double ma2 = TraderEnvSingleton::instance()->_shortTermMA->result();

		double avgMa = (ma1+ma2)/2.0;

		double hedgeAsk = instr.askNoLock();
		double hedgeBid = instr.bidNoLock();

		double closeBaseAsk = avgMa * hedgeAsk;
		double closeBaseBid = avgMa * hedgeBid;

		theoBid	= std::max<double>(closeBaseBid,theoBid);
		theoAsk = std::min<double>(closeBaseAsk,theoAsk);

		FSB_LOG( " Close only mode " 
					<< " raw bid fv " << theoBid
					<< " raw ask fv " << theoAsk
					<< " close fv bid " << closeBaseBid
					<< " close fv ask " << closeBaseAsk);
	}

	MMKRWorkLiqPriceMode::MMKRWorkLiqPriceMode()
	{
		_roundPolicy = (RoundingPolicy*)PolicyFactorySingleton::instance()->create("RoundAwayPolicy");
		_pricePolicy = (OrderPricePolicy*)PolicyFactorySingleton::instance()->create("SQZ");
		
	}
	bool MMKRWorkLiqPriceMode::calcPrice(Instrument& instr,bool buyFlag,double& fv, double& sendPrice)
	{		
		if( TraderEnvSingleton::instance()->_params.pricePolicyChanged() ) {
			std::string ppname(TraderEnvSingleton::instance()->_params.pricePolicy());
			FSB_LOG(" Price policy has changed from : " << _pricePolicy->name()
				<< " to : " << ppname );
			TraderEnvSingleton::instance()->_params.pricePolicyChanged(false);
			_pricePolicy = (OrderPricePolicy*)PolicyFactorySingleton::instance()->create(ppname);
		}
		if( buyFlag ) {
			double roundbid = _roundPolicy->round(instr.fsbticksize(),true,fv);
			sendPrice = _pricePolicy->calcPrice(&instr,true,roundbid);
		} else {
			double roundask = _roundPolicy->round(instr.fsbticksize(),false,fv);
			sendPrice = _pricePolicy->calcPrice(&instr,false,roundask);
		}		
		return true;
	}

	SpiderWorkLiqPriceMode::SpiderWorkLiqPriceMode()
	{
		_roundPolicy = (RoundingPolicy*)PolicyFactorySingleton::instance()->create("RoundAwayPolicy");
		_pricePolicy = (OrderPricePolicy*)PolicyFactorySingleton::instance()->create("JOIN");	
	}

	bool SpiderWorkLiqPriceMode::calcPrice(Instrument& instr,bool buyFlag,double& fv, double& sendPrice)
	{
		if( buyFlag ) {
			double roundbid = _roundPolicy->round(instr.fsbticksize(),true,fv);
			sendPrice = _pricePolicy->calcPrice(&instr,true,roundbid);
		} else {
			double roundask = _roundPolicy->round(instr.fsbticksize(),false,fv);
			sendPrice = _pricePolicy->calcPrice(&instr,false,roundask);
		}		
		return true;
	}

	ProspectPriceMode::ProspectPriceMode()
	{
		//_pricePolicy = (OrderPricePolicy*)PolicyFactorySingleton::instance()->create("CROSS");		
	}

	bool ProspectPriceMode::calcPrice(Instrument& instr,bool buyFlag,double& fv, double& sendPrice)
	{
		sendPrice = 0.0;
		double baseMid = instr.midNoLock();
		if( !buyFlag ) {
			double baseMktBid = instr.bidNoLock();
			if(fv < baseMid) { // fvAsk 
				sendPrice = baseMktBid;
				return true;
			}
		} else {
			double baseMktAsk = instr.askNoLock();
			if( fv > baseMid) { //fvBid
				sendPrice = baseMktAsk; 
				return true;
			}
		}
		FSB_LOG(" Prospect liq: Did not set up "
			<< " " << fv << " " << baseMid
			<< " " << (buyFlag ? "BUY" : "SELL") );

		return false;

	}

	bool CrossPriceMode::calcPrice(Instrument& instr,bool buyFlag,double& fv, double& sendPrice)
	{
		if(instr.askNoLock() <= 0.0 || instr.bidNoLock() <= 0.0 ) {
			sendPrice = 0.0;
			FSB_LOG(" Zero prices " << instr.bidNoLock() <<" X " << instr.askNoLock());
			return false;
		}
		sendPrice = buyFlag ? instr.askNoLock() : instr.bidNoLock();
		return true;
	}
}
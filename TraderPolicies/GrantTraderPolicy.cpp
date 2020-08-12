#include "stdafx.h"
#include <process.h>
#include "GrantTraderPolicy.h"
#include <fstream>
#include <boost/tokenizer.hpp>


namespace fsb {

	void GrantTrader::evalUserInput(fsb::Orders::OrdersVector &neworders, fsb::Orders::OrdersVector &cancels)
	{
			//run grant program
			//read crazy_eight_list list	
			//Read the weights file
		if(TraderEnvSingleton::instance()->_params.priceType() == PT_MARKET) {
			_eodTradingPolicy.priceType(PT_MARKET);
			_eodTradingPolicy.tif(TIF_MOC);

		}else {
			_eodTradingPolicy.priceType(PT_LIMIT);
			_eodTradingPolicy.tif(TIF_DAY);
		}
		std::for_each(TraderEnvSingleton::instance()->_base.begin(),
				TraderEnvSingleton::instance()->_base.end(), copy_instrument(_bases));
		try {
			readCrazyEightList();
			int itrnum = 0;
			double fvBid=0.0,fvAsk=0.0;
			int numNewOrders=0;
			FSB_LOG("Evaluating");
			for(Instrument::VectorOfInstrs::iterator i = _bases.begin(); i!= _bases.end(); ++i,itrnum++) {
				//std::cout << "  " << i->symbol() << itrnum << std::endl;
				if(i->weight() > 0.0) {
					FSB_LOG(" BUY Symbol " << i->symbol() << " " << i->weight());
				if( _eodTradingPolicy.evaluate(*i,
						TraderEnvSingleton::instance()->_base[itrnum]->_orders,
						fvBid,fvAsk,true,neworders,cancels) ) {
							FSB_LOG(" WERT LOG "
								<< " BUY " << " "
								<< i->symbol() << " "
								<< i->weight() << " "
								<< i->bidNoLock() << "X "
								<< i->askNoLock() << " "
								<< neworders[numNewOrders]._price << " "
								<< neworders[numNewOrders]._quantity);
							numNewOrders++;
				}
				}else if(i->weight() <= 0.0) {
					FSB_LOG(" SELL Symbol " << i->symbol() << " " << i->weight());
					if( _eodTradingPolicy.evaluate(*i,
						TraderEnvSingleton::instance()->_base[itrnum]->_orders,
						fvBid,fvAsk,false,neworders,cancels) ) {

							FSB_LOG(" WERT LOG "
								<< " SELL " << " "
								<< i->symbol() << " "
								<< i->weight() << " "
								<< i->bidNoLock() << "X "
								<< i->askNoLock() << " "
								<< neworders[numNewOrders]._price << " "
								<< neworders[numNewOrders]._quantity);
							numNewOrders++;
					}
				}
			}
			/*for(int i=0;i<neworders.size();i++) {
				std::cout << " Sending " << neworders[i]._symbol << " " << neworders[i]._quantity
					<< " " << neworders[i]._marketType << " " << neworders[i]._priceType << std::endl;
			}*/
			
		} catch(CrazyEightProblem)
		{
			FSB_LOG(" Problem with Grant stuff check it out....");
		}
		if(!_bases.empty())
				_bases.clear();
	}

	void GrantTrader::evalOrderCXLD(Orders::OrdersVector& neworders,
			Orders::OrdersVector& cancels,
			const OrderReplyInfo& reply)
	{
		InstrumentKey instrK(reply._symbol, TraderEnvSingleton::instance()->_base[0]->_instr.exchange());
		InstrumentItr iitr = TraderEnvSingleton::instance()->_instruments.find(instrK);
		if( iitr !=  TraderEnvSingleton::instance()->_instruments.end()) {
			FSB_LOG("Replacing " << iitr->second->symbol() << iitr->second->exchange());
			Instrument base = *(iitr->second);
			_eodTradingPolicy.evaluate(base,TraderEnvSingleton::instance()->_base[base.index()]->_orders,0.0,0.0,reply._isBuy,neworders,cancels);	
		} else {
			FSB_LOG("Cannot find " << reply._symbol << ":" << reply._exchange);
		}
	}

	void GrantTrader::readCrazyEightList()
	{
		ifstream in("C:\\FSB\\Grant\\CrazyEight\\crazy_eight_list.csv");
		if(!in.is_open()) {
			FSB_LOG("Problem opening crazy_eight_list.csv");
			throw CrazyEightProblem();
		}
		ofstream out("C:\\FSB\\Grant\\CrazyEight\\live_prices.csv");
		if(!out.is_open()) {
			FSB_LOG("Problem opening live_prices.csv");
			throw CrazyEightProblem();
		}
		typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;
		std::vector< int > vec;
		vec.resize(_bases.size());
		std::string line;
		int symbolIndex = 0;
		while (getline(in,line))
		{
			Tokenizer tok(line);
			Instrument instr(line,_bases[0].exchange(),0,1);
			fsb::Instrument::InstrsIterator itr = std::find(_bases.begin(),_bases.end(),instr);
			if( itr == _bases.end()) {
				FSB_LOG("Check.. Check.... We are missing a symbol " << line << " in our config file");
				throw CrazyEightProblem();
			} else {
				std::cout << " " << line << " | " << itr->symbol() << itr->index() << std::endl;				
				vec[symbolIndex++] = itr->index();
				//TraderEnvSingleton::instance()->_base[itr->index()]->_instr.refIndex(symbolIndex++);
				out << itr->midNoLock() << "," << std::endl;
			}
		}
		out.close();
		in.close();
		//Run crazy_eight.exe
		if( ::system("C:\\FSB\\Grant\\CrazyEight\\ce.bat") )
		{
			FSB_LOG("Problem executing ce.bat");
			throw CrazyEightProblem();
		}
		//Read the weights
		ifstream wts("C:\\FSB\\Grant\\CrazyEight\\weights.csv");
		if(!wts.is_open()) {
			FSB_LOG("Problem opening weights.csv");
			throw CrazyEightProblem();
		}
		symbolIndex=0;
		std::istringstream wtstring;
		float weight=0.0;
		while (getline(wts,line))
		{
			Tokenizer tok(line);
			wtstring.str(line);
			wtstring >> weight;
			wtstring.clear();

			_bases[vec[symbolIndex]].weight(weight);
			TraderEnvSingleton::instance()->_base[vec[symbolIndex++]]->_instr.weight(weight);
			
			FSB_LOG(line << " " << TraderEnvSingleton::instance()->_base[vec[symbolIndex-1]]->_instr.symbol());
		}
		wts.close();
	}

	void GrantTrader::writeTradesFile()
	{
		ofstream out("C:\\FSB\\data\\grant_trades.txt");
		if(!out.is_open()) {
			FSB_LOG("Problem Writing trades to file grant_trades.txt");
			throw CrazyEightProblem();
		}
		out << "[INCOMING]" << std::endl;
		for(int i = 0; i < TraderEnvSingleton::instance()->_instrObjItrs.size();i++) {
			int net = (*TraderEnvSingleton::instance()->_instrObjItrs[i])->_orders.net() +
				(*TraderEnvSingleton::instance()->_instrObjItrs[i])->_instr.incomingInv();

			out << (*TraderEnvSingleton::instance()->_instrObjItrs[i])->_instr.symbol()
				<< "="
				<< net
				<< ";" << std::endl;

		}
		out << "[END]" << std::endl;
		out.close();
		
	}
}
#include "stdafx.h"
#include "PairTrader.h"

namespace fsb {

	Trader::~Trader()
	{
		//close the file
		/*if(_tradesStream->is_open())
			_tradesStream->close();*/
	}

	void Trader::OpenTradesFile(const std::string& filename)
	{
		if(filename.empty()) {
			FSB_LOG(" The filename is empty, will not write trades file");
			return;
		}
		_tradesFilename = filename;

		/*_tradesStream = new ofstream(filename.c_str(),);
		if(!_tradesStream->is_open()) {
			FSB_LOG("Problem Opening file " << filename);
			throw TradesFileProblem();
		}*/
	}

	void Trader::WriteTradesToFile()
	{
		if( _tradesFilename.empty())
			return;
		ofstream out(_tradesFilename.c_str());
		if(!out.is_open()) {
			FSB_LOG("Problem Writing trades to file " << _tradesFilename);
			throw TradesFileProblem();
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

		/*if(!_tradesStream->is_open()) {
			FSB_LOG("Problem Writing trades to file, it is not open ");
			throw TradesFileProblem();
		}

		_tradesStream->seekp(0);

		(*_tradesStream) << "[INCOMING]" << std::endl;
		for(size_t i = 0; i < TraderEnvSingleton::instance()->_instrObjItrs.size();i++) {
			int net = (*TraderEnvSingleton::instance()->_instrObjItrs[i])->_orders.net() +
				(*TraderEnvSingleton::instance()->_instrObjItrs[i])->_instr.incomingInv();

			(*_tradesStream) << (*TraderEnvSingleton::instance()->_instrObjItrs[i])->_instr.symbol()
				<< "="
				<< net
				<< ";" << std::endl;

		}
		(*_tradesStream) << "[END]" << std::endl;*/
	}
}

fsb::PolicyRegistrar<fsb::CurrencyPair> CurrencyPairPrototype;
fsb::PolicyRegistrar<fsb::PairTrader1by1> PairTrader1by1Prototype;
fsb::PolicyRegistrar<fsb::PairTrader1byN> PairTrader1byNPrototype;
fsb::PolicyRegistrar<fsb::MarketMaker> MarketMakerPrototype;
fsb::PolicyRegistrar<fsb::ETFTrader> ETFTraderPrototype;
fsb::PolicyRegistrar<fsb::ETFPaperTrader> ETFPaperTraderPrototype;
fsb::PolicyRegistrar<fsb::GTrader> GrantTraderPrototype;
fsb::PolicyRegistrar<fsb::CEFTrader> CEFTraderPrototype;
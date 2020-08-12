#include "stdafx.h"
#include <iostream>
#include <Utilities/genericfns.h>
#include "PolicyFactory.h"
#include "MovingAverage.h"
#include "TraderEnv.h"

namespace fsb {

	MASpreadPolicy::MASpreadPolicy(const std::string& name,
		const std::string& pricePolicyname)
			:_previous(0.0),
			_ma(0.),
			_dp0(0.0),
			_numerator(0.),
			_denominator(0.)
			//_datapointPolicy(pricePolicyname)
		{
			for( size_t i = 0; i < TraderEnvSingleton::instance()->_base.size(); ++i)
			{
				_base.push_back(&(TraderEnvSingleton::instance()->_base[i]->_instr));

			}
			for( size_t i = 0; i < TraderEnvSingleton::instance()->_hedge.size(); ++i)
			{
				_hedge.push_back(&(TraderEnvSingleton::instance()->_hedge[i]->_instr));
			}
			FSB_LOG(" Price policy " << pricePolicyname << " ma type policy " << name);
			_priceType = dynamic_cast<PriceTypePolicy*>(fsb::PolicyFactorySingleton::instance()->create(pricePolicyname));
			try {
				_maPolicy = dynamic_cast<MATypePolicy*>(fsb::PolicyFactorySingleton::instance()->create(name));
			} catch(UnknownPolicy)
			{
				FSB_LOG("Error Unknown MA policy " << name);
				throw UnknownPolicy();
			}
	}

	double MASpreadPolicy::calculate(int length,int frequency,time_t secs)
	{
		_numerator = (*_priceType)(*(_base[0]));
		_denominator = (*_priceType)(*(_hedge[0]));

		//_dp0 = _datapointPolicy.calculate(*(_base[0]),*(_hedge[0]));
		_dp0 = (*_maPolicy)(_numerator,_denominator);
		if(!_previous ) {
			FSB_LOG(" Beginning of the day, start val is 0 before the first snapshot");
			_previous = _dp0;
		}

		_ma = ((_dp0 * frequency) + (_previous * (length - frequency)))/length;
		_previous = _ma;
		return _ma;
	}

	double MASpreadPolicy::numerator()
	{
		_numerator = 0.0;
		if(_base[0]->bid() > 0.0 && _base[0]->ask() > 0.0)
			_numerator = (_base[0]->bid() + _base[0]->ask())/2.0;

		return _numerator;
	}

	double MASpreadPolicy::denominator()
	{
		double bid = 0.0, ask = 0.0;
		Instrument::InstrPtrsIterator itr = _hedge.begin();
		for(itr; itr != _hedge.end(); ++itr)
		{
			if( (*itr)->bid() <= 0 || (*itr)->ask() <= 0 ) break;

			bid += ( (*itr)->bid()* (*itr)->factor());
			ask += ( (*itr)->ask()* (*itr)->factor());
		}
		if( bid > 0.0 && ask > 0.0)
			_denominator = (bid + ask)/2.0;
		else
			_denominator = 0.0;

		return _denominator;
	}

	void MASpreadPolicy::print()
	{
		FSB_LOG(" " << _base[0]->symbol()
			<< " " << _numerator
			<< " " << _hedge[0]->symbol()
			<< " " << _denominator
			<< " previous " << _previous
			<< " _dataPoint0 " << _dp0
			<< " _result " << _ma);
	}
//*****************************************************************
	InstrumentMAPolicy::InstrumentMAPolicy(const std::string& maTypename,
		const std::string& pricePolicyname)
		:_datapointPolicy(pricePolicyname)
	{
		try {
		_maPolicy = dynamic_cast<MATypePolicy*>(fsb::PolicyFactorySingleton::instance()->create(maTypename));
		} catch(UnknownPolicy)
		{
			FSB_LOG("Error Unknown MA policy " << maTypename);
			throw UnknownPolicy();
		}
		_MAs.resize(TraderEnvSingleton::instance()->_base.size() + TraderEnvSingleton::instance()->_hedge.size());

		for( size_t i = 0; i < TraderEnvSingleton::instance()->_base.size(); i++)
			_instrs.push_back(&(TraderEnvSingleton::instance()->_base[i]->_instr));
			
		for( size_t i = 0; i < TraderEnvSingleton::instance()->_hedge.size(); i++)
			_instrs.push_back(&(TraderEnvSingleton::instance()->_hedge[i]->_instr));

		FSB_LOG("There are " << _instrs.size());
		for(size_t i = 0; i < _instrs.size(); i++) {
			int instrIndx = _instrs[i]->index();
			_MAs[instrIndx]._instrIdx = instrIndx;
			_MAs[instrIndx]._symbol = _instrs[i]->symbol();
			_MAs[instrIndx]._exchange = _instrs[i]->exchange();
			InstrumentKey refkey = _instrs[i]->refKey();
			InstrumentPtrMap::iterator itr;
			if( (itr = TraderEnvSingleton::instance()->_instruments.find(refkey)) != TraderEnvSingleton::instance()->_instruments.end()) {
				_MAs[instrIndx]._refKeyIndex = itr->second->index();
			} else {
				FSB_LOG("Problem with Ref Key " << refkey._symbol << " " << refkey._exchange << " not available ");
			}

			FSB_LOG(" " << _MAs[instrIndx]._symbol 
				<< " " << _MAs[instrIndx]._exchange
				<< " " << instrIndx
				<< " " << _MAs[instrIndx]._refKeyIndex
				<< " " << refkey._symbol << " " << refkey._exchange);
		}
	}

	double InstrumentMAPolicy::calculate(int length,int frequency,time_t secs)
	{
		int idx = 0;
		Instrument::InstrPtrsIterator itr = _instrs.begin();
		for(itr; itr != _instrs.end(); ++itr)
		{		
			idx = (*itr)->index();
			/*FSB_LOG("Calculating: " << (*itr)->symbol() 
				<< " " <<  idx 
				<< " " << (*itr)->refKey()._symbol);*/
			calculate(length,frequency,itr,_MAs[idx]);
		}
		return( (*_maPolicy)(_MAs[0]._ma, _MAs[1]._ma) );
	}

	void InstrumentMAPolicy::calculate(int length,
		int frequency,
		Instrument::InstrPtrsIterator itr,
		MAInfo& maInfo)
	{
		 
		if( (*itr)->bid() <= 0 || (*itr)->ask() <= 0 ) {
			FSB_LOG(" WIll not calc. MA due to zero vals. " << (*itr)->bid()
				<< " " << (*itr)->ask() << " " << (*itr)->symbol());
			return;
		}

		maInfo._dp0 = _datapointPolicy.calculate( *(*itr));

		double prev = maInfo._previous;
		if( !prev) {
			FSB_LOG(" Beginning of the day, start val is 0 before the first snapshot");
			prev = maInfo._dp0;
			maInfo._previous = prev;
		} else
			maInfo._previous = maInfo._ma;

		maInfo._ma = ((maInfo._dp0 * frequency) + (prev * (length - frequency)))/(double)length;
	}

	double InstrumentMAPolicy::ma()
	{
		return  (*_maPolicy)(_MAs[0]._ma,_MAs[1]._ma);
	}

	double InstrumentMAPolicy::datapoint0()
	{
		return (*_maPolicy)(_MAs[0]._dp0,_MAs[1]._dp0);
	}
	double InstrumentMAPolicy::ma(const Instrument& i)
	{
		return _MAs[i.index()]._ma;

	}
	double InstrumentMAPolicy::maByPolicy(const Instrument& i)
	{
		int idx = i.index();
		int refidx = _MAs[idx]._refKeyIndex;

		double mavg = (*_maPolicy)(_MAs[idx]._ma , _MAs[refidx]._ma);
		/*FSB_LOG(" Requesting MA for " << i.symbol()
			<< " " << _MAs[refidx]._symbol
			<< " " <<  _MAs[idx]._ma
			<< " " << _MAs[refidx]._ma
			<< " " << mavg);*/
		return mavg;
	}

	void InstrumentMAPolicy::print()
	{
		for(size_t i = 0;i<_MAs.size();i++)
		{
			FSB_LOG(" " << _MAs[i]._symbol
				<< " " << _MAs[i]._exchange
				<< " " << _MAs[i]._dp0
				<< " " << _MAs[i]._previous
				<< " " << _MAs[i]._ma);
		}
	}
	//*******************************************************************

	SpiderMAPolicy::SpiderMAPolicy(const std::string& maTypename,
									const std::string& pricePolicyname)
									:_previous(0.0),
									_ma(0.0),
									_timestamp(0)
	{
		_secsToMidnight = fsbutils::DateFns::getSecsToMidnight();
		FSB_LOG(" MA: secs to midnight " << _secsToMidnight);
	}

	double SpiderMAPolicy::calculate(int length,
									int frequency,
									time_t secs)
	{
		_dataPoint0 = TraderEnvSingleton::instance()->_basket.basis();
		if(_dataPoint0 == 0.0) return 0.0;
		if(!_previous) {
			FSB_LOG(" Start of the day will set previous to " << _dataPoint0);
			_previous = _dataPoint0;
		}
		if(_timestamp == 0) {
			FSB_LOG("Beginning of the day ?, timestamp is " << _timestamp);
			_timestamp = secs;
		}
		
		//time_t basisTS = TraderEnvSingleton::instance()->_basket.timestamp();
		double timeStep = (double)(secs - _timestamp);
		_ma = ( (timeStep/(double)length)*_dataPoint0) + ((1.0 - (timeStep/(double)length)) * _previous);
		//std::cout << secs << " " << _timestamp << " " << timeStep << " " << _ma << " " << _previous << " " << _dataPoint0 << std::endl;
		//std::cout << (timeStep/1000) << " " << (1.0 - (double)(timeStep/1000)) << std::endl;
		_previous= _ma;		
		if(timeStep > length) { 
			FSB_LOG(" Hmm... basis has not been calculated for a while?");
			_ma = _dataPoint0;
		}
		_timestamp = secs;
		return _ma;
	}
	void SpiderMAPolicy::print()
	{
		FSB_LOG(" DPO " << _dataPoint0
			<< " MA " << _ma
			<< " Prev " << _previous
			<< " TS " << _timestamp);
	}
//***********************************************************************
	MovingAverage::MovingAverage(const std::string& name,
		const std::string& maPolicyname,
		const std::string& mapricePolicyname,
		const std::string& matypePolicyname)
			:_name(name),
			_startMA(false),
			_numSecsToStartMA(0)
	{
		//Get that factory class working ( that takes base and manufactured obj
		if( maPolicyname == "MABySpread")
			_maPolicy = new MASpreadPolicy(matypePolicyname,mapricePolicyname);
		else if( maPolicyname == "Spider")
			_maPolicy = new SpiderMAPolicy(name,mapricePolicyname);
		else
			_maPolicy = new InstrumentMAPolicy(matypePolicyname,mapricePolicyname);
	}

	double MovingAverage::calculate(time_t secs)
	{
		if(secs && (_numSecsToStartMA < secs && _numSecsToStopMA > secs))
			return _maPolicy->calculate(_length,_frequency,secs);
		if( _numSecsToStopMA < secs)
			return _maPolicy->ma();
		return 0.0;
	}

	void MovingAverage::print()
	{
		FSB_LOG("name " << _name
			<< " _frequency " << _frequency
			<< " _length " << _length
			);
		_maPolicy->print();
		
	}

	void MovingAverage::notify(const std::string& name,const std::string& value)
	{
		if( name == _name) {
			std::istringstream dstr(value);
			double sval;
			dstr >> sval;
			FSB_LOG("Changing Start Value from " << startValue() << " to " << sval);
			startValue(sval);
		} else if( (_name + "_LENGTH") == name)
		{
			std::istringstream istr(value);
			int sval;
			istr >> sval;
			FSB_LOG("Changing " << _name << "_LENGTH Value from " << length() << " to " << sval);
			length(sval);
		} else if( (_name + "FREQ") == name)
		{
			std::istringstream istr(value);
			int sval;
			istr >> sval;
			FSB_LOG("Changing " << _name << "_FREQ Value from " << length() << " to " << sval);
			frequency(sval);
		}

	}

} //namespace
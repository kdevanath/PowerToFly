#ifndef __MOVING_AVG_H__
#define __MOVING_AVG_H__

#include "TraderPolicyExports.h"
#include "MovingAveragePolicy.h"
#include <Data/Pricers.h>
#include <Observer/Observer.h>

namespace fsb {

	class MASpreadPolicy : public MAPolicy
	{
	public:
		MASpreadPolicy(const std::string& name,
			const std::string& pricePolicyname="MidPrice");
		double calculate(int length,int frequency,time_t secs=0);
		 double datapoint0()  { return _dp0;}
		 double ma()  { return _ma;}
		double previous() const { return _previous;}
		void previous( const double& p) { _previous = p;}
		void print();
	private:
		double numerator();
		double denominator();
		Instrument::VectorOfInstrPtrs _base;
		Instrument::VectorOfInstrPtrs _hedge;
		double _previous;
		double _ma;
		double _numerator;
		double _denominator;
		double _dp0;
		PriceTypePolicy* _priceType;
		//DP<DPRatio> _datapointPolicy;
		MATypePolicy* _maPolicy;
		//DPTypePolicy* _datapointPolicy;
	};

	class InstrumentMAPolicy : public MAPolicy
	{
	public:
		InstrumentMAPolicy(const std::string& = "MARatio",
			const std::string& pricePolicyname="MidPrice");
		double calculate(int length,int frequency,time_t secs=0);
		double datapoint0();
		double ma(const Instrument&);
		double maByPolicy(const Instrument&);
		double ma(); //ratio
		void previous( const double& p) { ; }
		double previous() const { return 0.0;} //_baseMAs[0]._ma;}
		void print();
	private:
		void calculate(int, int , Instrument::InstrPtrsIterator,MAInfo&);
		MAInfo::Vect _MAs;
		Instrument::VectorOfInstrPtrs _instrs;
		DP<DPSingle> _datapointPolicy;
		MATypePolicy* _maPolicy;
	};

	class SpiderMAPolicy : public MAPolicy
	{
	public:
		SpiderMAPolicy(const std::string& = "MARatio",
			const std::string& pricePolicyname="MidPrice");
		double calculate(int length,int frequency,time_t secs = 0);
		double datapoint0()  { return _dataPoint0;}
		//double ma(const Instrument&);
		double maByPolicy(const Instrument&) { return 0.0;}
		double ma()  { return _ma;}
		void previous( const double& p) { _previous =p; }
		double previous() const { return _previous;} 
		void print();
	private:
		double _previous;
		double _dataPoint0;
		double _ma;
		time_t _secsToMidnight;
		time_t _timestamp;
	};

	
	class TRADERPOLICIES_API MovingAverage : public DisplayObserver
	{
	public:

		MovingAverage(const std::string& name,
			const std::string& maPolicyname="BySpread",
			const std::string& mapricePolicyname="MidPrice",
			const std::string& matypePolicyname="MARatio");
	
		~MovingAverage()
		{
		}

		virtual void notifyStop()
		{
		}

		virtual void notifyStart()
		{
		}

		virtual void notify(const std::string& name,const std::string& value);

		double result() const { return ma();}
		double result(const Instrument& i) { return _maPolicy->ma(i);}

		double startValue()  { 
			ACE_READ_GUARD_RETURN(ACE_RW_Thread_Mutex,mon,_MAmutex,0.0);
			return _maPolicy->previous(); 
		}
		void   startValue( const double& s) { 			
			ACE_WRITE_GUARD(ACE_RW_Thread_Mutex,mon,_MAmutex);
			_maPolicy->previous(s);
		}

		int frequency()  { 
			ACE_READ_GUARD_RETURN(ACE_RW_Thread_Mutex,mon,_MAmutex,0);
			return _frequency; 
		}
		void frequency( const int& s) { 
			ACE_WRITE_GUARD(ACE_RW_Thread_Mutex,mon,_MAmutex);
			_frequency = s;
		}

		int length() 
		{  
			ACE_READ_GUARD_RETURN(ACE_RW_Thread_Mutex,mon,_MAmutex,0);
			return _length; 
		}
		void   length( const int& s) 
		{
			ACE_WRITE_GUARD(ACE_RW_Thread_Mutex,mon,_MAmutex);
			_length = s;
		}

		void setTime(const std::string& matime="09:15",
			const std::string& endMa = "16:15") {
			int hr=0,min=0;
			sscanf(matime.c_str(),"%d:%d",&hr,&min);
			time_t ltime;
			time(&ltime);
			struct tm timenow;
			localtime_s(&timenow,&ltime);
			timenow.tm_hour = hr; //actually in est, but needs to be in CST
			timenow.tm_sec = 0;
			timenow.tm_min = min;
			_numSecsToStartMA = mktime(&timenow);
			sscanf(endMa.c_str(),"%d:%d",&hr,&min);
			timenow.tm_hour = hr;
			timenow.tm_sec = 0;
			timenow.tm_min = min;
			_numSecsToStopMA = mktime(&timenow);
			FSB_LOG("Moving average calculation will start from " << _numSecsToStartMA);
			FSB_LOG("Moving average calculation will end at " << _numSecsToStopMA);
		}

		double calculate(time_t secs = 0);
		double datapoint0() const { return _maPolicy->datapoint0();}
		double ma() const { return _maPolicy->ma();}

		void name(const std::string& s) { _name = s;}
		std::string name() { return _name;}

		void  print(); 

	private:
		std::string _name;
		int _frequency;
		int _length;
		time_t _numSecsToStartMA;
		time_t _numSecsToStopMA;
		bool _startMA;
		MAPolicy* _maPolicy;
		ACE_RW_Thread_Mutex _MAmutex;

	};
};
#endif
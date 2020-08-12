#ifndef __SIZE_POLICY_H_
#define __SIZE_POLICY_H_

#include "Policy.h"
#include "PolicyIds.h"
#include <Data/Instruments.h>
#include <Data/OrderReplyInfo.h>

namespace fsb {

	class SizePolicy : public Policy
	{
	public:
		
		virtual void calcQuantity(int& buy,int& sell) = 0;
	};

	class TakeHedgeSizePolicy
	{
	public:
		void calcQuantity(const Instrument& instr,int& buy,int& sell);
	};


	class  GenericSizePolicy
	{
	public:
		void calcQuantity(const Instrument& instr, int& buy,int& sell);
	};

	class RandomizeSizePolicy
	{
		public:
			void calcQuantity(int& buy,int& sell);	
	};

	class TakeLiqSizePolicy
	{
	public:
		void calcQuantity(const Instrument& instr,int& buy,int& sell);
	protected:
		~TakeLiqSizePolicy(){}

	};

	class  WorkLiqSizePolicy
	{
	public:
		WorkLiqSizePolicy();		
		void calcQuantity(const Instrument& instr,int& buy,int& sell);

	protected:
		~WorkLiqSizePolicy() { }

	private:
		RandomizeSizePolicy _randomize;
	};

	class DeltaCloseUsingLevRatio
	{
	public:
		DeltaCloseUsingLevRatio() {}		
		void calcQuantity(const Instrument&,const OrderReplyInfo& reply, int& buy,int& sell);
	protected:
		~DeltaCloseUsingLevRatio() {}
	};

	class  DeltaCloseSizePolicy
	{
	public:
		DeltaCloseSizePolicy() {}		

		void calcQuantity(const OrderReplyInfo&,int& buy,int& sell);
	protected:

		~DeltaCloseSizePolicy() {}

	};

	class  HedgeSizePolicy
	{
	public:
		HedgeSizePolicy() {}

		void calcQuantity(const Instrument& instr,int& buy,int& sell);

	protected:
		virtual ~HedgeSizePolicy() {}		
	};

	class  EmptySizePolicy
	{
	public:
		EmptySizePolicy() {}

		void calcQuantity(const Instrument&,int& buy,int& sell){ return;}

	protected:
		 ~EmptySizePolicy() {}		
	};

	class MultipleHedgesSizePolicy
	{
		public:
			MultipleHedgesSizePolicy()
				:_residue(0.0){ }

			void calcQuantity(const Instrument&,const int&,int& buy,int& sell);
			void calcResidue(const Instrument& hedge, const int& , bool isBuy, int&, int&);
			void resetResidue() { _residue = 0.0;}
	protected:
		~MultipleHedgesSizePolicy() { }
		double _residue;
	};

	class MultipleHedgesDeltaCloseSizePolicy
	{
		public:
			MultipleHedgesDeltaCloseSizePolicy() { }

			void calcQuantity(const Instrument& hedge, int& buy,int& sell);
	protected:
		~MultipleHedgesDeltaCloseSizePolicy() { }
	};

	class HedgeByBaseFills
	{
	public:
		void calcQuantity(const OrderReplyInfo& reply, int& buy, int& sell);
	protected:
		~HedgeByBaseFills() { }
	};

	class InverseHedgeByBaseFills2
	{
	public:
		void calcQuantity(const OrderReplyInfo& reply, int& buy, int& sell)
		{
			buy = sell = abs((int)(reply._qtyFilled/2.0));
		}
	protected:
		~InverseHedgeByBaseFills2() { }
	};

	class InverseDeltaCloseSizePolicy
	{
	public:
		void calcQuantity(const OrderReplyInfo&,int& buy,int& sell);
	protected:
		~InverseDeltaCloseSizePolicy() { }
	};

	class ETFWorkLiqSizePolicy
	{
	public:
		ETFWorkLiqSizePolicy() { }
		void calcQuantity(const Instrument&,int& buy,int& sell);
	protected:
		~ETFWorkLiqSizePolicy() { }
	};

	class MMKRWorkSizePolicy
	{
		public:
			void calcQuantity(const Instrument& instr, int& buy, int& sell);
		protected:
			~MMKRWorkSizePolicy() { }
	};

	class SpiderBase1SizePolicy
	{
		public:
			void calcQuantity(const Instrument& instr, int& buy, int& sell);
		protected:
			~SpiderBase1SizePolicy() { }
	};
	class SpiderBaseNSizePolicy
	{
		public:
			void calcQuantity(const Instrument& instr, int& buy, int& sell);
		protected:
			~SpiderBaseNSizePolicy() { }
	};
	class SpiderHedgeSizePolicy
	{
		public:
			SpiderHedgeSizePolicy();
			void calcQuantity(const Instrument&,const OrderReplyInfo& reply,int& buy,int& sell);
		protected:
			~SpiderHedgeSizePolicy() { }

	};

	class ReachedTargetShares{};

	class NoMaxPosPolicy
	{
		public:
			int calcMaxPosition(const Instrument&,int net=0){ return 0; }
	};
	class DefaultMaxPositionPolicy
	{
	public:
		int calcMaxPosition(const Instrument&,int net=0);

	};
	class MaxPositionBaseNPolicy
	{
	public:
		int calcMaxPosition(const Instrument&,int net=0);

	};

	class ETFPaperTraderHedgeSizePolicy
	{
	public:
		ETFPaperTraderHedgeSizePolicy() { }
			void calcQuantity(const Instrument&,const OrderReplyInfo& reply,int& buy,int& sell);
		protected:
			~ETFPaperTraderHedgeSizePolicy() { }
	};

	class ProspectSizePolicy
	{
	public:
		ProspectSizePolicy() { }
			void calcQuantity(const Instrument& instr, int& buy, int& sell);
		protected:
			~ProspectSizePolicy() { }
	};

	class WeightsSizePolicy
	{
	public:
		WeightsSizePolicy() { }
			void calcQuantity(const Instrument& instr, int& buy, int& sell);
		protected:
			~WeightsSizePolicy() { }
	};

	class MaxPosUsingCloseTarget
	{
	public:
		MaxPosUsingCloseTarget() { }
		int calcMaxPosition(const Instrument&,int net);
	protected:
		~MaxPosUsingCloseTarget() { }
	};
}
#endif
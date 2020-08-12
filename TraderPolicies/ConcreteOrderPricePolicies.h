#ifndef  _CONCRETE_ORDER_PRICE_POLICIES_H
#define _CONCRETE_ORDER_PRICE_POLICIES_H

#include "OrderPricePolicy.h"

//Price Policies
class JoinPricePolicy : public OrderPricePolicy {
public:
  JoinPricePolicy() : OrderPricePolicy() {}
  virtual double calcPrice(Instrument* i,double theoPrice, bool buy){ return buy ? i->bid() : i->ask();}
};

#endif
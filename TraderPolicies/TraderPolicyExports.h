#ifndef __TRADER_POLICY_EXPORTS_H__
#define __TRADER_POLICY_EXPORTS_H__

#ifdef TRADERPOLICIES_EXPORTS
#define TRADERPOLICIES_API __declspec(dllexport)
#define EXPIMP_TRADERPOLICY_TEMPLATE
#else
#define TRADERPOLICIES_API __declspec(dllimport)
#define EXPIMP_TRADERPOLICY_TEMPLATE extern
#endif

#endif
#ifdef FACTORYPOLICIES_EXPORTS
#define FACTORYPOLICIES_API __declspec(dllexport)
#define EXPIMP_FACTORYPOLICY_TEMPLATE
#else
#define FACTORYPOLICIES_API __declspec(dllimport)
#define EXPIMP_FACTORYPOLICY_TEMPLATE extern
#endif
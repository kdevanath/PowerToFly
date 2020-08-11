#ifndef _YAMMER_BYTE_ORDER_H_
#define _YAMMER_BYTE_ORDER_H_

#include <netinet/in.h>

// Host-to-network/network-to-host byte order routines
// These just wrap the standard (usually optimized) routines.  Aside from 
// providing 64 bit versions (for doubles), they mainly save you from needing
// to make lots of casts and from worrying about the size of the type.
// Assumptions are based on an ILP32 architecture: doubles are 64 bits, others,
// besides char, bool, and short, are 32 bits (for example, signed and unsigned
// int and long, float, and enum will default to 32 bits).  Needs to change
// when moving to LP64 architectures.

namespace Yammer {

#ifdef i386  // little-endian (intel-based Linux)

  const bool isBigEndian = false;

  template <typename T>
  void ntoh(T &val)
  {
    uint32_t *i = (uint32_t*)&val;
    *i = ntohl(*i);
  }

  template <typename T>
  void hton(T &val)
  {
    uint32_t *i = (uint32_t*)&val;
    *i = htonl(*i);
  }

  // specialize for double
  template <>
  inline void ntoh(double &val)
  {
    uint32_t *i = (uint32_t*)&val, temp = i[0];
    i[0] = ntohl(i[1]);
    i[1] = ntohl(temp);
  }

  template <>
  inline void hton(double &val)
  {
    uint32_t *i = (uint32_t*)&val, temp = i[0];
    i[0] = htonl(i[1]);
    i[1] = htonl(temp);
  }

  // specialize for unsigned short
  template <>
  inline void ntoh(unsigned short &val)
  {
    uint16_t *i = (uint16_t*)&val;
    *i = ntohs(*i);
  }

  template <>
  inline void hton(unsigned short &val)
  {
    uint16_t *i = (uint16_t*)&val;
    *i = htons(*i);
  }

  // specialize for short
  template <>
  inline void ntoh(short &val)
  {
    uint16_t *i = (uint16_t*)&val;
    *i = ntohs(*i);
  }

  template <>
  inline void hton(short &val)
  {
    uint16_t *i = (uint16_t*)&val;
    *i = htons(*i);
  }

  // specialize for unsigned char 
  template <> inline void hton(unsigned char &val) {}
  template <> inline void ntoh(unsigned char &val) {}

  // specialize for char 
  template <> inline void hton(char &val) {}
  template <> inline void ntoh(char &val) {}

  // specialize for bool 
  template <> inline void hton(bool &val) {}
  template <> inline void ntoh(bool &val) {}

  // Partially specialize for const types.  This will prevent unintentional
  // const casts.  Instantiation will force a compile-time error.
  template <typename T> void ntoh(const T &val) { val = val; }
  template <typename T> void hton(const T &val) { val = val; }

#elif defined sparc  // big-endian 

  const bool isBigEndian = true;

  template <typename T> void ntoh(T &val) {}
  template <typename T> void hton(T &val) {}

#else 
  #error unknown platform
#endif

  // byte amounts - byte, word, double word - used by the stream operators when
  // copying a bool, int, or double
  const int byte = 1, word = 4, dword = 8;

}

#endif


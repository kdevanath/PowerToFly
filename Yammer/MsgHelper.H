#ifndef _YAMMER_MSG_HELPER_H_
#define _YAMMER_MSG_HELPER_H_

#include <string>
#include <vector> 

namespace Yammer {

  using std::string;
  using std::vector;

  // Helper functions to write primitive types and std string into a
  // vector<char> buffer and perform network byte order conversion.
  
  vector<char> &operator<<(vector<char> &buffer, bool val);
  vector<char> &operator<<(vector<char> &buffer, int val);
  vector<char> &operator<<(vector<char> &buffer, size_t val);
  vector<char> &operator<<(vector<char> &buffer, double val);
  vector<char> &operator<<(vector<char> &buffer, const string &val);

}

#endif


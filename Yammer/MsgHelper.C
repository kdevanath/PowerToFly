#include <MsgHelper.H> 
#include <ByteOrder.H> 

namespace Yammer {

  vector<char> &operator<<(vector<char> &buffer, bool val)
  {
    buffer.push_back(val);
    return buffer;
  }

  vector<char> &operator<<(vector<char> &buffer, int val)
  {
    hton(val);
    char *p = reinterpret_cast<char*>(&val);
    buffer.insert(buffer.end(), p, p + word);  // append
    return buffer;
  }

  vector<char> &operator<<(vector<char> &buffer, size_t val)
  {
    hton(val);
    char *p = reinterpret_cast<char*>(&val);
    buffer.insert(buffer.end(), p, p + word);  // append
    return buffer;
  }

  vector<char> &operator<<(vector<char> &buffer, double val)
  {
    hton(val);
    char *p = reinterpret_cast<char*>(&val);
    buffer.insert(buffer.end(), p, p + dword);  // append
    return buffer;
  }

  vector<char> &operator<<(vector<char> &buffer, const std::string &val)
  {
    size_t size = val.size();
    const char *p = val.c_str();
    buffer << size;  // append size
    buffer.insert(buffer.end(), p, p + size);  // and string
    return buffer;
  }

}


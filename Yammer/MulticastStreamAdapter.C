#include <MulticastStreamAdapter.H> 

namespace Yammer {

  MulticastStreamAdapter::MulticastStreamAdapter() {}

  MulticastStreamAdapter::MulticastStreamAdapter(const ACE_SOCK_Dgram_Mcast
    &socket, const ACE_INET_Addr &addr) : socket_(socket),
    addr_(addr)
  {
    u_char ttl = 31;
    if (socket.ACE_SOCK::set_option(IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) == -1)
      std::cout << "Unable to set TTL option on socket!" << std::endl;

  }

  int MulticastStreamAdapter::read(void *buffer, size_t len)
    throw (NetworkError)
  {
    int ret = socket_.recv(buffer, len, addr_);
    if (ret == -1)
      throw SocketError();

    return ret;
  }

  int MulticastStreamAdapter::write(const void *buffer, size_t len)
    throw (NetworkError)
  {
    int ret = socket_.send(buffer, len);
    if (ret == -1)
      throw SocketError();

    return ret;
  }

  int MulticastStreamAdapter::readv(const iovec *vec, int len)
    throw (NetworkError)
  {
    // perform const cast due to ACE's non-standard interface
    iovec *v = const_cast<iovec*>(vec);
    int ret = socket_.recv(v, len, addr_);
    if (ret == -1)
      throw SocketError();

    return ret;
  }

  int MulticastStreamAdapter::writev(const iovec *vec, int len)
    throw (NetworkError)
  {
    int ret = socket_.send(vec, len);
    if (ret == -1)
      throw SocketError();

    return ret;
  }

  void MulticastStreamAdapter::close()
  {
    socket_.close();
  }

}


#include <ServiceAccessPoint.H> 
#include <stdlib.h> 
#include <memory> 
#include <string> 
#include <Task.H> 
#include <Service.H> 
#include <ServiceFactory.H> 
#include <TaskStreams.H> 
#include <BufferedStream.H> 
#include <TCPStreamAdapter.H> 
#include <TCPLogHelper.H> 
#include <NetworkExceptions.H> 
#include <Logger.H> 

namespace Yammer {

  using std::auto_ptr;
  using std::string;

  ServiceAccessPoint::ServiceAccessPoint(int port)
    throw (ServiceAccessPoint::BindFailed)
  {
    // enable socket address reuse, then bind
    if (acceptor_.open(ACE_INET_Addr(port), 1) == -1)  
      throw BindFailed();

    ACE_Thread::spawn(run, this, THR_BOUND|THR_DETACHED);
  }

  void *ServiceAccessPoint::run(void *This)
  {
    ServiceAccessPoint *that = reinterpret_cast<ServiceAccessPoint*>(This);
    auto_ptr<ServiceAccessPoint> deleter(that);
    ServiceFactory *serviceFactory = ServiceFactorySingleton::instance();
    ACE_SOCK_Acceptor acceptor = that->socket();
    ACE_SOCK_Stream peer;

    for (;;) {
      if (acceptor.accept(peer) == -1) {  // blocks
        LOG("accept failed");
        exit(0);
      }

      auto_ptr<Stream> tcpStream(new TCPStreamAdapter(peer));
      BufferedStream stream(tcpStream.get());

      string peerInfo = tcpPeerInfo(peer);
      LOG("A client is connecting from " << peerInfo);

      try {
        auto_ptr<Task> task;
        stream >> task;

        Service *service = serviceFactory->createService(task.get());
        service->spawn(tcpStream.release(), task.release());

      } catch (Task &exception) {  // UnknownTask or UnknownServiceRequest 
        LOG(exception.name() << ": client=" << peerInfo);
        try { stream << exception; } catch (NetworkError&) {}
        LOG("Disconnecting from client " << peerInfo);
        stream.close();
      } catch (NetworkError &exception) {
        LOG(exception.what() << ": client=" << peerInfo);
        LOG("Disconnecting from client " << peerInfo);
        stream.close();
      }
    }

    return 0;
  }

}



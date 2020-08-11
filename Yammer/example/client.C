#include <memory>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <Task.H>
#include <NetworkExceptions.H>
#include <TaskStreams.H>
#include <BufferedStream.H>
#include <TCPStreamAdapter.H>
#include <ExampleTypeIds.H>
#include <ExampleServiceRequest.H>
#include <ServiceRequestAck.H>
#include <ExampleRequest.H>
#include <ExampleReply.H>
#include <DisconnectionRequest.H>
#include <UnknownTask.H>

using namespace std;
using namespace Yammer;
using namespace ReferenceImplementation;

main(int argc, char *argv[])
{
  if (argc < 2) {
    cerr << "usage: " << argv[0]
         << " <ip:port>" << endl;
    return 1;
  }

  ACE_SOCK_Connector connector;
  ACE_SOCK_Stream peer;
  ACE_INET_Addr addr(argv[1]);
  if (connector.connect(peer, addr) == -1) {
    perror("connect");
    return 1;
  }

  BufferedStream stream(new TCPStreamAdapter(peer));

  try {
    ExampleServiceRequest connect;
    stream << connect;
    cout << "sent a " << connect.name() << endl;

    auto_ptr<Task> response;
    stream >> response;
    cout << "rcvd a " << response->name() << endl;
    if (*response != ServiceRequestAck())
      return 1;
      
    ExampleRequest request;
    request.setString("test");
    request.setInt(42);
    request.setDouble(2.14);
    stream << request;
    cout << "sent a " << request.name() 
         << "\n\t string: " << request.getString()
         << "\n\t int: " << request.getInt()
         << "\n\t double: " << request.getDouble() << endl;

    stream >> response;
    cout << "rcvd a " << response->name() << endl;
    if (*response != ExampleReply())
      return 1;

    ExampleReply &reply = dynamic_cast<ExampleReply&>(*response);
    cout << "\t string: " << reply.getString()
         << "\n\t int: " << reply.getInt()
         << "\n\t double: " << reply.getDouble() << endl;

    DisconnectionRequest disconnect;
    stream << disconnect;
    cout << "sent a " << disconnect.name() << endl;
    stream.close();

  } catch (NetworkError &ex) { 
    cout << "rcvd a network error: " << ex.what() << endl;
  } catch (UnknownTask) { 
    cout << "rcvd an unknown task" << endl;
  }

  return 0;
}


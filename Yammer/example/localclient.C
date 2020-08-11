#include <memory>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <Task.H>
#include <NetworkExceptions.H>
#include <TaskStreams.H>
#include <BufferedStream.H>
#include <TCPStreamAdapter.H>
#include <ExampleTypeIds.H>
#include <ExampleLocalServiceRequest.H>
#include <ServiceRequestAck.H>
#include <ExampleRequest.H>
#include <ExampleReply.H>
#include <DisconnectionRequest.H>
#include <UnknownTask.H>
#include <IPCQueue.H>
#include <IPCQueueStreamAdapter.H>

using namespace std;
using namespace Yammer;
using namespace ReferenceImplementation;

main(int argc, char *argv[])
{
  if (argc < 6) {
    cerr << "usage: " << argv[0] << " <port> <request queue name> "
      "<request queue size> <reply queue name> <reply queue size>" << endl;
    return 1;
  }

  ACE_SOCK_Connector connector;
  ACE_SOCK_Stream peer;
  string ip("localhost:");
  ip += argv[1];
  ACE_INET_Addr addr(ip.c_str());
  if (connector.connect(peer, addr) == -1) {
    perror("connect");
    return 1;
  }

  // We connect to the server through the ServiceAccessPoint to initiate the
  // connection and create the queues; all subsequent communication is through
  // the local interprocess queues.
  BufferedStream stream(new TCPStreamAdapter(peer));

  try {
    // Send the request to the server to start the service and create
    // request and reply queues with these names and sizes.
    ExampleLocalServiceRequest connect(argv[2], atoi(argv[3]), argv[4],
      atoi(argv[5]));
    stream << connect;
    cout << "sent a " << connect.name() << endl;

    auto_ptr<Task> response;
    stream >> response;
    cout << "rcvd a " << response->name() << endl;
    if (*response != ServiceRequestAck())
      return 1;

    // We received the acknowledgment.  We now know the server has created
    // the queues.

    // Connect to the queues; use false as the second parameter as they are 
    // already created.
    IPCQueue requestQ(argv[2], false);
    IPCQueue replyQ(argv[4], false);

    // wrap them with a stream adapter
    IPCQueueStreamAdapter *requestAdapter =
      new IPCQueueStreamAdapter(&requestQ);
    IPCQueueStreamAdapter *replyAdapter = new IPCQueueStreamAdapter(&replyQ);

    // Use a BufferedStream as a decorator class (multiple decorator classes
    // can be used together, for instance, to compress or encrypt the data).
    BufferedStream requestStream(requestAdapter);
    BufferedStream replyStream(replyAdapter);
      
    ExampleRequest request;
    request.setString("test");
    request.setInt(42);
    request.setDouble(2.14);
    requestStream << request;
    cout << "sent a " << request.name() 
         << "\n\t string: " << request.getString()
         << "\n\t int: " << request.getInt()
         << "\n\t double: " << request.getDouble() << endl;

    replyStream >> response;
    cout << "rcvd a " << response->name() << endl;
    if (*response != ExampleReply())
      return 1;

    ExampleReply &reply = dynamic_cast<ExampleReply&>(*response);
    cout << "\t string: " << reply.getString()
         << "\n\t int: " << reply.getInt()
         << "\n\t double: " << reply.getDouble() << endl;

    DisconnectionRequest disconnect;
    requestStream << disconnect;
    cout << "sent a " << disconnect.name() << endl;
    stream.close();

  } catch (NetworkError &ex) { 
    cout << "rcvd a network error: " << ex.what() << endl;
  } catch (UnknownTask) { 
    cout << "rcvd an unknown task" << endl;
  }

  return 0;
}


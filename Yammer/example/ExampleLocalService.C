#include <ExampleLocalService.H> 
#include <memory> 
#include <typeinfo> 
#include <ace/Thread.h> 
#include <Task.H> 
#include <BufferedStream.H>
#include <TaskStreams.H> 
#include <ExampleLocalServiceRequest.H> 
#include <ServiceRequestAck.H> 
#include <DisconnectionRequest.H> 
#include <ProtocolError.H> 
#include <UnknownTask.H> 
#include <NetworkExceptions.H> 
#include <TaskDispatcher.H> 
#include <ServiceFactory.H> 
#include <TaskSync.H> 
#include <ExampleRequest.H> 
#include <TCPLogHelper.H> 
#include <Logger.H> 
#include <IPCQueue.H> 
#include <IPCQueueStreamAdapter.H> 

namespace ReferenceImplementation {

  using std::auto_ptr;
  using Yammer::Task;
  using Yammer::BufferedStream;
  using Yammer::DisconnectionRequest;
  using Yammer::ServiceRequestAck;
  using Yammer::ProtocolError;
  using Yammer::UnknownTask;
  using Yammer::NetworkError;
  using Yammer::TaskSync;
  using Yammer::TaskDispatcher;
  using Yammer::TaskDispatcherSingleton;
  using Yammer::IPCQueue;
  using Yammer::IPCQueueStreamAdapter;

  ExampleLocalService::ExampleLocalService() : stream_(0) {}

  ExampleLocalService::~ExampleLocalService()
  {
    if (stream_) {
      stream_->close();
      delete stream_;
    }
  }

  void ExampleLocalService::spawn(Stream *stream, Task *startTask) throw()
  {
    // Because we are created by a factory, we are guaranteed to be allocated
    // off the heap.  If necessary, deleter will cleanup after ourselves.
    auto_ptr<ExampleLocalService> deleter(this);  // "delete this" 
    // transfers ownership of task and stream 
    auto_ptr<Task> startTaskDeleter(startTask);
    stream_ = stream;

    string peer = tcpPeerInfo(stream);

    ExampleLocalServiceRequest *request =
      dynamic_cast<ExampleLocalServiceRequest*>(startTask);

    if (request == 0) {  // shouldn't happen
      LOG("ERROR: ExampleLocalServiceRequest expected: received_task=" <<
        startTask->name() << " client=" << peer);
      try { *stream_ << ProtocolError("ExampleLocalServiceRequest expected"); }
      catch (NetworkError&) {}
      return;
    }

    // at this point, you would save any values of interest from startTask to
    // this Service
    requestQueueName_ = request->getRequestQueueName();
    requestQueueSize_ = request->getRequestQueueSize();
    replyQueueName_ = request->getReplyQueueName();
    replyQueueSize_ = request->getReplyQueueSize();

    LOG("ExampleLocalService started for client on " << peer);

    deleter.release();  // safe to release 
    ACE_Thread::spawn(run, this, THR_BOUND|THR_DETACHED);
  }

  void *ExampleLocalService::run(void *This)
  {
    ExampleLocalService *that = reinterpret_cast<ExampleLocalService*>(This);
    auto_ptr<ExampleLocalService> deleter(that);
    BufferedStream stream(that->stream());
    TaskDispatcher *threadPool = TaskDispatcherSingleton::instance();
    string peer = tcpPeerInfo(that->stream());

    // create local interprocess queues
    IPCQueue requestQ(that->getRequestQueueName(), true, 
      that->getRequestQueueSize());
    IPCQueue replyQ(that->getReplyQueueName(), true, 
      that->getReplyQueueSize());

    // wrap them with a stream adapter
    IPCQueueStreamAdapter *requestAdapter =
      new IPCQueueStreamAdapter(&requestQ);
    IPCQueueStreamAdapter *replyAdapter = new IPCQueueStreamAdapter(&replyQ);

    // Use a BufferedStream as a decorator class (multiple decorator classes
    // can be used together, for instance, to compress or encrypt the data).
    BufferedStream requestStream(requestAdapter);
    BufferedStream replyStream(replyAdapter);

    try {
      ServiceRequestAck ack;
      stream << ack;

      // IPCQueues will be used from now on

      DisconnectionRequest disconnect;  // sentinal
      TaskSync syncObj;

      for (;;) {
        auto_ptr<Task> task;
        requestStream >> task;

        if (*task == disconnect) break;

        // at this point, you can perform a dynamic_cast on task and
        // call methods specific to this Task/Service 

        // request-only
        if (task->needsReply() == false) {
          threadPool->dispatch(task.release());
          continue;
        }

        // request-reply
        task->sync(&syncObj);
        threadPool->dispatch(task.release());
        auto_ptr<Task> reply(syncObj.wait());

        replyStream << reply;
      }

    } catch (UnknownTask &error) {
      LOG(error.name() << ": client=" << peer);
      try { replyStream << error; } catch (NetworkError&) {}
    } catch (NetworkError &ex) { 
      LOG(ex.what() << ": client=" << peer);
    }

    LOG("Disconnecting from client on " << peer);
    return 0;

  }

  Yammer::ServiceRegistrar<ExampleLocalService, ExampleLocalServiceRequest>
    ExampleLocalServicePrototype;
}


#include <ExampleService.H> 
#include <memory>
#include <string>
#include <typeinfo> 
#include <ace/Thread.h> 
#include <Task.H> 
#include <TaskStreams.H> 
#include <BufferedStream.H>
#include <ExampleServiceRequest.H> 
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

namespace ReferenceImplementation {

  using std::auto_ptr;
  using std::string;
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

  ExampleService::ExampleService() : stream_(0) {}

  ExampleService::~ExampleService()
  {
    if (stream_) {
      stream_->close();
      delete stream_;
    }
  }

  void ExampleService::spawn(Stream *stream, Task *startTask) throw()
  {
    // Because we are created by a factory, we are guaranteed to be allocated
    // off the heap.  If necessary, deleter will cleanup after ourselves.
    auto_ptr<ExampleService> deleter(this);  // "delete this" 
    // transfers ownership of task and stream 
    auto_ptr<Task> startTaskDeleter(startTask);
    stream_ = stream;

    string peer = tcpPeerInfo(stream);

    ExampleServiceRequest *request =
      dynamic_cast<ExampleServiceRequest*>(startTask);

    if (request == 0) {  // shouldn't happen
      LOG("ERROR: ExampleServiceRequest expected: received_task=" <<
        startTask->name() << " client=" << peer);
      try { *stream_ << ProtocolError("ExampleServiceRequest expected"); }
      catch (NetworkError&) {}
      return;
    }

    // at this point, you would save any values of interest from startTask to
    // this Service (we have none)

    LOG("ExampleService started for client on " << peer);

    deleter.release();  // safe to release 
    ACE_Thread::spawn(run, this, THR_BOUND|THR_DETACHED);
  }

  void *ExampleService::run(void *This)
  {
    ExampleService *that = reinterpret_cast<ExampleService*>(This);
    auto_ptr<ExampleService> deleter(that);
    BufferedStream stream(that->stream());
    TaskDispatcher *threadPool = TaskDispatcherSingleton::instance();
    string peer = tcpPeerInfo(that->stream());

    try {
      ServiceRequestAck ack;
      stream << ack;

      DisconnectionRequest disconnect;  // sentinal
      TaskSync syncObj;

      for (;;) {
        auto_ptr<Task> task;
        stream >> task;

        if (*task == disconnect) break;

        // at this point, you may want to perform a dynamic_cast on task and
        // call methods that may be specific to this Service 

        // request-only
        if (task->needsReply() == false) {
          threadPool->dispatch(task.release());
          continue;
        }

        // request-reply
        task->sync(&syncObj);
        threadPool->dispatch(task.release());
        auto_ptr<Task> reply(syncObj.wait());

        stream << reply;
      }

    } catch (UnknownTask &error) {
      LOG(error.name() << ": client=" << peer);
      try { stream << error; } catch (NetworkError&) {}
    } catch (NetworkError &ex) { 
      LOG(ex.what() << ": client=" << peer);
    }

    LOG("Disconnecting from client on " << peer);
    return 0;

  }

  Yammer::ServiceRegistrar<ExampleService, ExampleServiceRequest>
    ExampleServicePrototype;
}


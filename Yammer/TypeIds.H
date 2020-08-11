#ifndef _YAMMER_TYPE_IDS_H_
#define _YAMMER_TYPE_IDS_H_

namespace Yammer {

  // This enum is used to create a monotonic sequence of integral IDs.
  // The IDs will be used by the Factory classes to create new types of Tasks
  // and Services.  IDs 0 - 99 are reserved for the use by the framework; this
  // leaves 4 billion - 100 values available for the application.  Application
  // builders should create a similar file for application-level IDs using
  // FirstAvailableId as the starting value.

  enum {
    // protocol related
    ProtocolErrorId,
    UnknownServiceRequestId,
    UnknownTaskId,

    // server related
    InternalStopTaskId,
    ImportTaskId,

    // service related
    ServiceRequestAckId,
    DisconnectionRequestId,

    FirstAvailableId = 100
  };

}

#endif


#ifndef _EXAMPLE_TYPE_IDS_H_
#define _EXAMPLE_TYPE_IDS_H_

#include <TypeIds.H>

namespace ReferenceImplementation {

  // the IDs will be used by the Factory classes to create new types
  // must begin with Yammer::FirstAvailableId

  enum {
    ExampleServiceRequestId = Yammer::FirstAvailableId,
    ExampleLocalServiceRequestId,
    ExampleRequestId,
    ExampleReplyId
  };

}

#endif


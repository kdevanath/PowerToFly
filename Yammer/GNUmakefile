# factor out platform differences between Linux and Sun
# be sure ACE is built with the same version of STL as the
# compiler-supplied version
ifeq ($(shell uname),Linux)
  CXX = g++ 
  THREAD_LIB = -pthread
  SHARED_LIB_OPTION = -shared -fPIC
  RUN_PATH_OPTION = -Wl,-R
  DEPENDS_OPTION = -MM -isystem $(ACE_ROOT) 
  EXCL_CMD = cat
  STLPORT_LIB = -lstlport_gcc
else
  # Workshop 8
  #CXX = CC -library=stlport4 -mt
  # Workshop 7
  CXX = /apps/WS7/prod/bin/CC -library=rwtools7_std -mt
  THREAD_LIB = -lpthread
  SHARED_LIB_OPTION = -G -KPIC
  RUN_PATH_OPTION = -R
  DEPENDS_OPTION = -xM1
  # exclude ACE headers from the dependency file
  EXCL_CMD = egrep -v 'ace' 
  STLPORT_LIB = -lstlport_sunpro
endif

# "gmake test" (default) builds with debug symbols/not optimized
# "gmake prod" builds optimized
OPTIMIZE = 

CXXFLAGS = \
  -g \
  $(OPTIMIZE) \
  $(STL_INC) \
  -I$(ACE_ROOT) \
  -I. \

CPPFLAGS = \
  -DACE_HAS_EXCEPTIONS \
  -D__ACE_INLINE__ \

SOURCES = \
  BufferedStream.C \
  DisconnectionRequest.C \
  ImportTask.C \
  InterruptableStream.C \
  IPCQueue.C \
  IPCQueueStreamAdapter.C \
  Logger.C \
  MsgHelper.C \
  MulticastStreamAdapter.C \
  ProtocolError.C \
  ServerProcess.C \
  ServiceAccessPoint.C \
  ServiceRequestAck.C \
  TaskDispatcher.C \
  TaskStreams.C \
  TaskSync.C \
  TCPStreamAdapter.C \
  UDPStreamAdapter.C \
  UnknownServiceRequest.C \
  UnknownTask.C \

OBJS = $(SOURCES:.C=.o)

LDFLAGS = \
  -L$(ACE_ROOT)/ace $(RUN_PATH_OPTION)$(ACE_ROOT)/ace \
  $(STL_LDFLAGS)

LIBS = \
  $(THREAD_LIB) \
  -ldl \
  -lACE \
  $(STL_LIBS)

BINARIES = libappsrv.so 

all: test

test: $(BINARIES)

prod: OPTIMIZE = -O
prod: $(BINARIES)

# "make USE_STLPORT=1" builds the library using STLport; otherwise
# the default compiler-supplied STL library will be used
ifdef USE_STLPORT
  STL_INC = -I$(STLPORT_ROOT)/stlport
  STL_LDFLAGS = -L$(STLPORT_ROOT)/lib $(RUN_PATH_OPTION)$(STLPORT_ROOT)/lib 
  STL_LIBS = $(STLPORT_LIB)
  CXXFLAGS += -DUSE_STLPORT
endif

# Generate a dependency file (a makefile of prerequisites) for each source file.
# Use sed to make each .d file depend on all the source and header files that 
# the corresponding .o file depends on.  This way, make will regenerate the .d
# file whenever a source or header file changes.
%.d: %.C
	@set -e; echo Generating dependencies for $<; \
  $(CXX) $(DEPENDS_OPTION) $(CXXFLAGS) $(CPPFLAGS) $< >$@.$$$$; \
  $(EXCL_CMD) $@.$$$$ | sed '/:/ s// $@:/' >$@; rm -f $@.$$$$ 

-include $(SOURCES:.C=.d)

# When running make clean, if there are no dependency files, they will be 
# created first (due to the above rules) then deleted by the rm command.
# If the dependency files exist, they will just get deleted.
clean-all:
clean:
	@\rm -f *.o *.d *.d.* $(BINARIES)

libappsrv.so: $(OBJS) 
	@echo Linking $@
	$(CXX) $(SHARED_LIB_OPTION) -o $@ $^ $(LDFLAGS) $(LIBS)


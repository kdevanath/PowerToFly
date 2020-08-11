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
  STL = 
else
  # Workshop 8
  #CXX = CC -library=stlport4 -mt
  CXX = /apps/WS7/prod/bin/CC -library=rwtools7_std -mt
  THREAD_LIB = -lpthread
  SHARED_LIB_OPTION = -G -KPIC
  RUN_PATH_OPTION = -R
  DEPENDS_OPTION = -xM1
  # exclude ACE headers from the dependency file
  EXCL_CMD = egrep -v 'ace' 
  # needed due to ACE's dependency on STLport
  #STL = -L$(STLPORT_ROOT)/lib -R$(STLPORT_ROOT)/lib -lstlport_sunpro
endif

APP_SVR_ROOT = ..

# "gmake test" (default) builds with debug symbols/not optimized
# "gmake prod" builds optimized
OPTIMIZE = 

CXXFLAGS = \
  -g \
  $(OPTIMIZE) \
  -I$(ACE_ROOT) \
  -I$(APP_SVR_ROOT) \
  -I. \

CPPFLAGS = \
  -DACE_HAS_EXCEPTIONS \
  -D__ACE_INLINE__ \

SOURCES = \
  ExampleLocalService.C \
  ExampleLocalServiceRequest.C \
  ExampleReply.C \
  ExampleRequest.C \
  ExampleServer.C \
  ExampleService.C \
  ExampleServiceRequest.C \
  ExampleWebServer.C \

OBJS = $(SOURCES:.C=.o)

LDFLAGS = \
  -L$(ACE_ROOT)/ace $(RUN_PATH_OPTION)$(ACE_ROOT)/ace \
  -L$(STLPORT_ROOT)/lib $(RUN_PATH_OPTION)$(STLPORT_ROOT)/lib \
  -L$(APP_SVR_ROOT) $(RUN_PATH_OPTION)$(APP_SVR_ROOT) \

LIBS = \
  $(THREAD_LIB) \
  -ldl \
  -lrt \
  -lACE $(STL) \
  -lappsrv

BINARIES = server client localclient

all: test

test: $(BINARIES)

prod: OPTIMIZE = -O
prod: $(BINARIES)

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
clean:
	@\rm -f *.o *.d *.d.* $(BINARIES)

server: $(OBJS) main.o 
	@echo Linking $@
	$(PURIFY) $(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

client: $(OBJS) client.o 
	@echo Linking $@
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

localclient: $(OBJS) localclient.o 
	@echo Linking $@
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

loadlib: $(OBJS) loadlib.o
	@echo Linking $@
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)


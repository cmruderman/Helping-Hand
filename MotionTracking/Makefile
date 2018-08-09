OS := $(shell uname)
ARCH := $(shell uname -m)

ifeq ($(OS), Linux)
  ifeq ($(ARCH), x86_64)
    LEAP_LIBRARY := ./LeapSDK/lib/x64/libLeap.so -Wl,-rpath,./LeapSDK/lib/x64
  else
    LEAP_LIBRARY := ./LeapSDK/lib/x86/libLeap.so -Wl,-rpath,./LeapSDK/lib/x86
  endif
else
  # OS X
  LEAP_LIBRARY := ./LeapSDK/lib/libLeap.dylib
endif

HandTracking: HandTracking.cpp arduino-serial-lib.c
	$(CXX) -pthread -Wall -g --std=c++11 -I./LeapSDK/include HandTracking.cpp arduino-serial-lib.c -o HandTracking $(LEAP_LIBRARY)
ifeq ($(OS), Darwin)
	install_name_tool -change @loader_path/libLeap.dylib ./LeapSDK/lib/libLeap.dylib HandTracking
endif

clean:
	rm -rf HandTracking HandTracking.dSYM

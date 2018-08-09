OS := $(shell uname)
ARCH := $(shell uname -m)

ifeq ($(OS), Linux)
  ifeq ($(ARCH), x86_64)
    LEAP_LIBRARY := ../LeapSDK/lib/x64/libLeap.so -Wl,-rpath,../LeapSDK/lib/x64
  else
    LEAP_LIBRARY := ../LeapSDK/lib/x86/libLeap.so -Wl,-rpath,../LeapSDK/lib/x86
  endif
else
  # OS X
  LEAP_LIBRARY := ../LeapSDK/lib/libLeap.dylib
endif

all: PlaybackDataPoints CaptureTrackingDataToCSV RecordDataPoints ManualMovement

PlaybackDataPoints: PlaybackDataPoints.cpp
	$(CXX) -pthread -std=c++11 -Wall -g -I../LeapSDK/include PlaybackDataPoints.cpp -o PlaybackDataPoints $(LEAP_LIBRARY)
ifeq ($(OS), Darwin)
	install_name_tool -change @loader_path/libLeap.dylib ../LeapSDK/lib/libLeap.dylib PlaybackDataPoints
endif

CaptureTrackingDataToCSV: CaptureTrackingDataToCSV.cpp
	$(CXX) -pthread -std=c++11 -Wall -g -I../LeapSDK/include CaptureTrackingDataToCSV.cpp -o CaptureTrackingDataToCSV $(LEAP_LIBRARY)
ifeq ($(OS), Darwin)
	install_name_tool -change @loader_path/libLeap.dylib ../LeapSDK/lib/libLeap.dylib CaptureTrackingDataToCSV
endif

RecordDataPoints: RecordDataPoints.cpp
	$(CXX) -pthread -std=c++11 -Wall -g -I../LeapSDK/include RecordDataPoints.cpp -o RecordDataPoints $(LEAP_LIBRARY)
ifeq ($(OS), Darwin)
	install_name_tool -change @loader_path/libLeap.dylib ../LeapSDK/lib/libLeap.dylib RecordDataPoints
endif

ManualMovement: ManualMovement.cpp
	$(CXX) -pthread -std=c++11 -Wall -g ManualMovement.cpp -o ManualMovement

clean:
	rm -rf PlaybackDataPoints PlaybackDataPoints.dSYM
	rm -rf CaptureTrackingDataToCSV CaptureTrackingDataToCSV.dSYM
	rm -rf RecordDataPoints RecordDataPoints.dSYM
	rm -f ManualMovement


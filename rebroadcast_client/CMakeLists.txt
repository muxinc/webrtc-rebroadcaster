cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)

project(webrtc_rebroadcaster)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(X11 REQUIRED)

find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

add_definitions( -DWEBRTC_POSIX=1 -DWEBRTC_LINUX=1 )

find_path( AVFORMAT_INCLUDE_DIR libavformat/avformat.h )
find_library( AVUTIL_LIBRARY avutil )
find_library( AVFORMAT_LIBRARY avformat )
find_library( AVCODEC_LIBRARY avcodec )

add_executable(main manager.cpp websocket_client.cpp transcoder.cpp main.cpp json.cc )
target_include_directories( main PRIVATE
	"/opt/webrtc-checkout/src"
	"/opt/webrtc-checkout/src/third_party/jsoncpp/source/include"
	"/opt/webrtc-checkout/src/third_party/abseil-cpp"
	"/opt/webrtc-checkout/src/third_party/libyuv/include"
	${AVCODEC_INCLUDE_DIR} )
target_link_libraries(
	main
	${Boost_LIBRARIES}
	Threads::Threads
	${CMAKE_SOURCE_DIR}/libwebrtc.a
	${CMAKE_SOURCE_DIR}/libjsoncpp.a
	${X11_LIBRARIES}
	${CMAKE_DL_LIBS}
	${AVUTIL_LIBRARY}
	${AVFORMAT_LIBRARY}
	${AVCODEC_LIBRARY} )

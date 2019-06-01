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

add_executable(main manager.cpp websocket_client.cpp main.cpp json.cc )
target_include_directories( main PRIVATE
	"/home/nick/public_src/webrtc-checkout/src"
	"/home/nick/public_src/webrtc-checkout/src/third_party/jsoncpp/source/include"
	"/home/nick/public_src/webrtc-checkout/src/third_party/abseil-cpp"
	"/home/nick/public_src/webrtc-checkout/src/third_party/libnyuv/include" )
target_link_libraries(
	main
	${Boost_LIBRARIES}
	Threads::Threads
	${CMAKE_SOURCE_DIR}/libwebrtc.a
	${CMAKE_SOURCE_DIR}/libjsoncpp.a
	${X11_LIBRARIES}
	 ${CMAKE_DL_LIBS} )

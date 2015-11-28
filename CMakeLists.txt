cmake_minimum_required(VERSION 3.3)
project(SshEpoll)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -lpthread")

set(SOURCE_FILES main.c settings.c settings.h socket.c socket.h signal.c signal.h threads.c threads.h epoll.c epoll.h event.c event.h)
add_executable(SshEpoll ${SOURCE_FILES})
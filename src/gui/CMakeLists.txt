set(Y86_SRCS main.cpp MainWindow.cpp VM.cpp VMWorker.cpp Memory.cpp Wire.cpp)

if(WIN32)
    add_executable(Y86 WIN32 ${Y86_SRCS})
else(WIN32)
    add_executable(Y86 ${Y86_SRCS})
endif(WIN32)

target_link_libraries(Y86 ${QT_LIBRARIES})

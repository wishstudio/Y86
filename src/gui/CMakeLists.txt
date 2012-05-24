include_directories(..)

set(Y86_SRCS main.cpp Assembler.cpp Segment.cpp MainWindow.cpp StageViewer.cpp VM.cpp VMWorker.cpp Memory.cpp Wire.cpp)

set(Y86_RCCS Components.qrc)

qt4_add_resources(Y86_RCC_SRCS ${Y86_RCCS})

if(WIN32)
    add_executable(Y86 WIN32 ${Y86_SRCS} ${Y86_RCC_SRCS})
else(WIN32)
    add_executable(Y86 ${Y86_SRCS} ${Y86_RCC_SRCS})
endif(WIN32)

target_link_libraries(Y86 ${QT_LIBRARIES})

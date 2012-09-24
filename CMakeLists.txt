project(PanFluExercise)

cmake_minimum_required(VERSION 2.4.0)

# show all warnings
set(CMAKE_CXX_FLAGS "-Wall")

# keep the library path in installed executables
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# find and setup Qt4
# see http://cmake.org/cmake/help/cmake2.6docs.html#module:FindQt4 for details
set(QT_USE_QTOPENGL TRUE)

if(WIN32)
    set(QT_USE_QTMAIN TRUE)
endif()

find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})
set(LIBS ${LIBS} ${QT_LIBRARIES})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
set(LIBS ${LIBS} ${OPENGL_LIBRARIES})

set(SRCS ${SRCS}
    src/main.cpp
    src/MainWindow.cpp
)

set(MOC_HEADERS ${MOC_HEADERS}
    src/MainWindow.h
)

qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})

add_executable(exercise ${SRCS} ${MOC_OUTFILES})

target_link_libraries(exercise ${LIBS})

# install executable
INSTALL(TARGETS exercise
    RUNTIME DESTINATION bin
)

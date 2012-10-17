project(PanFluExercise)

cmake_minimum_required(VERSION 2.4.0)

# needed for VTK library
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# show all warnings
set(CMAKE_CXX_FLAGS "-Wall")

# keep the library path in installed executables
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# path for additional modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# find and setup Qt4
# see http://cmake.org/cmake/help/cmake2.6docs.html#module:FindQt4 for details
set(QT_USE_QTOPENGL TRUE)
set(QT_USE_QTSVG TRUE)

if(WIN32)
    set(QT_USE_QTMAIN TRUE)
endif()

find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})
set(LIBS ${LIBS} ${QT_LIBRARIES})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
set(LIBS ${LIBS} ${OPENGL_LIBRARIES})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

find_package(GDAL REQUIRED)
include_directories(${GDAL_INCLUDE_DIR})
set(LIBS ${LIBS} ${GDAL_LIBRARY})

find_package(NetCDF REQUIRED)
include_directories(${NetCDF_INCLUDE_DIRS})
set(LIBS ${LIBS} ${NetCDF_LIBRARIES})

find_package(Blitz REQUIRED)
include_directories(${Blitz_INCLUDE_DIRS})
set(LIBS ${LIBS} ${BLITZ_LIBRARIES})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
set(LIBS ${LIBS} QVTK vtkCharts)

set(SRCS ${SRCS}
    src/ChartWidget.cpp
    src/ChartWidgetLine.cpp
    src/ColorMap.cpp
    src/EpidemicChartWidget.cpp
    src/EpidemicDataSet.cpp
    src/EpidemicInfoWidget.cpp
    src/log.cpp
    src/main.cpp
    src/MainWindow.cpp
    src/MapShape.cpp
    src/MapWidget.cpp
)

set(MOC_HEADERS ${MOC_HEADERS}
    src/EpidemicChartWidget.h
    src/EpidemicInfoWidget.h
    src/MainWindow.h
    src/MapWidget.h
)

qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})

add_executable(exercise ${SRCS} ${MOC_OUTFILES})

target_link_libraries(exercise ${LIBS})

# install executable
INSTALL(TARGETS exercise
    RUNTIME DESTINATION bin
)

# install data files
INSTALL(DIRECTORY data DESTINATION .)

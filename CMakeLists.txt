project(Exercise)

cmake_minimum_required(VERSION 2.4.0)

# needed for VTK library
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# keep the library path in installed executables
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# path for additional modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# OpenGL antialiasing optional
set(USE_OPENGL_ANTIALIASING ON CACHE BOOL "OpenGL antialiasing support.")

if(USE_OPENGL_ANTIALIASING)
    add_definitions(-DUSE_OPENGL_ANTIALIASING)
endif(USE_OPENGL_ANTIALIASING)

# find and setup Qt4
# see http://cmake.org/cmake/help/cmake2.6docs.html#module:FindQt4 for details
set(QT_USE_QTOPENGL TRUE)
set(QT_USE_QTSVG TRUE)
set(QT_USE_QTXMLPATTERNS TRUE)
set(QT_USE_QTXML TRUE)
set(QT_USE_QTNETWORK TRUE)

if(WIN32)
    set(CMAKE_CXX_FLAGS "/Ox /W0 /EHsc")
    set(QT_USE_QTMAIN TRUE)
    add_definitions(-DWIN32)
else(WIN32)
    # show all warnings
    set(CMAKE_CXX_FLAGS "-Wall")
endif(WIN32)

find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})
set(LIBS ${LIBS} ${QT_LIBRARIES})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
set(LIBS ${LIBS} ${OPENGL_LIBRARIES})

find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

find_package(GDAL REQUIRED)
include_directories(${GDAL_INCLUDE_DIR})
set(LIBS ${LIBS} ${GDAL_LIBRARY})

# NetCDF support optional
set(USE_NETCDF OFF CACHE BOOL "NetCDF file support.")

if(USE_NETCDF)
    add_definitions(-DUSE_NETCDF)
    find_package(NetCDF REQUIRED)
    include_directories(${NetCDF_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${NetCDF_LIBRARIES})
endif(USE_NETCDF)

find_package(Blitz REQUIRED)
include_directories(${Blitz_INCLUDE_DIRS})
set(LIBS ${LIBS} ${BLITZ_LIBRARIES})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
set(LIBS ${LIBS} vtkGUISupportQt vtkChartsCore vtkRenderingCore vtkRenderingFreeTypeOpenGL vtkRenderingVolumeOpenGL vtkViewsContext2D vtkIOExport)

find_package(GSL REQUIRED)
include_directories(${GSL_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GSL_LIBRARIES})

# DisplayCluster support optional
set(USE_DISPLAYCLUSTER OFF CACHE BOOL "DisplayCluster streaming support.")

if(USE_DISPLAYCLUSTER)
    add_definitions(-DUSE_DISPLAYCLUSTER)

    find_package(DisplayCluster REQUIRED)
    include_directories(${DisplayCluster_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${DisplayCluster_LIBRARIES})

    find_package(LibJpegTurbo REQUIRED)
    include_directories(${LibJpegTurbo_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${LibJpegTurbo_LIBRARIES})
endif(USE_DISPLAYCLUSTER)

set(SRCS ${SRCS}
    src/ChartWidget.cpp
    src/ChartWidgetLine.cpp
    src/ColorMap.cpp
    src/EpidemicCasesWidget.cpp
    src/EpidemicChartWidget.cpp
    src/EpidemicDataSet.cpp
    src/EpidemicInfoWidget.cpp
    src/EpidemicInitialCasesWidget.cpp
    src/EpidemicMapWidget.cpp
    src/EpidemicSimulation.cpp
    src/Event.cpp
    src/EventGroupThreshold.cpp
    src/EventMonitor.cpp
    src/EventMonitorWidget.cpp
    src/IliMapWidget.cpp
    src/log.cpp
    src/main.cpp
    src/MainWindow.cpp
    src/MapShape.cpp
    src/MapWidget.cpp
    src/Npi.cpp
    src/NpiWidget.cpp
    src/NpiDefinitionWidget.cpp
    src/Parameters.cpp
    src/ParametersWidget.cpp
    src/PriorityGroup.cpp
    src/PriorityGroupWidget.cpp
    src/PriorityGroupDefinitionWidget.cpp
    src/PriorityGroupSelections.cpp
    src/PriorityGroupSelectionsWidget.cpp
    src/Stockpile.cpp
    src/StockpileConsumptionWidget.cpp
    src/StockpileMapWidget.cpp
    src/StockpileNetwork.cpp
    src/StockpileNetworkDistribution.cpp
    src/StockpileNetworkWidget.cpp
    src/StockpileNetworkDistributionWidget.cpp
    src/StockpileChartWidget.cpp
    src/TimelineWidget.cpp
    src/models/random.cpp
    src/models/disease/iliView.cpp
    src/models/disease/StochasticSEATIRD.cpp
    src/models/disease/StochasticSEATIRDSchedule.cpp
)

set(MOC_HEADERS ${MOC_HEADERS}
    src/EpidemicChartWidget.h
    src/EpidemicInfoWidget.h
    src/EpidemicInitialCasesWidget.h
    src/EventMonitor.h
    src/EventMonitorWidget.h
    src/MainWindow.h
    src/MapWidget.h
    src/NpiWidget.h
    src/NpiDefinitionWidget.h
    src/Parameters.h
    src/ParametersWidget.h
    src/PriorityGroupWidget.h
    src/PriorityGroupDefinitionWidget.h
    src/PriorityGroupSelectionsWidget.h
    src/Stockpile.h
    src/StockpileConsumptionWidget.h
    src/StockpileNetworkWidget.h
    src/StockpileNetworkDistribution.h
    src/StockpileNetworkDistributionWidget.h
    src/StockpileChartWidget.h
    src/TimelineWidget.h
)

qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})

add_executable(exercise MACOSX_BUNDLE WIN32
    ${SRCS} ${MOC_OUTFILES})

target_link_libraries(exercise ${LIBS})

# install executable
INSTALL(TARGETS exercise
    RUNTIME DESTINATION bin COMPONENT Runtime
    BUNDLE DESTINATION . COMPONENT Runtime
)


# on Apple platform make the .app bundle; on Windows make the installer
if(APPLE OR WIN32)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Exercise.")
    set(CPACK_PACKAGE_EXECUTABLES "exercise" "Exercise")
    set(CPACK_PACKAGE_NAME "Exercise")
    set(CPACK_PACKAGE_VENDOR "The University of Texas at Austin")
    set(CPACK_PACKAGE_VERSION_MAJOR 1)
    set(CPACK_PACKAGE_VERSION_MINOR 0)
    set(CPACK_PACKAGE_VERSION_PATCH 1)

    if(APPLE)
        set(APPS "\${CMAKE_INSTALL_PREFIX}/Exercise.app")

        # Qt plugins and qt.conf required so we don't accidentally dynamically link against system-installed Qt libraries
        SET(PLUGIN_DEST_DIR Exercise.app/Contents/MacOS)
        SET(QTCONF_DEST_DIR Exercise.app/Contents/Resources)

        SET(DATA_DEST_DIR Exercise.app/Contents/Resources)
    elseif(WIN32)
        set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/Exercise.exe")

        # Qt plugins and qt.conf required so we don't accidentally dynamically link against system-installed Qt libraries
        SET(PLUGIN_DEST_DIR bin)
        SET(QTCONF_DEST_DIR bin)
        SET(DATA_DEST_DIR .)
    endif()

    INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${PLUGIN_DEST_DIR}/plugins COMPONENT Runtime)

    INSTALL(CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${QTCONF_DEST_DIR}/qt.conf\" \"\")
        " COMPONENT Runtime)

    # install data files
    INSTALL(DIRECTORY data DESTINATION ${DATA_DEST_DIR} COMPONENT Runtime)

    if(APPLE)
        # directories to look for dependencies
        set(DEPENDENCY_DIRS ${VTK_LIBRARY_DIRS})
    elseif(WIN32)
        # directories to look for dependencies
        #AARONBAD -- need to manually specify VTK binaries dir where dll's live. Everything else is ok.
        set(DEPENDENCY_DIRS ${QT_BINARY_DIR} "C:/Users/amk3264/Downloads/gdalwin32-1.4.1/bin/" "C:/Users/amk3264/Downloads/VTK-6.0.0-win32-install/bin")
        include(InstallRequiredSystemLibraries)
    endif()

    INSTALL(CODE "
        file(GLOB_RECURSE QTPLUGINS \"\${CMAKE_INSTALL_PREFIX}/${PLUGIN_DEST_DIR}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
        include(BundleUtilities)
        fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DEPENDENCY_DIRS}\")
        " COMPONENT Runtime)

    # to create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" for example
    # where CPackConfig.cmake is created by including CPack
    if(APPLE)
        set(CPACK_BINARY_DRAGNDROP ON)
    elseif(WIN32)
        set(CPACK_BINARY_NSIS ON)
    endif()

    include(CPack)
else()
    # Linux

    # install data files
    INSTALL(DIRECTORY data DESTINATION . COMPONENT Runtime)

    # install utility files (Linux only)
    INSTALL(DIRECTORY util DESTINATION . COMPONENT Runtime)
endif()

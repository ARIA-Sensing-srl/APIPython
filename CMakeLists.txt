cmake_minimum_required(VERSION 3.15)
project(AHMAPIpy LANGUAGES C CXX)

if (NOT SKBUILD)
  message(WARNING "\
  This CMake file is meant to be executed using 'scikit-build'. Running
  it directly will almost certainly not produce the desired result. If
  you are a user trying to install this package, please use the command
  below, which will install all necessary build dependencies, compile
  the package in an isolated environment, and then install it.
  =====================================================================
   $ pip install .
  =====================================================================
  If you are a software developer, and this is your own package, then
  it is usually much more efficient to install the build dependencies
  in your environment once and use the following command that avoids
  a costly creation of a new virtual environment at every compilation:
  =====================================================================
   $ pip install nanobind scikit-build-core[pyproject]
   $ pip install --no-build-isolation -ve .
  =====================================================================
  You may optionally add -Ceditable.rebuild=true to auto-rebuild when
  the package is imported. Otherwise, you need to re-run the above
  after editing C++ files.")
endif()

# Find nanobind and Python
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
#find_package(nanobind REQUIRED PATHS "C:/Users/AndreaPort/AppData/Local/Programs/Python/Python313/Lib/site-packages/nanobind/cmake" NO_DEFAULT_PATH)
execute_process(
  COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE nanobind_ROOT)
find_package(nanobind CONFIG REQUIRED)
#find_package(nanobind REQUIRED PATHS "/home/nbodei/.local/lib/python3.12/site-packages/nanobind/cmake" NO_DEFAULT_PATH)
set (SUBMODULE_PATH "AHM_C_API")
#set(CMAKE_C_FLAGS_INIT "-std=c99")

if (MSVC)
add_definitions(-DWIN)
endif (MSVC)
# Create the module implementation
nanobind_add_module(
  AHMAPIpy
  binding/bindings.cpp
  binding/radarhandler.cpp
)
#set (srclist )
list ( APPEND SRCLIST binding/bindings.cpp
    binding/radarhandler.cpp
    binding/radarhandler.h
    ${SUBMODULE_PATH}/ahm_cmds/ahm_comm_def.h
    ${SUBMODULE_PATH}/ahm_cmds/ahm_common.h
    ${SUBMODULE_PATH}/ahm_cmds/rdr_common_types.h
    ${SUBMODULE_PATH}/inc/f16tosingle.h
    ${SUBMODULE_PATH}/inc/msgEncoderDecored.h
    ${SUBMODULE_PATH}/radar_data/radarData.cpp
    ${SUBMODULE_PATH}/radar_data/radarData.h
    ${SUBMODULE_PATH}/radar_data/radarDataVolume.cpp
    ${SUBMODULE_PATH}/radar_data/radarDataVolume.h
    ${SUBMODULE_PATH}/radar_handlers/radar_AHM2D.cpp
    ${SUBMODULE_PATH}/radar_handlers/radar_AHM2D.h
    ${SUBMODULE_PATH}/radar_handlers/radar_AHM2DL.cpp
    ${SUBMODULE_PATH}/radar_handlers/radar_AHM2DL.h
    ${SUBMODULE_PATH}/radar_handlers/radar_AHM3D.cpp
    ${SUBMODULE_PATH}/radar_handlers/radar_AHM3D.h
    ${SUBMODULE_PATH}/radar_handlers/radar_api_base.cpp
    ${SUBMODULE_PATH}/radar_handlers/radar_api_base.h
    ${SUBMODULE_PATH}/src/f16tosingle.cpp
    ${SUBMODULE_PATH}/src/msgEncoderDecored.cpp
    ${SUBMODULE_PATH}/uart/radarif_ll.cpp
    ${SUBMODULE_PATH}/uart/radarif_ll.h
	${SUBMODULE_PATH}/uart/radarif_ll_uart.h
	${SUBMODULE_PATH}/uart/radarif_ll_uart.cpp
	${SUBMODULE_PATH}/wiringPiSource/wiringSerial.h
	
    
)
	
if (MSVC)
list ( APPEND SRCLIST ${SUBMODULE_PATH}/wiringPiSource/winSerial.c )
else (MSVC)
list ( APPEND SRCLIST ${SUBMODULE_PATH}/wiringPiSource/wiringSerial.c )


find_library(HAVE_MPSSE_LIB NAMES mpsse OPTIONAL)
find_library(HAVE_FTD2XX NAMES ftd2xx OPTIONAL)
find_library(HAVE_WIRINGPI NAMES wiringPi OPTIONAL)

if ((HAVE_MPSSE_LIB AND HAVE_FTD2XX) OR HAVE_WIRINGPI)
list (APPEND sources ${SUBMODULE_PATH}/spi/radarif_ll_spi.cpp)
list ( APPEND SRCLIST ${SUBMODULE_PATH}/spi/radarif_ll_spi.h 
	${SUBMODULE_PATH}/spi/spi_interface.h)
add_definitions(-DCONFIG_USE_SPI)
message(STATUS "Enable FTDI cable support")

if (HAVE_WIRINGPI)
list (APPEND sources ${SUBMODULE_PATH}/spi/spi_interface.cpp)
else (HAVE_WIRINGPI)
list (APPEND sources ${SUBMODULE_PATH}/spi/spi_interface_FTDI.cpp)
endif (HAVE_WIRINGPI)


endif ((HAVE_MPSSE_LIB AND HAVE_FTD2XX) OR HAVE_WIRINGPI)

endif (MSVC)

target_sources(AHMAPIpy PRIVATE ${SRCLIST})



target_include_directories(AHMAPIpy PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/binding
  ${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULE_PATH}/ahm_cmds
  ${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULE_PATH}/radar_data
  ${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULE_PATH}/radar_handlers
  ${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULE_PATH}/uart
  ${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULE_PATH}/wiringPiSource
  ${CMAKE_CURRENT_SOURCE_DIR}/${SUBMODULE_PATH}/inc
)

if ((HAVE_MPSSE_LIB AND HAVE_FTD2XX) OR HAVE_WIRINGPI)
if (HAVE_WIRINGPI)
	target_link_libraries(AHMAPIpy -lwiringPi)
else (HAVE_WIRINGPI)
	target_link_libraries(AHMAPIpy -lmpsse -lftd2xx)
endif (HAVE_WIRINGPI)
endif ((HAVE_MPSSE_LIB AND HAVE_FTD2XX) OR HAVE_WIRINGPI)

install(TARGETS AHMAPIpy DESTINATION .)

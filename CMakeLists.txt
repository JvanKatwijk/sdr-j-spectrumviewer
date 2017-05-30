#
cmake_minimum_required( VERSION 2.8.11 )
set (objectName sdr-j-spectrumviewer-1.1)
add_definitions ( -Wall)
#	modify if you want

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr/local/bin" CACHE PATH "default install path" FORCE )
endif()
#set (CMAKE_INSTALL_PREFIX /usr/local/bin)

if(MINGW)
    add_definitions ( -municode)
endif()

########################################################################
# select the release build type by default to get optimization flags
########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

### make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

	add_definitions ( -Wall )
##########################################################################
#       R E A D  T H I S
#####################################################################

if(DEFINED AIRSPY)
   set(AIRSPY true)
endif ()

if(DEFINED SDRPLAY)
   set(SDRPLAY true)
endif ()

if(DEFINED DABSTICK)
   set(DABSTICK true)
endif ()

if(DEFINED RTLTCP)
   set(RTLTCP true)
endif ()

if(DEFINED ELAD_S1)
   set(ELAD_S1 true)
endif ()

if(DEFINED SOUNDCARD)
   set(SOUNDCARD true)
endif()

########################################################################

	find_package (PkgConfig)

	find_package (Qt5Core REQUIRED)
	find_package (Qt5Widgets REQUIRED)
	find_package (Qt5Network REQUIRED)
#	find_package (Qt5Declarative REQUIRED)
#	find_package (Qt5 5.1.0 COMPONENTS Xml)
	include_directories (
	      ${Qt5Network_INCLUDE_DIRS}
	)

        find_package(Qwt)
        if (NOT QWT_FOUND)
            message(FATAL_ERROR "please install qwt library")
        endif ()

	find_package(FFTW3f)
        if (NOT FFTW3F_FOUND)
            message(FATAL_ERROR "please install FFTW3")
        endif ()

	find_library (DYNAMIC_LOAD dl)
	if(NOT(DYNAMIC_LOAD))
	   message(FATAL_ERROR "please install -ldl")
	else(NOT(DYNAMIC_LOAD))
	   set(extraLibs ${DYNAMIC_LOAD})
	endif(NOT(DYNAMIC_LOAD))
#
	find_library (PTHREADS pthread)
	if (NOT(PTHREADS))
	   message (FATAL_ERROR "please install libpthread")
	else (NOT(PTHREADS))
	   set (extraLibs ${extraLibs} ${PTHREADS})
	endif (NOT(PTHREADS))

	find_library(USB1 usb-1.0 HINTS /usr/lib)
	if (NOT(USB1))
	   message(FATAL_ERROR "please install libusb-1.0")
	else (NOT(USB1))
	   set(extraLibs ${extraLibs} ${USB1})
	endif(NOT(USB1))
#####################################################################
#	here we really start                                        #
#####################################################################

	set ( ${objectName}_UIS ./sdrgui.ui)

	include_directories (
                   ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
                   ${QT5Widgets_INCLUDES}
                   ${QT_QTCORE_INCLUDE_DIR}
                   ${QT_QTGUI_INCLUDE_DIR}
                   .
	   	   ./
	   	   ../
	   ./includes
	   ./src
	   ./devices
	   /usr/include/
	)

	set ( ${objectName}_HDRS
	      ${${objectName}_HDRS}
	      ./viewer.h 
	      ./includes/spectrum-constants.h 
              ./includes/fft.h 
              ./includes/ringbuffer.h 
	      ./includes/spectrogramdata.h 
	      ./includes/scope.h  
	      ./devices/device-handler.h 
	)

	set ( ${objectName}_SRCS
	      ${${objectName}_SRCS}
	      ./main.cpp 
              ./viewer.cpp 
	      ./src/scope.cpp 
              ./src/fft.cpp 
	      ./devices/device-handler.cpp 
	)

##########################################################################
#	devices                                                          #
##########################################################################
	if (SDRPLAY)
	   find_path (SDRPLAYLIB_INCLUDE_DIR
	              NAMES mirsdrapi-rsp.h
	              PATHS
	              /usr/local/include/
	   )
	   include_directories (${SDRPLAYLIB_INCLUDE_DIR})
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/sdrplay-handler/sdrplay-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/sdrplay-handler/sdrplay-handler.h
	        ./devices/sdrplay-handler/sdrplayselect.h
	   )

	   include_directories (
	        ./devices/sdrplay-handler
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/sdrplay-handler/sdrplay-handler.h
	        ./devices/sdrplay-handler/sdrplayselect.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/sdrplay-handler/sdrplay-handler.cpp
	        ./devices/sdrplay-handler/sdrplayselect.cpp
	   )

	   add_definitions (-DHAVE_SDRPLAY)
	endif (SDRPLAY)

	if (AIRSPY)
	   find_library (AIRSPY_LIB airspy)
	   if (NOT(AIRSPY_LIB))
	      message (FATAL_ERROR "please install airspy library")
	   else (NOT(AIRSPY_LIB))
	      set (extraLibs ${extraLibs} ${AIRSPY_LIB})
	   endif()
	   find_path (AIRSPYLIB_INCLUDE_DIR
	              NAMES airspy.h
	              PATHS
	              /usr/local/include/
	              /usr/local/include/libairspy
	   )
	   include_directories (${AIRSPYLIB_INCLUDE_DIR})

	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/airspy-handler/airspy-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/airspy-handler/airspy-handler.h
	   )

	   include_directories (
	     ./devices/airspy-handler
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/airspy-handler/airspy-handler.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/airspy-handler/airspy-handler.cpp
	   )

	   add_definitions (-DHAVE_AIRSPY)
	endif (AIRSPY)

	if (DABSTICK)
	   find_library (RTLSDR rtlsdr)
	   if (NOT(RTLSDR))
	      message (FATAL_ERROR "please install librtlsdr")
	   else (NOT(RTLSDR))
	      set (extraLibs ${extraLibs} ${RTLSDR})
	   endif()
	   find_path (RTLSDR_INCLUDE_DIR
	              NAMES mirsdrapi-rsp.h
	              PATHS
	              /usr/local/include/
	              /usr/local/include/libairspy
	   )
	   include_directories (${RTLSDR_INCLUDE_DIR})

	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/rtlsdr-handler/dabstick-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/rtlsdr-handler/dongleselect.h
	        ./devices/rtlsdr-handler/rtlsdr-handler.h
	   )

	   include_directories (
	        ./devices/rtlsdr-handler/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.h 
	        ./devices/rtlsdr-handler/dongleselect.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.cpp
	        ./devices/rtlsdr-handler/dongleselect.cpp
	   )

	   add_definitions (-DHAVE_RTLSDR)
	endif()

	if (ELAD_S1)
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/sw-elad-s1/elad-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/sw-elad-s1/elad-s1.h
	        ./devices/sw-elad-s1/elad-worker.h
	   )

	   include_directories (
	        ./devices/sw-elad-s1/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./devices/sw-elad-s1/elad-s1.h
	        ./devices/sw-elad-s1/elad-loader.h
	        ./devices/sw-elad-s1/elad-worker.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/sw-elad-s1/elad-s1.cpp
	        ./devices/sw-elad-s1/elad-loader.cpp
	        ./devices/sw-elad-s1/elad-worker.cpp
	   )

	   add_definitions (-DHAVE_ELAD_S1)
	endif()

	if (SOUNDCARD)
	   find_library (PORTAUDIO_LIB portaudio)
	   if (NOT(PORTAUDIO_LIB))
	      message (FATAL_ERROR "please install portaudio library")
	   else (NOT(PORTAUDIO_LIB))
	      set (extraLibs ${extraLibs} ${PORTAUDIO_LIB})
	   endif()
	   find_path (PORTAUDIO_INCLUDE_DIR
	              NAMES portaudio.h
	              PATHS
	              /usr/local/include/
	   )
	   include_directories (${PORTAUDIO_INCLUDE_DIR})
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/soundcard/soundcard-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/soundcard/soundcard.h
	        ./devices/soundcard/pa-reader.h
	   )

	   include_directories (
	        ./devices/soundcard/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./devices/soundcard/soundcard.h
	        ./devices/soundcard/pa-reader.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/soundcard/soundcard.cpp
	        ./devices/soundcard/pa-reader.cpp
	   )

	   add_definitions (-DHAVE_SOUNDCARD)
	endif()
########################################################################

	set (${objectName}_UIS ${${objectName}_UIS} ./sdrgui.ui)

	QT5_WRAP_UI(UIS ${${objectName}_UIS})

	set ( ${objectName}_MOCS
	      ${${objectName}_MOCS}
	      ./viewer.h
	      ./includes/scope.h
	      ./devices/device-handler.h
	)

	include_directories (${QWT_INCLUDE_DIRS}
	                     ${FFTW_INCLUDE_DIRS}
	)

	QT5_WRAP_CPP (MOCS ${${objectName}_MOCS})

	add_executable (${objectName}
	                ${${objectName}_SRCS}
	                ${UIS}
	                ${SRCS}
	                ${TRS}
	                ${MOCS}
	)

	target_link_libraries (${objectName}
	                       Qt5::Widgets
	                       ${FFTW3F_LIBRARIES}
	                       ${extraLibs}
	                       ${FAAD_LIBRARIES}
	                       ${QWT_LIBRARIES}
	)

	INSTALL(TARGETS ${objectName} DESTINATION ./linux-bin)

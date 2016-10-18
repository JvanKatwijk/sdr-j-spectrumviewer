#
cmake_minimum_required( VERSION 2.8.11 )
set (objectName sdr-j-spectrumviewer-1.1)
add_definitions ( -Wall)
#	modify if you want
set (CMAKE_INSTALL_PREFIX ../)

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
#	adjust to your likings
#	if you want support for any of these devices, uncomment the line
	set(SDRPLAY true)
	set(AIRSPY true)
	set(DABSTICK true)
	set(ELAD_S1 true)
	set(SOUNDCARD true)

########################################################################

	find_package (PkgConfig)

	find_package (Qt5Core REQUIRED)
	find_package (Qt5Widgets REQUIRED)
	find_package (Qt5Network REQUIRED)
	find_package (Qt5Declarative REQUIRED)
	find_package (Qt5 5.1.0 COMPONENTS Xml)
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
	   ./src/input
	   /usr/include/
	)

	set ( ${objectName}_HDRS
	      ${${objectName}_HDRS}
	      ./includes/gui.h 
	      ./includes/spectrum-constants.h 
              ./includes/fft.h 
              ./includes/ringbuffer.h 
	      ./includes/spectrogramdata.h 
	      ./includes/scope.h  
	      ./src/input/virtual-input.h 
	)

	set ( ${objectName}_SRCS
	      ${${objectName}_SRCS}
	      ./src/main.cpp 
              ./src/gui.cpp 
	      ./src/scope.cpp 
              ./src/fft.cpp 
	      ./src/input/virtual-input.cpp 
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
	        ./src/input/sdrplay-exp/sdrplay-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./src/input/sdrplay-exp/sdrplay.h
	   )

	   include_directories (
	        ./src/input/sdrplay-exp
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./src/input/sdrplay-exp/sdrplay.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./src/input/sdrplay-exp/sdrplay.cpp
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
	        ./src/input/airspy/airspy-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./src/input/airspy/airspy-handler.h
	   )

	   include_directories (
	     ./src/input/airspy
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./src/input/airspy/airspy-handler.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./src/input/airspy/airspy-handler.cpp
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
	        ./src/input/dabstick/dabstick-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./src/input/dabstick/dongleselect.h
	        ./src/input/dabstick/dabstick.h
	   )

	   include_directories (
	        ./src/input/dabstick/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./src/input/dabstick/dabstick.h 
	        ./src/input/dabstick/dongleselect.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./src/input/dabstick/dabstick.cpp
	        ./src/input/dabstick/dongleselect.cpp
	   )

	   add_definitions (-DHAVE_DABSTICK)
	endif()

	if (ELAD_S1)
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./src/input/sw-elad-s1/elad-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./src/input/sw-elad-s1/elad-s1.h
	        ./src/input/sw-elad-s1/elad-worker.h
	   )

	   include_directories (
	        ./src/input/sw-elad-s1/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./src/input/sw-elad-s1/elad-s1.h
	        ./src/input/sw-elad-s1/elad-loader.h
	        ./src/input/sw-elad-s1/elad-worker.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./src/input/sw-elad-s1/elad-s1.cpp
	        ./src/input/sw-elad-s1/elad-loader.cpp
	        ./src/input/sw-elad-s1/elad-worker.cpp
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
	        ./src/input/soundcard/soundcard-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./src/input/soundcard/soundcard.h
	        ./src/input/soundcard/pa-reader.h
	   )

	   include_directories (
	        ./src/input/soundcard/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./src/input/soundcard/soundcard.h
	        ./src/input/soundcard/pa-reader.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./src/input/soundcard/soundcard.cpp
	        ./src/input/soundcard/pa-reader.cpp
	   )

	   add_definitions (-DHAVE_SOUNDCARD)
	endif()
########################################################################

	set (${objectName}_UIS ${${objectName}_UIS} ./sdrgui.ui)

	QT5_WRAP_UI(UIS ${${objectName}_UIS})

	set ( ${objectName}_MOCS
	      ${${objectName}_MOCS}
	      ./includes/gui.h
	      ./includes/scope.h
	      ./src/input/virtual-input.h
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
	                       ${FFTW_LIBRARIES}
	                       ${extraLibs}
	                       ${FAAD_LIBRARIES}
	                       ${QWT_LIBRARIES}
	)

	INSTALL(TARGETS ${objectName} DESTINATION ./linux-bin)

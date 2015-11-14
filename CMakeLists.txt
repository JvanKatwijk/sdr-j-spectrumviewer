#
	cmake_minimum_required( VERSION 2.8 )
	set (CMAKE_BUILD_TYPE Release )
	set (objectName sdr-j-spectrumviewer)
	set (CMAKE_INSTALL_PREFIX ../)
	add_definitions ( -Wall )
##########################################################################
#       R E A D  T H I S
#####################################################################
#	adjust to your likings
#	if you want support for any of these devices, uncomment the line
	set(SDRPLAY true)
	set(AIRSPY true)
	set(DABSTICK true)

########################################################################
	find_package (Qt5Widgets REQUIRED)
	find_package (Qt5Declarative REQUIRED)

	find_library (DYNAMIC_LOAD dl)
	if(NOT(DYNAMIC_LOAD))
	   message(FATAL_ERROR "please install -ldl")
	else(NOT(DYNAMIC_LOAD))
	   set(extraLibs ${DYNAMIC_LOAD})
	endif(NOT(DYNAMIC_LOAD))
#

#	find_library(QWT qwt)
	if (QWT_INCLUDE_DIRS)
#	Already in cache, be silent
	   set (QWT_FIND_QUIETLY TRUE)
	endif (QWT_INCLUDE_DIRS)

	find_path (QWT_INCLUDE_DIR
	           NAMES qwt_plot.h
	           PATHS
	           /usr/local/include/qwt-qt5
	           /usr/local/include/qwt
	           /usr/local/include/qwt6
	           /usr/include/qwt6
	           /usr/include/qwt
	)

	list (APPEND QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )
	find_library (QWT_LIBRARIES
	              NAMES qwt-qt5 qwt qwt6
#	              PATHS /usr/local/lib /usr/lib
                      PATH_SUFFIXES lib64 Lib64 lib Lib
	)

#	message (STATUS ${QWT_LIBRARIES} ${QWT_INCLUDE_DIRS})
#	handle the QUIETLY and REQUIRED arguments and set QWT_FOUND to TRUE if 
#	all listed variables are TRUE
	include (FindPackageHandleStandardArgs)
	find_package_handle_standard_args (
	                     Qwt DEFAULT_MSG QWT_LIBRARIES QWT_INCLUDE_DIRS )
	MARK_AS_ADVANCED (QWT_LIBRARIES QWT_INCLUDE_DIRS)
	MARK_AS_ADVANCED (QWT_LIBRARIES QWT_INCLUDE_DIRS)
#	Find_Package(PkgConfig)
#	pkg_check_modules(FFTW fftw3f REQUIRED)
#	link_directories(${FFTW_INCLUDE_DIRS})
#	 Find the native FFTW includes and library
#
#	FFTW_INCLUDES    - where to find fftw3f.h
#	FFTW_LIBRARIES   - List of libraries when using FFTW.
#	FFTW_FOUND       - True if FFTW found.

	if (FFTW_INCLUDE_DIRS)
	   # Already in cache, be silent
	   set (FFTW_FIND_QUIETLY TRUE)
	endif (FFTW_INCLUDE_DIRS)

	find_path (FFTW_INCLUDE_DIR fftw3.h
              HINTS
                  ENV FFTW_ROOT
              PATHS 
              PATH_SUFFIXES
                  include
                  Include
	)
	list( APPEND FFTW_INCLUDE_DIRS ${FFTW_INCLUDE_DIR} )
	find_library (FFTW_LIBRARIES NAMES fftw3f
              HINTS ENV FFTW_ROOT
              PATHS
              PATH_SUFFIXES lib64 Lib64 lib Lib)

	message (STATUS ${FFTW_LIBRARIES} ${FFTW_INCLUDE_DIRS})
#	handle the QUIETLY and REQUIRED arguments and set FFTW_FOUND to TRUE if
#	all listed variables are TRUE

	include (FindPackageHandleStandardArgs)
	find_package_handle_standard_args (FFTW DEFAULT_MSG FFTW_LIBRARIES FFTW_INCLUDE_DIRS)
	mark_as_advanced (FFTW_LIBRARIES FFTW_INCLUDE_DIRS)

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


	if (SDRPLAY)
	   find_library (SDRPLAYLIB libmirsdrapi-rsp.so)
	   if (NOT(SDRPLAYLIB))
	      message (FATAL_ERROR "please install sdrplay library")
	   else (NOT(SDRPLAYLIB))
	      set (extraLibs ${extraLibs} ${SDRPLAYLIB})
	   endif()
	   find_path (SDRPLAYLIB_INCLUDE_DIR
	              NAMES mirsdrapi-rsp.h
	              PATHS
	              /usr/local/include/
	   )
	   include_directories (${SDRPLAYLIB_INCLUDE_DIR})
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./input/sdrplay/sdrplay-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./input/sdrplay/sdrplay.h
	        ./input/sdrplay/sdrplay-worker.h
	   )

	   include_directories (
	        ./input/sdrplay
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./input/sdrplay/sdrplay.h
	        ./input/sdrplay/sdrplay-worker.h
	        ./input/sdrplay/sdrplay-loader.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./input/sdrplay/sdrplay.cpp
	        ./input/sdrplay/sdrplay-worker.cpp
	        ./input/sdrplay/sdrplay-loader.cpp
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
	        ./input/airspy/airspy-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./input/airspy/airspy-handler.h
	   )

	   include_directories (
	     ./input/airspy
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./input/airspy/airspy-handler.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./input/airspy/airspy-handler.cpp
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
	        ./input/dabstick/dabstick-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./input/dabstick/dongleselect.h
	        ./input/dabstick/dabstick.h
	   )

	   include_directories (
	        ./input/dabstick/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./input/dabstick/dabstick.h 
	        ./input/dabstick/dongleselect.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./input/dabstick/dabstick.cpp
	        ./input/dabstick/dongleselect.cpp
	   )

	   add_definitions (-DHAVE_DABSTICK)
	endif()

	include_directories (
	   ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	   ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
	   .
	   ./
	   ../
	   ./includes
	   ./input
	   ./src
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
	      ./input/virtual-input.h 
	)

	set ( ${objectName}_SRCS
	      ${${objectName}_SRCS}
	      ./src/main.cpp 
              ./src/gui.cpp 
	      ./src/scope.cpp 
              ./src/fft.cpp 
	      ./input/virtual-input.cpp 
	)

	set (${objectName}_UIS ${${objectName}_UIS} ./sdrgui.ui)

	QT5_WRAP_UI(UIS ${${objectName}_UIS})

	set ( ${objectName}_MOCS
	      ${${objectName}_MOCS}
	      ./includes/gui.h
	      ./includes/scope.h
	      ./input/virtual-input.h
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

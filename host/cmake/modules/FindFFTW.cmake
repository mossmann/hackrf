# - Find FFTW
# Find the native FFTW includes and library
# Components:
#	MPI Fortran SinglePrecision QuadPrecision
#  FFTW_COMPILE_OPTIONS   - Compile options to apply when using fftw
#  FFTW_INCLUDES          - where to find fftw3.h
#  FFTW_LIBRARIES	      - List of libraries when using FFTW.
#  FFTW_FOUND             - True if FFTW found.
#  FFTW_IS_SHARED         - True if FFTW is being linked as a shared library
#  FFTW_<Component>_FOUND - Whether the given component was found.
#
# Note that on some systems, FFTW splits itself into multiple libraries,
# single precision (fftwf), double precision (fftw), and quad precision (fftwl).
# By default FindFFTW will find only the standard double precision variant.
# To guarantee that you get the other variants, request the relevant components.
#
# Why is there a "Fortran" component you ask?  It's because some systems lack the Fortran headers (e.g. fftw3.f03)
#
# when using components:
#   FFTW_INCLUDES_SERIAL   - include path for FFTW serial
#	FFTW_LIBRARIES_SERIAL  - library for use of fftw from C and Fortran
#	FFTW_INCLUDES_MPI      - include path for FFTW MPI
#	FFTW_LIBRARIES_MPI 	   - extra FFTW library to use MPI
#

if (FFTW_FOUND)
  # Already in cache, be silent
  set (FFTW_FIND_QUIETLY TRUE)
endif()

include(CheckCSourceCompiles)

set(FFTW_REQUIRED_VARIABLES )
set(FFTW_COMPILE_OPTIONS )

set(FIND_FFTW_DEBUG FALSE)

# utility function for finding multiple precisions
macro(fftw_find_precision COMPONENT PREC LIBNAMES CHECK_FUNCTION TYPE)
	
	if(FIND_FFTW_DEBUG)
		message("Searching for split ${TYPE} FFTW library (${COMPONENT}) with names ${LIBNAMES}")
	endif()

	# first find the library
	find_library(FFTW${PREC}_LIBRARY_${TYPE} NAMES ${LIBNAMES})

	if(EXISTS "${FFTW${PREC}_LIBRARY_${TYPE}}")

		if(FIND_FFTW_DEBUG)
			message("Found ${COMPONENT} ${TYPE} FFTW library: ${FFTW${PREC}_LIBRARY_${TYPE}}")
		endif()

		# now check if it works
		set(CMAKE_REQUIRED_LIBRARIES ${FFTW${PREC}_LIBRARY_${TYPE}})

		if("${TYPE}" STREQUAL "SERIAL")
			# only link serial FFTW
			list(APPEND CMAKE_REQUIRED_LIBRARIES ${FFTW_LIBRARY_SERIAL})
		else()
			# link serial and parallel
			list(APPEND CMAKE_REQUIRED_LIBRARIES ${FFTW_LIBRARY_MPI} ${FFTW_LIBRARY_SERIAL})
		endif()

		fftw_check_library(${CHECK_FUNCTION} FFTW${PREC}_${TYPE}_WORKS)
	else()
		if(FIND_FFTW_DEBUG)
			message("Could not find ${COMPONENT} ${TYPE} FFTW library: ${FFTW${PREC}_LIBRARY_${TYPE}}")
		endif()

		set(FFTW${PREC}_${TYPE}_WORKS FALSE)
	endif()

	if(FFTW${PREC}_${TYPE}_WORKS)
		set(FFTW_LIBRARIES ${FFTW${PREC}_LIBRARY_SERIAL} ${FFTW_LIBRARIES})
		set(FFTW_LIBRARIES_${TYPE} ${FFTW${PREC}_LIBRARY_SERIAL} ${FFTW_LIBRARIES_${TYPE}})

		# if the component is already set to not found, then don't make it found
		if(NOT (DEFINED FFTW_${COMPONENT}_FOUND AND NOT FFTW_${COMPONENT}_FOUND))
			set(FFTW_${COMPONENT}_FOUND TRUE)
		endif()
	endif()

	if(FFTW_FIND_REQUIRED_${COMPONENT})
		list(APPEND FFTW_REQUIRED_VARIABLES FFTW${PREC}_LIBRARY_${TYPE} FFTW${PREC}_${TYPE}_WORKS)
	endif()

endmacro(fftw_find_precision)

# Check if a given function can be found in a given library.
# Similar to CheckFunctionExists, but can handle DLL imports on Windows.
# Reads CMAKE_REQUIRED_LIBRARIES for link libraries.
function(fftw_check_library FUNCTION RESULT_VARIABLE)

	if(FIND_FFTW_DEBUG)
		message("Looking for ${FUNCTION} in ${CMAKE_REQUIRED_LIBRARIES}")
	endif()

	# mirror the logic in fftw3.h
	if(FFTW_DLL_IMPORT)
		set(FUNCTION_PREFIX "extern __declspec(dllimport)")
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
		set(FUNCTION_PREFIX "extern")
	else()
		set(FUNCTION_PREFIX "")
	endif()

	set(CHECK_FUNCTION_C_SOURCE "
	
	${FUNCTION_PREFIX} void ${FUNCTION}();

	int main(int argc, char** args)
	{
		${FUNCTION}();
		return 0;
	}")
	
	check_c_source_compiles("${CHECK_FUNCTION_C_SOURCE}" ${RESULT_VARIABLE})
endfunction(fftw_check_library)

# headers
# --------------------------------------------------------------------

find_path (FFTW_INCLUDES_SERIAL fftw3.h)

set(FFTW_INCLUDES ${FFTW_INCLUDES_SERIAL})
list(APPEND FFTW_REQUIRED_VARIABLES FFTW_INCLUDES_SERIAL)

# serial libraries
# --------------------------------------------------------------------

find_library(FFTW_LIBRARY_SERIAL NAMES fftw3-3 fftw3 fftw)
set(FFTW_LIBRARIES ${FFTW_LIBRARY_SERIAL})
set(FFTW_LIBRARIES_SERIAL ${FFTW_LIBRARY_SERIAL})

if(FIND_FFTW_DEBUG AND EXISTS "${FFTW_LIBRARY_SERIAL}")
	message("Found main FFTW library: ${FFTW_LIBRARY_SERIAL}")
endif()

# set up DLL compile flags (need to check type of library)
set(FFTW_IS_SHARED FALSE)
set(FFTW_DLL_IMPORT FALSE)
if(EXISTS "${FFTW_LIBRARY_SERIAL}")
	get_lib_type(${FFTW_LIBRARY_SERIAL} FFTW_LIB_TYPE)

	if(FIND_FFTW_DEBUG)
		message("Lib type: ${FFTW_LIB_TYPE}")
	endif()

	if(FFTW_LIB_TYPE STREQUAL "SHARED" OR FFTW_LIB_TYPE STREQUAL "IMPORT")
		set(FFTW_IS_SHARED TRUE)
	endif()

	if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND FFTW_LIB_TYPE STREQUAL "IMPORT")
		set(FFTW_DLL_IMPORT TRUE)
		set(FFTW_COMPILE_OPTIONS -DFFTW_DLL)
	endif()

endif()


set(CMAKE_REQUIRED_LIBRARIES ${FFTW_LIBRARY_SERIAL})
if(EXISTS "${FFTW_LIBRARY_SERIAL}")
	fftw_check_library(fftw_execute FFTW_WORKS)
else()
	set(FFTW_WORKS FALSE)
endif()

set(FFTW_REQUIRED_VARIABLES FFTW_LIBRARY_SERIAL FFTW_WORKS ${FFTW_REQUIRED_VARIABLES})


# now check if it is split-precision
set(FFTW_SPLIT FALSE)
if(FFTW_WORKS)
	fftw_check_library(fftwf_execute FFTW_SINGLE_LIB)

	if(NOT FFTW_SINGLE_LIB)
		if(FIND_FFTW_DEBUG)
			message("Split FFTW library detected")
		endif()

		set(FFTW_SPLIT TRUE)
	endif()
endif()

if(FFTW_SPLIT)

	# search for precision libraries
	if("${FFTW_FIND_COMPONENTS}" MATCHES "SinglePrecision")
		fftw_find_precision(SinglePrecision F "fftw3f-3;fftw3f;fftwf" fftwf_execute SERIAL)
	endif()

	if("${FFTW_FIND_COMPONENTS}" MATCHES "QuadPrecision")
		fftw_find_precision(QuadPrecision L "fftw3l-3;fftw3l;fftwl" fftwl_execute SERIAL)
	endif()

else()
	# Precision components already found
	set(FFTW_SinglePrecision_FOUND TRUE)
	set(FFTW_QuadPrecision_FOUND TRUE)
endif()

# Fortran component
# --------------------------------------------------------------------

if("${FFTW_FIND_COMPONENTS}" MATCHES "Fortran")

	# should exist if Fortran support is present
	set(FFTW_FORTRAN_HEADER "${FFTW_INCLUDES}/fftw3.f03")
	
	if(NOT EXISTS "${FFTW_INCLUDES_SERIAL}")
		message(STATUS "Cannot search for FFTW Fortran headers because the serial headers were not found")
		set(FFTW_Fortran_FOUND FALSE)
	elseif(EXISTS "${FFTW_FORTRAN_HEADER}")
		set(FFTW_Fortran_FOUND TRUE)
	else()
		set(FFTW_Fortran_FOUND FALSE)
		message(STATUS "Cannot find FFTW Fortran headers - ${FFTW_FORTRAN_HEADER} is missing.")
	endif()

	if(FFTW_FIND_REQUIRED_Fortran)
		list(APPEND FFTW_REQUIRED_VARIABLES FFTW_Fortran_FOUND)
	endif()

endif()

# MPI component
# --------------------------------------------------------------------
if("${FFTW_FIND_COMPONENTS}" MATCHES "MPI")
	find_library(FFTW_LIBRARY_MPI NAMES fftw3_mpi fftw_mpi)
	set(FFTW_LIBRARIES ${FFTW_LIBRARIES_MPI} ${FFTW_LIBRARIES})
	
	find_path(FFTW_INCLUDES_MPI fftw3-mpi.h)
	list(APPEND FFTW_INCLUDES ${FFTW_INCLUDES_MPI})

	set(CMAKE_REQUIRED_LIBRARIES ${FFTW_LIBRARY_MPI})
	if(EXISTS "${FFTW_LIBRARY_MPI}")
		fftw_check_library(fftw_mpi_init FFTW_MPI_WORKS)
	else()
		set(FFTW_MPI_WORKS FALSE)
	endif()

	if(FFTW_MPI_WORKS)
		set(FFTW_MPI_FOUND TRUE)
	endif()

	if(FFTW_FIND_REQUIRED_MPI)
		list(APPEND FFTW_REQUIRED_VARIABLES FFTW_LIBRARY_MPI FFTW_INCLUDES_MPI FFTW_MPI_WORKS)
	endif()

	if("${FFTW_FIND_COMPONENTS}" MATCHES "SinglePrecision")
		fftw_find_precision(SinglePrecision F "fftw3f_mpi-3;fftw3f_mpi;fftwf_mpi" fftwl_mpi_init MPI)
	endif()

	if("${FFTW_FIND_COMPONENTS}" MATCHES "QuadPrecision")
		fftw_find_precision(QuadPrecision L "fftw3l_mpi-3;fftw3l_mpi;fftwl_mpi" fftwl_mpi_init MPI)
	endif()
	
	# also look for MPI fortran header if we requested fortran and MPI
	if("${FFTW_FIND_COMPONENTS}" MATCHES "Fortran" AND FFTW_Fortran_FOUND)
	
		set(FFTW_FORTRAN_MPI_HEADER "${FFTW_INCLUDES_MPI}/fftw3-mpi.f03")
		
		# reevaluate our life choices
		if(EXISTS "${FFTW_FORTRAN_MPI_HEADER}")
			set(FFTW_Fortran_FOUND TRUE)
		else()
			set(FFTW_Fortran_FOUND FALSE)
			message(STATUS "Cannot find FFTW Fortran headers - ${FFTW_FORTRAN_MPI_HEADER} is missing")
		endif()
	endif()
	
	mark_as_advanced (FFTW_LIBRARIES_MPI FFTW_INCLUDES_MPI)
	
endif()

# handle the QUIETLY and REQUIRED arguments and set FFTW_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (FFTW REQUIRED_VARS ${FFTW_REQUIRED_VARIABLES})

mark_as_advanced(${FFTW_REQUIRED_VARIABLES})

# dump variables in debug mode
if(FIND_FFTW_DEBUG)
	message("FFTW_COMPILE_OPTIONS: ${FFTW_COMPILE_OPTIONS}")
	message("FFTW_INCLUDES: ${FFTW_INCLUDES}")
	message("FFTW_LIBRARIES: ${FFTW_LIBRARIES}")
	message("FFTW_FOUND: ${FFTW_FOUND}")
	message("FFTW_IS_SHARED: ${FFTW_IS_SHARED}")
endif()

# don't leak required libraries
set(CMAKE_REQUIRED_LIBRARIES "")
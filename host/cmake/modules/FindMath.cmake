# - Find Math
# Find the C math library.
# Results:
#  Math_LIBRARY   - Library to link to to get C math functions.  May be empty.
#  Math_FOUND     - Whether the math library was found.
#  Math_IN_STDLIB - Whether the C math functions are available without explicitly linking any libraries.

include(CheckCSourceCompiles)

# figure out if we need a math library
# we actually need to be a little careful here, because most of the math.h functions are defined as GCC intrinsics, so check_function_exists() might not find them.
# So, we use this c source instead.
set(CMAKE_REQUIRED_LIBRARIES "")
set(CHECK_SINE_C_SOURCE "#include <math.h>

int main(int argc, char** args)
{
	return sin(argc - 1);
}")

check_c_source_compiles("${CHECK_SINE_C_SOURCE}" Math_IN_STDLIB)

if(Math_IN_STDLIB)
	message(STATUS "Found math library functions in standard library.")
	set(Math_LIBRARY "")
else()
	find_library(Math_LIBRARY NAMES m math libm)
	if(EXISTS "${Math_LIBRARY}")
		set(CMAKE_REQUIRED_LIBRARIES ${Math_LIBRARY})
		check_c_source_compiles("${CHECK_SINE_C_SOURCE}" Math_LIBRARY_WORKS)
	endif()
endif()

if(Math_IN_STDLIB)
	set(Math_REQUIRED_VARS Math_IN_STDLIB)
else()
	set(Math_REQUIRED_VARS Math_LIBRARY Math_LIBRARY_WORKS)
endif()

find_package_handle_standard_args(Math
	REQUIRED_VARS ${Math_REQUIRED_VARS})
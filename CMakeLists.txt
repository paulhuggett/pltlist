cmake_minimum_required (VERSION 3.10)
project (pltlist CXX)
add_executable (pltlist
    compilation.cpp
    compilation.hpp
    fragment.hpp
    main.cpp
    symbol.cpp
    symbol.hpp
)

if (MSVC)
    target_compile_options(pltlist PUBLIC /W4)
else()
    target_compile_options(pltlist PUBLIC -Wall -Wextra -pedantic -pedantic-errors)
endif()

set_target_properties (pltlist PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED Yes
)

set (CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package (Threads)
target_link_libraries (pltlist PUBLIC ${CMAKE_THREAD_LIBS_INIT})

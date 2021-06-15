cmake_minimum_required(VERSION 3.16)
project(main)

set(CMAKE_CXX_STANDARD 14)
if(POLICY CMP0053)
    # Only set CMP0053 to OLD with CMake<3.10, otherwise there is a warning.
    if(NOT POLICY CMP0070)
        cmake_policy(SET CMP0053 OLD)
    else()
        cmake_policy(SET CMP0053 NEW)
    endif()
endif()
if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()
#CGAL_Qt5 is needed for the drawing.
find_package(CGAL REQUIRED OPTIONAL_COMPONENTS Qt5)
if(CGAL_Qt5_FOUND)
    #required to use basic_viewer
    add_definitions(-DCGAL_USE_BASIC_VIEWER -DQT_NO_KEYWORDS)
endif()
if ( CGAL_FOUND )
    #create the executable of the application
    create_single_source_cgal_program("main.cpp")
    if(CGAL_Qt5_FOUND )
        #link it with the required CGAL libraries
        target_link_libraries(main PUBLIC CGAL::CGAL_Qt5)
    endif()
else()
    message(STATUS "This program requires the CGAL library, and will not be compiled.")
endif()

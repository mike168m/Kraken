cmake_minimum_required(VERSION 3.0)


set( CMAKE_SYSTEM_NAME Generic )

#======= OPTIONS ==============================

option( BUILD_AVR  "BUILD_AVR"  OFF )
option( BUILD_X86  "BUILD_x86"  OFF )
option( BUILD_X64  "BUILD_X64"  OFF )
option( BUILD_ARM  "BUILD_ARM"  OFF )
option( BUILD_DOCS "BUILD_DOCS" OFF )

#======= END OPTIONS ==========================


#======= COMPILER SELECTION ====================

if ( BUILD_AVR ) 
    set( ARCH_TYPE          "AVR" )
    set( ENV{CC}            "/usr/bin/avr-gcc" )
    set( ENV{CXX}           "/usr/bin/avr-g++" )
    set( CMAKE_C_COMPILER   "/usr/bin/avr-gcc" )
    set( CMAKE_CXX_COMPILER "/usr/bin/avr-g++" )
elseif ( BUILD_X86 OR BUILD_X64 )
    set( ARCH_TYPE          "X86" )
    set( CMAKE_C_COMPILER   "/usr/bin/gcc" )
    set( CMAKE_CXX_COMPILER "/usr/bin/g++" )
endif( BUILD_AVR )
message( "Env $ENV{CC} $ENV{CXX}")
message( "Configured to build for ${ARCH_TYPE}")

#======= COMPILER SELECTION =====================


project( kraken )


set( CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "" )
set( CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" )


set(CMAKE_C_STANDARD 11)


include_directories(.)


add_executable( kraken_demo
                kraken.h 
                kraken_demo.c )


set( GCC_DEBUG_OPTIONS "-pg" )
set( GCC_PRINT_VERSION "-v"  )
add_compile_options( ${GCC_DEBUG_OPTIONS}
                     ${GCC_PRINT_VERSION} )

# taken from 
#:wgithub.com/mkleemann/cmake-avr/blob/master/CMakeLists.txt.sample
if ( BUILD_AVR )
    # compiler options for all build types
    add_definitions( "-DF_CPU=8000")
    add_definitions( "-fpack-struct")
    add_definitions( "-fshort-enums")
    #add_definitions("-Wall")
    #add_definitions("-Werror")
    #add_definitions("-pedantic")
    #add_definitions("-pedantic-errors")
    add_definitions( "-funsigned-char" )
    add_definitions( "-funsigned-bitfields" )
    add_definitions( "-ffunction-sections" )
    add_definitions( "-c" )
    add_definitions( "-std=gnu99" ) 
    add_definitions( "-mmcu=atmega8" )
endif()

if ( BUILD_DOCS )
    add_custom_target( KRAKEN_DOCS
                       COMMAND echo '====== building docs ====='
                       COMMAND stddoc < kraken.h > docs/kraken_doc.html
                       WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} )

    add_dependencies( kraken_demo KRAKEN_DOCS )
endif()


add_executable( kraken_test
                kraken.h
                kraken_test.c )


if ( BUILD_AVR ) 
    target_link_libraries( kraken_test "m" "c" "g" )
endif()

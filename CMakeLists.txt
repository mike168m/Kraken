cmake_minimum_required(VERSION 3.0)


project(kraken C)


set(CMAKE_C_STANDARD 11)


include_directories(.)


add_executable( kraken_demo
                kraken.h 
                kraken_demo.c )


SET( GCC_DEBUG_OPTIONS "-pg" )
add_compile_options( ${GCC_DEBUG_OPTIONS} )

if ( BUILD_DOCS )
    add_custom_target( KRAKEN_DOCS
                       COMMAND echo '====== building docs ====='
                       COMMAND stddoc < kraken.h > docs/kraken_doc.html
                       WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} )

    add_dependencies( ${PROJECT_NAME} KRAKEN_DOCS )
endif()


add_executable( kraken_test
                kraken.h
                kraken_test.c )



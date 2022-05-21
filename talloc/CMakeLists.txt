# generated file
cmake_minimum_required( VERSION 3.22 )
project(talloc)

add_definitions(-DBUILDNO=TEST)

## Install the executable to <install_directory>/bin
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
## Install libraries (if any) to <install_directory>/bin
# set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin )

file(GLOB WEB talloc.w)
message("WEB: ${WEB}")
set(TEX talloc.tex)
set(EXE talloc_test)
set(PDF talloc.pdf)

add_library(talloc  talloc.c)

add_executable( ${EXE} test.c )
target_link_libraries( ${EXE} PUBLIC talloc )
install( TARGETS ${EXE} RUNTIME DESTINATION bin )

# add_executable( httpd-min  websocket_threaded_example.c lexer.c  )
# target_link_libraries( httpd-min PUBLIC ${HTTP_LIB} ${PTHREAD_LIB} )
# install( TARGETS httpd-min RUNTIME DESTINATION bin )

# add_executable( static-files  fileserver_example.c )
# target_link_libraries( static-files PUBLIC ${HTTP_LIB} ${PTHREAD_LIB} )
# install( TARGETS static-files RUNTIME DESTINATION bin )

add_custom_command( OUTPUT config.h replace.h talloc.h talloc.c
                    COMMAND ctangle ${WEB} || true
                    DEPENDS ${WEB} )
add_custom_command( OUTPUT ${TEX} 
                    COMMAND cweave ${WEB} || true
                    DEPENDS ${WEB})
add_custom_command( OUTPUT ${PDF} 
                    COMMAND pdftex ${TEX} 
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/doc
                    COMMAND ${CMAKE_COMMAND} -E copy ${PDF} ${PROJECT_SOURCE_DIR}/doc
                    DEPENDS ${TEX} )
add_custom_target( doc ALL DEPENDS ${PDF} )

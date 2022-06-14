cmake_minimum_required(VERSION 3.23)

project(tt VERSION 1.0.0)
# set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
find_package(Doxygen
             REQUIRED dot
             # OPTIONAL_COMPONENTS mscgen dia
             )
file(   COPY 
            doc/footer.html 
            doc/head.tex 
            doc/foot.tex 
            doc/style.tex 
        DESTINATION 
            ${CMAKE_CURRENT_BINARY_DIR}/doc
)
include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/talloc
                    ${CMAKE_CURRENT_BINARY_DIR})

add_compile_definitions( __STDC_WANT_LIB_EXT1__=1 )

set(PARSER ${PROJECT_SOURCE_DIR}/tt.y)

add_compile_options( -g -save-temps )

add_library(ttl     class_enter.c
                    lib.c
                    tt.c
                    talloc/talloc.c
                    namelist.c
                    method_enter.c
                    method_name.c
                    method_stmts.c
                    require_classes.c
                    require_current_class.c )

add_executable(ttc  ttc.c)
target_link_libraries(ttc ttl)

add_custom_command( OUTPUT tt.c
    COMMAND lemon -d${CMAKE_CURRENT_BINARY_DIR} -T${PROJECT_SOURCE_DIR}/lempar.c ${PARSER} 
    DEPENDS ${PARSER} )


add_executable(tt_test tt_test.c)
target_link_libraries(tt_test ttl)


#file(COPY mk-refman DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${PROJECT_SOURCE_DIR}/doxy.in doxy.cfg )
file(   GENERATE 
        OUTPUT 
            mk-refman
        CONTENT 
"cd ${CMAKE_CURRENT_BINARY_DIR}/doc/latex

echo \"===== \$PWD\"
pdflatex refman
pdflatex refman"
        FILE_PERMISSIONS 
            OWNER_READ 
            OWNER_WRITE 
            OWNER_EXECUTE
)

add_custom_target( refman 
    ${CMAKE} -E copy 
                doc/footer.html 
                doc/head.tex 
                doc/foot.tex 
                doc/style.tex 
                ${CMAKE_CURRENT_BINARY_DIR}/doc
    COMMAND doxygen doxy.cfg 
    COMMAND ./mk-refman "${CMAKE_CURRENT_BINARY_DIR}"
    )
#doxygen_add_docs(refman)

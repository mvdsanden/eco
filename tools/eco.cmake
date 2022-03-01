include(GoogleTest)

function(eco_add_submodule)

set(NAME ${arg1})

file(GLOB_RECURSE testSources "*.test.cpp")
file(GLOB_RECURSE sources "*.cpp")
list(FILTER sources EXCLUDE ".+\.test\.cpp")

add_library(${NAME} ${sources})

foreach(testSource ${testSources})  
get_filename_component(component ${testSource} NAME_WE)
set(target "${component}.tsk")
add_executable(${target} ${testSource})
add_test(NAME ${testSource} COMMAND ${target})
target_include_directories(${target} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(${target} ${NAME})
endforeach(testSource)

endfunction(eco_add_submodule)

function(eco_add_library PATH)
    get_filename_component(DIRECTORY ${PATH} DIRECTORY)
    get_filename_component(NAME ${DIRECTORY} NAME)

    message("Adding library ${PATH} -- ${NAME}.")

    file(GLOB_RECURSE ECO_TEST_SOURCES "${DIRECTORY}/src/*.test.cpp")
    file(GLOB_RECURSE ECO_SOURCES "${DIRECTORY}/src/*.cpp")
    list(FILTER ECO_SOURCES EXCLUDE REGEX ".+\.test\.cpp")

    message("Sources: ${ECO_SOURCES}.")
    message("Test sources: ${ECO_TEST_SOURCES}.")

    set(ECO_DEPENDENCIES "")
    include(${PATH})

    message("Dependencies: ${ECO_DEPENDENCIES}.")
    message("Directory: ${DIRECTORY}.")

    add_library(${NAME} ${ECO_SOURCES})
    target_include_directories(${NAME} PUBLIC "${DIRECTORY}/src/")
    target_link_libraries(${NAME} ${ECO_DEPENDENCIES})
    #set_target_properties(${NAME} CXX_STANDARD 17)

    foreach(testSource ${ECO_TEST_SOURCES})
        get_filename_component(TEST_DIRECTORY ${testSource} DIRECTORY)
        file(RELATIVE_PATH RELATIVE_TEST_DIRECTORY "${DIRECTORY}/src" "${TEST_DIRECTORY}")
        get_filename_component(component ${testSource} NAME_WE)
        set(TEST_NAME "${NAME}_${RELATIVE_TEST_DIRECTORY}_${component}.tsk")
        message("Add test ${TEST_NAME} ${DIRECTORY}/src")
        add_executable(${TEST_NAME} ${testSource})
        target_include_directories(${TEST_NAME} PUBLIC "${DIRECTORY}/src/")
        target_link_libraries(${TEST_NAME} ${NAME} gtest_main gmock_main)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        gtest_discover_tests(${TEST_NAME})
    endforeach(testSource)

endfunction(eco_add_library)



function(eco_project_add_libs)
    
    file(GLOB libraries "libs/*/build.cmake")
    message("Libs: ${libraries}")

    foreach(library ${libraries})
        eco_add_library(${library})    
    endforeach(library ${libraries})
    

endfunction(eco_project_add_libs)



function(eco_project)

eco_project_add_libs()

endfunction(eco_project)

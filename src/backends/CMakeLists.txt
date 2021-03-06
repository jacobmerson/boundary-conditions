
option(YAML-CPP "Enables yaml-cpp backend. Requires yaml-cpp." ON)
if (YAML-CPP)
    find_package(yaml-cpp REQUIRED)
    set(SOURCES ${SOURCES} bcBackendYamlCpp.cc bcBackendYamlCppConvert.cc)
endif ()

if (ENABLE_SIMMETRIX)
    find_package(SIMMODSUITE MODULE REQUIRED)
    set(SOURCES ${SOURCES} bcBackendSimmetrix.cc bcBackendSimmetrixConvert.cc)
endif ()

add_library(bcIOBackend ${SOURCES})
target_link_libraries(bcIOBackend PUBLIC modelTraits)

if (YAML-CPP)
    target_include_directories(bcIOBackend PRIVATE {CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(bcIOBackend PRIVATE yaml-cpp)
endif ()
if (ENABLE_SIMMETRIX)
    target_link_libraries(bcIOBackend PRIVATE ${SIMMODSUITE_LIBS})
    target_include_directories(bcIOBackend PRIVATE ${SIMMODSUITE_INCLUDE_DIR})
    target_compile_definitions(bcIOBackend PRIVATE -DBC_HAVE_SIMMETRIX)
endif ()

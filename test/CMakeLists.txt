find_package(Catch2 3 REQUIRED)

add_executable(test_model_traits
               test_category_node.cc
               test_model_traits.cc)
target_link_libraries(test_model_traits Catch2::Catch2WithMain modelTraits)

add_executable(generic_test generic_test.cc)
target_link_libraries(generic_test modelTraits)

add_executable(read_yaml test_reader.cc)
target_link_libraries(read_yaml modelTraits bcBackendYamlCpp)

find_package(fmt REQUIRED)
add_executable(exprtk_test exprtk_test.cc)
target_link_libraries(exprtk_test exprtk fmt::fmt)

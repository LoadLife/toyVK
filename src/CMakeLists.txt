add_executable(main main.cpp)
target_link_libraries(main PUBLIC ${PROJECT_SOURCE_DIR}/third_part/libs/libglfw3.a
                                  ${PROJECT_SOURCE_DIR}/third_part/libs/vulkan-1.lib)
target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/third_part/include)
   

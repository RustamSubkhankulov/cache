set(SRC src)
set(INC inc)
set(CNF conf)

macro(TEST_TARGET TEST)
  add_executable(${TEST} ${SRC}/${TEST}.cpp)
  set_target_properties(
        ${TEST} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
  target_link_libraries(${TEST} PRIVATE cache)
  target_include_directories(${TEST} PRIVATE ${INC})
endmacro()

TEST_TARGET(interactive)
TEST_TARGET(auto)

option(GEN_EXTRA "Enables additional testing" OFF)
if (GEN_EXTRA)
        if (NOT DEFINED GEN_EXTRA_NUM)
                set(GEN_EXTRA_NUM 1)
        endif()
else()
        set(GEN_EXTRA_NUM 0)
endif()

configure_file(${CNF}/auto_conf.hpp.in auto_conf.hpp @ONLY)
target_include_directories(auto PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

find_package (Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories (${TEST_SOURCE_DIR}/src
                     ${Boost_INCLUDE_DIRS}
                     )
add_definitions (-DBOOST_TEST_DYN_LINK)
set(sources
  main_test.cpp
  test_geom.cpp)

set(libHeaders
   ../lib/BasicTypes.h
   ../lib/BitLib.h
   ../lib/GeomLib.h
   ../lib/NumLib.h
   ../lib/StrLib.h
   ../lib/StrStrmLib.h
   ../lib/VectorLib.h)
source_group("Lib Headers" FILES ${libHeaders})

add_executable (Test ${sources} ${libHeaders})

target_link_libraries (Test
                       ${Boost_FILESYSTEM_LIBRARY}
                       ${Boost_SYSTEM_LIBRARY}
                       ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                       )
add_test (NAME MyTest COMMAND Test)
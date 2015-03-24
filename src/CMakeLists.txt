project(libnss_wrapper C)

include_directories(${CMAKE_BINARY_DIR})
add_library(nss_wrapper SHARED nss_wrapper.c)
target_link_libraries(nss_wrapper ${NWRAP_REQUIRED_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

if (BSD)
    add_definitions(-DBSD)
endif (BSD)

set_target_properties(
  nss_wrapper
    PROPERTIES
      VERSION
        ${LIBRARY_VERSION}
      SOVERSION
        ${LIBRARY_SOVERSION}
)

install(
  TARGETS
    nss_wrapper
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

# This needs to be at the end
if (POLICY CMP0026)
    cmake_policy(SET CMP0026 OLD)
endif()
get_target_property(NWRAP_LOCATION nss_wrapper LOCATION)
set(NSS_WRAPPER_LOCATION ${NWRAP_LOCATION} PARENT_SCOPE)

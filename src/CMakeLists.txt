project(libnss_wrapper C)

include_directories(${CMAKE_BINARY_DIR})
add_library(nss_wrapper SHARED nss_wrapper.c)
target_link_libraries(nss_wrapper ${NSSWRAP_REQUIRED_LIBRARIES})

install(
  TARGETS
    nss_wrapper
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

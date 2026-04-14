#==============================================================================
foreach(lvar IN ITEMS HbdMonitor;)
  set(EXEC ${lvar})
  add_executable(${EXEC}
    ${EXEC}.cxx;
  )

  target_include_directories(${EXEC} PUBLIC
    ${Boost_INCLUDE_DIRS};
    ${FairLogger_INCDIR};
    ${FairMQ_INCDIR};
    ${CMAKE_SOURCE_DIR};
#    ${CMAKE_CURRENT_SOURCE_DIR}/mydecoder;
    ${CMAKE_SOURCE_DIR}/unpacker
  )

  target_link_directories(${EXEC} PUBLIC
    ${Boost_LIBRARY_DIRS};
    ${FairLogger_LIBDIR};
    ${FairMQ_LIBDIR};
  )

  if ((FairMQ_VERSION VERSION_GREATER_EQUAL 1.8.0)
    AND (FairMQ_VERSION VERSION_LESS_EQUAL 1.8.9))
    target_link_libraries(${EXEC} PUBLIC
      ${Boost_LIBRARIES};
      ${fmt_LIB};
      FairLogger;
      fairmq;
#      mydecoder;
      NestdaqUnpacker;
    )
  elseif ((FairMQ_VERSION VERSION_GREATER_EQUAL 1.4.55)
    AND (FairMQ_VERSION VERSION_LESS_EQUAL 1.4.56))
    target_link_libraries(${EXEC} PUBLIC
      ${Boost_LIBRARIES};
      ${fmt_LIB};
      FairLogger;
      FairMQ;
#      mydecoder;
      NestdaqUnpacker;
    )
  else()
    message(FATAL_ERROR "Unsupported FairMQ version ${FairMQ_VERSION}")
  endif()

  install(TARGETS
    ${EXEC};
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endforeach()

#add_subdirectory(mydecoder)

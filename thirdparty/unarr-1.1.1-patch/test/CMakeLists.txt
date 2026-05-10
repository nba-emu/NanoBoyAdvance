# project (unarr-test C)

include(ProcessorCount)
ProcessorCount(N)

if(BUILD_FUZZER)
  add_executable(fuzzer fuzzer.c)
  set_target_properties(fuzzer PROPERTIES LINK_FLAGS "${sanitize_opts}")
  target_link_libraries(fuzzer unarr)

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/corpus)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/corpus/fuzzed)

  add_test(NAME fuzzer_test
           COMMAND fuzzer ${CMAKE_CURRENT_SOURCE_DIR}/corpus/fuzzed
                   ${CMAKE_CURRENT_SOURCE_DIR}/corpus -jobs=${N})
endif()

if(BUILD_INTEGRATION_TESTS)
  add_executable(unarr-test main.c)
  target_link_libraries(unarr-test unarr)

  function(run_test file)
    add_test(NAME ${file} COMMAND unarr-test
                                  ${CMAKE_CURRENT_SOURCE_DIR}/corpus/${file})
    set_tests_properties(
      ${file}
      PROPERTIES ENVIRONMENT
                 "PATH=${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE};$ENV{PATH}")
  endfunction(run_test file)

  # Rar
  run_test(integration/lipsum_rar4_store.rar)
  run_test(integration/lipsum_rar4_default.rar)
  run_test(integration/lipsum_rar4_max.rar)
  # Non-standard compression methods
  run_test(integration/lipsum_rar4_ppmd.rar) # Text
  run_test(integration/lipsum_rar4_delta.rar)
  run_test(integration/silent_rar4_audio.rar)
  # TODO bmp - RGB color (bmp?)

  # Tar
  run_test(integration/lipsum.tar)

  # Zip
  run_test(integration/lipsum_zip_copy.zip)
  run_test(integration/lipsum_zip_default.zip)
  run_test(integration/lipsum_zip_max.zip)
  # Non-standard compression methods
  if(BZIP2_FOUND)
    run_test(integration/lipsum_zip_bzip2.zip)
  endif()
  run_test(integration/lipsum_zip_deflate64.zip)
  run_test(integration/lipsum_zip_lzma1.zip)
  run_test(integration/lipsum_zip_ppmd.zip)
  if(LIBLZMA_FOUND)
    run_test(integration/lipsum_zip_xz.zip)
  endif()

  # 7z
  if(ENABLE_7Z)
    run_test(integration/lipsum_7z_store.7z)
    run_test(integration/lipsum_7z_default.7z)
    run_test(integration/lipsum_7z_max.7z)
    # Non-standard compression methods
    run_test(integration/lipsum_7z_lzma1.7z)
    run_test(integration/lipsum_7z_ppmd.7z)
  endif()
endif()

if(BUILD_UNIT_TESTS)
  find_package(cmocka CONFIG REQUIRED)
  add_executable(crc32-test crc32-test.c)
  if(TARGET cmocka::cmocka)
    target_link_libraries(crc32-test PRIVATE cmocka::cmocka)
  else()
    target_include_directories(crc32-test PRIVATE ${CMOCKA_INCLUDE_DIR})
    target_link_libraries(crc32-test PRIVATE ${CMOCKA_LIBRARY})
  endif()
  add_test(crc32 crc32-test)
endif()

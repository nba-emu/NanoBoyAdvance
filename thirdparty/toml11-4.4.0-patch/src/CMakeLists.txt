set(TOML11_FWD_HEADERS
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/color_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/comments_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/datetime_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/error_info_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/format_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/literal_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/location_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/region_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/scanner_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/source_location_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/syntax_fwd.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/fwd/value_t_fwd.hpp
    )
set(TOML11_IMPL_HEADERS
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/color_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/comments_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/datetime_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/error_info_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/format_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/literal_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/location_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/region_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/scanner_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/source_location_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/syntax_impl.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/impl/value_t_impl.hpp
    )
set(TOML11_MAIN_HEADERS
    ${PROJECT_SOURCE_DIR}/include/toml11/color.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/comments.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/compat.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/context.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/conversion.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/datetime.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/error_info.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/exception.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/find.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/format.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/from.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/get.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/into.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/literal.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/location.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/ordered_map.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/parser.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/region.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/result.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/scanner.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/serializer.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/skip.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/source_location.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/spec.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/storage.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/syntax.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/traits.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/types.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/utility.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/value.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/value_t.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/version.hpp
    ${PROJECT_SOURCE_DIR}/include/toml11/visit.hpp
    )

set(TOML11_ROOT_HEADER
    ${PROJECT_SOURCE_DIR}/include/toml.hpp
    )

if(TOML11_PRECOMPILE)
    add_library(toml11
        ${TOML11_FWD_HEADERS}
        ${TOML11_IMPL_HEADERS}
        ${TOML11_MAIN_HEADERS}
        ${TOML11_ROOT_HEADER}
        color.cpp
        context.cpp
        comments.cpp
        datetime.cpp
        error_info.cpp
        format.cpp
        literal.cpp
        location.cpp
        parser.cpp
        region.cpp
        scanner.cpp
        serializer.cpp
        skip.cpp
        source_location.cpp
        syntax.cpp
        types.cpp
        value_t.cpp
        )
    target_compile_definitions(toml11 PUBLIC
        -DTOML11_COMPILE_SOURCES
        $<$<BOOL:${TOML11_ENABLE_ACCESS_CHECK}>: -DTOML11_ENABLE_ACCESS_CHECK>
        )
    target_include_directories(toml11 PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

    # required options to use toml11 with MSVC
    if(MSVC)
        target_compile_options(toml11 PUBLIC "/utf-8")
        target_compile_options(toml11 PRIVATE "/W4" "/WX")
        if(MSVC_VERSION LESS 1910)
            message(STATUS "MSVC < 1910. DEFINE_CONVERSION_NON_INTRUSIVE is disabled")
            target_compile_definitions(toml11 PUBLIC -DTOML11_WITHOUT_DEFINE_NON_INTRUSIVE)
        elseif(MSVC_VERSION LESS 1920) # MSVC 2017
            target_compile_options(toml11 PUBLIC "/experimental:preprocessor")
        else() # MSVC 2019
            target_compile_options(toml11 PUBLIC "/Zc:preprocessor")
        endif()
    else()
        target_compile_options(toml11 PRIVATE
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WALL}>:                 -Wall                >
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WEXTRA}>:               -Wextra              >
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WPEDANTIC}>:            -Wpedantic           >
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WERROR}>:               -Werror              >
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WSIGN_CONVERSION}>:     -Wsign-conversion    >
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WCONVERSION}>:          -Wconversion         >
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WDUPLICATED_COND}>:     -Wduplicated-cond    >
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WDUPLICATED_BRANCHES}>: -Wduplicated-branches>
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WLOGICAL_OP}>:          -Wlogical-op         >
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WDOUBLE_PROMOTION}>:    -Wdouble-promotion   >
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WRANGE_LOOP_ANALYSIS}>: -Wrange-loop-analysis>
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WUNDEF}>:               -Wundef              >
            $<$<BOOL:${TOML11_COMPILER_SUPPORTS_WSHADOW}>:              -Wshadow             >
        )
    endif()
else()
    add_library(toml11 INTERFACE)
    target_compile_definitions(toml11 INTERFACE
        $<$<BOOL:${TOML11_ENABLE_ACCESS_CHECK}>: -DTOML11_ENABLE_ACCESS_CHECK>
        )
    target_include_directories(toml11 INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

    # required options to use toml11 with MSVC
    if(MSVC)
        target_compile_options(toml11 INTERFACE "/utf-8")
        if(MSVC_VERSION LESS 1910)
            message(STATUS "MSVC < 1910. DEFINE_CONVERSION_NON_INTRUSIVE is disabled")
            target_compile_definitions(toml11 PUBLIC -DTOML11_WITHOUT_DEFINE_NON_INTRUSIVE)
        elseif(MSVC_VERSION LESS 1920) # MSVC 2017
            target_compile_options(toml11 INTERFACE "/experimental:preprocessor")
        else() # MSVC 2019
            target_compile_options(toml11 INTERFACE "/Zc:preprocessor")
        endif()
    endif()
endif()


if(TOML11_INSTALL)

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(${TOML11_CONFIG_VERSION}
        VERSION ${toml11_VERSION}
        COMPATIBILITY SameMajorVersion
        )

    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/toml11Config.cmake.in
        ${TOML11_CONFIG}
        INSTALL_DESTINATION ${TOML11_INSTALL_CMAKE_DIR}
        PATH_VARS TOML11_INSTALL_CMAKE_DIR TOML11_INSTALL_INCLUDE_DIR
        )

    install(FILES ${TOML11_CONFIG} ${TOML11_CONFIG_VERSION}
        DESTINATION ${TOML11_INSTALL_CMAKE_DIR})

    install(FILES ${TOML11_ROOT_HEADER}
        DESTINATION ${TOML11_INSTALL_INCLUDE_DIR}
        )
    install(FILES ${TOML11_MAIN_HEADERS}
        DESTINATION ${TOML11_INSTALL_INCLUDE_DIR}/toml11
        )
    install(FILES ${TOML11_FWD_HEADERS}
        DESTINATION ${TOML11_INSTALL_INCLUDE_DIR}/toml11/fwd
        )

    install(FILES ${TOML11_IMPL_HEADERS}
        DESTINATION ${TOML11_INSTALL_INCLUDE_DIR}/toml11/impl
        )

    install(TARGETS toml11 EXPORT toml11Targets)
    install(EXPORT toml11Targets
        FILE toml11Targets.cmake
        DESTINATION ${TOML11_INSTALL_CMAKE_DIR}
        NAMESPACE toml11::
        )
endif()

add_library(toml11::toml11 ALIAS toml11)

target_compile_options(toml11 INTERFACE -Wno-stringop-overflow -Wno-unknown-warning-option)
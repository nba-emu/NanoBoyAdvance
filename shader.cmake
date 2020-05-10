file(GLOB_RECURSE shader_files RELATIVE "${SRCDIR}/" "${SRCDIR}/*")

message(STATUS ${shader_files})

foreach(file IN LISTS shader_files)
    configure_file(${SRCDIR}/${file} ${DSTDIR}/${file} COPYONLY)
endforeach()
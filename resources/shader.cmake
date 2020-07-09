file(GLOB_RECURSE shader_files RELATIVE "${SRCDIR}/" "${SRCDIR}/*")

foreach(file IN LISTS shader_files)
    configure_file(${SRCDIR}/${file} ${DSTDIR}/${file} COPYONLY)
endforeach()

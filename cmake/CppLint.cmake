find_package(PythonInterp)

set(STYLE_FILTER)
#set(STYLE_FILTER "${STYLE_FILTER}-build/include_subdir,")
set(STYLE_FILTER "${STYLE_FILTER}-build/include_order,")
set(STYLE_FILTER "${STYLE_FILTER}-build/c++11,")

function(add_cpplint_check)
  cmake_parse_arguments(
    cpplint_args
    ""  # boolean
    "TARGET"  # one value
    "FILES"  # multiple value
    ${ARGN}
  )
  if(NOT PYTHONINTERP_FOUND)
    message(WARNING "No python interpretor found. Skip cpplint check.")
    return()
  endif()

  set(target ${cpplint_args_TARGET}_cpplint)

  add_custom_target(
    ${target}
    COMMAND "${CMAKE_COMMAND}" -E chdir
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${PYTHON_EXECUTABLE}"
            "${CMAKE_SOURCE_DIR}/cmake/misc/cpplint.py"
            "--filter=${STYLE_FILTER}"
            "--extensions=cpp,hpp,cc,h,c"
            "--linelength=80"
            ${cpplint_args_FILES}
    DEPENDS ${SOURCES_LIST}
    COMMENT "Cpplint: ${cpplint_args_TARGET}..."
    VERBATIM)

  add_dependencies(${cpplint_args_TARGET} ${target})
endfunction()

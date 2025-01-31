cmake_minimum_required(VERSION 3.15)


file(GLOB_RECURSE REFLECT_SOURCE
    "src/*.h"
    "src/*.cpp"
)

set(HEADER_FILE "${CMAKE_SOURCE_DIR}/src/utils/reflect_directories.h")

function(extract_p_dir_from_file input_file output_var)
    set(${output_var} "default" PARENT_SCOPE)
    get_filename_component(parent_dir "${input_file}" DIRECTORY)
    string(REPLACE "/" ";" dir_parts "${parent_dir}")
    list(REVERSE dir_parts)
    foreach(part IN LISTS dir_parts)
        if(part MATCHES "reflect")
            return()
        endif()
        if(part MATCHES "${NVTX_FOLDER_REGEXP}")
            set(${output_var} "${part}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

file(WRITE ${HEADER_FILE} "#pragma once\n#include <array>\n#include <string_view>\n#include <nvtx/nvtx3.hpp>\n\n// This file is generated automatically, you should not modify it unless you know what you are doing.\n")

list(LENGTH REFLECT_SOURCE FILE_COUNT)

file(APPEND ${HEADER_FILE} "constexpr std::array<std::pair<const char*, const char*>, ${FILE_COUNT}> lookup_table{{\n")

set(DISTINCT_DIR_NAMES "")

foreach(file IN LISTS REFLECT_SOURCE)
    extract_p_dir_from_file("${file}" dir_name)
    if(dir_name)
        if(WIN32)
            string(REPLACE "/" "\\\\" file_path "${file}") 
        endif()
        file(APPEND ${HEADER_FILE} "{\"${file_path}\",\"${dir_name}\"},\n")
    endif()
    if(dir_name AND NOT "${dir_name}" STREQUAL "default")
        list(FIND DISTINCT_DIR_NAMES "${dir_name}" index)
        if(index EQUAL -1)
            list(APPEND DISTINCT_DIR_NAMES "${dir_name}")
        endif()
    endif()
endforeach()
file(APPEND ${HEADER_FILE} "}}; \n")
# foreach(dir IN LISTS DISTINCT_DIR_NAMES)
#     file(APPEND ${HEADER_FILE} "nvtxDomainHandle_t ${dir}_domain = nvtxDomainCreateA(\"${dir}\");\n")
# endforeach()
# file(APPEND ${HEADER_FILE} "nvtxDomainHandle_t default_domain = nvtxDomainCreateA(\"reflect\");\n")
# file(APPEND ${HEADER_FILE} "#define GET_NVTX_DOMAIN(FILE)\\\n")
# foreach(dir IN LISTS DISTINCT_DIR_NAMES)
#     file(APPEND ${HEADER_FILE} "(std::string_view(find_file_category(FILE)) == \"${dir}\" ? &${dir}_domain :\\\n")
# endforeach()
# file(APPEND ${HEADER_FILE} "&default_domain ")
# foreach(dir IN LISTS DISTINCT_DIR_NAMES)
#     file(APPEND ${HEADER_FILE} ")")
# endforeach()

foreach(dir IN LISTS DISTINCT_DIR_NAMES)
    file(APPEND ${HEADER_FILE} "struct ${dir}_domain { static constexpr char const* name{ \"${dir}\" }; };\n")
endforeach()
file(APPEND ${HEADER_FILE} "struct default_domain { static constexpr char const* name{ \"default\" }; };\n")
file(APPEND ${HEADER_FILE} "#define GET_NVTX_DOMAIN(FILE) ")
foreach(dir IN LISTS DISTINCT_DIR_NAMES)
    file(APPEND ${HEADER_FILE} "std::conditional_t<(std::string_view(find_file_category(FILE)) == \"${dir}\"),  ${dir}_domain, ")
endforeach()
file(APPEND ${HEADER_FILE} "default_domain")
foreach(dir IN LISTS DISTINCT_DIR_NAMES)
    file(APPEND ${HEADER_FILE} ">")
endforeach()

file(APPEND ${HEADER_FILE} "
constexpr const char * find_file_category(std::string_view key) {
    for (const auto& [k, v] : lookup_table) {
        if (k == key) return v;
    }
    return \"default\";
}\n")
    
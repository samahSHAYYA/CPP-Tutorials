# Adding subdirectories
set(SUB_DIRS test_type_traits_extensions test_node test_tree)

foreach(SUB_DIR ${SUB_DIRS})
    add_subdirectory(${SUB_DIR})
endforeach()

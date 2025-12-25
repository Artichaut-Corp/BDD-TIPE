pybind11_add_module(py_bddtipe python_bindings.cpp)
target_link_libraries(py_bddtipe PRIVATE TIPE-BDD_lib)

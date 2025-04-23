#include <pybind11/pybind11.h>
#include <raygpu.h>
namespace py = pybind11;
int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(pybindtest, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("add", &add, "A function that adds two numbers");
    m.def("InitWindow", &InitWindow, "A function that adds two numbers");
    
}
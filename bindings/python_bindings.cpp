#include "database.h"
#include "pybind11/pybind11.h"

namespace py = pybind11;

PYBIND11_MODULE(py_bddtipe, m, py::mod_gil_not_used())
{

    py::class_<Database::DatabaseSetting>(m, "DatabaseSetting")
        .def(py::init<>())
        //.def(py::init<const std::string&, const std::string&>())
        .def(py::init<const std::string&, bool, uint8_t,
            bool,
            bool,
            bool,
            bool,
            int,
            int>());

    py::class_<Database::DatabaseEngine>(m, "DatabaseEngine")
        .def(py::init<>())
        .def("Init", &Database::DatabaseEngine::Init)
        .def("Run", &Database::DatabaseEngine::Run)
        .def("Exec", &Database::DatabaseEngine::Exec)
        .def("InsertCsvData", &Database::DatabaseEngine::InsertCsvData)
        ;
}

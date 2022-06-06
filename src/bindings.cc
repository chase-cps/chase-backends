#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "chase-core.hh"

#include "GR1CPrinter.hh"
#include "SlugsPrinter.hh"
#include "NuSMVPrinter.hh"

namespace py = pybind11;
using namespace chase;

template <typename... Args>
using overload_cast_ = pybind11::detail::overload_cast_impl<Args...>;

PYBIND11_MODULE(pychase_backends, m) {
    m.doc() = "CHASE Python wrapper module for the base backends.";

    /**
     * BACKENDS BINDINGS
     */  

    //GR1C Printer
    py::class_<GR1CPrinter, std::unique_ptr<GR1CPrinter,
        py::nodelete>, GuideVisitor>(m, "GR1CPrinter")
            .def(py::init<>())
            .def("print", &GR1CPrinter::print,
                    py::arg("contract").none(false),
                    py::arg("path").none(false))
            .def("visitIntegerValue", 
                    &GR1CPrinter::visitIntegerValue,
                    py::arg("o").none(false))
            .def("visitExpression", 
                    &GR1CPrinter::visitExpression,
                    py::arg("o").none(false))
            .def("visitIdentifier", 
                    &GR1CPrinter::visitIdentifier,
                    py::arg("o").none(false))
            .def("visitProposition", 
                    &GR1CPrinter::visitProposition,
                    py::arg("o").none(false))
            .def("visitBooleanConstant", 
                    &GR1CPrinter::visitBooleanConstant,
                    py::arg("o").none(false))
            .def("visitBinaryBooleanOperation", 
                    &GR1CPrinter::visitBinaryBooleanOperation,
                    py::arg("o").none(false))
            .def("visitUnaryBooleanOperation", 
                    &GR1CPrinter::visitUnaryBooleanOperation,
                    py::arg("o").none(false))
            .def("visitUnaryTemporalOperation", 
                    &GR1CPrinter::visitUnaryTemporalOperation,
                    py::arg("o").none(false))
            .def("visitBinaryTemporalOperation", 
                    &GR1CPrinter::visitBinaryTemporalOperation,
                    py::arg("o").none(false));

    //Slugs Printer
    py::class_<SlugsPrinter, std::unique_ptr<SlugsPrinter,
        py::nodelete>, GuideVisitor>(m, "SlugsPrinter")
            .def(py::init<>())
            .def("print", &SlugsPrinter::print,
                    py::arg("contract").none(false),
                    py::arg("path").none(false))
            .def("visitIntegerValue", 
                    &SlugsPrinter::visitIntegerValue,
                    py::arg("o").none(false))
            .def("visitExpression", 
                    &SlugsPrinter::visitExpression,
                    py::arg("o").none(false))
            .def("visitIdentifier", 
                    &SlugsPrinter::visitIdentifier,
                    py::arg("o").none(false))
            .def("visitProposition", 
                    &SlugsPrinter::visitProposition,
                    py::arg("o").none(false))
            .def("visitBooleanConstant", 
                    &SlugsPrinter::visitBooleanConstant,
                    py::arg("o").none(false))
            .def("visitBinaryBooleanOperation", 
                    &SlugsPrinter::visitBinaryBooleanOperation,
                    py::arg("o").none(false))
            .def("visitUnaryBooleanOperation", 
                    &SlugsPrinter::visitUnaryBooleanOperation,
                    py::arg("o").none(false))
            .def("visitUnaryTemporalOperation", 
                    &SlugsPrinter::visitUnaryTemporalOperation,
                    py::arg("o").none(false))
            .def("visitBinaryTemporalOperation", 
                    &SlugsPrinter::visitBinaryTemporalOperation,
                    py::arg("o").none(false));

    /*
     * BACKEND BINDING - VERIFICATION
     */
    py::class_<NuSMVPrinter, std::unique_ptr<NuSMVPrinter,
        py::nodelete>, GuideVisitor>(m, "NuSMVPrinter")
            .def(py::init<std::string &>())
            .def("print", &NuSMVPrinter::print,
                    py::arg("contract").none(false))
            .def("printDeclarations", &NuSMVPrinter::printDeclarations)
            .def("printAssumptions", &NuSMVPrinter::printAssumptions)
            .def("printGuarantees", &NuSMVPrinter::printGuarantees)
            .def("visitIntegerValue", &NuSMVPrinter::visitIntegerValue,
                    py::arg("o").none(false))
            .def("visitExpression", &NuSMVPrinter::visitExpression,
                    py::arg("o").none(false))
            .def("visitIdentifier",
                    &NuSMVPrinter::visitIdentifier,
                    py::arg("o").none(false))
            .def("visitProposition",
                    &NuSMVPrinter::visitProposition,
                    py::arg("o").none(false))
            .def("visitBooleanConstant",
                    &NuSMVPrinter::visitBooleanConstant,
                    py::arg("o").none(false))
            .def("visitBinaryBooleanOperation",
                    &NuSMVPrinter::visitBinaryBooleanOperation,
                    py::arg("o").none(false))
            .def("visitUnaryBooleanOperation",
                    &NuSMVPrinter::visitUnaryBooleanOperation,
                    py::arg("o").none(false))
            .def("visitUnaryTemporalOperation",
                    &NuSMVPrinter::visitUnaryTemporalOperation,
                    py::arg("o").none(false))
            .def("visitBinaryTemporalOperation",
                    &NuSMVPrinter::visitBinaryTemporalOperation,
                    py::arg("o").none(false));


}


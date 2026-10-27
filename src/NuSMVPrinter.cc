/**
 * @author      <a href="mailto:michele.lora@univr.it">Michele Lora</a>
 * @date        11/16/2020
 *              This project is released under the 3-Clause BSD License.
 *
 */


#include "NuSMVPrinter.hh"

using namespace chase;

namespace {
    // Finds the real types and the real numbers in what it visits.
    class RealFinder : public GuideVisitor {
    public:
        bool found = false;
        int visitReal(Real &) override { found = true; return 1; }
        int visitRealValue(RealValue &) override { found = true; return 1; }
    };
}

bool chase::needsNuXmv(Contract *contract)
{
    RealFinder finder;
    for (auto & declaration : contract->declarations)
        declaration->accept_visitor(finder);
    for (auto & spec : contract->assumptions)
        spec.second->accept_visitor(finder);
    for (auto & spec : contract->guarantees)
        spec.second->accept_visitor(finder);
    return finder.found;
}

NuSMVPrinter::NuSMVPrinter( std::string path ) :
    GuideVisitor(),
    _contract(nullptr),
    _reals(false),
    _realVariable(false)
{
    _fout.open(path);
}

NuSMVPrinter::~NuSMVPrinter()
{
    if(_fout.is_open()) _fout.close();
}

void NuSMVPrinter::print(Contract *contract)
{
    _contract = contract;
    _reals = needsNuXmv(contract);
    _realVariable = false;
    if (_reals)
        _fout << "-- Real variables or real numbers: NuSMV cannot read this file.\n"
              << "-- Check it with nuXmv: read_model, go_msat, check_ltlspec_ic3\n"
              << "-- (or msat_check_ltlspec_bmc -k 20).\n";
    printDeclarations();
    printAssumptions();
    printGuarantees();
}

void NuSMVPrinter::printDeclarations() {
    _fout << "MODULE main" << std::endl;
    _fout << "VAR" << std::endl;
    std::vector<Constant *> constants;
    for (auto & declaration : _contract->declarations)
    {
        if( declaration->IsA() == constant_node )
            constants.push_back(reinterpret_cast<Constant*>(declaration));
        if( declaration->IsA() != variable_node ) continue;
        auto var = reinterpret_cast<Variable*>(declaration);
        var->accept_visitor(*this);
    }
    // go_msat needs a variable with an infinite domain.
    if (_reals && !_realVariable)
        _fout << "\tchase_real : real;" << std::endl;
    for (auto c : constants)
    {
        _fout << "DEFINE " << c->getName()->getString() << " := ";
        c->getValue()->accept_visitor(*this);
        _fout << ";" << std::endl;
    }
    for (auto var : _boundedReals)
    {
        auto r = reinterpret_cast<Real *>(var->getType());
        std::string name = var->getName()->getString();
        if (r->getMin() > -infinity)
            _fout << "INVAR " << name << " >= " << realText(r->getMin()) << ";" << std::endl;
        if (r->getMax() < infinity)
            _fout << "INVAR " << name << " <= " << realText(r->getMax()) << ";" << std::endl;
    }
    _boundedReals.clear();
}

void NuSMVPrinter::printAssumptions()
{
    _fout << "\tLTLSPEC (" << std::endl << "\t\t";
    auto it = _contract->assumptions.find(logic);
    // No assumptions. Surely true.
    if(it == _contract->assumptions.end()) {
        _fout << "\t\tTRUE" << std::endl;
        return;
    }
    // Else...
    it->second->accept_visitor(*this);
    _fout << " )\n\n";
}

void NuSMVPrinter::printGuarantees()
{
    _fout << "\tLTLSPEC (" << std::endl << "\t\t";
    auto it = _contract->guarantees.find(logic);
    // No assumptions. Surely true.
    if(it == _contract->guarantees.end()) {
        _fout << "\t\tTRUE" << std::endl;
        return;
    }
    // Else...
    it->second->accept_visitor(*this);
    _fout << " )\n\n";
}

void NuSMVPrinter::printSpec(LogicFormula * formula, const std::string & comment)
{
    if (!comment.empty())
        _fout << "\t-- " << comment << std::endl;
    _fout << "\tLTLSPEC (" << std::endl << "\t\t";
    formula->accept_visitor(*this);
    _fout << " )\n\n";
}

std::string NuSMVPrinter::_printBooleanOperator(BooleanOperator op) {
    std::string sop;
    switch(op)
    {
        case op_and:        return "&";
        case op_or:         return "|";
        case op_implies:    return "->";
        case op_iff:        return "<->";
        case op_xor:        return "xor";
        case op_xnor:       return "xnor";
        case op_not:        return "!";
        case op_nand:
            messageError("Unsupported operator: NAND.");
            break;
        case op_nor:
            messageError("Unsupported operator: NOR.");
            break;
    }
    return std::string();
}

std::string NuSMVPrinter::_printTemporalOperator(TemporalOperator op) {
    std::string sop;
    switch(op)
    {
        case op_globally:   return "G";
        case op_future:     return "F";
        case op_next:       return "X";
        case op_until:      return "U";
        case op_release:
            messageError("Unsupported operator: Release");
            break;
    }
    return std::string();
}

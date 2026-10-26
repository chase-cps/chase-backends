/**
 * @author      <a href="mailto:michele.lora@univr.it">Michele Lora</a>
 * @date        12/3/2019
 *              This project is released under the 3-Clause BSD License.
 *
 */
#include "SlugsPrinter.hh"

using namespace chase;

SlugsPrinter::SlugsPrinter() :
    GuideVisitor(),
    _inNext(false)
{
}

SlugsPrinter::~SlugsPrinter()
{
    if(fout.is_open())
        fout.close();
}

void SlugsPrinter::print(Contract *contract, std::string path)
{
    // Open the file and get the contract to be printed.
    _contract = contract;
    _checkGR1();
    fout.open(path);

    // Print the declarations.
    _printDeclarations();

    // Print the initial conditions of the contract.
    _printInit();
    // Print the safety properties of the contract.
    _printSafety();
    // Print the liveness properties of the contract.
    _printLiveness();

}



// True if the formula contains a temporal operator; X is allowed when
// allowNext is set. The node types are tested with IsA(): the formulas may
// come from another module, where dynamic_cast does not work.
static bool hasTemporal(LogicFormula * f, bool allowNext)
{
    if(f == nullptr) return false;
    switch(f->IsA()) {
        case unaryTemporalOperation_node: {
            auto u = reinterpret_cast< UnaryTemporalFormula * >(f);
            if(u->getOp() == op_next && allowNext)
                return hasTemporal(u->getFormula(), allowNext);
            return true;
        }
        case binaryTemporalOperation_node:
            return true;
        case unaryBooleanOperation_node:
            return hasTemporal(
                reinterpret_cast< UnaryBooleanFormula * >(f)->getOp1(), allowNext);
        case binaryBooleanOperation_node: {
            auto b = reinterpret_cast< BinaryBooleanFormula * >(f);
            return hasTemporal(b->getOp1(), allowNext) ||
                   hasTemporal(b->getOp2(), allowNext);
        }
        case largeBooleanFormula_node:
            for(auto o : reinterpret_cast< LargeBooleanFormula * >(f)->operands)
                if(hasTemporal(o, allowNext)) return true;
            return false;
        default:
            return false;
    }
}

static void checkGR1Term(LogicFormula * t)
{
    // Initial condition.
    if(!hasTemporal(t, false)) return;
    if(t->IsA() == unaryTemporalOperation_node) {
        auto g = reinterpret_cast< UnaryTemporalFormula * >(t);
        if(g->getOp() == op_globally) {
            auto f = g->getFormula();
            // Liveness: G F f, f may refer to the next step as in slugs.
            if(f->IsA() == unaryTemporalOperation_node) {
                auto u = reinterpret_cast< UnaryTemporalFormula * >(f);
                if(u->getOp() == op_future && !hasTemporal(u->getFormula(), true))
                    return;
            }
            // Safety: G f, with X only.
            if(!hasTemporal(f, true)) return;
        }
    }
    messageError("Not a GR1 Specification: " + t->getString());
}

void SlugsPrinter::_checkGR1()
{
    for(auto part : {&_contract->assumptions, &_contract->guarantees}) {
        auto formulae = part->find(logic);
        if(formulae == part->end()) continue;
        auto f = reinterpret_cast< LogicFormula * >(formulae->second);
        // The sections below print the operands of a conjunction; anything
        // else would be left out of the file.
        if(f->IsA() != largeBooleanFormula_node ||
           reinterpret_cast< LargeBooleanFormula * >(f)->getOp() != op_and)
            messageError("Not a GR1 Specification: expected a conjunction of terms: "
                         + f->getString());
        for(auto o : reinterpret_cast< LargeBooleanFormula * >(f)->operands)
            checkGR1Term(o);
    }
}


void SlugsPrinter::_printDeclarations()
{
    // GR(1) synthesis is finite-state: a real variable would be written as a
    // Boolean one.
    for (auto & declaration : _contract->declarations)
    {
        if (declaration->IsA() != variable_node) continue;
        auto var = reinterpret_cast<Variable *>(declaration);
        if (var->getType()->IsA() == real_node)
            messageError("The real variable " + var->getName()->getString() +
                         " cannot be written for slugs: GR(1) synthesis is finite-state.");
    }

    // Print the environment variables.
    fout << "[INPUT]" << std::endl;
    for( auto vit = _contract->declarations.begin();
        vit != _contract->declarations.end(); ++vit)
    {
        if( (*vit)->IsA() != variable_node ) continue;
        auto var = reinterpret_cast<Variable *>(*vit);
        if( var->getCausality() == input )
        {
            std::string name = var->getName()->getString();
            Type * type = var->getType();
            fout << name;
            if(type->IsA() == integer_node)
            {
                auto integ = reinterpret_cast<Integer *>(type);
                auto lv = integ->getMin();
                auto rv = integ->getMax();

                fout << ": " << lv << "..." << rv;
            }
            fout << std::endl;
        }
    }
    fout << std::endl << std::endl;


    // Print the system variables.
    fout << "[OUTPUT]" << std::endl;
    for(auto & declaration : _contract->declarations)
    {
        if( declaration->IsA() != variable_node ) continue;
        auto var = reinterpret_cast<Variable *>(declaration);
        if( var->getCausality() == output )
        {
            std::string name = var->getName()->getString();
            Type * type = var->getType();
            fout << name;
            if(type->IsA() == integer_node)
            {
                auto integ = reinterpret_cast<Integer *>(type);
                auto lv = integ->getMin();
                auto rv = integ->getMax();

                fout << ": " << lv << "..." << rv;
            }
            fout << std::endl;
        }
    }

    fout << std::endl << std::endl;

}



void SlugsPrinter::_printInit() {
    // Assumptions.
    auto formulae = _contract->assumptions.find(logic);
    if( formulae == _contract->assumptions.end() )
        messageError("No Logic in Assumptions");

    auto assumptions = formulae->second;

    if( assumptions->IsA() == largeBooleanFormula_node ) {
        auto lbf = reinterpret_cast< LargeBooleanFormula * >(assumptions);
        if( lbf->getOp() != op_and )
            messageError("Not a GR1 Specification.");


        _curr = std::string("");
        for( size_t f = 0; f < lbf->operands.size(); ++f )
        {

            if( lbf->operands[f]->IsA() != unaryTemporalOperation_node &&
                    lbf->operands[f]->IsA() != binaryTemporalOperation_node )
            {
                _curr += "\n";

                lbf->operands[f]->accept_visitor(*this);

            }
        }
        if(_curr != "")
            fout<< "[ENV_INIT]" << _curr
                << std::endl << std::endl;
    }

    // Guarantees
    formulae = _contract->guarantees.find(logic);
    if( formulae == _contract->guarantees.end() )
        messageError("No Logic in Assumptions");

    auto guarantees = formulae->second;

    if( guarantees->IsA() == largeBooleanFormula_node ) {
        auto lbf = reinterpret_cast< LargeBooleanFormula * >(guarantees);
        if( lbf->getOp() != op_and )
            messageError("Not a GR1 Specification.");

        _curr = std::string("");

        for( size_t f = 0; f < lbf->operands.size(); ++f )
        {
            if( lbf->operands[f]->IsA() != unaryTemporalOperation_node &&
                lbf->operands[f]->IsA() != binaryTemporalOperation_node )
            {
                _curr += "\n";
                lbf->operands[f]->accept_visitor(*this);
            }

        }
        if(_curr != "")
            fout<< "[SYS_INIT]" << _curr
                << std::endl << std::endl;
    }
}

void SlugsPrinter::_printSafety() {
    // Assumptions.
    auto formulae = _contract->assumptions.find(logic);
    if( formulae == _contract->assumptions.end() )
        messageError("No Logic in Assumptions");

    auto assumptions = formulae->second;

    if( assumptions->IsA() == largeBooleanFormula_node ) {
        auto lbf = reinterpret_cast< LargeBooleanFormula * >(assumptions);
        if( lbf->getOp() != op_and )
            messageError("Not a GR1 Specification.");

        _curr = std::string("");

        for( size_t f = 0; f < lbf->operands.size(); ++f )
        {
            if( lbf->operands[f]->IsA() == unaryTemporalOperation_node )
            {
                auto uto =
                        reinterpret_cast<UnaryTemporalFormula*>(lbf->operands[f]);
                if(uto->getOp() != op_globally)
                    messageError("Not a GR1 Specification");

                if(uto->getFormula()->IsA() == unaryTemporalOperation_node)
                {
                    auto inner = reinterpret_cast<UnaryTemporalFormula*>(
                            uto->getFormula());
                    if( inner->getOp() == op_future )
                        continue; // Is a Liveness Property.
                }
                _curr += "\n";
                uto->accept_visitor(*this);
            }
        }

        if(_curr != "")
            fout<< "[ENV_TRANS]"
                << _curr << std::endl << std::endl;
    }
    else
    {
        std::cout << "Not a large boolean formula." << std::endl;
    }

    // Guarantees
    formulae = _contract->guarantees.find(logic);
    if( formulae == _contract->guarantees.end() )
        messageError("No Logic in Assumptions");

    auto guarantees = formulae->second;

    if( guarantees->IsA() == largeBooleanFormula_node ) {
        auto lbf = reinterpret_cast< LargeBooleanFormula * >(guarantees);
        if (lbf->getOp() != op_and)
            messageError("Not a GR1 Specification.");

        _curr = std::string("");
        for (size_t f = 0; f < lbf->operands.size(); ++f) {
            if (lbf->operands[f]->IsA() == unaryTemporalOperation_node) {
                auto uto =
                        reinterpret_cast<UnaryTemporalFormula *>(lbf->operands[f]);
                if (uto->getOp() != op_globally)
                    messageError("Not a GR1 Specification");

                if (uto->getFormula()->IsA() == unaryTemporalOperation_node) {
                    auto inner = reinterpret_cast<UnaryTemporalFormula *>(
                            uto->getFormula());
                    if (inner->getOp() == op_future)
                        continue; // Is a Liveness Property.
                }
                _curr += "\n";
                uto->accept_visitor(*this);
            }
        }
        if (_curr != "")
            fout << "[SYS_TRANS]"
                 << _curr << std::endl << std::endl;
    }
}

void SlugsPrinter::_printLiveness() {
    // Assumptions.
    auto formulae = _contract->assumptions.find(logic);
    if( formulae == _contract->assumptions.end() )
        messageError("No Logic in Assumptions");

    auto assumptions = formulae->second;

    if( assumptions->IsA() == largeBooleanFormula_node ) {
        auto lbf = reinterpret_cast< LargeBooleanFormula * >(assumptions);
        if (lbf->getOp() != op_and)
            messageError("Not a GR1 Specification.");

        _curr = std::string("");

        for (size_t f = 0; f < lbf->operands.size(); ++f) {
            if (lbf->operands[f]->IsA() == unaryTemporalOperation_node) {
                auto uto =
                        reinterpret_cast<UnaryTemporalFormula *>(lbf->operands[f]);
                if (uto->getOp() != op_globally)
                    messageError("Not a GR1 Specification");

                if (uto->getFormula()->IsA() == unaryTemporalOperation_node) {
                    auto inner = reinterpret_cast<UnaryTemporalFormula *>(
                            uto->getFormula());
                    if (inner->getOp() == op_future) {
                        _curr += "\n";
                        uto->accept_visitor(*this);
                    }
                }
            }
        }
        if (_curr != "")
            fout << "[ENV_LIVENESS]"
                 << _curr << std::endl << std::endl;
    }

    // Guarantees
    formulae = _contract->guarantees.find(logic);
    if( formulae == _contract->guarantees.end() )
        messageError("No Logic in Assumptions");

    auto guarantees = formulae->second;

    if( guarantees->IsA() == largeBooleanFormula_node ) {
        auto lbf = reinterpret_cast< LargeBooleanFormula * >(guarantees);
        if( lbf->getOp() != op_and )
            messageError("Not a GR1 Specification.");


        _curr = std::string("");

        for( size_t f = 0; f < lbf->operands.size(); ++f )
        {
            if( lbf->operands[f]->IsA() == unaryTemporalOperation_node )
            {
                auto uto =
                        reinterpret_cast<UnaryTemporalFormula*>(lbf->operands[f]);
                if(uto->getOp() != op_globally)
                    messageError("Not a GR1 Specification");

                if(uto->getFormula()->IsA() == unaryTemporalOperation_node)
                {
                    auto inner = reinterpret_cast<UnaryTemporalFormula*>(
                            uto->getFormula());
                    if( inner->getOp() == op_future ) {
                        _curr += "\n\t";
                        uto->accept_visitor(*this);
                    }
                }

            }
        }
        if (_curr != "")
            fout << "[SYS_LIVENESS]"
                  << _curr << std::endl << std::endl;
    }
}

/**
 * @author      <a href="mailto:michele.lora@univr.it">Michele Lora</a>
 * @date        11/16/2020
 *              This project is released under the 3-Clause BSD License.
 *
 */

#include "NuSMVPrinter.hh"

#include <cstdio>
#include <cstdlib>

using namespace chase;

// The shortest decimal that reads back as the same double (0.1 rather than
// 0.100000), always with a dot: nuXmv reads 20 as an integer.
std::string chase::realText(double value)
{
    char text[512];
    for (int digits = 1; digits < 400; ++digits) {
        std::snprintf(text, sizeof(text), "%.*f", digits, value);
        if (std::strtod(text, nullptr) == value) break;
    }
    return text;
}

int NuSMVPrinter::visitBooleanValue(BooleanValue &value) {
    if(value.getValue() == true)
        _fout << "TRUE";
    else
        _fout << "FALSE";
    return GuideVisitor::visitBooleanValue(value);
}

int NuSMVPrinter::visitIdentifier(Identifier &identifier) {
    std::string name = identifier.getDeclaration()->getName()->getString();
    if (!identifier.isPrimed()) _fout << identifier.getString();
    else {
        _fout << "next(" << name << ")";
    }
    return GuideVisitor::visitIdentifier(identifier);
}

int NuSMVPrinter::visitVariable(Variable &variable) {
    auto type = variable.getType();
    auto name = variable.getName()->getString();
    if (!(type->IsA() == boolean_node || type->IsA() == integer_node ||
          type->IsA() == real_node))
        messageError(
                "Trying to use a non supported type variable in NuSMV. " + name);

    if (type->IsA() == boolean_node) {
        std::string stype("boolean");
        _fout << "\t" << name << " : " << stype << ";" << std::endl;
    }
    if (type->IsA() == integer_node)
    {
        _fout << "\t" << name << " : ";
        auto i = reinterpret_cast< Integer * >(type);
        _fout << i->getMin() << ".."
            << i->getMax() << ";" << std::endl;
    }
    // Real variables need nuXmv (go_msat, then check_ltlspec_ic3).
    if (type->IsA() == real_node)
    {
        _fout << "\t" << name << " : real;" << std::endl;
        _realVariable = true;
        auto r = reinterpret_cast< Real * >(type);
        if (r->getMin() > -infinity || r->getMax() < infinity)
            _boundedReals.push_back(&variable);
    }

    return GuideVisitor::visitVariable(variable);
}

int NuSMVPrinter::visitProposition(Proposition &proposition)
{
    return GuideVisitor::visitProposition(proposition);
}

int NuSMVPrinter::visitBinaryBooleanOperation(BinaryBooleanFormula &formula)
{
    std::string op = _printBooleanOperator(formula.getOp());
    _fout << "(";
    int rv = formula.getOp1()->accept_visitor(*this);
    _fout << " " << op << " ";
    rv |= formula.getOp2()->accept_visitor(*this);
    _fout << ")";
    return rv;
}

int NuSMVPrinter::visitUnaryBooleanOperation(UnaryBooleanFormula &formula) {
    std::string op = _printBooleanOperator(formula.getOp());
    _fout << " " << op << "(";
    int rv = formula.getOp1()->accept_visitor(*this);
    _fout << ")";
    return rv;
}

int NuSMVPrinter::visitLargeBooleanFormula(LargeBooleanFormula &formula) {
    std::string op( _printBooleanOperator(formula.getOp()));
    auto i = formula.operands.begin();
    if( i == formula.operands.end() )
        messageError("Empty large boolean formula. Invalid formula.");

    int rv = (*i)->accept_visitor(*this);
    ++i;
    for(; i != formula.operands.end(); ++i)
    {
        _fout << " " << op << " ";
        rv |= (*i)->accept_visitor(*this);
    }
    return rv;
}

int NuSMVPrinter::visitUnaryTemporalOperation(UnaryTemporalFormula &formula) {
    std::string op = _printTemporalOperator(formula.getOp());
    if( formula.getOp() == op_globally)
        _fout << "\n\t\t";
    _fout << "(";
    _fout << " " << op << " ";
    int rv = formula.getFormula()->accept_visitor(*this);
    _fout << ")";
    return rv;
}

int NuSMVPrinter::visitBinaryTemporalOperation(BinaryTemporalFormula &formula) {
    std::string op = _printTemporalOperator(formula.getOp());
    _fout << "(";
    int rv = formula.getFormula1()->accept_visitor(*this);
    _fout << " " << op << " ";
    rv |= formula.getFormula2()->accept_visitor(*this);
    _fout << ")";
    return rv;
}

int NuSMVPrinter::visitExpression(Expression &expression) {
    auto op = expression.getOperator();
    std::string sop;
    switch(op)
    {
        case op_divide:
            sop = "/";
            break;
        case op_plus:
            sop = "+";
            break;
        case op_minus:
            sop = "-";
            break;
        case op_multiply:
            sop = "*";
            break;
        case op_mod:
            sop = "%";
            break;
        case op_eq:
            sop = "=";
            break;
        case op_neq:
            sop = "!=";
            break;
        case op_lt:
            sop = "<";
            break;
        case op_gt:
            sop = ">";
            break;
        case op_le:
            sop = "<=";
            break;
        case op_ge:
            sop = ">=";
            break;
        case op_none:
        default:
            messageWarning("Missing operator in expression: "
                + expression.getString());
            break;
    }
    _fout << "( ";
    int ret = expression.getOp1()->accept_visitor(*this);
    _fout << " " << sop << " ";
    ret |= expression.getOp2()->accept_visitor(*this);
    _fout << " )";
    return ret;
}

int NuSMVPrinter::visitIntegerValue(IntegerValue &value) {
    _fout << value.getString();
    return 1;
}

int NuSMVPrinter::visitRealValue(RealValue &value) {
    _fout << realText(value.getValue());
    return 1;
}

int NuSMVPrinter::visitBooleanConstant(BooleanConstant &constant) {
    if(constant.getValue()) _fout << "TRUE";
    else _fout << "FALSE";
    return 1;
}

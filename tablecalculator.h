/** @file tablecalculator.h
* @brief the calculator of the data table.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the calculator of the result table.
*/

#ifndef TABLECALCULATOR_H
#define TABLECALCULATOR_H

#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <utility>
#include <string>
#include <map>

namespace client
{
void assign(std::vector<double> &x,double y);
void negative(std::vector<double> &x,const std::vector<double> &y);
void plusAssign(std::vector<double> &x,const std::vector<double> &y);
void minusAssign(std::vector<double> &x,const std::vector<double> &y);
void multipleAssign(std::vector<double> &x,const std::vector<double> &y);
void divideAssign(std::vector<double> &x,const std::vector<double> &y);
void powAssign(std::vector<double> &x, const std::vector<double> &y);

typedef double (*unaryfun)(double);
typedef double (*reduction)(const std::vector<double> &);
void initUnarymap(std::map<std::string, unaryfun> &map);
void initRedumap(std::map<std::string, reduction> &map);
void initConstmap(std::map<std::string, double> &map);

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

struct funcall
{
    std::string name;
    std::vector<double> argument;
};
}

BOOST_FUSION_ADAPT_STRUCT(
    client::funcall,
    (std::string, name)
    (std::vector<double>, argument)
)

class TableCalculator;

namespace client
{
///////////////////////////////////////////////////////////////////////////////
//  Our calculator grammar
///////////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    struct calculator : qi::grammar<Iterator, std::vector<double>(), ascii::space_type>
    {
        calculator(TableCalculator *parent);
        void invoke(const funcall &pr, std::vector<double> &val);

        qi::rule<Iterator, std::vector<double>(), ascii::space_type> expression, term, power, factor,start;
        qi::rule<Iterator, std::string(), ascii::space_type> identifier;
        qi::rule<Iterator, funcall(), ascii::space_type> funcallrule;
        std::map<std::string, reduction> redumap;
        std::map<std::string, unaryfun> unarymap;
        std::map<std::string, double> constmap;
    private:
        const TableCalculator &parent;
    };
}

class TableResult;
class QVariant;
class QString;

class TableCalculator
{
public:
    TableCalculator(const TableResult *table);
    QVariant caculate(int row,const QString &formular);
    std::vector<double> parseId(const std::string &name) const;
private:
    typedef std::string::const_iterator iterator_type;
    typedef client::calculator<iterator_type> calculator;
private:
    calculator calc;
    int _row;
    const TableResult *table;
};

namespace client
{
template <typename Iterator>
calculator<Iterator>::calculator(TableCalculator *parent) :
    calculator::base_type(start), parent(*parent)
{
    using qi::double_;
    using qi::_val;
    using qi::lexeme;
    using qi::alpha;
    using qi::alnum;
    using qi::_1;
    using qi::eoi;
    using qi::char_;

    start = '=' >> expression[_val = _1] >> (eoi | ';') ;

    expression =
            term[_val = qi::_1]
        >>  *(  ('+' >> term  [phoenix::bind(&plusAssign,_val,_1)])
            |   ('-' >> term  [phoenix::bind(&minusAssign,_val,_1)]) );

    term =
            power[_val = qi::_1]
        >>  *(  ('*' >> power   [phoenix::bind(&multipleAssign,_val,_1)])
            |   ('/' >> power   [phoenix::bind(&divideAssign,_val,_1)]) );

    power = factor[_val = qi::_1] >> *('^' >> factor [phoenix::bind(&powAssign,_val,_1)]);

    factor =
                double_             [phoenix::bind(&assign,_val,_1)]
            |   ('(' >> expression  [_val = qi::_1] >> ')')
            |   ('-' >> factor      [phoenix::bind(&negative,_val,_1)])
            |   ('+' >> factor      [_val = qi::_1])
            |   funcallrule  [phoenix::bind(&calculator::invoke,*this,_1,_val)]
            |   identifier [_val = phoenix::bind(&TableCalculator::parseId, parent, _1)];

    identifier %= lexeme [ ( alpha | '_') >> *( alnum | char_("_.") ) ] ;
    funcallrule %= lexeme [ ( alpha | '_') >> *( alnum | '_') ] >> '(' >> expression >> ')';

    initConstmap(constmap);
    initRedumap(redumap);
    initUnarymap(unarymap);
}

template <typename Iterator>
void calculator<Iterator>::invoke(const funcall &pr, std::vector<double> &val)
{
    std::map<std::string, unaryfun>::iterator uit = unarymap.find(pr.name);
    if (uit != unarymap.end())
    {
        for (unsigned i=0;i<pr.argument.size();++i)
            val.push_back(uit->second(pr.argument[i]) );
    }
    else
    {
        std::map<std::string, reduction>::iterator  rit= redumap.find(pr.name);
        if (rit != redumap.end())
            val.push_back(rit->second(pr.argument) );
    }
}

}

#endif // TABLECALCULATOR_H

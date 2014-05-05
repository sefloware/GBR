/** @file stscparser.h
* @brief the parser about space confinement blocks.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the parser about space confinement
* blocks.
*/

#ifndef STSCPARSER_H
#define STSCPARSER_H

#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <string>
#include <map>
#include "assert.h"

namespace client
{
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

struct dimeninfo : public std::string
{
    dimeninfo(const std::string &str) : std::string(str) {}
    dimeninfo() : std::string() {}
    std::string name;
    std::string outgrad;
    std::string exp;
};

typedef std::pair<std::string, std::string> range;
typedef std::pair<dimeninfo, std::vector<range> > dimenssion;
typedef std::map<dimeninfo, std::vector<range> > dimenmap;

typedef std::vector<std::string> termtype;
typedef std::vector<termtype> expressiontype;

struct surface
{
    std::string name;
    std::string distexp;
    std::string distgrad;
    std::string distineq;

    std::string distname() const;
    dimenssion todimenssion() const;
};

//LogcalGrammar
expressiontype andop(const expressiontype &right, const expressiontype &left);
expressiontype andop(const std::vector<expressiontype> &v);
expressiontype toexpression(const std::string &identifier);
void merge(const std::vector<expressiontype> &source,expressiontype &dest);

//termtype negatelog(const termtype &v);
//std::string negatelog(std::string v);
expressiontype negatelog(const expressiontype &v);

//output
std::string termconditiontext(const termtype&term, const dimenmap &dimap);
extern "C" std::string expressionconditiontext(const expressiontype& exp, const dimenmap &dimap);

std::string tologexp(const std::string &fr, const range &rg);
std::string tologexp(const std::string &fr, const std::vector<range> &rgvec);
std::string singleconditiontext(unsigned idx,  const dimeninfo &diminfo,  const std::vector<client::range> &rgvec,  const std::vector<std::string> &exlogexpvec);
std::string singleconditiontext(const std::string name,const std::string exp,const std::string &outgrad,const range &rg, const std::vector<std::string> &exlogexpvec,unsigned idx);
std::vector<range> negate(const std::vector<range> &rgvec);

//other useful tool
std::string negate(std::string poly, bool minus = false);
void replace_word(const std::string& from, const std::string& to, std::string& sentence);
std::vector<std::string> split(const std::string &str,const std::string &token);
std::string join(const std::vector<std::string> &v,const std::string &sep);
std::string join(const std::vector<std::string> &v, int pos, int n, const std::string &sep);

}

BOOST_FUSION_ADAPT_STRUCT(
    client::surface,
    (std::string, name)
    (std::string, distexp)
    (std::string, distgrad)
    (std::string, distineq)
)

namespace client
{
    template <typename Iterator>
    struct LogcalGrammar : qi::grammar<Iterator, expressiontype(), ascii::space_type>
    {
        LogcalGrammar() : LogcalGrammar::base_type(start)
        {
            using qi::_val;
            using qi::lexeme;
            using qi::alpha;
            using qi::alnum;
            using qi::_1;
            using qi::eoi;

            start %= expression >> (eoi | ';') ;
            expression = (term % "||") [phoenix::bind(&merge,_1,_val)];
            term = (factor % "&&") [_val = phoenix::bind(&andop,_1)];
            factor =   identifier [_val = phoenix::bind(&toexpression,_1)]
                    | '(' >> expression[_val = _1] >>')'
                    | '!' >> factor[_val = phoenix::bind(&negatelog,_1)];
            identifier %= lexeme [ ( alpha | '_') >> *( alnum | '_') ];
        }

        qi::rule<Iterator,expressiontype() ,ascii::space_type> factor,term, expression,start;
        qi::rule<Iterator, std::string(), ascii::space_type> identifier;
    };

    template <typename Iterator>
    struct PlaneGrammar : qi::grammar<Iterator, surface(), ascii::space_type>
    {
        PlaneGrammar() : PlaneGrammar::base_type(start)
        {
            using qi::lit;
            using qi::lexeme;
            using qi::alpha;
            using qi::alnum;
            using qi::char_;
            using qi::as_string;

            start = lit("//!") >> '@' >> identifier >> '{'
                                >> lit("f(r)") >> '=' >> lexeme[+(char_-';')] >> ';'
                                >> lit("df(r)") >> '=' >> lexeme[+(char_-';')] >> ';'
                                >> lexeme[+(char_-'}')] >> '}';

            identifier %= lexeme [ ( alpha | '_') >> *( alnum | '_') ];
        }
        qi::rule<Iterator, surface(), ascii::space_type> start;
        qi::rule<Iterator, std::string(), ascii::space_type> identifier;
    };
}

#endif // STSCPARSER_H

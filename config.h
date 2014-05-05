/** @file config.h
* @brief Reading configuration files
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file provides methods for reading configuration files,
* such as computing profiles, project logo files. It also
* provides some function to return the path of some certain
* folders in one project, and one model that contains the C++
* key words.
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <QMap>
#include "assert.h"

class cppkeywords;
QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QString;
class QStringList;
class QWidget;
QT_END_NAMESPACE

namespace client
{
namespace qi        = boost::spirit::qi;
namespace phoenix   = boost::phoenix;
namespace ascii     = boost::spirit::ascii;

struct compilertype
{
    std::string name;
    std::string optimize;
    std::string include;
    std::string macro;
    std::string output;
    
    std::string tooltip() const
    { return name + " " + optimize + " " + include + " " + macro + " " + output; }
};

struct runenvtype
{
    std::string name;
    std::string threadnum;
    std::string platform;
    compilertype compiler;

    std::string toottip() const
    { return name + "  " + platform + " " + threadnum + ":  " + compiler.tooltip(); }

    std::string show() const
    { return name + "  " + platform + " " + threadnum + ":  " + compiler.name; }
};

}

BOOST_FUSION_ADAPT_STRUCT(
client::compilertype,
(std::string, name)
(std::string, optimize)
(std::string, include)
(std::string, macro)
(std::string, output)
)

BOOST_FUSION_ADAPT_STRUCT(
client::runenvtype,
(std::string, name)
(std::string, threadnum)
(std::string, platform)
(client::compilertype, compiler)
)

namespace client {

template <typename Iterator>
struct RunenvGrammar : qi::grammar<Iterator, std::vector<runenvtype> (), ascii::space_type>
{
    RunenvGrammar() : RunenvGrammar::base_type(start)
    {
        using qi::lit;
        using qi::lexeme;
        using qi::alpha;
        using qi::alnum;
        using qi::char_;
        using qi::string;
        using qi::omit;

        start   %= +runenv;
        runenv  %= omit[*(char_-'@')] >> lit('@') >> term  >> '{'
                            >> lit("threadnum") >>  '=' >> lexeme[+(char_-';')]  >> ';'
                            >> lit("platform") >>   '=' >> lexeme[+(char_-';')]  >> ';'
                            >> lit("compiler") >>   '=' >> compiler               >> ';'
                            >> char_('}');

        compiler %= term    >> '[' >> option >> lit("optimize")>> ']'
                            >> '[' >> option >> lit("include") >> ']'
                            >> '[' >> option >> lit("macro")   >> ']'
                            >> '[' >> option >> lit("output")  >> ']';

        term     %= lexeme[+(char_-(qi::space|'{'))];
        option   %= lexeme[(char_-alnum) >> +(char_-(qi::space)) >> *(+qi::space >> (char_-alnum) >> +(char_-(qi::space)) )];
    }
    qi::rule<Iterator, std::vector<runenvtype> (), ascii::space_type> start;
    qi::rule<Iterator, runenvtype(), ascii::space_type> runenv;
    qi::rule<Iterator, compilertype(), ascii::space_type> compiler;
    qi::rule<Iterator, std::string(), ascii::space_type>  term, option;
};

}

QStandardItemModel &runevmodel();
void editrunenv(QWidget *parent);
void readRunEnv();
cppkeywords &cppkeys();
bool insureDirectory(const QString &directory);

enum FolderFlag {ProjectFolder, SourceFolder, ConfigFolder};

extern const QString folderName[];
QString absolutePath(FolderFlag flag);

QMap<QString, QString> parseIdentification(const QString &identification);
QString identification(const QString &name, const QStringList &outputs, const QStringList &ids, const QString version);

#endif // CONFIG_H

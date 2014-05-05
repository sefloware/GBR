/** @file tablecalculator.cpp
* @brief the calculator of the data table.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the calculator of the result table.
*/

#include "tablecalculator.h"
#include "table.h"
#include <algorithm>
#include <QString>

void client::assign(std::vector<double> &x, double y)
{
    x.resize(1);
    x[0] = y;
}

void client::negative(std::vector<double> &x, const std::vector<double> &y)
{
    x.resize(y.size());
    for(unsigned i=0;i<x.size();++i)
        x[i]=-y[i];
}

void client::plusAssign(std::vector<double> &x,const std::vector<double> &y)
{
    if(x.size()==1)
    {
        double tmp = x[0];
        x.resize(y.size());
        for(unsigned i=0; i<x.size(); ++i)
            x[i] = tmp + y[i];
    }
    else if(y.size()==1)
        for(unsigned i=0; i<x.size(); ++i)
            x[i] += y[0];
    else if(x.size()==y.size())
        for(unsigned i=0; i<x.size(); ++i)
            x[i] += y[i];
    else
        x.clear();
}

void client::minusAssign(std::vector<double> &x,const std::vector<double> &y)
{
    if(x.size()==1)
    {
        double tmp = x[0];
        x.resize(y.size());
        for(unsigned i=0; i<x.size(); ++i)
            x[i] = tmp - y[i];
    }
    else if(y.size()==1)
        for(unsigned i=0; i<x.size(); ++i)
            x[i] -= y[0];
    else if(x.size()==y.size())
        for(unsigned i=0; i<x.size(); ++i)
            x[i] -= y[i];
    else
        x.clear();
}

void client::multipleAssign(std::vector<double> &x,const std::vector<double> &y)
{
    if(x.size()==1)
    {
        double tmp = x[0];
        x.resize(y.size());
        for(unsigned i=0; i<x.size(); ++i)
            x[i] = tmp * y[i];
    }
    else if(y.size()==1)
        for(unsigned i=0; i<x.size(); ++i)
            x[i] *= y[0];
    else if(x.size()==y.size())
        for(unsigned i=0; i<x.size(); ++i)
            x[i] *= y[i];
    else
        x.clear();
}

void client::divideAssign(std::vector<double> &x,const std::vector<double> &y)
{
    if(y.size()==1)
        for(unsigned i=0; i<x.size(); ++i)
            x[i] /= y[0];
    else if(x.size()==y.size())
        for(unsigned i=0; i<x.size(); ++i)
            x[i] /= y[i];
    else
        x.clear();
}

void client::powAssign(std::vector<double> &x, const std::vector<double> &y)
{
    if(y.size() == 1)
        for(unsigned i=0; i<x.size(); ++i)
            x[i] = std::pow(x[i],y[0]);
    else
        x.clear();
}

void client::initUnarymap(std::map<std::string, unaryfun> &map)
{
    map.insert(std::pair<std::string,unaryfun>("sin",std::sin));
    map.insert(std::pair<std::string,unaryfun>("cos",std::cos));
    map.insert(std::pair<std::string,unaryfun>("tan",std::tan));
    map.insert(std::pair<std::string,unaryfun>("asin",std::asin));
    map.insert(std::pair<std::string,unaryfun>("acos",std::acos));
    map.insert(std::pair<std::string,unaryfun>("atan",std::atan));
    map.insert(std::pair<std::string,unaryfun>("cosh",std::cosh));
    map.insert(std::pair<std::string,unaryfun>("sinh",std::sinh));
    map.insert(std::pair<std::string,unaryfun>("tanh",std::tanh));
    map.insert(std::pair<std::string,unaryfun>("exp",std::exp));
    map.insert(std::pair<std::string,unaryfun>("log",std::log));
    map.insert(std::pair<std::string,unaryfun>("log10",std::log10));
    map.insert(std::pair<std::string,unaryfun>("sqrt",std::sqrt));
    map.insert(std::pair<std::string,unaryfun>("ceil",std::ceil));
    map.insert(std::pair<std::string,unaryfun>("floor",std::floor));
    map.insert(std::pair<std::string,unaryfun>("fabs",std::fabs));
    map.insert(std::pair<std::string,unaryfun>("abs",std::abs));
}

double sum(const std::vector<double> &x)
{
    double sum = 0.0;
    for(unsigned i=0;i<x.size();++i)
        sum += x[i];
    return sum;
}

double mean(const std::vector<double> &x)
{
    if(! x.empty())
        return sum(x)/x.size();
    else
        return 0;
}

double stdev(const std::vector<double> &x)
{
    if( x.size() <2)
        return 0;

    double m = mean(x);

    double result = 0.0;
    for(unsigned i=0; i<x.size(); ++i)
        result += (x[i]-m)*(x[i]-m);

    result/=(x.size()-1);

    return std::sqrt(result);
}

double max(const std::vector<double> &x)
{ return *std::max_element(x.begin(),x.end()); }

double min(const std::vector<double> &x)
{ return *std::min_element(x.begin(),x.end()); }

double count(const std::vector<double> &x)
{ return x.size(); }

void client::initRedumap(std::map<std::string, reduction> &map)
{
    map.insert(std::pair<std::string,reduction>("sum",sum));
    map.insert(std::pair<std::string,reduction>("mean",mean));
    map.insert(std::pair<std::string,reduction>("std",stdev));
    map.insert(std::pair<std::string,reduction>("max",max));
    map.insert(std::pair<std::string,reduction>("min",min));
    map.insert(std::pair<std::string,reduction>("count",count));
}

void client::initConstmap(std::map<std::string, double> &map)
{
    map.insert(std::pair<std::string,double>("PI",3.14) );
}

TableCalculator::TableCalculator(const TableResult *table)
    :calc(this), _row(-1), table(table) {}

QVariant TableCalculator::caculate(int row, const QString &formular)
{
    _row = row;
    QByteArray tmp = formular.toLocal8Bit();
    const std::string str(tmp.data());

    using boost::spirit::ascii::space;
    std::vector<double> result;
    iterator_type iter = str.begin();
    iterator_type end = str.end();

    bool r = phrase_parse(iter, end, calc, space, result);
    if (r && iter == end)
        //Parsing succeeded\n;
        if(result.size())
        {
            return result.front();
        }

    return QVariant();
}

std::vector<double> TableCalculator::parseId(const std::string &name) const
{ return table->data(_row,QString(name.c_str())); }

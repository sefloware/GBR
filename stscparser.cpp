/** @file stscparser.cpp
* @brief the parser about space confinement blocks.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the parser about space confinement
* blocks.
*/

#include "stscparser.h"
#include <deque>

std::string client::surface::distname() const
{ return name+"_f"; }

// the base geometry's nonequation.
client::dimenssion client::surface::todimenssion() const
{
    using qi::phrase_parse;
    using qi::char_;
    using qi::eoi;
    using qi::_1;
    using ascii::space;
    using phoenix::push_back;
    using phoenix::push_front;
    using qi::as_string;
    using qi::string;

    std::string::const_iterator beg = this->distineq.begin();
    std::string::const_iterator end = this->distineq.end();
    std::deque<std::string>  v;
    bool r = phrase_parse (beg, end,
                          (string("f(r)") | as_string[+(char_-char_("<>"))])[push_back (phoenix::ref(v),_1)]
            >> ( +('>' >> (string("f(r)") | as_string[+(char_-char_("<>"))])[push_front(phoenix::ref(v),_1)] )
            |    +('<' >> (string("f(r)") | as_string[+(char_-char_("<>"))])[push_back (phoenix::ref(v),_1)] ) )
            >> eoi,
            space);

    if(!r) return dimenssion();

    if(v.size() < 2)
        return dimenssion();

    dimenssion result;

    if(v[0]=="f(r)" && v[1] != "f(r)")
        result.second.push_back(range(std::string(),v[1] ) );

    for(unsigned i=1; i<v.size()-1; ++i)
    {
        if(v[i] == "f(r)")
        {
            if(v[i-1] == "f(r)" || v[i+1] == "f(r)")
                return dimenssion();

            result.second.push_back(range(v[i-1],v[i+1]) );
            ++i;
        }
    }

    if(v[v.size()-1]=="f(r)" && v[v.size()-2] != "f(r)" )
        result.second.push_back(range(v[v.size()-2],std::string()) );


    result.first = name;
    result.first.name = distname();

    result.first.exp = "const double ";
    result.first.exp.append( distname()).append(" = ").append(distexp).append(";");

    result.first.outgrad = distgrad;
    replace_word("f(r)",result.first.name,result.first.outgrad);

    return result;
}


client::expressiontype client::andop(const expressiontype &right, const expressiontype &left)
{
    assert(!right.empty() && !left.empty());

    expressiontype result;
    for (unsigned i=0; i<right.size(); ++i)
        for (unsigned j=0; j<left.size(); ++j)
        {
            termtype tmp = right[i];
            tmp.insert(tmp.end(), left[j].begin(), left[j].end());
            result.push_back(tmp);
        }

    return result;
}

client::expressiontype client::andop(const std::vector<expressiontype> &v)
{
    if(v.empty())
        return expressiontype();

    expressiontype result = v[0];
    for (unsigned i=1; i<v.size(); ++i)
        result = andop(result,v[i]);

    return result;
}

client::expressiontype client::toexpression(const std::string &identifier)
{ return expressiontype( 1,termtype(1,identifier) ); }
void client::merge(const std::vector<expressiontype> &source, expressiontype &dest)
{
    dest.clear();
    for(unsigned i=0; i<source.size(); ++i)
        dest.insert(dest.end(), source[i].begin(), source[i].end());
}

//negate the logic expression.
std::string negatelog(std::string v)
{
    assert(!v.empty());

    if(v[0]=='!') v.erase(0,1);
    else v.insert(v.begin(),'!');
    return v;
}

client::expressiontype negatelog(const client::termtype &v)
{
    assert(!v.empty());

    client::expressiontype result;
    for(unsigned i=0; i<v.size(); ++i)
        result.push_back(client::termtype(1,negatelog(v[i])));
    return result;
}

client::expressiontype client::negatelog(const expressiontype &v)
{
    assert(!v.empty());

    std::vector<expressiontype> tmp;
    for (unsigned i=0; i<v.size(); ++i)
        tmp.push_back(::negatelog(v[i]) );

    return andop(tmp);
}

//*********************************************
std::string client::singleconditiontext(unsigned idx,
                                        const dimeninfo &diminfo,
                                        const std::vector<client::range> &rgvec,
                                        const std::vector<std::string> &exlogexpvec)
{
    std::string result;

    for(unsigned i=0; i<rgvec.size(); ++i)
    {
        if(! rgvec[i].first.empty() )
        {
            result.append( singleconditiontext (
                               diminfo.name,
                               ( rgvec[i].first=="0" ? diminfo.name : diminfo.name + negate(rgvec[i].first,true) ),
                               diminfo.outgrad,
                               range(rgvec[i].first,( rgvec[i].first=="0" ? "s5DT" : rgvec[i].first+"+s5DT") ),
                               exlogexpvec,
                               idx) );
        }

        if(! rgvec[i].second.empty() )
        {
            result.append( singleconditiontext (
                               diminfo.name,
                               ( rgvec[i].second=="0" ? "-" + diminfo.name : rgvec[i].second+ "-" + diminfo.name ),
                               negate( diminfo.outgrad),
                               range( ( rgvec[i].second=="0" ? "-s5DT" : rgvec[i].second+"-s5DT"),rgvec[i].second),
                               exlogexpvec,
                               idx) ) ;
        }
    }

    return result;
}

std::string client::singleconditiontext(const std::string name,const std::string exp,const std::string &outgrad,const range &rg,
                                        const std::vector<std::string> &exlogexpvec,unsigned idx)
{
    std::string result;

    std::string condition;
    condition.append(join(exlogexpvec,0,idx," && "));
    if(! condition.empty())
        condition.append(" && ");
    condition.append("(").append( tologexp(name,rg) ).append(")");

    std::string tmp = join(exlogexpvec,idx+1,-1," && ");
    if(! tmp.empty())
        condition.append(" && ").append( tmp);

    result.append("if(").append(condition).append(")").append("\n    lincs.plusX(i, rebound_PB(sDt,lincs.generator,").
            append(exp).append(")*").append(outgrad).append(" );\n\n");
    return result;
}

std::vector<client::range> client::negate(const std::vector<range> &rgvec)
{
    assert(!rgvec.empty());

    std::vector<range> result;

    if(! rgvec[0].first.empty() )
        result. push_back( range(std::string(), rgvec[0].first));

    for(unsigned i=0; i<rgvec.size()-1; ++i)
        result.push_back( range(rgvec[i].second,rgvec[i+1].first) );

    if(! rgvec[rgvec.size()-1].second.empty() )
        result. push_back( range(rgvec[rgvec.size()-1].second, std::string() ));

    return result;
}

std::string client::tologexp(const std::string &fr,const std::vector<range> &rgvec)
{
    std::string result;
    for(unsigned i=0; i<rgvec.size(); ++i)
    {
        std::string str = tologexp( fr,rgvec[i]);
        if(str.empty())
            return str;

        result.append(str).append(" || ");
    }
    if(! result.empty())
        result.erase(result.size()-4);
    return result;
}

std::string client::tologexp(const std::string &fr, const range &rg)
{
    assert(!fr.empty());

    std::string left;
    if(! rg.first.empty())
        left.append(rg.first).append("<").append(fr);

    std::string right;
    if(! rg.second.empty())
        right.append(fr).append("<").append(rg.second);

    if( left.empty() )
        return right;
    if( right.empty())
        return left;
    else
        return left+" && "+right;
}


std::string client::termconditiontext(const termtype &term, const dimenmap &dimap)
{
    std::vector<std::string> exlogexpvec;
    for(unsigned i=0; i<term.size(); ++i)
    {
        assert(! term[i].empty());
        if(term[i][0]=='!')
        {
            std::string name = term[i].substr(1);
            assert(!name.empty());
            dimenmap::const_iterator it = dimap.find(name);
            if(it == dimap.end())
                return std::string();

            std::vector<range> rgvec = negate(it->second);
            exlogexpvec.push_back( tologexp(it->first.name,rgvec));
        }
        else
        {
            dimenmap::const_iterator it = dimap.find(term[i]);
            if(it == dimap.end())
                return std::string();
            exlogexpvec.push_back( tologexp(it->first.name,it->second));
        }
    }

    std::string result;

    for(unsigned i=0; i<term.size(); ++i)
    {
        assert(!term[i].empty());
        if(term[i][0]=='!')
        {
            std::string name = term[i].substr(1);
            assert(!name.empty());
            dimenmap::const_iterator it = dimap.find(name);
            assert(it != dimap.end()) ;

            result.append(singleconditiontext(i,it->first,negate(it->second),exlogexpvec) );
        }
        else
        {
            dimenmap::const_iterator it = dimap.find(term[i]);
            assert(it != dimap.end());
            result.append(singleconditiontext(i,it->first,it->second,exlogexpvec) );
        }

    }

    return result;
}

std::string client::expressionconditiontext(const expressiontype& exp, const dimenmap &dimap)
{
    if(exp.empty() || dimap.empty())
        return std::string();

    std::string result;

    for( dimenmap::const_iterator it = dimap.begin(); it != dimap.end(); ++it)
        result.append(it->first.exp).append("\n");

    if(!result.empty())
        result.append("\n");

    for (unsigned i=0; i<exp.size(); ++i)
        result.append( termconditiontext(exp[i],dimap));
    return result;
}

std::vector<std::string> client::split(const std::string &str,const std::string &token)
{
    assert(!str.empty() && !token.empty());
    std::vector<std::string> result;

    std::size_t pos = 0;
    std::size_t beg = 0;
    while( (pos = str.find(token,beg)) != std::string::npos)
    {       
        result.push_back(str.substr(beg,pos-beg) );
        beg = pos + 2;
    }
    result.push_back(str.substr(beg));
    return result;
}

std::string client::join(const std::vector<std::string> &v,int pos, int n, const std::string &sep)
{
    assert(pos > -1);
    if(pos >= static_cast<int>(v.size()) )
        return std::string();
    if( n == -1 || pos+n > static_cast<int>(v.size() ) )
        n = static_cast<int>(v.size() ) - pos;

    std::string result;
    for (int i=pos; i<pos+n; ++i)
        result.append(v[i]).append(sep);

    if(! result.empty())
        result.erase(result.size()-sep.length());

    return result;

}

std::string client::join(const std::vector<std::string> &v,const std::string &sep)
{
    std::string result;
    for (unsigned i=0; i<v.size(); ++i)
        result.append(v[i]).append(sep);

    if(! result.empty())
        result.erase(result.size()-sep.length());

    return result;
}

void client::replace_word(const std::string& from, const std::string& to, std::string& sentence)
{
    std::size_t pos = 0;
    while ((pos = sentence.find(from,pos)) != std::string::npos) {
        if ( ( ! pos                                    || !(isalpha(sentence[pos - 1])) ) &&
             (pos + from.length() ==sentence.length()   || !(isalpha(sentence[pos + from.length()]))) )
        {
            sentence.replace(pos,from.length(),to);
            pos += to.length();
        }
        else
            pos += from.length();
    }
}

std::string client::negate(std::string poly, bool minus)
{
    assert(! poly.empty());

    int rb = 0;
    for( unsigned i=0; i<poly.size(); ++i)
    {
        if(rb < 0)
            return std::string();
        switch(poly[i])
        {
        case '+':
            if(! rb) poly[i] = '-';
            break;
        case '-':
            if(! rb) poly[i] = '+';
            break;
        case '(':
            ++rb;
            break;
        case ')':
            --rb;
            break;
        }
    }

    switch (poly[0])
    {
    case '+':
        if(! minus) poly.erase(0,1);
        break;
    case '-':
        break;
    default:
        poly.insert(0,"-");
    }

    return poly;
}



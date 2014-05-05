/** @file item.cpp
* @brief the base cpp item Class
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file define the CppItem Class, the base C++
* item and some item-data roles.
*/

#include "item.h"

const QMap<Item::CppTypeFlags,QIcon> &Item::cppIconMap()
{
    static QMap<CppTypeFlags,QIcon> map;
    map.insert(Class,QIcon(":/icon/images/class.png"));
    map.insert(Namespace,QIcon(":/icon/images/namespace.png"));
    map.insert(Enum,QIcon(":/icon/images/enum.png"));
    map.insert(Enumerator,QIcon(":/icon/images/enumerator.png"));
    map.insert(Function,QIcon(":/icon/images/function.png"));
    map.insert(Variable,QIcon(":/icon/images/variable.png"));
    map.insert(SquareOperator,QIcon(":/icon/images/squareoperator.png"));
    map.insert(RoundOperator,QIcon(":/icon/images/roundoperator.png"));
    map.insert(Include,QIcon(":/icon/images/directive.png"));
    map.insert(TypeKeyword,QIcon(":/icon/images/keyword.png"));
    map.insert(NonTypeKeyword,QIcon(":/icon/images/keyword.png"));
    map.insert(HFile,QIcon(":/icon/images/hfile.png"));
    return map;
}

const QMap<Item::HeaderFlags,QIcon> &Item::headerIconMap()
{
    static QMap<HeaderFlags,QIcon> map;
    map.insert(Parameter,QIcon(":/icon/images/variable.png"));
    map.insert(Formula,QIcon(":/icon/images/formula.png"));
    map.insert(FileSt,QIcon(":/icon/images/stoutput.png"));
    map.insert(FileOt,QIcon(":/icon/images/output.png"));
    return map;
}

#ifndef NET_STATS_H
#define NET_STATS_H
#include <iostream>
#include <string>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include "json/JSON_Defs.h"   //for the libJSON namespace
#include "json/jsonmain.h"    //for JSONNode
#include "put_data.h"

XERCES_CPP_NAMESPACE_USE

XMLCh* deviceTag;
XercesDOMParser* parser;

class StrX
{
public :

    StrX(const XMLCh* const toTranscode)
    {
        fLocalForm = XMLString::transcode(toTranscode);
    }

    ~StrX()
    {
        XMLString::release(&fLocalForm);
    }

    const char* localForm() const
    {
        return fLocalForm;
    }


private :
    char*   fLocalForm;
};

inline XERCES_STD_QUALIFIER ostream& operator<<(XERCES_STD_QUALIFIER ostream& target, const StrX& toDump)
{
    target << toDump.localForm();
    return target;
}


class ParseTree{
  
public:
  ParseTree();
  ~ParseTree();
  char** parseFile(std::string xmlFile) throw(std::runtime_error);
private:
  DOMElement* findElement(DOMElement* currentElement, const char* lostTag);
  bool hasChildren(DOMElement* elem);
};



#endif
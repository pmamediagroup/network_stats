#ifndef NET_STATS_H
#define NET_STATS_H

#include "json/JSON_Defs.h"   //for the libJSON namespace
#include "json/jsonmain.h"    //for JSONNode
#include "put_data.h"
#include "dependents.h"

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

struct cnfg {
  char* tube_name;
  char* beanstalk_addr;
  int bean_port;
  char mem_file[255];
  char* cpu_file;
  char* net_stats_xml;
};

class ParseTree{
  
public:
  ParseTree();
  ~ParseTree();
  char** parseFile(std::string xmlFile) throw(std::runtime_error);
  cnfg configs;
  void parse_config_file(char* filename);
  char* strrep(char* data, char* searchchar, char* repchar);
private:
  DOMElement* findElement(DOMElement* currentElement, const char* lostTag);
  bool hasChildren(DOMElement* elem);
};



#endif
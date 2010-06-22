#include "net_stats.h"

using namespace std;

static void usage()
{
    XERCES_STD_QUALIFIER cout << "\nUsage:\n"
            "    netstats [Options] <XML file>\n\n"
            "This program grabs the Rx and Tx of the network as printed by\n"
            "the xml output of the vnstat program. The xml file must already\n"
            "exist.\n\n"
            "Options:\n"
            "      -d=           - The device to look for\n"
            "      -?            - Show this help.\n\n"
         << XERCES_STD_QUALIFIER endl;
}

const char* device;
char* interfaceTag = "interface";
char* trafficTag = "traffic";
char* daysTag = "days";
char* dayTag = "day";
char* rxTag = "rx";
char* txTag = "tx";
static string xmlFile;

ParseTree::ParseTree(){
  try{
    XMLPlatformUtils::Initialize();
  } catch(const XMLException& error){
    XERCES_STD_QUALIFIER cerr << "Error during initialization: \n"
        << StrX(error.getMessage()) << XERCES_STD_QUALIFIER endl;
  }
}

ParseTree::~ParseTree(){
  
}

char** ParseTree::parseFile(string fileToParse) throw(runtime_error){
  struct stat fileStatus;
  char** rx_tx;
  
  rx_tx = (char**) malloc(1024);
  
  int iretStat = stat(fileToParse.c_str(), &fileStatus);
  if(iretStat == ENOENT)
    throw(runtime_error("File does not exist"));
  else if(iretStat == ENOTDIR)
    throw(runtime_error("Invalid path"));
  else if(iretStat == ELOOP)
    throw(runtime_error("Too many symbolic links encountered while traversing path"));
  else if(iretStat == EACCES)
    throw(runtime_error("Permission Denied"));
  else if(iretStat == ENAMETOOLONG)
    throw(runtime_error("File cannot be read :("));
  else if(iretStat == -1)  
    throw(runtime_error("Something went wrong. Check the provided file"));
  
  DOMElement* interfaceElem;
  DOMElement* trafficElem;
  DOMElement* daysElem;
  DOMElement* dayElem;
  DOMElement* rxElem;
  DOMElement* txElem;
  
  parser = new XercesDOMParser;
  parser->setValidationScheme(XercesDOMParser::Val_Never);
  parser->setDoNamespaces(false);
  parser->setDoSchema(false);
  parser->setLoadExternalDTD(false);
  
  try{
    parser->parse(fileToParse.c_str());
    DOMDocument* xmlDoc = parser->getDocument();
    DOMElement* elementRoot = xmlDoc->getDocumentElement();
    if(!elementRoot){
      XERCES_STD_QUALIFIER cerr << "Couldn't parse the provided file\nMaybe it's not valid XML?" << XERCES_STD_QUALIFIER endl;
      XMLPlatformUtils::Terminate();
    }
    DOMNodeList* dayElements = xmlDoc->getElementsByTagName(XMLString::transcode(dayTag));
    if(!(dayElements == NULL)){
      DOMNode* dayNode = dayElements->item(2);
      DOMNodeList* theMap = dayNode->getChildNodes();
      DOMNode* rxNode = theMap->item(1);
      DOMNode* txNode = theMap->item(2);
      rx_tx[0] = XMLString::transcode(rxNode->getTextContent());
      rx_tx[1] = XMLString::transcode(txNode->getTextContent());
    } else {
      cout << "fonud 0 elements" << endl;
    }
    
  } catch(const XMLException& e){
    XERCES_STD_QUALIFIER cerr << "Error: \n"
        << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
    XMLPlatformUtils::Terminate();
  }
  
  XMLPlatformUtils::Terminate();
  return rx_tx;
}

DOMElement* ParseTree::findElement(DOMElement* currentElement, const char* lostTag){
  XMLCh* tagSearchFor = XMLString::transcode(lostTag);
  DOMElement* elementFound;
  DOMNodeList* children = currentElement->getChildNodes();
  const XMLSize_t nodeCount = children->getLength();
  cout << "searching for elem <" << lostTag << ">" << endl;
  for(XMLSize_t xx = 0; xx < nodeCount; ++xx){
    DOMNode* currentNode = children->item(xx);
    if(currentNode->getNodeType() && currentNode->getNodeType() == DOMNode::ELEMENT_NODE){
      DOMElement* currentElement = dynamic_cast < DOMElement* > (currentNode);
      if(XMLString::equals(currentElement->getTagName(), tagSearchFor)){
        cout << "found tag " << lostTag << endl;
        return(currentElement);
      }
    }
  }
}

bool ParseTree::hasChildren(DOMElement* elem){
  const XMLSize_t count = elem->getChildElementCount();
  return count == 0 ? false : true;
}



int main(int argc, char* argv[]){
  
  ParseTree p;
  SendData send_data;
  JSONNode* root;
  JSONNode* name; 
  JSONNode* value;
  char* jsoned;
  char** rx_tx;
  
  if(argc < 2){
    usage();
    XMLPlatformUtils::Terminate();
    return 1;
  }
  
  int parmInd;
  for(parmInd = 1; parmInd < argc; parmInd++){
    if(argv[parmInd][0] != '-')
      break;
    
    if(!strncmp(argv[parmInd], "-d=", 3)){
      device = &argv[parmInd][3];
    }
      
    if(!strcmp(argv[parmInd], "-?")){
      XERCES_STD_QUALIFIER cout << "Printing Help" << XERCES_STD_QUALIFIER endl;
      usage();
      XMLPlatformUtils::Terminate();
      return 2;
    }
  }
  
  if(device == NULL){
    usage();
    XERCES_STD_QUALIFIER cerr << "\n\nYou must provide a device to look for " 
      << "(Hint: no spaces)\n" << XERCES_STD_QUALIFIER endl;
    XMLPlatformUtils::Terminate();
    return 3;
  }
  
  if(parmInd + 1 != argc){
    usage();
    XMLPlatformUtils::Terminate();
    return 1;
  }
  xmlFile = argv[parmInd];
  rx_tx = p.parseFile(xmlFile);
  in_addr* addr_info = send_data.get_ip_addr("localhost");
  send_data.connect_to_server(addr_info, 11300);
  send_data.select_tube("pingstatus");
  
  root = libJSON::NewNode("", "{}");
  name = root->AddNewStringChild("rx", rx_tx[0]);
  value = root->AddNewChild("tx", rx_tx[1]);
  jsoned = strdup((root->Write()).c_str());
  send_data.put_data(jsoned);
  
  
  return 0;
}

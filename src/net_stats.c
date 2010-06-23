#include "net_stats.h"
#include "file_drudge.h"

using namespace std;

static void usage()
{
    XERCES_STD_QUALIFIER cout << "\nUsage:\n"
            "    netstats <XML config file>\n\n"
            "This program grabs the Rx and Tx of the network as printed by\n"
            "the xml output of the vnstat program. The xml file must already\n"
            "exist.\n\n"
            "Options:\n"
            "      -?            - Show this help.\n\n"
         << XERCES_STD_QUALIFIER endl;
}

char* dayTag = "day";
char* config_file;

ParseTree::ParseTree(){
  try{
    XMLPlatformUtils::Initialize();
    parser = new XercesDOMParser;
    parser->setValidationScheme(XercesDOMParser::Val_Never);
    parser->setDoNamespaces(false);
    parser->setDoSchema(false);
    parser->setLoadExternalDTD(false);
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
  
  try{
    parser->parse(fileToParse.c_str());
    DOMDocument* xmlDoc = parser->getDocument();
    if(!xmlDoc){
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

char*
ParseTree::strrep(char *orig, char *search, char *repl) {
  int i, count = 0;
  int repllen = strlen(repl);
  int searchlen = strlen(search);
 
  for (i = 0; orig[i]; ++i)
    if (strstr(&orig[i], search) == &orig[i])
      ++count, i += searchlen - 1;
 
  char *ret = (char *) calloc(i + 1 + count * (repllen - searchlen), sizeof(char));
  if (!ret) return "";
 
  i = 0;
  while (*orig)
    if (strstr(orig, search) == orig)
      strcpy(&ret[i], repl),
      i += repllen,
      orig += searchlen;
    else
      ret[i++] = *orig++;
   
  ret[i] = '\0';

  return ret;
}

void
ParseTree::parse_config_file(char* file){
  struct stat fileStatus;
  char* node_name;
  char* node_text;
  DOMNode* current_node;
  
  int iretStat = stat(file, &fileStatus);
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

  try{
    parser->parse(file);
    DOMDocument* xmlDoc = parser->getDocument();
    if(!xmlDoc){
      XERCES_STD_QUALIFIER cerr << "Couldn't parse the provided file\nMaybe it's not valid XML?" << XERCES_STD_QUALIFIER endl;
      XMLPlatformUtils::Terminate();
    }
    DOMNodeList* configElement = xmlDoc->getElementsByTagName(XMLString::transcode("configurations"));
    DOMNodeList* child_elements = configElement->item(0)->getChildNodes();
    for(int el=0; el < child_elements->getLength(); el++){
      current_node = child_elements->item(el);
      node_name = XMLString::transcode(current_node->getNodeName());
      node_text = XMLString::transcode(current_node->getTextContent());
      if(node_text == "") continue;
      
      if(strcmp(node_name, "mem_file") == 0)
        strcpy(configs.mem_file, node_text);
      else if(strcmp(node_name, "cpu_file") == 0)
        configs.cpu_file = node_text;
      else if(strcmp(node_name, "beanstalk_host") == 0)
        configs.beanstalk_addr = node_text;
      else if(strcmp(node_name, "beanstalk_port") == 0)
        configs.bean_port = atoi(node_text);
      else if(strcmp(node_name, "tube_name") == 0)
        configs.tube_name = node_text;
      else if(strcmp(node_name, "network_stats_file") == 0)
        configs.net_stats_xml = node_text;
    }    
  } catch(const XMLException& e){
    XERCES_STD_QUALIFIER cerr << "Error: \n"
        << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
    XMLPlatformUtils::Terminate();
  }
  
  if(configs.mem_file == NULL)
    throw(runtime_error("You must specify the location of the memory file"));
  else if(configs.cpu_file == NULL)
    throw(runtime_error("You must specify the location of the load average file"));
  else if(configs.beanstalk_addr == NULL)
    throw(runtime_error("You must specify the address to the beanstalk server"));
  else if(configs.bean_port == 0)
    throw(runtime_error("You must specify the beanstalk port to connect to"));
  else if(configs.tube_name == NULL)
    throw(runtime_error("You must specify the beanstalk tube name to use"));
  else if(configs.net_stats_xml == NULL)
    throw(runtime_error("You must specify the location of the network stats xml file"));
}


int main(int argc, char* argv[]){
  FileDrudge fd;
  ParseTree p;
  SendData send_data;
  
  JSONNode* root;
  JSONNode* name; 
  JSONNode* value;
  JSONNode* network;
  JSONNode* mem_data;
  JSONNode* cpu_data;
  
  char* jsoned;
  char** rx_tx;
  char* mem_file_data;
  char* cpu_file_data;
  mem_file_data = (char*)malloc(51200);
  cpu_file_data = (char*)malloc(1024);
  
  if(argc < 2){
    usage();
    XMLPlatformUtils::Terminate();
    return 1;
  }
  
  int parmInd;
  for(parmInd = 1; parmInd < argc; parmInd++){
    if(argv[parmInd][0] != '-')
      break;
      
    if(!strcmp(argv[parmInd], "-?")){
      XERCES_STD_QUALIFIER cout << "Printing Help" << XERCES_STD_QUALIFIER endl;
      usage();
      XMLPlatformUtils::Terminate();
      return 2;
    }
  }
  
  if(parmInd + 1 != argc){
    usage();
    XMLPlatformUtils::Terminate();
    return 1;
  }
  
  config_file = argv[parmInd];
  p.parse_config_file(config_file);
  
  
  rx_tx = p.parseFile(p.configs.net_stats_xml);
  in_addr* addr_info = send_data.get_ip_addr(strdup(p.configs.beanstalk_addr));
  send_data.connect_to_server(addr_info, p.configs.bean_port);
  send_data.select_tube(p.configs.tube_name);
  
  mem_file_data = fd.read_file(p.configs.mem_file);
  cpu_file_data = fd.read_file(p.configs.cpu_file);
  
  //cout << mem_file_data << endl;
  
  root = libJSON::NewNode("", "{}");
  network = libJSON::NewNode("", "{}");
  mem_data = libJSON::NewNode("", "[]");
  cpu_data = libJSON::NewNode("", "[]");
  
  name = network->AddNewStringChild("rx", rx_tx[0]);
  value = network->AddNewChild("tx", rx_tx[1]);
  
  root->AddNewChild("network", network->Write());
  root->AddNewStringChild("mem_data", p.strrep(mem_file_data, "\n", "\\\\n"));
  root->AddNewStringChild("cpu_data", cpu_file_data);
  jsoned = strdup((root->Write()).c_str());
  send_data.put_data(p.strrep(jsoned, "\"", "\\\""));
  
  
  return 0;
}

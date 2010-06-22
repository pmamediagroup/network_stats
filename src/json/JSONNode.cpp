#include "JSONNode.h"
#include "JSON_Worker.h"
#include "JSON_Defs.h"
#include "Debug.h"
#ifdef _LINUX_
    #include "stdlib.h"
#endif

#ifdef DEBUG
	extern ErrorCallback Error;
#endif


JSONNode::JSONNode(const json_string & name_t, const json_string & value_t){
    _name = Worker::FixString(name_t, name_encoded);
    SetMyValue(value_t);
}

JSONNode::JSONNode(JSONNode * parent){
    libJSON_ASSERT(parent, TEXT("Adding this to a paren't, but no parent"));
    parent -> Children.push_back(this);
}

JSONNode::~JSONNode(void){
    libJSON_ASSERT(this, TEXT("Calling a destructor on a null pointer"));
    PurgeChildren();
}

void JSONNode::SetMyValue(const json_string & value_t){
    /*
	   This takes a json_string the way that it would appear in the JSON, meaing that
	   children and arrays still have the brackets around them, as importantly,
	   json_string literals still have the quotation marks around them

	   String are not unencoded, this way it's really fast, it will get unencoded if it gets requested
	   Children are not created, they are created if one of them gets requested, this makes ignored nodes faster
	   Arrays are not created, for the same reason
    */
    libJSON_ASSERT(this, TEXT("this ptr is null for SetMyValue"));

    if (value_t.empty()){
        type = JSON_NULL;
        return;
    }

    _string = value_t;
    const json_char firstchar = value_t[0];
    #ifdef DEBUG
        const json_char lastchar = value_t[value_t.length() - 1];
    #endif

    switch (firstchar){
        case TEXT('\"'):  //a json_string literal, still escaped and with leading and trailing quotes
            libJSON_ASSERT(lastchar == TEXT('\"'), TEXT("Unterminated quote"));
            type = JSON_STRING;
            Fetched = false;
            break;
        case TEXT('{'):  //a child node, or set of children
            libJSON_ASSERT(lastchar == TEXT('}'), TEXT("Missing }"));
            type = JSON_NODE;
            Fetched = false;
            break;
        case TEXT('['):  //an array
            libJSON_ASSERT(lastchar == TEXT(']'), TEXT("Missing ]"));
            type = JSON_ARRAY;
            Fetched = false;
            break;
        case TEXT('t'):
            libJSON_ASSERT(value_t == TEXT("true"), json_string(json_string(TEXT("unknown JSON literal: ")) + value_t).c_str());
            _value._bool = true;
            type = JSON_BOOL;
            Fetched = true;
            break;
        case TEXT('f'):
            libJSON_ASSERT(value_t == TEXT("false"), json_string(json_string(TEXT("unknown JSON literal: ")) + value_t).c_str());
            _value._bool = false;
            type = JSON_BOOL;
            Fetched = true;
            break;
        case TEXT('n'):
            libJSON_ASSERT(value_t == TEXT("null"), json_string(json_string(TEXT("unknown JSON literal: ")) + value_t).c_str());
            type = JSON_NULL;
            Fetched = true;
            break;
        default:
            libJSON_ASSERT(value_t.find_first_not_of(TEXT("0123456789.e+-")) == json_string::npos, json_string(json_string(TEXT("unknown JSON literal: ")) + value_t).c_str());
            Fetched = false;
            type = JSON_NUMBER;
            break;
    }
}

void JSONNode::PurgeChildren(void){
    libJSON_ASSERT(this, TEXT("this ptr is null for PurgeChildren"));
    //Remove all of the child nodes as well as delete them
    foreach(it, Children){
	   delete *it;
    }
    Children.clear();
}

void JSONNode::Fetch(void){
    libJSON_ASSERT(this, TEXT("this ptr is null for Fetch"));
    /*
	   Strings, objects, and arrays are not fully created on initalization, this is for speed reasons,
	   if the node is requested by the program, this method finishes the fetch
    */
    if (Fetched) return;
    switch (type){
	   case JSON_STRING:
		  libJSON_ASSERT(!_string.empty(), TEXT("JSON json_string type is empty?"));
		  libJSON_ASSERT(_string[0] == TEXT('\"'), TEXT("JSON json_string type doesn't start with a quotation?"));
		  libJSON_ASSERT(_string[_string.length() - 1] == TEXT('\"'), TEXT("JSON json_string type doesn't end with a quotation?"));
		  _string = Worker::FixString(_string.substr(1, _string.length() - 2), string_encoded);
		  break;
	   case JSON_NODE:
		  libJSON_ASSERT(!_string.empty(), TEXT("JSON node type is empty?"));
		  libJSON_ASSERT(_string[0] == TEXT('{'), TEXT("JSON node type doesn't start with a bracket?"));
		  libJSON_ASSERT(_string[_string.length() - 1] == TEXT('}'), TEXT("JSON node type doesn't end with a bracket?"));
		  Worker::DoNode(this, _string);
		  _string.clear();
		  _value.lastreal = Children.size();
		  break;
	   case JSON_ARRAY:
          libJSON_ASSERT(!_string.empty(), TEXT("JSON node type is empty?"));
          libJSON_ASSERT(_string[0] == TEXT('['), TEXT("JSON node type doesn't start with a square bracket?"));
          libJSON_ASSERT(_string[_string.length() - 1] == TEXT(']'), TEXT("JSON node type doesn't end with a square bracket?"));
		  Worker::DoArray(this, _string);
		  _string.clear();
		  _value.lastreal = Children.size();
		  break;
       case JSON_NUMBER:
            #ifdef JSON_UNICODE
                const size_t len = _string.length();
                char * temp = (char*)malloc(len + 1);
                wcstombs(temp, _string.c_str(), len);
                temp[len] = '\0';
                _value._number = atof(temp);
                free(temp);
            #else
                _value._number = atof(_string.c_str());
            #endif
            break;
    }
    Fetched = true;
}

void JSONNode::CloneTo(JSONNode * node){
    libJSON_ASSERT(this, TEXT("this ptr is null for CloneTo"));
	node -> type = type;
	node -> Fetched = Fetched;
	node -> _name = _name;
	node -> name_encoded = name_encoded;
	node -> _string = _string;
	node -> string_encoded = string_encoded;
	node -> _value = _value;
	foreach(it, Children){
        JSONNode * child = new JSONNode(node);
        libJSON_ASSERT(child, TEXT("Couldn't clone one of the children, out of memory"));
		(*it) -> CloneTo(child);
	}
}

void JSONNode::Add(JSONNode * child){
    libJSON_ASSERT(child, TEXT("Adding a null child"));
    Fetch();
    Children.push_back(child);
    libJSON_ASSERT(type == JSON_NODE || type == JSON_ARRAY, TEXT("Using the Add method for not a node or array"));
    _value.lastreal = Children.size();
}

json_string JSONNode::WriteName(bool formatted) const {
    libJSON_ASSERT(this, TEXT("Writing a name with no node"));
    //Write the name is the proper json way
    return (_name.empty()) ? TEXT("") : TEXT("\"") + Worker::UnfixString(_name, name_encoded) + ((formatted) ? TEXT("\" : ") : TEXT("\":"));
}

json_string JSONNode::WriteChildren(unsigned int indent){
    libJSON_ASSERT(this, TEXT("this ptr is null for WriteChildren"));
    //Iterate through the children and write them
    if (Children.empty()) return TEXT("");

    json_string indent_plus_one;
    json_string indent_this;
    json_string res;
    //handle whether or not it's formatted JSON
    if (indent != 0xFFFFFFFF){  //it's formatted, make the indentation strings
	   indent_this = TEXT("\r\n") + json_string(indent, TEXT('\t'));
	   indent_plus_one = TEXT("\r\n") + json_string(++indent, TEXT('\t'));
    }
    //else it's not formatted, leave the indentation strings empty
    const unsigned int size_minus_one = _value.lastreal - 1;
    unsigned int i = 0;
    bool commaafter;
    foreach(it, Children){
	   res += indent_plus_one + (*it) -> Write(indent, commaafter);
	   if (commaafter && i < size_minus_one) res += TEXT(",");  //the last one does not get a comma, but all of the others do
	   ++i;
    }
    return res + indent_this;
}

json_string JSONNode::WriteComment(unsigned int indent) const {
    libJSON_ASSERT(this, TEXT("Writing a comment with no object"));
    size_t pos = _string.find(TEXT('\n'));
    if (pos == json_string::npos){  //Single line comment
        return TEXT("//") + _string;
    }

    /*
        Multiline comments
    */
    json_string result = TEXT("/*\r\n") + json_string(indent + 1, TEXT('\t'));
    size_t old = 0;
    while(pos != json_string::npos){
        if (pos && _string[pos - 1] == TEXT('\r')) --pos;
        result += _string.substr(old, pos - old) + TEXT("\r\n") + json_string(indent + 1, TEXT('\t'));
        old = (_string[pos] == TEXT('\r')) ? pos + 2 : pos + 1;
        pos = _string.find(TEXT('\n'), old);
    }
    return result + _string.substr(old, pos - old) + TEXT("\r\n") + json_string(indent, TEXT('\t')) + TEXT("*/");
}

json_string JSONNode::Write(unsigned int indent, bool & commaafter){
    libJSON_ASSERT(this, TEXT("this ptr is null for Write"));
    commaafter = true;
    const bool formatted = indent != 0xFFFFFFFF;
    if (!(formatted || Fetched)){  //It's not formatted or fetched, just do a raw dump
        return WriteName(false) + _string;
    }

    //It's either formatted or fetched
    switch (type){
	   case JSON_NODE:   //got members, write the members
            Fetch();
            return WriteName(formatted) + TEXT("{") + WriteChildren(indent) + TEXT("}");
	   case JSON_ARRAY:	   //write out the child nodes int he array
            Fetch();
            return WriteName(formatted) + TEXT("[") + WriteChildren(indent) + TEXT("]");
	   case JSON_NUMBER:   //write out a literal, without quotes
	   case JSON_NULL:
	   case JSON_BOOL:
            return WriteName(formatted) + _string;
       case JSON_COMMENT:
            commaafter = false;
            return formatted ? WriteComment(indent) : TEXT("");
    }

    libJSON_ASSERT(type == JSON_STRING, TEXT("Writing an unknown JSON node type"));
    //If it go here, then it's a json_string
    if (Fetched) return WriteName(formatted) + TEXT("\"") + Worker::UnfixString(_string, string_encoded) + TEXT("\"");  //It's already been fetched, meaning that it's unescaped
    return WriteName(formatted) + _string;  //it hasn't yet been fetched, so it's already unescaped, just do a dump
}


#ifdef DEBUG
    unsigned int JSONNode::GetMemoryUsage(void){
        unsigned int total = sizeof(JSONNode) +
					   _name.capacity() * sizeof(json_char) +
					   _string.capacity() * sizeof(json_char);
        foreach(it, Children){
            total += (*it) -> GetMemoryUsage();
        }
        return total;
    }

    #define OUT result += json_string(indent, TEXT('\t')) +
    #include "JSON_StringConv.h"

    json_string JSONNode::MemDump(unsigned int indent){
        json_string result;
        switch(type){
            case JSON_NULL:
                OUT TEXT("Type: JSON_NULL\n");
                break;
            case JSON_STRING:
                OUT TEXT("Type: JSON_STRING\n");
                break;
            case JSON_NUMBER:
                OUT TEXT("Type: JSON_NUMBER\n");
                break;
            case JSON_BOOL:
                OUT TEXT("Type: JSON_BOOL\n");
                break;
            case JSON_ARRAY:
                OUT TEXT("Type: JSON_ARRAY\n");
                break;
            case JSON_NODE:
                OUT TEXT("Type: JSON_NODE\n");
                break;
            case JSON_COMMENT:
                OUT TEXT("Type: JSON_COMMENT\n");
                break;
            default:
                OUT TEXT("Type: undefined\n");
                break;
        }

        OUT TEXT("Fetched: ") + ((Fetched)?TEXT("true\n"):TEXT("false\n"));
        OUT TEXT("_name: ") + _name + TEXT("\n");
        OUT TEXT("name_encoded: ") + ((Fetched)?TEXT("true\n"):TEXT("false\n"));
        OUT TEXT("_name's allocation: ") + ToString(_name.capacity() * sizeof(json_char)) + TEXT(" bytes\n");
        OUT TEXT("_string: ") + _string + TEXT("\n");
        OUT TEXT("string_encoded: ") + ((string_encoded)?TEXT("true\n"):TEXT("false\n"));
        OUT TEXT("_string's allocation: ") + ToString(_string.capacity() * sizeof(json_char)) + TEXT(" bytes\n");
        OUT TEXT("_bool: ") + ((_value._bool)?TEXT("true\n"):TEXT("false\n"));
        OUT TEXT("_number: ") + ToString(_value._number) + TEXT("\n");
        OUT TEXT("lastreal: ") + ToString(_value.lastreal) + TEXT("\n");

        foreach(it, Children){
            result += (*it) -> MemDump(indent + 1) + TEXT("\n");
        }
        return result;
    }
#endif

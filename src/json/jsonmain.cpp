#include "Debug.h"

#include "jsonmain.h"
#include "JSON_Defs.h"
#include "JSONNode.h"
#include "JSON_Worker.h"
#ifdef _LINUX_
    #include "stdlib.h"
    #include <cstring>
#endif

#ifdef DEBUG
	ErrorCallback Error = 0;
#endif

using namespace std;

#ifndef JSON_DLL
    #define Parent this
    #define Nthis this
    #define RETURN_NODE JSONNODE *
    #define STRING json_string &
    #define STRINGREF const json_string &
    #define TOSTRINGREF(str) str
    #define PARENT_FUNC(funcname)\
	   JSONNode::funcname(const json_string & Name){\
	       libJSON_ASSERT(this, (json_string(TEXT(#funcname)) + TEXT(" with no this")).c_str());
    #define PARENT_FUNC2(funcname, other)\
	   JSONNode::funcname(other){\
	       libJSON_ASSERT(this, (json_string(TEXT(#funcname)) + TEXT(" with no this")).c_str());
    #define PARENT_FUNC3(funcname, other, other2)\
	   JSONNode::funcname(other, other2){\
	       libJSON_ASSERT(this, (json_string(TEXT(#funcname)) + TEXT(" with no this")).c_str());
    #define THIS_FUNC(funcname)\
        JSONNode::funcname(void){\
	       libJSON_ASSERT(this, (json_string(TEXT(#funcname)) + TEXT(" with no this")).c_str());
    #define THIS_FUNC2(funcname, other)\
        JSONNode::funcname(other){\
	       libJSON_ASSERT(this, (json_string(TEXT(#funcname)) + TEXT(" with no this")).c_str());
    #define ATTR_FUNC(funcname)\
	   JSONNode::funcname(void){
#else
    #define RETURN_NODE void *
    #define STRING json_char *
    #define STRINGREF const json_char *
    #define TOSTRINGREF(str) str.c_str()
    #define PARENT_FUNC(funcname)\
	   JSON_##funcname(const JSONNODE parent, const json_char * name){\
		  libJSON_ASSERT(parent, (json_string(TEXT("JSON_")) + TEXT(#funcname) + TEXT(" with no parent")).c_str());\
		  libJSON_ASSERT(name, (json_string(TEXT("JSON_")) + TEXT(#funcname) + TEXT(" with no name")).c_str());\
		  JSONNode * Parent = (JSONNode*)parent;\
		  const json_string Name(name);
    #define PARENT_FUNC2(funcname, other)\
        JSON_##funcname(const JSONNODE parent, other){\
           libJSON_ASSERT(parent, (json_string(TEXT("JSON_")) + TEXT(#funcname) + TEXT(" with no parent")).c_str());\
           JSONNode * Parent = (JSONNode*)parent;
    #define PARENT_FUNC3(funcname, other, other2)\
	   JSON_##funcname(const JSONNODE parent, other, other2){\
           libJSON_ASSERT(parent, (json_string(TEXT("JSON_")) + TEXT(#funcname) + TEXT(" with no parent")).c_str());\
           JSONNode * Parent = (JSONNode*)parent;
    #define THIS_FUNC(funcname)\
        JSON_##funcname(const JSONNODE node){\
           libJSON_ASSERT(node, (json_string(TEXT("JSON_")) + TEXT(#funcname) + TEXT(" with no node")).c_str());\
           JSONNode * Nthis = (JSONNode*)node;
    #define THIS_FUNC2(funcname, other)\
        JSON_##funcname(const JSONNODE node, other){\
           libJSON_ASSERT(node, (json_string(TEXT("JSON_")) + TEXT(#funcname) + TEXT(" with no node")).c_str());\
           JSONNode * Nthis = (JSONNode*)node;
    #define ATTR_FUNC(funcname)\
	   JSON_##funcname(const JSONNODE node){\
		  libJSON_ASSERT(node,(json_string(TEXT("JSON_")) + TEXT(#funcname) + TEXT(" with no node")).c_str());\
		  JSONNode * Nthis = (JSONNode*)node;
#endif

#ifdef JSON_DLL
RETURN_NODE JSON_Parse(const STRING json){
#else
RETURN_NODE libJSON::Parse(const STRING json){
#endif
    JSONNode * result = 0;
    #ifdef DEBUG
        json_char lastchar;
        json_char * s = Worker::RemoveWhiteSpace(json, lastchar);
    #else
        json_char * s = Worker::RemoveWhiteSpace(json);
    #endif
    const json_char firstchar = s[0];
    switch (firstchar){
        case '{':
        case '[':
            #ifdef DEBUG
                if (firstchar == '['){
                    libJSON_ASSERT(lastchar == ']', TEXT("Missing final ]"));
                } else {
                    libJSON_ASSERT(lastchar == '}', TEXT("Missing final ]"));
                }
            #endif
            result = new JSONNode(TEXT(""), s);
            break;
        default:
            libJSON_FAIL(TEXT("Not JSON!"));
            break;
    }

    free(s);
    return result;
}

#ifdef JSON_DLL
void JSON_Preparse(const JSONNODE node){
    libJSON_ASSERT(node, TEXT("Preparsing a null node"));
    JSONNode * Nthis = (JSONNode*)node;
#else
void JSONNode::Preparse(void){
#endif
    Nthis -> Fetch();
    foreach(it, Nthis -> Children){
        #ifdef JSON_DLL
            JSON_Preparse(*it);
        #else
            (*it) -> Preparse();
        #endif
    }
}

RETURN_NODE PARENT_FUNC(GetNode)
    Parent -> Fetch();
    foreach(it, Parent -> Children){
	   if (Name == (*it) -> _name) return *it;
    }
    return 0;
}


static bool AreEqualNoCase(const json_char * ch_one, const json_char * ch_two){
    while (*ch_one){  //only need to check one, if the other one terminates early, the check will cause it to fail
	   const json_char c_one = *ch_one;
	   const json_char c_two = *ch_two;
	   if (c_one != c_two){
			 if ((c_two > 64) && (c_two < 91)){  //A - Z
				if (c_one != (json_char)(c_two + 32)) return false;
			 } else if ((c_two > 96) && (c_two < 123)){  //a - z
				if (c_one != (json_char)(c_two - 32)) return false;
			 } else { //not a letter, so return false
				return false;
			 }
	   }
	   ++ch_one;
	   ++ch_two;

    }
    return *ch_two == '\0';  //this one has to be null terminated too, or else json_string two is longer, hence, not equal
}

RETURN_NODE PARENT_FUNC(GetNode_NoCase)
    Parent -> Fetch();
    foreach(it, Parent -> Children){
        #ifdef JSON_DLL
            if (AreEqualNoCase(name, (*it) -> _name.c_str())) return *it;
        #else
            if (AreEqualNoCase(Name.c_str(), (*it) -> _name.c_str())) return *it;
        #endif
    }
    return 0;
}

RETURN_NODE PARENT_FUNC(PopNode)
    Parent -> Fetch();
    foreach(it, Parent -> Children){
	   if (Name == (*it) -> _name){
			JSONNode * res = *it;
			Parent -> Children.erase(it);
			return res;
	   }
    }
    return 0;
}

RETURN_NODE PARENT_FUNC(PopNode_NoCase)
    Parent -> Fetch();
    foreach(it, Parent -> Children){
        #ifdef JSON_DLL
            if (AreEqualNoCase(name, (*it) -> _name.c_str())){
        #else
            if (AreEqualNoCase(Name.c_str(), (*it) -> _name.c_str())){
        #endif
			JSONNode * res = *it;
			Parent -> Children.erase(it);
			return res;
	   }
    }
    return 0;
}

RETURN_NODE PARENT_FUNC2(PopNodeAt, unsigned int position)
    libJSON_ASSERT(position < Parent -> Children.size(), TEXT("JSON_PopNodeAt out of bounds"));
    JSONNode * res = Parent -> Children[position];
    Parent -> Children.erase(Parent -> Children.begin() + position);
    return res;
}

#ifdef JSON_DLL
void JSON_Delete(const JSONNODE node){
#else
void libJSON::Delete(JSONNODE * node){
#endif
   libJSON_ASSERT(node, TEXT("JSON_Delete with no node"));
   delete (JSONNode*)node;
}


RETURN_NODE PARENT_FUNC2(NodeAt, unsigned int position)
    Parent -> Fetch();
    libJSON_ASSERT(Parent -> Children.size() > position, TEXT("JSON_NodeGetChildAt index out of range"));
    return Parent -> Children[position];
}

unsigned int ATTR_FUNC(NodeSize)
    Nthis -> Fetch();
    return Nthis -> Children.size();
}

char ATTR_FUNC(NodeType)
    return Nthis -> type;
}

STRINGREF ATTR_FUNC(NodeName)
    return TOSTRINGREF(Nthis -> _name);
}

STRINGREF ATTR_FUNC(NodeAsString)
    Nthis -> Fetch();
    return TOSTRINGREF(Nthis -> _string);
}

int ATTR_FUNC(NodeAsInt)
    libJSON_ASSERT(Nthis -> type == JSON_NUMBER, TEXT("JSON_NodeAsInt not of type number"));
    Nthis -> Fetch();
    libJSON_ASSERT(Nthis -> _value._number == (double)((int)Nthis -> _value._number), (json_string(TEXT("JSON_NodeAsInt, number truncated, actually a float value - ")) + Nthis -> _name + TEXT(" : ") + Nthis -> _string).c_str());
    return (int)Nthis -> _value._number;
}

bool ATTR_FUNC(NodeAsBool)
    libJSON_ASSERT(Nthis -> type == JSON_BOOL, TEXT("JSON_NodeAsBool not of type bool"));
    return Nthis -> _value._bool;
}

double ATTR_FUNC(NodeAsFloat)
    libJSON_ASSERT(Nthis -> type == JSON_NUMBER, TEXT("JSON_NodeAsFloat not of type number"));
    Nthis -> Fetch();
    return Nthis -> _value._number;
}

#ifdef JSON_DLL
JSONNODE JSON_NewNode(const json_char * name, const json_char * value){
#else
JSONNODE * libJSON::NewNode(const json_string & name, const json_string & value){
#endif
   return new JSONNode(name, value);
}

void PARENT_FUNC2(AddChild, const RETURN_NODE node)
    libJSON_ASSERT(node, TEXT("JSON_AddToChildren with no node"));
    Parent -> Add((JSONNode*)node);
}

RETURN_NODE PARENT_FUNC3(AddNewChild, const STRING name, const STRING value)
	JSONNode * n = new JSONNode(name, value);
	libJSON_ASSERT(n, TEXT("Couldn't create new child, out of memory"));
	Parent -> Add(n);
	return n;
}

#ifdef JSON_DLL
RETURN_NODE JSON_NewStringNode(const STRING name, const STRING value){
#else
RETURN_NODE libJSON::NewStringNode(const STRING name, const STRING value){
#endif
	JSONNode * n = new JSONNode(name, TEXT(""));
	libJSON_ASSERT(n, TEXT("Couldn't create new child, out of memory"));
	n -> type = JSON_STRING;
	n -> _string = value;
	n -> Fetched = true;
	return n;
}

RETURN_NODE PARENT_FUNC3(AddNewStringChild, const STRING name, const STRING value)
	JSONNode * n = new JSONNode(name, TEXT(""));
	libJSON_ASSERT(n, TEXT("Couldn't create new child, out of memory"));
	n -> type = JSON_STRING;
	n -> _string = value;
	n -> Fetched = true;
	Parent -> Add(n);
	return n;
}

RETURN_NODE THIS_FUNC(Duplicate)
	JSONNode * result = new JSONNode();
	libJSON_ASSERT(result, TEXT("Couldn't duplicate, out of memory"));
	Nthis -> CloneTo(result);
	return result;
}

void THIS_FUNC2(SetValue, const STRING value)
    //If it's got children, then the children are changing too
    switch (Nthis -> type){
       case JSON_NODE:
       case JSON_ARRAY:
          Nthis -> PurgeChildren();
    }

    Nthis -> SetMyValue(value);
}

void THIS_FUNC2(SetStringValue, const STRING value)
    libJSON_ASSERT(Nthis -> type == JSON_STRING, TEXT("JSON_SetStringValue, but not a json_string"));
    Nthis -> _string = value;
    Nthis -> Fetched = true;
}


//For writing JSON

void PARENT_FUNC2(AddComment, const STRING comment)
	JSONNode * n = new JSONNode();
	libJSON_ASSERT(n, TEXT("Couldn't create new child, out of memory"));
	n -> type = JSON_COMMENT;
	n -> _string = comment;
	n -> Fetched = true;
	Parent -> Fetch();
	Parent -> Children.push_back(n);
}

#ifdef JSON_DLL
    static json_char * writer(const JSONNODE node, unsigned int indent){
	   JSONNode * n = (JSONNode*)node;
	   bool unused;
	   const json_string res = n -> Write(indent, unused);
	   const unsigned int len = (res.length() + 1) * sizeof(json_char);
	   return (json_char*)memcpy(malloc(len), res.c_str(), len);
    }

    json_char * JSON_Write(const JSONNODE node){
	   libJSON_ASSERT(node, TEXT("JSON_Write with no node"));
	   return writer(node, 0xFFFFFFFF);
    }

    json_char * JSON_WriteFormatted(const JSONNODE node){
	   libJSON_ASSERT(node, TEXT("JSON_WriteFormatted with no node"));
	   return writer(node, 0);
    }

    void JSON_Free(json_char * str){
	   libJSON_ASSERT(str, TEXT("Freeing a null pointer"));
	   free(str);
    }
#else
    json_string JSONNode::Write(unsigned int indent){
	   bool temp;
	   return Write(indent, temp);
    }

    json_string JSONNode::WriteFormatted(void){
	    return Write(0);
    }
#endif

#ifdef DEBUG
    #ifdef JSON_DLL
    void JSON_RegisterErrorCallback(ErrorCallback method){
    #else
    void libJSON::RegisterErrorCallback(ErrorCallback method){
    #endif
		Error = method;
	}

    #include "JSON_StringConv.h"
    #ifdef JSON_DLL

    unsigned int JSON_GetMemoryUsage(const JSONNODE root){
        libJSON_ASSERT(root, TEXT("GetMemoryUsage a null node"));
        return ((JSONNode*)root) -> GetMemoryUsage();
    }


    json_char * JSON_Dump(const JSONNODE root){
        libJSON_ASSERT(root, TEXT("Dumping a null node"));
        JSONNode * Nthis = (JSONNode*) root;
    #else
    json_string JSONNode::Dump(void){
    #endif
        json_string result;  //can't use wstringstream because it's not supported on MinGW
        result += TEXT("Memory used: ") + ToString(Nthis -> GetMemoryUsage()) + TEXT(" bytes\n");
        result += TEXT("JSON Dump:\n") + Nthis -> MemDump(1) + TEXT("\n");

        #ifdef JSON_DLL
            const unsigned int len = (result.length() + 1) * sizeof(json_char);
            return (json_char*)memcpy(malloc(len), result.c_str(), len);
        #else
            return result;
        #endif
    }
#endif

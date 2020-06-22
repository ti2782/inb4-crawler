#ifndef METAHANDLER_H
#define METAHANDLER_H

#include <unordered_map>
#include <string>
#include <cstdio>
#include <memory>

#include "config.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

struct Meta
{
  std::string name, hash, text;
};

class MetaHandler
{
 private:
  std::unordered_map<std::string, Meta*> metaMap;
  std::string metaFileName;
  
 public:
  MetaHandler();
  ~MetaHandler();
  
  bool populateMetaMap();
  bool findMeta(const char* hash, std::string& text, std::string& name);
};

/* META JSON FILE STRUCTURE
{ 
  "metas" : [
     {
        "name": "",
	"hash": "",
	"txt": ""
     }
  ]
}
*/
#endif // METAHANDLER_H

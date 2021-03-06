#ifndef METAHANDLER_H
#define METAHANDLER_H

#include <unordered_map>
#include <string>
#include <cstdio>
#include <memory>
#include <iostream>

#include "config.h"
#include "meta.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"


class MetaHandler
{
 private:
  std::unordered_map<std::string, Meta*> metaMap;
  std::string configFileName;
  
 public:
  MetaHandler();
  ~MetaHandler();
  
  bool populateMetaMap();
  Meta* findMeta(const char* hash);
};

/* CONFIG JSON FILE STRUCTURE
{ 
  "accounts" [
     {
          "telegramChannel" : "@channel",
	  "telegramToken" : "8u348208730876986345",
	  "telegramPoll" : "Is this thread authorized?",	 
	  "twitterAccount" : "TweetKang"
     }
  ],
  "metas" : [
     {
        "name": "",
	"hash": "",
	"txt": "",
	"acc": 0
     }
  ]
}
*/
#endif // METAHANDLER_H

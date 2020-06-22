#include "metahandler.h"

using namespace rapidjson;

MetaHandler::MetaHandler()
{
  metaFileName.append(CONFIG_DIR);
  metaFileName.append(META_FILE);
}

MetaHandler::~MetaHandler()
{
  // FREE METAS
  for(const auto& meta : metaMap)
    if(meta.second)
      delete meta.second;
}

bool MetaHandler::populateMetaMap()
{
  // OPEN FILE
  FILE* metaFile = fopen(metaFileName.c_str(), "r");
  if(!metaFile)
    return false;

  char buff[2048];
  FileReadStream is(metaFile, buff, sizeof(buff));

  // PARSE JSON DOCUMENT
  Document doc;
  doc.ParseStream(is);
  fclose(metaFile);

  if(!doc.IsObject())
    return false;

  if(!doc.HasMember("metas"))
    return false;
  
  const Value& metaJson = doc["metas"];
  for( SizeType i = 0; i < metaJson.Size(); i++)    
  {
    Meta* meta = new Meta;
    if(metaJson[i].HasMember("name"))
      meta->name = metaJson[i]["name"].GetString();
    if(metaJson[i].HasMember("hash"))
      meta->hash = metaJson[i]["hash"].GetString();
    if(metaJson[i].HasMember("txt"))
      meta->text = metaJson[i]["txt"].GetString();

    if(!meta->name.empty() || !meta->hash.empty() || !meta->text.empty())
      metaMap.insert({meta->hash, meta});
  }
  return true;
}

bool MetaHandler::findMeta(const char* hash, std::string& text, std::string& name)
{
  auto search = metaMap.find(hash);
  if(search == metaMap.end())
    return false;

  text = search->second->text;
  name = search->second->name;
  return true;
}

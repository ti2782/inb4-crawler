#include "db.h"

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

Db::Db()
{
  dbName = DB;
}

Db::~Db()
{
  
}

void Db::addThread(int thread)
{
  mongocxx::client client{mongocxx::uri{}};
  mongocxx::database db = client[dbName.c_str()];
  mongocxx::collection coll = db["threadqueue"];

  bsoncxx::stdx::optional<bsoncxx::document::value> threadret = coll.find_one(document{} << "thread" << thread << finalize);
  if(threadret) {
    return;
  }
  
  bsoncxx::document::value document = bsoncxx::builder::basic::make_document(kvp("thread", thread));
  coll.insert_one(document.view()); 
}

int Db::getThread()
{
  std::vector<int> ret;
  mongocxx::client client{mongocxx::uri{}};
  mongocxx::database db = client[dbName.c_str()];
  mongocxx::collection coll = db["threadqueue"];
  auto opts = mongocxx::options::find{};
  opts.limit(1);
  
  mongocxx::cursor cursor = coll.find({}, opts);  
  
  for(auto&& doc : cursor)
    {       
      auto store = doc["thread"];      
      if(store)
	return store.get_int32();
    }
  
  return 0;
}

void Db::removeThread(int thread)
{
  mongocxx::client client{mongocxx::uri{}};
  mongocxx::database db = client[dbName.c_str()];
  mongocxx::collection coll = db["threadqueue"];
  
  bsoncxx::document::value document = bsoncxx::builder::basic::make_document(kvp("thread", thread));
  coll.delete_one(document.view());
}

void Db::clearThreads()
{
  mongocxx::client client{mongocxx::uri{}};
  mongocxx::database db = client[dbName.c_str()];
  mongocxx::collection coll = db["threadqueue"];

  auto document = bsoncxx::builder::stream::document{};
  coll.delete_one(document.view()); 
}

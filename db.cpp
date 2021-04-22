#include "db.h"
#include "config.h"

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

Db::Db()
{
  client = {mongocxx::uri{dbHost.c_str()}};
  db = client[dbName.c_str()];  
}

Db::~Db()
{
  
}

void Db::addThread(int thread)
{
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
  int ret = 0;
  mongocxx::collection coll = db["threadqueue"];
  auto opts = mongocxx::options::find{};
  opts.limit(1);
  
  mongocxx::cursor cursor = coll.find({}, opts);  
  
  for(auto&& doc : cursor)
    {       
      auto store = doc["thread"];      
      if(store)
	ret = store.get_int32();      
    }

  if(ret > 0)
    removeThread(ret);
  
  return ret;
}

void Db::removeThread(int thread)
{
  mongocxx::collection coll = db["threadqueue"];
  
  bsoncxx::document::value document = bsoncxx::builder::basic::make_document(kvp("thread", thread));
  coll.delete_one(document.view());
}

void Db::clearThreads()
{
  mongocxx::collection coll = db["threadqueue"];

  auto document = bsoncxx::builder::stream::document{};
  coll.delete_one(document.view()); 
}

void Db::addNotification(Notification* n)
{
  if(n)
    {
      mongocxx::collection coll = db["notifications"];
      
      bsoncxx::document::value document = bsoncxx::builder::basic::make_document(kvp("threadnum", n->threadnum),
										 kvp("postnum", n->postnum),
										 kvp("subject", n->subject),
										 kvp("comment", n->comment),
										 kvp("metatxt", n->metatxt),
										 kvp("name", n->name),
										 kvp("hashtags", n->hashtags),
										 kvp("account", n->account));

      bsoncxx::stdx::optional<bsoncxx::document::value> notret = coll.find_one(document.view());
      if(notret) {
	return;
      }
      
      coll.insert_one(document.view());
    }
}

std::vector<Notification*> Db::getNotifications()
{
  std::vector<Notification*> ret;
  mongocxx::collection coll = db["notifications"];
  auto opts = mongocxx::options::find{};
  opts.limit(1);
  
  mongocxx::cursor cursor = coll.find({}, opts);  
  
  for(auto&& doc : cursor)
    {
      Notification* n = new Notification;

      auto store = doc["threadnum"];
      if(store)
	n->threadnum = store.get_int32();

      store = doc["postnum"];
      if(store)
	n->postnum = store.get_int32();

      store = doc["account"];
      if(store)
	n->account = store.get_int32();

      store = doc["subject"];
      if(store)
	n->subject = store.get_utf8().value.to_string();

      store = doc["comment"];
      if(store)
	n->comment = store.get_utf8().value.to_string();
      
      store = doc["metatxt"];
      if(store)
	n->metatxt = store.get_utf8().value.to_string();

      store = doc["name"];
      if(store)
	n->name = store.get_utf8().value.to_string();

      store = doc["hashtags"];
      if(store)
	n->hashtags = store.get_utf8().value.to_string();

      ret.push_back(n);
    }
  
  return ret;
}

void Db::clearNotifications()
{
  mongocxx::collection coll = db["notifications"];

  auto document = bsoncxx::builder::stream::document{};
  coll.delete_one(document.view()); 
}

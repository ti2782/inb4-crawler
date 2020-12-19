#ifndef DB_H
#define DB_H

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/string/to_string.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

#define DB "inb4"

class Db
{
 private:
  mongocxx::instance instance{}; // This should be done only once.
  std::string dbName;
 public:
  Db();
  ~Db();

  void addThread(int thread);
  void removeThread(int thread);
  void clearThreads();
  int getThread();

  void switchDB(const char* db) { if(db) dbName.clear(); dbName = db; } ;
};

#endif

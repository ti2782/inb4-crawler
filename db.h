#ifndef DB_H
#define DB_H

#include "meta.h"

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
  mongocxx::client client;
  mongocxx::database db;
  std::string dbName = DB;
  std::string dbHost = "mongodb://127.0.0.1:27017";
 public:
  Db();
  ~Db();

  void addThread(int thread);
  void removeThread(int thread);
  void clearThreads();
  int getThread();
  void addNotification(Notification*);
  std::vector<Notification*> getNotifications();
  void clearNotifications();

  void switchDB(const char* db) { dbName = db; } ;
  void switchDBHost(const char* host) { dbHost = host; client = {mongocxx::uri{dbHost.c_str()}}; db = client[dbName.c_str()]; } ;
};

#endif

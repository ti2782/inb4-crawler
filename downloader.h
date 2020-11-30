#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <curl/curl.h>

#include "rapidjson/document.h"

#include "config.h"
#include "metahandler.h"

struct Notification
{
  int threadnum, postnum, account;
  std::string subject, comment, metatxt, name, hashtags;
};

class Downloader
{
 private:   
  MetaHandler metaHandler;
  
  std::vector<int> threadnums;
  std::mutex guard;
  std::vector<Notification*> notVec;
  
 public:
  Downloader();
  ~Downloader();

  std::vector<int> downloadThreadList();
  void downloadThread(int threadnum);
  std::vector<Notification*> getNotifications();
};

#endif

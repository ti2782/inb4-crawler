#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <vector>
#include <iostream>
#include <thread>
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
  CURL *curl;

  std::vector<int> threadnums;
  
 public:
  Downloader();
  ~Downloader();

  std::vector<int> downloadThreadList();
  std::vector<Notification*> downloadThread(int threadnum);
};

#endif

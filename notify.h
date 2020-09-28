#ifndef NOTIFY_H
#define NOTIFY_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "config.h"
#include <curl/curl.h>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"

struct Account
{
  std::string telegramChannel, telegramToken, telegramPoll, twitter;
};

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}
  
class Notify
{
 private:
  CURL* curl;
  CURLcode res;
  
  bool isUniqueNotification(std::string notification);
  std::vector<Account*> accountVec;
  
 public:
  Notify();
  ~Notify();

  void sendNotification(int threadnum, int postnum, std::string subject, std::string comment, std::string metatxt, std::string name, std::string hashtags, int account);
  void convertASCII(std::string& text);
  void removeHtml(std::string& text);
};

#endif // NOTIFY_H

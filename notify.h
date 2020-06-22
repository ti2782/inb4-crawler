#ifndef NOTIFY_H
#define NOTIFY_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "config.h"

class Notify
{
 private:
  std::string telegramToken;

  bool isUniqueNotification(std::string notification);
 public:
  Notify();
  ~Notify();

  void sendNotification(int threadnum, int postnum, std::string subject, std::string comment, std::string metatxt, std::string name);
  void convertASCII(std::string& text);
  
};

#endif // NOTIFY_H

#ifndef META_H
#define META_H

#include <string>

struct Meta
{
  std::string name, hash, text, hashtags;
  int account;
};

struct Notification
{
  int threadnum, postnum, account;
  std::string subject, comment, metatxt, name, hashtags;
};

#endif

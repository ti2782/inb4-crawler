#include "notify.h"

Notify::Notify()
{
  // READ TELEGRAM BOT TOKEN
  std::string tokenFileName(CONFIG_DIR);
  tokenFileName.append(TELEGRAMTOKEN_FILE);
  std::ifstream telegramFile(tokenFileName);
  if(!telegramFile.is_open())
    std::cout << ">>WARNING\nTelegram bot TOKEN required for notifications!" << std::endl;
  else
    {
      std::getline(telegramFile, telegramToken);
      telegramFile.close();
    }
}

Notify::~Notify()
{
  
}

void Notify::sendNotification(int threadnum, int postnum, std::string subject, std::string comment, std::string metatxt, std::string name)
{  
  // CRAFT LINKS
  std::stringstream chan, archive, cmd;
  chan << LINKURL << threadnum << "#p" << postnum;  
  archive << ARCHIVEURL << threadnum << "/#" << postnum;   
  
  // CONFIRM UNIQUENESS
  if(isUniqueNotification(chan.str()))
    {  
      // SEND NOTIFICATION
      cmd << "inb4-notifier " << telegramToken << " \"" << metatxt << "\" "
	  << chan.str() << " " << archive.str();
      if(!subject.empty())
	{
	  convertASCII(subject);
	  cmd << " -s \"" << subject << "\"";
	}
      
      std::cout << ">>NEW NOTIFICTION\n" << name << "\n";
      if(!subject.empty())
	std::cout << subject << "\n";
      std::cout << chan.str() << std::endl;
      
      system(cmd.str().c_str());
    }
}

bool Notify::isUniqueNotification(std::string notification)
{
  // PARSE PREVIOUS NOTIFICATIONS
  std::string notifyFileName(CONFIG_DIR);
  notifyFileName.append(NOTIFY_FILE);
  std::ifstream notifyFileIN(notifyFileName);
  if(!notifyFileIN.is_open())
    {
      std::cout << ">>ERROR\nFailed to open notification file." << std::endl;
      return false;
    }  
  
  std::unordered_map<std::string, int> notificationMap;
  std::vector<std::string> notifications;
  std::string buff;
  int count = 0;
  while(std::getline(notifyFileIN, buff))
    {      
      notificationMap.insert({buff, count});
      notifications.push_back(buff);
      buff.clear();
      count++;
    }
  notifyFileIN.close();

  // IS NOTIFICATION NEW?
  auto search = notificationMap.find(notification);
  if(search == notificationMap.end())
    {
      // APPEND NEW NOTIFICATION
      notifications.push_back(notification);      
      if(notifications.size() >= MAX_NOTIFICATIONS)
	{
	  // REMOVE OLDEST NOTIFICATION
	  notifications.erase(notifications.begin());
	}
      
      // WRITE NEW FILE
      std::ofstream notifyFile(notifyFileName, std::ios::trunc);
      if(!notifyFile.is_open())
	{
	  std::cout << ">>ERROR\nFailed to open notification file." << std::endl;
	  return false;
	}

      for(int i = 0; i < notifications.size(); i++)
	{
	  notifyFile << notifications[i];
	  if(i < (MAX_NOTIFICATIONS - 1))
	    notifyFile << "\n";
	}
      
      notifyFile.close();
      
      return true;
    }
  
  return false;
}

void Notify::convertASCII(std::string& text)
{
  for(std::size_t pos = text.find("&#"); pos != std::string::npos; pos = text.find("&#", pos+1))
    {
      std::size_t end = text.find(";", pos);
      int ascii = std::atoi(text.substr(pos+2, end).c_str());
      if(ascii > 32 && ascii < 127)       
	{
	  char encoded = ascii;
	  text.replace(pos, (end + 1) - pos, &encoded);
	}
    }
}

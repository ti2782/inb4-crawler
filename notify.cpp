#include "notify.h"

using namespace rapidjson;

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

  //SET URLS
  std::stringstream url;
  url << "https://api.telegram.org/bot" << telegramToken << "/sendMessage";
  telegramURL = url.str();

  std::stringstream pollUrl;
  pollUrl << "https://api.telegram.org/bot" << telegramToken << "/sendPoll";
  telegramPollURL = pollUrl.str();  
}

Notify::~Notify()
{
  if(curl)
    curl_easy_cleanup(curl);
}

void Notify::sendNotification(int threadnum, int postnum, std::string subject, std::string comment, std::string metatxt, std::string name)
{
  std::string buff;
  
  if(!subject.empty())
    convertASCII(subject);

  // INIT CURL
  curl = curl_easy_init();
  
  // CRAFT LINKS
  std::stringstream chan, archive, cmd, tweet;
  chan << LINKURL << threadnum << "#p" << postnum;  
  archive << ARCHIVEURL << threadnum << "/#" << postnum;

  // CRAFT TELEGRAM MSG
  std::string msg;
  msg.append("chat_id=");
  msg.append(TELEGRAMCHAT_ID);
  msg.append("&parse_mode=markdown&text=``` ");
  msg.append(metatxt);
  msg.append("```\nLINK ");
  msg.append(chan.str());
  msg.append("\nARCHIVE LINK ");
  msg.append(archive.str());
  
  if(!subject.empty())
    {
      msg.append("\n*");
      msg.append(subject);
      msg.append("*");
    }
  
  // CONFIRM UNIQUENESS
  if(isUniqueNotification(chan.str()))
    {
      std::cout << ">>NEW NOTIFICTION\n" << name << "\n";
      if(!subject.empty())
	std::cout << subject << "\n";
      std::cout << chan.str() << std::endl;
      
      // SEND TELEGRAM NOTIFICATION
      if(!curl)
	return;
      
      int telegramID;

      curl_easy_setopt(curl, CURLOPT_URL, telegramURL.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
      
      /* Perform the request, res will get the return code */ 
      res = curl_easy_perform(curl);
      /* Check for errors */ 
      if(res != CURLE_OK)
    	std::cout << curl_easy_strerror(res) << std::endl;
      else
    	{
    	  Document doc;
    	  doc.Parse(buff.c_str());

    	  if(doc.HasMember("result"))
	    telegramID = doc["result"]["message_id"].GetInt();
	  
	  /* always cleanup */
	  if(curl)
	    curl_easy_cleanup(curl);
#ifdef TELEGRAMPOLL_ENABLE
	  // INIT CURL
	  curl = curl_easy_init();
	  
	  // SEND TELEGRAM POLL
	  msg.clear();
	  buff.clear();
	  
	  Document pollDoc;
	  auto& alloc = pollDoc.GetAllocator();
	  Value options(kArrayType);
	  options.PushBack("REAL ✅", alloc);
	  options.PushBack("FAKE ❌", alloc);
	  
	  pollDoc.SetObject();	  	  
	  pollDoc.AddMember("chat_id", TELEGRAMCHAT_ID, alloc);
	  pollDoc.AddMember("question", "Is this thread legit?", alloc);
	  pollDoc.AddMember("options", options, alloc);
	  pollDoc.AddMember("disable_notification", true, alloc);
	  pollDoc.AddMember("reply_to_message_id", telegramID, alloc);
	  pollDoc.AddMember("open_period", 600, alloc);  // Max 600 seconds
	  
	  
	  StringBuffer buffer;
	  Writer<rapidjson::StringBuffer> writer(buffer);
	  pollDoc.Accept(writer);

	  struct curl_slist *headers = NULL;
	  headers = curl_slist_append(headers, "Content-Type: application/json");
	  
	  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);	  
	  curl_easy_setopt(curl, CURLOPT_URL, telegramPollURL.c_str());
	  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer.GetString());
	  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	  curl_easy_setopt(curl, CURLOPT_POST, 1L);
	  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
	  
	  /* Perform the request, res will get the return code */ 
	  res = curl_easy_perform(curl);
	  /* Check for errors */ 
	  if(res != CURLE_OK)
	    std::cout << curl_easy_strerror(res) << std::endl;

	  /* always cleanup */
	  if(curl)
	    {
	      curl_easy_cleanup(curl);
	      curl_slist_free_all(headers);
	    }
    	}

#endif // TELEGRAMPOLL_ENABLE
           
      // SEND TWITTER MSG
      tweet << metatxt << " LINK " << chan.str() << " ARCHIVE LINK " << archive.str();
      if(!subject.empty())
	tweet << " " << subject;
      tweet << " #inb4source";
      
      std::stringstream twurlcmd;
      twurlcmd << "/usr/local/bin/twurl -d possibly_sensitive=true -d status=\"" <<
	tweet.str() << "\" /1.1/statuses/update.json";
      
      system(twurlcmd.str().c_str());      
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
  else
    std::cout << ">>ALREADY NOTIFIED" << std::endl;
  
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

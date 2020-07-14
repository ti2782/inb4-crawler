#include "notify.h"

using namespace rapidjson;

Notify::Notify()
{
  // READ CONFIG FILE
  std::string configFileName(CONFIG_DIR);
  configFileName.append(CONFIG_FILE);
  
  // OPEN FILE
  FILE* configFile = fopen(configFileName.c_str(), "r");
  if(configFile)
    {
      char buff[2048];
      FileReadStream is(configFile, buff, sizeof(buff));

      // PARSE JSON DOCUMENT
      Document doc;
      doc.ParseStream(is);
      fclose(configFile);

      if(doc.IsObject())
	{
	  if(doc.HasMember("accounts"))
	    {
	      const Value& accountJson = doc["accounts"];
	      for( SizeType i = 0; i < accountJson.Size(); i++)    
		{
		  Account* account = new Account;
		  if(accountJson[i].HasMember("telegramChannel"))
		    account->telegramChannel = accountJson[i]["telegramChannel"].GetString();
		  if(accountJson[i].HasMember("telegramToken"))
		    account->telegramToken = accountJson[i]["telegramToken"].GetString();
		  if(accountJson[i].HasMember("telegramPoll"))
		    account->telegramPoll = accountJson[i]["telegramPoll"].GetString();
		  if(accountJson[i].HasMember("twitterAccount"))
		    account->twitter = accountJson[i]["twitterAccount"].GetString();
		  
		  if(!account->telegramChannel.empty() || !account->telegramToken.empty() || !account->twitter.empty())
		    accountVec.push_back(account);
		}
	    }
	}
    }
  else
    std::cout << ">>WARNING\nFailed to open config file!" << std::endl;
}

Notify::~Notify()
{
  // FREE ACCOUNTS
  for(int i = 0; i < accountVec.size(); i++)
    if(accountVec[i])
      delete accountVec[i];
}

void Notify::sendNotification(int threadnum, int postnum, std::string subject, std::string comment, std::string metatxt, std::string name, int account)
{
  std::string buff;

  // CHECK ACCOUNT
  if(accountVec.size() <= account)
    {
      std::cout << "WARNING\nInvalid Account <" << account << ">" << std::endl;
      return;
    }
    
  if(!subject.empty())
    convertASCII(subject);

  // INIT CURL
  curl = curl_easy_init();
  
  // CRAFT LINKS
  std::stringstream chan, archive, cmd, tweet;
  
  std::string telegramURL(TELEGRAM_URL);
  telegramURL.append(accountVec[account]->telegramToken);
  telegramURL.append("/sendMessage");
  std::string telegramPollURL(TELEGRAM_URL);
  telegramPollURL.append(accountVec[account]->telegramToken);
  telegramPollURL.append("/sendPoll");
  
  chan << LINKURL << threadnum << "#p" << postnum;  
  archive << ARCHIVEURL << threadnum << "/#" << postnum;

  // CRAFT TELEGRAM MSG
  std::string msg;
  msg.append("chat_id=");
  msg.append(accountVec[account]->telegramChannel);
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
 	  options.PushBack("Yes ✅", alloc);
 	  options.PushBack("NO ❌", alloc);
	  
 	  pollDoc.SetObject();
 	  pollDoc.AddMember("chat_id", StringRef(accountVec[account]->telegramChannel.c_str()), alloc);
 	  pollDoc.AddMember("question", StringRef(accountVec[account]->telegramPoll.c_str()), alloc);
 	  pollDoc.AddMember("options", options, alloc);
 	  pollDoc.AddMember("disable_notification", true, alloc);
 	  pollDoc.AddMember("reply_to_message_id", telegramID, alloc); 	  
	  
	  
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

      // SET TWURL ACCOUNT
      std::string accountSwitch = "/usr/local/bin/twurl set default ";
      accountSwitch.append(accountVec[account]->twitter);

      if(system(accountSwitch.c_str()) > 0)
	{
	  std::cout << "WARNING\nFailed to switch Twurl Account to " << accountVec[account]->twitter << std::endl;
	  return;
	}
      
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

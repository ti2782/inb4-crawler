#include "downloader.h"

using namespace rapidjson;
using namespace std::chrono_literals;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}


Downloader::Downloader()
{
  // POPULATE HASH MAP
  if(!metaHandler.populateMetaMap())
    std::cout << ">>ERROR\nFailed to Populate Meta HashMap" << std::endl;
}

Downloader::~Downloader()
{
  for(int i = 0; i < notVec.size(); i++)
    {
      if(notVec[i])
	delete notVec[i];
    }
}

std::vector<int> Downloader::downloadThreadList()
{
  // Clear Old Threadlist
  threadnums.clear();

  CURL* curl = curl_easy_init();
  if(!curl)
    return threadnums;

  
  std::string url(THREADLISTURL);
  std::string buff;
  CURLcode res;
  
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // URL
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);

  res = curl_easy_perform(curl);
  if(res != CURLE_OK)
    {
      std::cout << curl_easy_strerror(res) << std::endl;
      std::this_thread::sleep_for(1s);
      curl_easy_cleanup(curl);
      return threadnums;
    }
  else
    {
      Document doc;
      doc.Parse(buff.c_str());
      
      if(!doc.IsArray())
	{
	  curl_easy_cleanup(curl);
	  return threadnums;
	}
      
      // PAGES
      for (SizeType i = 0; i < doc.Size(); i++)
	{
	  // THREADS
	  const Value& threads = doc[i]["threads"];
	  for(SizeType k = 0; k < threads.Size(); k++)
	    {		 
	      threadnums.push_back(threads[k]["no"].GetInt());
	    }
	}
    }
  
  std::this_thread::sleep_for(1s);
  curl_easy_cleanup(curl);
  return threadnums;
}

void Downloader::downloadThread(int threadnum)  
{
  CURL* curl = curl_easy_init();
      
  if(!curl)
    return;
  
  std::string url(THREADURL);
  url.append(std::to_string(threadnum).append(".json"));
  std::string buff;
  CURLcode res;
  
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // URL
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);

  res = curl_easy_perform(curl);
  if(res != CURLE_OK)
    {
      std::cout << curl_easy_strerror(res) << std::endl;
      curl_easy_cleanup(curl);
      return;
    }
  else
    {
      Document doc;
      doc.Parse(buff.c_str());
      if(doc.IsObject())
	{
	  const Value& posts = doc["posts"];
	  std::string subject; // Keep OP subject for every post in the thread
	  
	  for(SizeType i = 0; i < posts.Size(); i++)
	    {
	      int postnum;
	      std::string comment, hash;
	      if(posts[i].HasMember("md5"))
		{		  
		  hash = posts[i]["md5"].GetString();
		  if(posts[i].HasMember("sub"))
		    subject = posts[i]["sub"].GetString(); // Keep SUBJECT line for notifications
		  
		  Meta* meta = metaHandler.findMeta(hash.c_str());
		  if(meta)
		    {		      
		      std::cout << ">>FOUND " << meta->name << std::endl;

		      if(posts[i].HasMember("no"))
			postnum = posts[i]["no"].GetInt();
		      if(posts[i].HasMember("com"))
			comment = posts[i]["com"].GetString();

		      std::lock_guard<std::mutex> lock(guard);
		      
		      // STORE SOCIAL MEDIA NOTIFICATION
		      Notification* notification = new Notification;
		      notification->threadnum = threadnum;
		      notification->postnum = postnum;
		      notification->account = meta->account;
		      notification->subject = subject.c_str();
		      notification->comment = comment.c_str();
		      notification->metatxt = meta->text.c_str();
		      notification->name = meta->name.c_str();
		      notification->hashtags = meta->hashtags.c_str();
		      notVec.push_back(notification);
		    }
		}	      
	    }      
	}
    }
      
  curl_easy_cleanup(curl);
  return;
}

std::vector<Notification*> Downloader::getNotifications()
{
  auto ret = notVec;
  notVec.clear();
  return ret;
}

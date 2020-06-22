#include <ctime>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <csignal>

#include <curl/curl.h>

#include "metahandler.h"
#include "notify.h"

using namespace std::chrono_literals;
using namespace rapidjson;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}
CURL *curl;
MetaHandler metaHandler;
Notify notify;
std::vector<int> threadnums;

void fetchThreads();
void downloadThread(int thread);
std::string getTimeStamp();
void terminate(int signal);

int main(int argc, char** argv)
{
  // Register Terminate Signal Handler
  signal(SIGINT, terminate);

  // POPULATE HASH MAP
  if(!metaHandler.populateMetaMap())
    {
      std::cout << ">>ERROR\nFailed to Populate Meta HashMap" << std::endl;
      return EXIT_FAILURE;
    }
  
  // INIT CURL
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();  

  while(true)
    {
      // CURL
      if(curl)
	{     
	  // DOWNLOAD THREADLIST      
	  fetchThreads();
	  std::this_thread::sleep_for(1s);
	  
	  std::cout << ">>PARSING\n" << getTimeStamp();
	  
	  // DOWNLOAD THREADS
	  for(int i = 0; i < threadnums.size(); i++)
	    {
	      downloadThread(threadnums[i]);
	      std::this_thread::sleep_for(1s);
	    }
	  
	  std::cout << ">>DONE PARSING\n" << getTimeStamp();
	  
	}
    }
  
  // Cleanup Curl
  curl_easy_cleanup(curl);
  curl_global_cleanup();  
  return EXIT_SUCCESS;
}

void fetchThreads()
{
  if(!curl)
    return;
  
  std::string url(THREADLISTURL);
  std::time_t timestamp = std::time(nullptr);
  std::string buff;
  CURLcode res;
  
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // URL
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);

  res = curl_easy_perform(curl);
  if(res != CURLE_OK)
    std::cout << curl_easy_strerror(res) << std::endl;
  else
    {
      Document doc;
      doc.Parse(buff.c_str());
      
      if(!doc.IsArray())
	return;
      
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
}

void downloadThread(int thread)
{
  if(!curl)
    return;

  std::string url(THREADURL);
  url.append(std::to_string(thread).append(".json"));
  std::time_t timestamp = std::time(nullptr);
  std::string buff;
  CURLcode res;
  
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // URL
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);

  res = curl_easy_perform(curl);
  if(res != CURLE_OK)
    std::cout << curl_easy_strerror(res) << std::endl;
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
		  std::string metatxt, name;
		  hash = posts[i]["md5"].GetString();
		  if(posts[i].HasMember("sub"))
		    subject = posts[i]["sub"].GetString(); // Keep SUBJECT line for notifications	  
		  if(metaHandler.findMeta(hash.c_str(), metatxt, name))
		    {
		      std::cout << ">>FOUND " << name << getTimeStamp() << std::endl;

		      if(posts[i].HasMember("no"))
			postnum = posts[i]["no"].GetInt();
		      if(posts[i].HasMember("com"))
			comment = posts[i]["com"].GetString();
		      
		      // SEND NOTIFICATION TO SOCIAL MEDIA
		      notify.sendNotification(thread, postnum, subject, comment, metatxt, name);
		      }
		}	      
	    }      
	}
    }
}

std::string getTimeStamp()
{
  std::time_t timestamp = time(0);
  std::string timestring(ctime(&timestamp));
  return timestring.c_str();
}

void terminate(int signal)
{
  std::cout << ">>TERMINATE\n" << "Interrupt signal " << signal << std::endl;
  // Cleanup Curl
  if(curl)
    curl_easy_cleanup(curl);
  curl_global_cleanup();
  
  exit(signal);
}

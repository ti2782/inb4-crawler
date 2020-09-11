#include <ctime>
#include <chrono>
#include <cstdlib>
#include <csignal>

#include "notify.h"
#include "downloader.h"

using namespace rapidjson;

Notify notify;
Downloader downloader;

void terminate(int signal);

int main(int argc, char** argv)
{ 
  // Register Terminate Signal Handler
  signal(SIGINT, terminate);  
  
  // INIT CURL
  curl_global_init(CURL_GLOBAL_ALL);  
  
  while(true)
    {      
      // DOWNLOAD THREADLIST      
      auto threads = downloader.downloadThreadList();    
      std::cout << ">>PARSING " << std::endl;
      for(int i = 0; i < threads.size(); i++)
	{	  
	  // DOWNLOAD THREAD
	  auto notVec = downloader.downloadThread(threads[i]);
	  
	  // SEND NOTIFICATINOS
	  for(int c = 0; c < notVec.size(); c++)
	    {
	      std::unique_ptr<Notification> notification(notVec[c]);
	      if(notification)
		notify.sendNotification(notification->threadnum, notification->postnum, notification->subject, notification->comment, notification->metatxt, notification->name, notification->hashtags, notification->account);
	    }
	}      
    }
  
  //Cleanup Curl
  curl_global_cleanup();  
  return EXIT_SUCCESS;
}

void terminate(int signal)
{
  std::cout << ">>TERMINATE\n" << "Interrupt signal " << signal << std::endl;
  
  // Cleanup Curl
  curl_global_cleanup();
  
  exit(signal);
}

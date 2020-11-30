#include <ctime>
#include <chrono>
#include <cstdlib>
#include <csignal>
#include <thread>

#include "notify.h"
#include "downloader.h"

using namespace rapidjson;
using namespace std::chrono_literals;

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
      std::cout << ">>INFO\nPARSING " << threads.size() << " threads " << std::endl;
      for(int i = 0; i < threads.size(); i+=2)
	{
	  std::thread t1, t2;
	  
	  // DOWNLOAD THREAD
	  t1 = std::thread(&Downloader::downloadThread, &downloader, threads[i]);	  
	  std::this_thread::sleep_for(1s);
	  
	  if(i+1 < threads.size())
	    {
	      t2 = std::thread(&Downloader::downloadThread, &downloader, threads[i+1]);
	      std::this_thread::sleep_for(1s);
	      t2.join();
	    }
	  
	  t1.join();

	  auto notVec = downloader.getNotifications();
	  
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

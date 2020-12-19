#include <ctime>
#include <chrono>
#include <cstdlib>
#include <csignal>
#include <thread>

#include "notify.h"
#include "downloader.h"
#include "db.h"

using namespace rapidjson;
using namespace std::chrono_literals;

Notify notify;
Downloader downloader;
Db db;

void terminate(int signal);

int main(int argc, char** argv)
{ 
  // Register Terminate Signal Handler
  signal(SIGINT, terminate);  
  
  // INIT CURL
  curl_global_init(CURL_GLOBAL_ALL);

  db.clearThreads();

  std::vector<int> threads;
  int tnum1 = 0;
  int tnum2 = 0;
  
  while(true)
    {      
      // DOWNLOAD THREADLIST
      if(tnum1 <= 0)
	{
	  threads = downloader.downloadThreadList();
	  
	  // Write to Download Queue
	  for(int i = 0; i < threads.size(); i++)
	    db.addThread(threads[i]);
	  
	  std::cout << ">>INFO\nADDING " << threads.size() << " to Download Queue " << std::endl;
	}      

      // Get Thread      
      tnum1 = db.getThread();
      db.removeThread(tnum1);
      tnum2 = db.getThread();
      db.removeThread(tnum2);
      
      std::thread t1, t2;
	  
      // DOWNLOAD THREAD
      if(tnum1 > 0)
	{
	  std::cout << "DOWNLOADING THREAD: " << tnum1 << std::endl;
	  t1 = std::thread(&Downloader::downloadThread, &downloader, tnum1);
	  std::this_thread::sleep_for(1s);
	}
      if(tnum2 > 0)
	{
	  std::cout << "DOWNLOADING THREAD: " << tnum2 << std::endl;
	  t2 = std::thread(&Downloader::downloadThread, &downloader, tnum2);
	  std::this_thread::sleep_for(1s);
	}

      if(t1.joinable())
	t1.join();
      if(t2.joinable())
	t2.join();

      auto notVec = downloader.getNotifications();
      
      // SEND NOTIFICATINOS
      for(int c = 0; c < notVec.size(); c++)
	{
	  std::unique_ptr<Notification> notification(notVec[c]);
	  if(notification)
	    notify.sendNotification(notification->threadnum, notification->postnum, notification->subject, notification->comment, notification->metatxt, notification->name, notification->hashtags, notification->account);
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

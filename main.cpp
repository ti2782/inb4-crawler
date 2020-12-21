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

bool bNode;
std::thread t1, t2;

void terminate(int signal);

int main(int argc, char** argv)
{
  // Register Terminate Signal Handler
  signal(SIGINT, terminate);  
  
  // Parse Arguments
  if(argc > 1)
    {
      std::string arg(argv[1]);
      if(argc < 3)
	{
	  std::cout << "ERROR\nMissing host argument.\ninb4-crawler --node localhost:27017" << std::endl;
	  return EXIT_FAILURE;
	}
      
      std::string host("mongodb://");
      host.append(argv[2]);
      if(arg.compare("--node") == 0)
	{
	  bNode = true;
	  std::cout << ">>INFO\nStarting Crawler Node attached to Host: " << host << std::endl;
	  db.switchDBHost(host.c_str());
	}
      else
	{
	  std::cout << "ERROR\nWrong arguments\ninb4-crawler or\ninb4-crawler --node localhost:27017" << std::endl;
	  return EXIT_FAILURE;
	}
    }
  
  // INIT CURL
  curl_global_init(CURL_GLOBAL_ALL);

  if(!bNode)
    db.clearThreads();

  std::vector<int> threads;
  int tnum1 = 0;
  int tnum2 = 0;
  
  while(true)
    {
      if(!bNode)
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
	}
      
      // Get Thread      
      tnum1 = db.getThread();
      tnum2 = db.getThread();     
	  
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

      // HANDLE NOTIFICATINOS
      for(int c = 0; c < notVec.size(); c++)
	{
	  std::unique_ptr<Notification> notification(notVec[c]);
	  if(notification)
	    {
	      if(bNode) // STORE NOTIFICATION
		db.addNotification(notification.get()); 
	      else // SEND NOTIFICATION
		notify.sendNotification(notification.get());
	    }
	}
      // GET STORED NOTIFICATIONS
      if(!bNode)
       	{
       	  notVec.clear();
       	  notVec = db.getNotifications();
       	  db.clearNotifications();
       	  for(int c = 0; c < notVec.size(); c++)
       	    {
       	      std::unique_ptr<Notification> notification(notVec[c]);
       	      if(notification)
       	       	notify.sendNotification(notification.get());
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

  if(t1.joinable())
    t1.join();
  if(t2.joinable())
    t2.join();
  
  // Cleanup Curl
  curl_global_cleanup();
  
  exit(signal);
}

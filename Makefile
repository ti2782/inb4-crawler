inb4-crawler: *.cpp *.h
	c++ -std=c++17 -o inb4-crawler main.cpp metahandler.cpp notify.cpp downloader.cpp -I/usr/local/include -lcurl -lpthread

install: inb4-crawler
	install inb4-crawler /bin/

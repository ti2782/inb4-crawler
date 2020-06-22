inb4-crawler: main.cpp metahandler.cpp notify.cpp
	c++ -g -std=c++17 -o inb4-crawler main.cpp metahandler.cpp notify.cpp -I/usr/local/include -lcurl

install: inb4-crawler
	install inb4-crawler /bin/

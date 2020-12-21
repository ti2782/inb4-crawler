DESTDIR=/bin/
inb4-crawler: *.cpp *.h
	c++ -std=c++17 -o inb4-crawler *.cpp -I/usr/local/include -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/bsoncxx/v_noabi -Wl,-rpath,/usr/local/lib -L/usr/local/lib -lcurl -lpthread -lmongocxx -lbsoncxx -lssl -lcrypto

install: inb4-crawler
	install inb4-crawler /usr/local/bin/ #$(DESTDIR)

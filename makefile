CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cpp  ./timer/lst_timer.cpp ./http/http_conn.cpp ./log/log.cpp ./CGImysql/sql_connection_pool.cpp  webserver.cpp  ./utils/c/parse_xml.c ./configure.cpp ./router/router.cpp devlop/UserMain.cpp
	$(CXX) -o server -g --debug $^ $(CXXFLAGS) -lpthread -lmysqlclient -lxml2 -I/usr/include/libxml2/ -L/usr/local/lib

clean:
	rm  -r server.o

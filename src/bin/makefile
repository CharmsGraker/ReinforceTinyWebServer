CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: ../main/main.cpp timer/*.cpp \
				http/*.cpp \
				log/log.cpp \
				CGImysql/sql_connection_pool.cpp \
				threadpool/threadpool.h  \
				webserver/webserver.cpp  \
				webserver/server_configuration_loader/configure.cpp \
				netroute/router.h \
				../main/devlop/UserMain.cpp		\
				../main/devlop/threadpool_default_scheduler.cpp \
				extensions/render/*.cpp \
				threadpool/task.h \
				config/runtime_config.cpp \
				socket/*.cpp \
				socket/ustdnet.h \
				concurrent/ThreadFunction.cpp \
				extensions/redis_utils/*.cpp \
				extensions/loadbalancer/*.cpp \
				webserver/signal_handler \
				extensions/heartbeats/*.cpp \
				extensions/heartbeats/*.h \
				container/url_storage.cpp \
				socket/getLocalIP.cpp \
				http/http_outstream/http_response_header.cpp

	$(CXX) -o server -std=c++14 -g --debug $^ $(CXXFLAGS) -lpthread -lmysqlclient -lhiredis -lxml2 -lctemplate -lboost_system -lboost_filesystem -I/usr/include/boost -I/usr/include/libxml2/ -L/usr/local/lib

clean:
	rm  -r server.o

CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cpp  bin/timer/lst_timer.cpp \
				bin/http/*.cpp bin/log/log.cpp \
				bin/CGImysql/sql_connection_pool.cpp \
				bin/threadpool/threadpool.h  \
				 webserver.cpp  \
				./utils/c/parse_xml.c ./configure.cpp \
				./bin/netroute/router.h \
				devlop/UserMain.cpp		\
				devlop/threadpool_default_scheduler.cpp \
				./utils/string_utils.cpp \
				bin/render/render_utils.cpp ./utils/helpfulUtils.cpp \
				bin/threadpool/task.h \
				runtime_config.cpp \
				bin/signal_handler/server_handler.h \
				bin/signal_handler/server_handler.cpp \
				bin/container/url_storage.cpp

	$(CXX) -o server -g --debug $^ $(CXXFLAGS) -lpthread -lmysqlclient -lxml2 -lctemplate -I/usr/include/libxml2/ -L/usr/local/lib

clean:
	rm  -r server.o

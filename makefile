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
				bin/threadpool/threadpool.h bin/threadpool/threadpool.cpp \
				 webserver.cpp  \
				./utils/c/parse_xml.c ./configure.cpp \
				./bin/netroute/router.h \
				devlop/UserMain.cpp		\
				devlop/threadpoolScheduler.cpp \
				./utils/string_utils.cpp config.h \
				bin/render/render_utils.cpp ./utils/helpfulUtils.cpp \
				bin/threadpool/task.h

	$(CXX) -o server -g --debug $^ $(CXXFLAGS) -lpthread -lmysqlclient -lxml2 -lctemplate -I/usr/include/libxml2/ -L/usr/local/lib

clean:
	rm  -r server.o

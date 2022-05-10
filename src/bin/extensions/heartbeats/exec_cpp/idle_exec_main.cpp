#include <iostream>
#include <fstream>
#include "../SocketHeartBeatThread.h"
#include "../../redis_utils/redis_utils.h"
#include "../../../timer/CTimer.h"
// must include after socketHeartThread
#include "../cluster/Cluster.h"
#include <cstring>

fstream error_file;
#define WRITE_ERROR(log) (error_file<<log);
#define GRAKER_ERROR_FILE "./idle-error"
char buf_[1024];

void write2fd(int fd, const char *format_, ...) {
    memset(buf_, 0, sizeof buf_);
    va_list valst;
    va_start(valst, format_);
    vsnprintf(buf_, 514, format_, valst);
    buf_[512] = '\n';
    buf_[513] = '\0';

    va_end(valst);
    write(fd, buf_, sizeof buf_);
}

int listen_fd = -1;

static InetAddress serverAddress;
static InetAddress redisAddress;
static std::string clkey;

void deal_arg(int argc, const char *argv[]) {
    printf("enter into deal_arg()\n");

    assert(argv);
    if (!error_file)
        std::abort();
    serverAddress = InetAddress::Deserialize(argv[1]);

    if (!argv[2]) {
        listen_fd = BuildSocket(9999);
    } else {
        listen_fd = *(int *) argv[2];
    }
    redisAddress = InetAddress::Deserialize(argv[3]);
    clkey = argv[4];

}

int std_out;
int BUF_SIZE = 4 * 1024;

void
deal_pipe(int pipeFd) {
    printf("enter into deal_pipe()\n");
    vector<std::string> vec;
    char buffer[BUF_SIZE];
    memset(buffer, '\0', sizeof buffer);
    int nRead = read(pipeFd, buffer, sizeof buffer);
    if (nRead < 0) {
        char error_msg[128];
        sprintf(error_msg, "%s: at %s\n", strerror(errno), __FILE__);
        printf("%s", error_msg);
        WRITE_ERROR(error_msg);
        std::abort();
    }
    buffer[nRead] = '\0';
    auto p = buffer;
    for (int i = 0; i < nRead; ++i) {
        if (buffer[i] == '\0') {
            vec.push_back(p);
            write2fd(std_out, vec.back().c_str());
            p = buffer + i + 1;
        }
    }
    printf("hello main!\n");
    write2fd(std_out, "read %d nBytes\n", nRead);

    serverAddress = InetAddress::Deserialize(vec[0]);
    redisAddress = InetAddress::Deserialize(vec[1]);
    clkey = string(vec[2]);

}

int main(int argc, const char *argv[]) {
    error_file.open(GRAKER_ERROR_FILE, ios::out | ios::app);

    if (argc == 5) {
        deal_arg(argc, argv);
    } else {
        // read from pipe
        int pipe_fd;
        sscanf(argv[1], "%d,%d", &pipe_fd, &std_out);
        deal_pipe(pipe_fd);
    }
    printf("idleThread prepare....\n");

    Cluster<> cluster{redisAddress, clkey};

    SocketHeartBeatThread idleThread;
    idleThread.setTimeSpan(20);
    idleThread.setFdOnSlave(listen_fd);
    idleThread.setClusterCenter(cluster);
    yumira::heartbeat::addExcludeAddress(serverAddress);

    // start thread
    WRITE_ERROR("[Idle] Main Thread: " << std::this_thread::get_id() << std::endl);
    idleThread.start();

    // avoid Main Thread exit
//    WRITE_ERROR("[Idle] main thread wait for exit\n");
    printf("idleThread successfully run....\n");
    idleThread.join();
}
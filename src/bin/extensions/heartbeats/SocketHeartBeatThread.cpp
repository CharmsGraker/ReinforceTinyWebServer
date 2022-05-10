#include "SocketHeartBeatThread.h"
#include "GrakerHeartBeat.h"
#include <iostream>
#include <map>
#include "../../grakerThread/thread.h"
#include "cluster/Cluster.h"
#include <boost/filesystem.hpp>
#include <boost/system/system_error.hpp>
#include <sys/wait.h>

#ifndef GRAKER_SERVER
#define GRAKER_SERVER(x) x
#endif
#define READ_IDLE_STDOUT
namespace yumira {
    extern InetAddress
    GetServerInetAddress();
    namespace heartbeat {
        int out_pipe[2];
    }
};

void yumira::heartbeat::addExcludeAddress(const InetAddress &inetAddress) {
    yumira::heartbeat::exclude_address_.insert(inetAddress.toString());
}

int SocketHeartBeatThread::N_CONN_RETRY = 10;

int
Recv(int fd_, void *buf) {
    auto ret = recv(fd_, buf, sizeof buf, 0);
    return ret;
}

int
Listen2(int sock_fd, int queueLen = 10) {
    auto ret = listen(sock_fd, queueLen);
    return ret;
}

SocketHeartBeatThread::SocketHeartBeatThread() : worker_alive(false),
                                                 coordinator_(nullptr),
                                                 type(CANDIDATE),
                                                 cluster_(nullptr),
                                                 slave_callback(nullptr) {

}

int SocketHeartBeatThread::getSocketFd() const {
    assert(type == MASTER);
    return ((HBMaster *) (coordinator_))->getListenFd();
}

void
SocketHeartBeatThread::setSocketFd(int fd) {
    assert(type == SLAVE);
    ((HBSlave *) (coordinator_))->setListenFd(fd);
}

void SocketHeartBeatThread::run() {
    if (!worker_alive) {
        tryCandidate();
    }
    coordinator_->doWork();
}

void *
GRAKER_SERVER(createIdle)(int agrc_, char *args_[]) {
    /***/
    printf("start createIdle()\n");
    // this is in main process, so workspace at top level, we should use bin/extension/... to locate exec_file
    int pipe1[2];
    pipe(pipe1);
    pipe(yumira::heartbeat::out_pipe);
    char buffer[128];
    auto pid_ = fork();
    if (pid_ == 0) {
        // this is idle process, so workspace create in current file path
        // idle process
        auto current_file_abspath = string(__FILE__);
        auto idx = current_file_abspath.rfind('/');
        auto file_dir = current_file_abspath.substr(0, idx);
        auto f_idle_exec_path = file_dir + "/exec_file/idle_exec_main";
        boost::system::error_code error;

        cout << "pwd: " << f_idle_exec_path << std::endl;
        if (!boost::filesystem::is_regular_file(f_idle_exec_path, error)) {
            std::cerr << "boost error: " << error.message() << std::endl;
        }
        dup2(yumira::heartbeat::out_pipe[1], STDOUT_FILENO);
        // send pipe of read fd to child
        sprintf(buffer, "%d,%d", pipe1[0], yumira::heartbeat::out_pipe[1]);
        auto ret = execl(f_idle_exec_path.c_str(), f_idle_exec_path.c_str(), buffer, (char *) nullptr);
        if (ret == -1) {
            printf("child process error, %s\n", strerror(errno));
        }
        // should be unreachable code
        std::exit(-1);
    } else if (pid_ == -1) {
        printf("fork() error\n");
        return (void *) -1;
    } else {
        close(pipe1[0]);
        // fork idle thread for server process success
        int n_param = *(int *) args_[0];
        for (int i = 1; i <= n_param; ++i) {
            printf("parent write str=%s,n_byte=%ld to idle-pipe()\n", args_[i], strlen(args_[i]));
            // should include '\0'
            write(pipe1[1], args_[i], strlen(args_[i]) + 1);
        }
#ifdef READ_IDLE_STDOUT
        char buf[1024];
        memset(buf, 0, sizeof buf);
        auto nRead = read(yumira::heartbeat::out_pipe[0], buf, sizeof buf);
        if (nRead != -1)
            fprintf(stdout, "%s\n", buf);
#endif
        std::cerr << "create idle-thread in pid=" << pid_ << std::endl;
        return nullptr;
    }
}


bool SocketHeartBeatThread::reactFd(int fd) {
    if (Thread::getCurrentThread() == std::this_thread::get_id()) {
        return coordinator_->reactFd(fd);
    } else {
        // queue
//        coordinator_->addObserverFd(fd);
    }
    return false;
}

void
SocketHeartBeatThread::tryCandidate() {
    if (!cluster_) {
#ifdef WRITE_ERROR
        WRITE_ERROR("unalloc cluster! tryCandidate() early abort.\n");
#endif
        std::abort();
    }
    if (!cluster_->isAllocKey()) {
        std::cerr << "[FATAL] missing clusterKey!\n";
        std::exit(1);
    } else if (worker_alive)
        return;

    RedisConnection *redis_conn = RedisConnection::Fetch();
    bool successConn = false;
    type = CANDIDATE;

    for (int n_fail = 0; !successConn && n_fail < 3; ++n_fail) {
        if (n_fail > N_CONN_RETRY)
            break;
        if (!redis_conn) {
            std::cerr << "alloc connect fail\n";
            continue;
        }
        successConn = redis_conn->Connect(cluster_->getCenter());
        std::cout << "Thread: " << std::this_thread::get_id() << " in tryCandidate()" << std::endl;

        if (successConn) {
            auto &&ret = redis_conn->Setnx(cluster_->getMasterKey(), "1");
            if (ret) {
                // define a heartbeat server, it will detect other node if they still alive
                auto &&master = new HBMaster({InetAddress::GetLocalHost(), HEARTBEAT_PORT});
                type = MASTER;
                master->setRedisConn(redis_conn);
                master->tryDiscoverCluster(cluster_->getClusterKey());
                coordinator_ = master;
//                addExcludeAddressCallback = std::bind(addExcludeAddress);

            } else {
                // become a slave, master is Alive
                coordinator_ = new HBSlave();
                type = SLAVE;
                if (slave_callback)
                    slave_callback();
                delete redis_conn;
                redis_conn = nullptr;
                break;
            }
        } else {
            sleep(1);
        }
    }
    if (!successConn) {
        std::cerr << "fail to Connect Redis at " << cluster_->getCenter().toString() << std::endl;
        std::cerr << "IdleThread abort.\n";
        std::exit(1);
    } else {
        std::cout << "[INFO] SocketHeartBeatThread thread::id: " << getSelfId() << " successfully Connect to Redis"
                  << std::endl;
        std::cout << "[INFO] SocketHeartBeatThread thread::id: " << getSelfId() << " become " << printState() << endl;
        worker_alive = true;
    }
}

SocketHeartBeatThread::~SocketHeartBeatThread() {
    std::cout << "destroy [SocketHeartBeatThread] at: " << this << endl;

    if (worker_alive)
        delete coordinator_;
}
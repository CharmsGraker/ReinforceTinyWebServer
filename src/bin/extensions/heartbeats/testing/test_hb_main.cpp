#include "../SocketHeartBeatThread.h"
#include <iostream>
int main() {
    SocketHeartBeatThread thread;
    SocketHeartBeatThread thread2;

    std::cout<<"Main Thread: " << std::this_thread::get_id() << std::endl;
    std::string key = "grakerWebServerLb";
    InetAddress redisAddress = {"localhost", 6379};
    thread.setClusterCenter({redisAddress, key});
    thread.setTimeSpan(100);
    thread.start();
    thread2.setClusterCenter({redisAddress, key});
    thread2.setTimeSpan(100);
    thread2.start();
    std::cout<<"Thread: "<<thread.getSelfId() << std::endl;
    std::cout<<"Thread: "<<thread2.getSelfId() << std::endl;

    getchar();
//    thread.join();
}
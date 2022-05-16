#include "ingress.h"
#include "args.h"
namespace yumira {
    struct Ingress::ingress_operation : public socket_op {
        unsigned int m_user_count;
        unsigned int threshold;
        InetAddress redisAddress;
        unique_ptr<LoadBalancer> loadBalancer;

    public:
        explicit ingress_operation(const InetAddress &r) : m_user_count(0),
                                                           threshold(1),
                                                           redisAddress(r),
                                                           loadBalancer(new RedisLoadBalancer(r,yumira::REDIS_CLUSTER_KEY)) {}

        void onAccept(AcceptArgs &args, AcceptReply &reply) override {
            reply.connFd = args.accept();
            std::cout << "new connection on fd:" << reply.connFd << endl;

            ++m_user_count;
        };

        void onRead(const SocketContext &ctx, void *args) override {
            printf("read\n");
            char buffer[2048];
            long nRead = ctx.socketPtr->read(buffer);

            HttpRequest httpRequest;
            httpRequest.tryDecode(buffer, nRead);

            auto m_method = httpRequest.getMethod();
//            for(auto&& h:httpRequest.getHeaders()) {
//                cout << h.first <<":" << h.second << endl;
//            }
            auto m_url = httpRequest.getUrl();
            if (loadBalancer.get() && m_user_count > threshold && loadBalancer->isAlive()) {
                printf("Invoke select\n");
                auto newServerInetAddressStr = loadBalancer->select(m_url);
                if(!newServerInetAddressStr.empty()) {
                    auto http_response = SocketContext::response_maker::makeResponse();
                    printf("newServerInetAddressStr=%s\n",newServerInetAddressStr.c_str());
                    http_response->addLocation("http://" + newServerInetAddressStr + httpRequest.getUrl());
                }
                Utils::modfd(ctx.epollFd, ctx.socketFd(), EPOLLOUT, 0);
            }
        };

        void onWrite(const yumira::SocketContext &ctx, void *s_) override {
            printf("dealWrite\n");
            auto http_response = ctx.response;
            auto &&buf = (*http_response).render().c_str();
            printf("response: %s", buf);
            auto nWrite = ctx.socketPtr->write(buf, sizeof buf);
        };

        void onClose(const SocketContext&,void*) {
            --m_user_count;
        }
    };

    Ingress::Ingress(const InetAddress &redisIpPort) : m_listenSocket(),
                                                       m_stop(false),
                                                       m_socket_op(new ingress_operation(redisIpPort)) {
    };

    Ingress::~Ingress() {
        delete m_socket_op;
        for (auto &&[fd, ctx]: cachedCtx) {
             cachedCtx.erase(fd);
            close(fd);
        }
    }
}
//
// Created by nekonoyume on 2022/5/15.
//

#ifndef TINYWEB_SOCKET_OP_H
#define TINYWEB_SOCKET_OP_H
namespace yumira {
    struct SocketContext;
    struct AcceptArgs;
    struct AcceptReply;
    struct socket_op {
    protected:
        virtual void onAccept(AcceptArgs &, AcceptReply &) = 0;
        virtual void onRead(const SocketContext &, void *) = 0;
        virtual void onWrite(const SocketContext &, void *) = 0;
        virtual void onClose(const SocketContext &,void *) = 0;
        virtual void initializer() {};
    public:
        void dealAccept(AcceptArgs &, AcceptReply &);
        void dealRead(const SocketContext & ctx, void * args);
        void dealWrite(const SocketContext & ctx, void * args);
        void dealClose(const SocketContext & ctx, void * args);
    };
}

#endif //TINYWEB_SOCKET_OP_H

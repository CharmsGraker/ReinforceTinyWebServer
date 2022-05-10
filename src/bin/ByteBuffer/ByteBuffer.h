//
// Created by nekonoyume on 2022/4/17.
//

#ifndef TINYWEB_BYTEBUFFER_H
#define TINYWEB_BYTEBUFFER_H

#include <arpa/inet.h>
#include <vector>

namespace yumira {
    namespace net_io {
        class ByteBuffer {
            size_t bufferSize_;
            uint32_t curIdx;
            uint32_t readIdx;
            uint32_t writeIdx;
            std::vector<char> data_;
        public:
            ByteBuffer();

            bool
            append(char *buffer, int len) {

            }

            void
            clear() {

            };

            char *begin() {
                return &data_.at(0);
            }

            uint32_t
            getOffset();

            uint32_t indexOfWrite() const {
                return writeIdx;
            }

            uint32_t indexOfRead() const {
                return readIdx;
            }

            void
            write(char x, int n) {
                if (n < 0)
                    return;
                auto&& cur = current();
                int n_write = n;
                while (n_write--) {
                    *cur++ = x;
                }
                readIdx += n;
            }

            char *bytes();

            char *
            current() {
                return &data_.at(0) + readIdx;
            }

            char *
            end() {
                return &data_.at(0) + writeIdx;
            }

            long
            readFromFd(int fd) {
                printf("readFromFd(), size()=%d, writeIdx=%d\n", size(), writeIdx);
//                auto curSize = size();
//                while (writeIdx >= curSize)
//                    curSize *= 2;
//                data_.resize(curSize);

                auto nRead = recv(fd, (void *) (bytes() + writeIdx), size() - writeIdx, 0);
                writeIdx += nRead;
                printf("recv n=%d from sock \n", nRead);
                return nRead;
            }

            uint64_t
            size();

            char *
            next() {
                if (readIdx < writeIdx)
                    return current() + 1;
                return nullptr;
            }

            bool skip(int offset_);

            bool
            read_n_bytes(char *out, int size_);

            char
            at(uint32_t idx) {
                return data_.at(idx);
            }

            bool
            eof();

            void
            erase(int size_);

            bool
            require(int extra);

            void
            append(const char *bytes_, int len);

            virtual
            ~ByteBuffer();
        };
    }
}

#endif //TINYWEB_BYTEBUFFER_H

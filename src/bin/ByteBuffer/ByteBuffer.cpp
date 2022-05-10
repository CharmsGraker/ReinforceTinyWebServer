//
// Created by nekonoyume on 2022/4/17.
//
#include <cstring>
#include <cstdint>
#include <cstdio>
#include "ByteBuffer.h"

namespace yumira {
    namespace net_io {
        ByteBuffer::ByteBuffer() : curIdx(0), readIdx(0), writeIdx(0),data_(2048,'\0') {


        }

        bool
        ByteBuffer::require(int extra) {
            if(extra < 0)
                return true;
            return extra <= size() - writeIdx;
        }

        bool ByteBuffer::skip(int offset_) {
            if (require(offset_)) {
                readIdx += offset_;
                return true;
            }
            return false;
        }

        bool
        ByteBuffer::read_n_bytes(char *out, int size_) {
            if (require(size_)) {
                memcpy(out, &data_.at(readIdx), size_);
                readIdx += size_;
                return true;
            }
            return false;
        };

        char *ByteBuffer::bytes() {
            return (size() == 0) ? nullptr : &data_.at(0);
        }

        uint32_t
        ByteBuffer::getOffset() {
            return readIdx;
        }

        uint64_t ByteBuffer::size() {
            return data_.size();
        }

        bool
        ByteBuffer::eof() {
            return readIdx == writeIdx;
        }

        void
        ByteBuffer::erase(int size_) {
            if (size_ <= 0) {
                return;
            }
            if (size_ >= size()) {
                data_.clear();
                readIdx = writeIdx = curIdx = 0;
                return;
            }
            data_.erase(data_.begin(), data_.begin() + size_);
            readIdx -= size_;
            writeIdx -= size_;
        }

        void
        ByteBuffer::append(const char *bytes_, int len) {
            data_.insert(data_.begin() + writeIdx, bytes_, bytes_ + len);
            writeIdx += len;
        }

        ByteBuffer::~ByteBuffer() {

        }
    }
}

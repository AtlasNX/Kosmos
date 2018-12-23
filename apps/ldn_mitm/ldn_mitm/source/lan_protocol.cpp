#include "lan_protocol.hpp"
#include "debug.hpp"
#include <poll.h>
#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

#define POLL_UNKNOWN (~(POLLIN | POLLPRI | POLLOUT))
// Only used when debuging
#define DISABLE_COMPRESS 0

int Pollable::Poll(Pollable *fds[], size_t nfds, int timeout) {
    struct pollfd pfds[nfds];
    for (size_t i = 0; i < nfds; i++) {
        pfds[i].fd = fds[i] ? fds[i]->getFd() : -1;
        pfds[i].events = POLLIN;
        pfds[i].revents = 0;
    }
    int rc = poll(pfds, nfds, timeout);
    if (rc < 0) {
        LogFormat("Pollable::Poll failed %d", rc);
        return -1;
    }
    if (rc == 0) {
        return 0;
    }
    for (size_t i = 0; i < nfds; i++) {
        const struct pollfd &pfd = pfds[i];

        if (pfd.revents != 0) {
            if (pfd.revents & POLL_UNKNOWN) {
                LogFormat("Poll: %zu(%d) revents=0x%08X", i, pfd.fd, pfd.revents);
            }
            if (pfd.revents & (POLLERR | POLLHUP)) {
                LogFormat("Poll: (POLLERR | POLLHUP) %zu(%d) revents=0x%x", i, pfd.fd, pfd.revents);
                fds[i]->onClose();
            } else if (pfd.revents & (POLLIN | POLLPRI)) {
                int rc = fds[i]->onRead();
                if (rc != 0) {
                    LogFormat("Pollable::Poll close %d", rc);
                    fds[i]->onClose();
                }
            }
        }
    }

    return 0;
}

LanSocket::~LanSocket() {
    this->close();
}

void LanSocket::close() {
    if (this->fd != -1) {
        ::close(this->fd);
        this->fd = -1;
    }
}

void LanSocket::resetRecvSize() {
    this->recvSize = 0;
}

int LanSocket::recvPartPacket(u8 *buffer, size_t bufLen, struct sockaddr_in *addr) {
    constexpr int HeaderSize = sizeof(LANPacketHeader);
    ssize_t len = this->recvfrom((void *)(this->buffer + this->recvSize), sizeof(this->buffer) - this->recvSize, addr);
    if (len <= 0) {
        return static_cast<int>(len);
    }
    this->recvSize += len;

    if (this->recvSize < HeaderSize) {
        LogFormat("recvPartPacket this->recvSize < HeaderSize");
        return 0;
    }

    LANPacketHeader *header = (decltype(header))this->buffer;
    if (header->magic != LANMagic) {
        LogFormat("recvPartPacket header->magic != LANMagic");
        this->resetRecvSize();
        return 0;
    }

    const auto total = HeaderSize + header->length;
    if (total > BufferSize) {
        LogFormat("recvPartPacket total > BufferSize");
        this->resetRecvSize();
        return 0;
    }
    if (this->recvSize < total) {
        LogFormat("recvPartPacket this->recvSize < total. len: %d total: %d", static_cast<int>(len), static_cast<int>(total));
        return 0;
    }

    std::memcpy(buffer, this->buffer, total);
    this->recvSize -= total;
    std::memmove(this->buffer, this->buffer + total, this->recvSize);

    return total;
}

int LanSocket::recvPacket(MessageCallback callback) {
    constexpr int HeaderSize = sizeof(LANPacketHeader);
    u8 buffer[BufferSize];
    u8 decompressBuffer[BufferSize];

    ssize_t len;
    struct sockaddr_in addr;

    len = this->recvPartPacket(buffer, sizeof(buffer), &addr);

    if (len <= 0) {
        return len;
    }

    LANPacketHeader *header = (decltype(header))buffer;

    auto body = buffer + HeaderSize;
    auto bodyLen = len - HeaderSize;
    if (header->compressed) {
        size_t outSize = sizeof(decompressBuffer);
        if (decompress(body, bodyLen, decompressBuffer, &outSize) != 0) {
            LogFormat("Decompress error");
            LogHex(header, HeaderSize);
            LogHex(body, bodyLen);
            return -1;
        }
        if (outSize != header->decompress_length) {
            LogFormat("Decompress error length not match");
            return -1;
        }
        body = decompressBuffer;
        bodyLen = outSize;
    }

    ReplyFunc reply = [&](LANPacketType type, const void *data, size_t size) {
        return this->sendPacket(type, data, size, &addr);
    };
    return callback(header->type, body, bodyLen, reply);
}
int LanSocket::sendPacket(LANPacketType type, const void *data, size_t size) {
    return this->sendPacket(type, data, size, nullptr);
}
int LanSocket::sendPacket(LANPacketType type, const void *data, size_t size, struct sockaddr_in *addr) {
    LANPacketHeader header;
    this->prepareHeader(header, type);
    if (data == NULL) {
        size = 0;
    }
    header.length = size;
    u8 buf[size + sizeof(header)];
    if (size > 0) {
        u8 compressed[size];
        size_t outSize = size;
        if (this->compress(data, size, compressed, &outSize) == 0) {
            std::memcpy(buf + sizeof(header), compressed, outSize);
            header.decompress_length = header.length;
            header.length = outSize;
            header.compressed = true;
        } else {
            std::memcpy(buf + sizeof(header), data, size);
        }
    }
    std::memcpy(buf, &header, sizeof(header));

    return this->sendto(buf, header.length + sizeof(header), addr);
}

void LanSocket::prepareHeader(LANPacketHeader &header, LANPacketType type) {
    header.magic = LANMagic;
    header.type = type;
    header.compressed = false;
    header.length = 0;
    header.decompress_length = 0;
    header._reserved[0] = 0;
    header._reserved[1] = 0;
}

ssize_t TcpLanSocketBase::recvfrom(void *buf, size_t len, struct sockaddr_in *addr) {
    auto rc = ::recvfrom(this->fd, buf, len, 0, nullptr, 0);
    if (rc == 0) {
        return -0xFD23;
    }
    return rc;
}
int TcpLanSocketBase::sendto(const void *buf, size_t len, struct sockaddr_in *addr) {
    return ::sendto(this->fd, buf, len, 0, nullptr, 0);
}

ssize_t UdpLanSocketBase::recvfrom(void *buf, size_t len, struct sockaddr_in *addr) {
    socklen_t addr_len = sizeof(*addr);
    return ::recvfrom(this->fd, buf, len, 0, (struct sockaddr *)addr, &addr_len);
}
int UdpLanSocketBase::sendto(const void *buf, size_t len, struct sockaddr_in *addr) {
    return ::sendto(this->fd, buf, len, 0, (struct sockaddr *)addr, sizeof(*addr));
}

int UdpLanSocketBase::sendBroadcast(LANPacketType type, const void *data, size_t size) {
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(this->getBroadcast());
    addr.sin_port = htons(this->listenPort);

    return this->sendPacket(type, data, size, &addr);
}
int UdpLanSocketBase::sendBroadcast(LANPacketType type) {
    return this->sendBroadcast(type, nullptr, 0);
}

int LanSocket::compress(const void *input, size_t input_size, uint8_t *output, size_t *output_size) {
#if DISABLE_COMPRESS
    return -1;
#endif
    const uint8_t *in = (decltype(in))input;
    const uint8_t *in_end = in + input_size;
    uint8_t *out = output;
    uint8_t *out_end = output + *output_size;

    while (out < out_end && in < in_end) {
        uint8_t c = *in++;
        uint8_t count = 0;

        if (c == 0) {
            while (*in == 0 && in < in_end && count < 0xFF) {
                count += 1;
                in++;
            }
        }

        if (c == 0x00) {
            *out++ = 0;

            if (out == out_end)
                return -1;
            *out++ = count;
        } else {
            *out++ = c;
        }
    }

    *output_size = out - output;

    return in == in_end ? 0 : -1;
}

int LanSocket::decompress(const void *input, size_t input_size, uint8_t *output, size_t *output_size) {
    const uint8_t *in = (decltype(in))input;
    const uint8_t *in_end = in + input_size;
    uint8_t *out = output;
    uint8_t *out_end = output + *output_size;

    while (in < in_end && out < out_end) {
        uint8_t c = *in++;

        *out++ = c;
        if (c == 0) {
            if (in == in_end) {
                return -1;
            }
            uint8_t count = *in++;
            for (int i = 0; i < count; i++) {
                if (out == out_end) {
                    break;
                }
                *out++ = c;
            }
        }
    }

    *output_size = out - output;

    return in == in_end ? 0 : -1;
}

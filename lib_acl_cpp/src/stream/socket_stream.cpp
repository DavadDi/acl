#include "acl_stdafx.hpp"
#ifndef ACL_PREPARE_COMPILE
#include "acl_cpp/stdlib/snprintf.hpp"
#include "acl_cpp/stdlib/log.hpp"
#include "acl_cpp/stream/socket_stream.hpp"
#include "acl_cpp/stream/socket_stream.hpp"
#endif

#if defined(__linux__) && defined(MSG_ZEROCOPY)
#include <linux/errqueue.h>  // 定义了 sock_extended_err 结构
#endif

namespace acl {

socket_stream::socket_stream()
{
}

socket_stream::~socket_stream()
{
	close();
}

bool socket_stream::open(ACL_SOCKET fd, bool udp_mode /* = false */)
{
	ACL_VSTREAM* conn = acl_vstream_fdopen(fd, O_RDWR,
		8192, -1, ACL_VSTREAM_TYPE_SOCK);
	acl_assert(conn);
	return open(conn, udp_mode);
}

bool socket_stream::open(const char* addr, int conn_timeout, int rw_timeout,
    time_unit_t unit /* time_unit_s */)
{
	ACL_VSTREAM* conn;
	if (unit == time_unit_ms) {
		conn = acl_vstream_timed_connect(addr, ACL_BLOCKING,
			conn_timeout, rw_timeout, 8192, NULL);
	} else {
		conn = acl_vstream_connect(addr, ACL_BLOCKING,
			conn_timeout, rw_timeout, 8192);
	}
	if (conn == NULL) {
		return false;
	}

	return open(conn);
}

bool socket_stream::open(ACL_VSTREAM* vstream, bool udp_mode /* = false */)
{
	// 先关闭旧的流对象
	if (stream_) {
		acl_vstream_close(stream_);
	}
	stream_ = vstream;
	eof_    = false;
	opened_ = true;
	//acl_tcp_set_nodelay(ACL_VSTREAM_SOCK(vstream));
	if (udp_mode) {
		acl_vstream_set_udp_io(stream_);
	}
	return true;
}

// 将 C++ API 的 flags 转变为 C API 的 flags.
static unsigned to_oflags(unsigned flags)
{
	unsigned oflags = 0;

	if (flags & OPEN_FLAG_NONBLOCK) {
		oflags |= ACL_NON_BLOCKING;
	} else {
		oflags |= ACL_BLOCKING;
	}

	if (flags & OPEN_FLAG_REUSEPORT) {
		oflags |= ACL_INET_FLAG_REUSEPORT;
	}

	// It's only useful for TCP.
	if (flags & OPEN_FLAG_FASTOPEN) {
		oflags |= ACL_INET_FLAG_FASTOPEN;
	}

	if (flags & OPEN_FLAG_EXCLUSIVE) {
		oflags |= ACL_INET_FLAG_EXCLUSIVE;
	}

	if (flags & OPEN_FLAG_MULTICAST_LOOP) {
		oflags |= ACL_INET_FLAG_MULTILOOP_ON;
	}

	return oflags;
}

bool socket_stream::bind_udp(const char* addr, int rw_timeout /* = -1 */,
	unsigned flags /* = 0 */)
{
	if (stream_) {
		acl_vstream_close(stream_);
	}

	unsigned oflags = to_oflags(flags);
	stream_ = acl_vstream_bind(addr, rw_timeout, oflags);
	if (stream_ == NULL) {
		return false;
	}
	eof_    = false;
	opened_ = true;
	return true;
}

bool socket_stream::bind_multicast(const char *addr, const char *iface,
	int port, int rw_timeout, unsigned int flags)
{
	if (stream_) {
		acl_vstream_close(stream_);
	}

	unsigned oflags = to_oflags(flags);
	stream_ = acl_vstream_bind_multicast(addr, iface, port,
			rw_timeout, oflags);
	if (stream_ == NULL) {
		return false;
	}
	eof_    = false;
	opened_ = true;
	return true;
}

bool socket_stream::multicast_set_ttl(int ttl)
{
	if (eof_ || stream_ == NULL) {
		logger_error("Socket not opened yet");
		return false;
	}

	return acl_multicast_set_ttl(ACL_VSTREAM_SOCK(stream_), ttl) == 0;
}

bool socket_stream::multicast_set_if(const char *iface)
{
	if (eof_ || stream_ == NULL) {
		logger_error("Socket not opened yet");
		return false;
	}

	return acl_multicast_set_if(ACL_VSTREAM_SOCK(stream_), iface) == 0;
}

bool socket_stream::multicast_drop(const char *addr, const char *iface)
{
	if (eof_ || stream_ == NULL) {
		logger_error("Socket not opened yet");
		return false;
	}

	return acl_multicast_drop(ACL_VSTREAM_SOCK(stream_), addr, iface) == 0;
}

bool socket_stream::shutdown_read()
{
	if (stream_ == NULL) {
		logger_error("stream_ null");
		return false;
	}
	return acl_socket_shutdown(ACL_VSTREAM_SOCK(stream_), SHUT_RD) == 0;
}

bool socket_stream::shutdown_write()
{
	if (stream_ == NULL) {
		logger_error("stream_ null");
		return false;
	}
	return acl_socket_shutdown(ACL_VSTREAM_SOCK(stream_), SHUT_WR) == 0;
}

bool socket_stream::shutdown_readwrite()
{
	if (stream_ == NULL) {
		logger_error("stream_ null");
		return false;
	}
	return acl_socket_shutdown(ACL_VSTREAM_SOCK(stream_), SHUT_RDWR) == 0;
}

ACL_SOCKET socket_stream::sock_handle() const
{
	if (stream_ == NULL) {
		return ACL_SOCKET_INVALID;
	}
	return ACL_VSTREAM_SOCK(stream_);
}

ACL_SOCKET socket_stream::unbind_sock()
{
	if (stream_ == NULL) {
		return ACL_SOCKET_INVALID;
	}
	ACL_SOCKET sock  = ACL_VSTREAM_SOCK(stream_);
	stream_->fd.sock = ACL_SOCKET_INVALID;
	eof_    = true;
	opened_ = false;
	return sock;
}

int socket_stream::sock_type() const
{
	if (stream_ == NULL) {
		return -1;
	}

	if ((stream_->type & ACL_VSTREAM_TYPE_INET4)) {
		return AF_INET;
#ifdef AF_INET6
	} else if ((stream_->type & ACL_VSTREAM_TYPE_INET6)) {
		return AF_INET6;
#endif
#ifdef AF_UNIX
	} else if ((stream_->type & ACL_VSTREAM_TYPE_UNIX)) {
		return AF_UNIX;
#endif
	} else {
		return -1;
	}
}

const char* socket_stream::get_peer(bool full /* = false */) const
{
	if (stream_ == NULL) {
		return "";
	}

	// xxx: acl_vstream 中没有对此地址赋值
	const char* ptr = ACL_VSTREAM_PEER(stream_);
	if (ptr == NULL || *ptr == 0) {
		char  buf[256];
		if (acl_getpeername(ACL_VSTREAM_SOCK(stream_),
			buf, sizeof(buf)) == -1) {

			return "";
		}
		acl_vstream_set_peer(stream_, buf);
	}

	if (full) {
		return ACL_VSTREAM_PEER(stream_);
	} else {
		return get_peer_ip();
	}
}

const char* socket_stream::get_peer_ip() const
{
	if (stream_ == NULL) {
		return "";
	}

	const char* ptr = ACL_VSTREAM_PEER(stream_);
	if (ptr == NULL || *ptr == 0) {
		char buf[256];
		if (acl_getpeername(ACL_VSTREAM_SOCK(stream_),
			buf, sizeof(buf)) == -1) {

			return "";
		}
		acl_vstream_set_peer(stream_, buf);
	}

	return const_cast<socket_stream*>
		(this)->get_ip(ACL_VSTREAM_PEER(stream_),
			const_cast<socket_stream*> (this)->ipbuf_);
}

bool socket_stream::set_peer(const char* addr)
{
	if (stream_ == NULL) {
		logger_error("stream not opened yet!");
		return false;
	}

	acl_vstream_set_peer(stream_, addr);
	return true;
}

const char* socket_stream::get_local(bool full /* = false */) const
{
	if (stream_ == NULL) {
		return "";
	}

	// xxx: acl_vstream 中没有对此地址赋值
	const char* ptr = ACL_VSTREAM_LOCAL(stream_);
	if (ptr == NULL || *ptr == 0) {
		char buf[256];
		if (acl_getsockname(ACL_VSTREAM_SOCK(stream_),
			buf, sizeof(buf)) == -1) {

			return "";
		}
		acl_vstream_set_local(stream_, buf);
	}

	if (full) {
		return ACL_VSTREAM_LOCAL(stream_);
	} else {
		return get_local_ip();
	}
}

const char* socket_stream::get_local_ip() const
{
	if (stream_ == NULL) {
		return "";
	}

	const char* ptr = ACL_VSTREAM_LOCAL(stream_);
	if (ptr == NULL || *ptr == 0) {
		char buf[256];
		if (acl_getsockname(ACL_VSTREAM_SOCK(stream_),
			buf, sizeof(buf)) == -1) {

			return "";
		}
		acl_vstream_set_local(stream_, buf);
	}

	return const_cast<socket_stream*>
		(this)->get_ip(ACL_VSTREAM_LOCAL(stream_),
			const_cast<socket_stream*>(this)->ipbuf_);
}

bool socket_stream::set_local(const char* addr)
{
	if (stream_ == NULL) {
		logger_error("stream not opened yet!");
		return false;
	}

	acl_vstream_set_local(stream_, addr);
	return true;
}

const char* socket_stream::get_ip(const char* addr, std::string& out)
{
	char buf[256];
	safe_snprintf(buf, sizeof(buf), "%s", addr);
	char* ptr = strchr(buf, ':');
	if ((ptr = strrchr(buf, ACL_ADDR_SEP)) || (ptr = strrchr(buf, ':'))) {
		*ptr = 0;
	}
	if (buf[0] == 0) {
		return "";
	}
	out = buf;
	return out.c_str();
}

bool socket_stream::alive(double* tc1 /* NULL */, double* tc2 /* NULL */) const
{
	if (stream_ == NULL) {
		return false;
	}
#if 0
	if (acl_vstream_probe_status(stream_) == 0)
		return true;
	else
		return false;
#else
	if (tc1 || tc2) {
		return acl_socket_alive2(ACL_VSTREAM_SOCK(stream_), tc1, tc2) != 0;
	}
	return acl_socket_alive(ACL_VSTREAM_SOCK(stream_)) != 0;
#endif
}

socket_stream& socket_stream::set_tcp_nodelay(bool on)
{
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return *this;
	}
	acl_tcp_nodelay(sock, on ? 1 : 0);

	return *this;
}

socket_stream& socket_stream::set_tcp_solinger(bool on, int linger)
{
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return *this;
	}
	acl_tcp_so_linger(sock, on ? 1 : 0, linger);

	return *this;
}

socket_stream& socket_stream::set_tcp_sendbuf(int size)
{
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return *this;
	}
	acl_tcp_set_sndbuf(sock, size);

	return *this;
}

socket_stream& socket_stream::set_tcp_recvbuf(int size)
{
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return *this;
	}
	acl_tcp_set_rcvbuf(sock, size);

	return *this;
}

socket_stream& socket_stream::set_tcp_non_blocking(bool on)
{
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return *this;
	}
	(void) acl_non_blocking(sock, on ? ACL_NON_BLOCKING : ACL_BLOCKING);

	return *this;
}

bool socket_stream::get_tcp_nodelay() const
{
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return false;
	}

	return acl_get_tcp_nodelay(sock) != 0;
}

int socket_stream::get_tcp_solinger() const
{
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return -1;
	}

	return acl_get_tcp_solinger(sock);
}

int socket_stream::get_tcp_sendbuf() const
{
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return -1;
	}

	return acl_tcp_get_sndbuf(sock);
}

int socket_stream::get_tcp_recvbuf() const
{
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return -1;
	}

	return acl_tcp_get_rcvbuf(sock);
}

bool socket_stream::get_tcp_non_blocking() const
{
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return false;
	}

	return acl_is_blocking(sock) == 0;
}

bool socket_stream::set_zerocopy(bool yes)
{
#ifdef MSG_ZEROCOPY
	ACL_SOCKET sock = sock_handle();
	if (sock == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return false;
	}

	int on = yes ? 1 : 0;
	if (setsockopt(sock, SOL_SOCKET, SO_ZEROCOPY, &on, sizeof(on)) != 0) {
		logger_error("set SO_ZEROCOPY %s error %s",
			on ? "on" : "off", last_serror());
		return false;
	}
	return true;
#else
	(void) yes;
	return false;
#endif
}

bool socket_stream::wait_iocp(int ms) const
{
#ifdef MSG_ZEROCOPY
	ACL_SOCKET fd = sock_handle();
	if (fd == ACL_SOCKET_INVALID) {
		logger_error("invalid socket handle");
		return false;
	}

	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLERR | POLLIN;

	int ret = poll(&pfd, 1, ms);
	if (ret < 0) {
		return false;
	}
	if (ret == 0) {
		acl_set_error(ACL_ETIMEDOUT);
		return false;
	}

	char cmsgbuf[1024];
	char payload[1];
	struct msghdr msg;
	struct iovec iov;
	memset(&msg, 0, sizeof(msg));
	iov.iov_base = payload;
	iov.iov_len = sizeof(payload);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);

	ssize_t n = ::recvmsg(fd, &msg, MSG_ERRQUEUE);
	if (n < 0) {
		return false;
	}

	struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
	if (cmsg == NULL) {
		return false;
	}

	if (cmsg->cmsg_level != SOL_IP || cmsg->cmsg_type != IP_RECVERR) {
		return false;
	}

	struct sock_extended_err *serr = (struct sock_extended_err *)CMSG_DATA(cmsg);
	if (serr && serr->ee_origin == SO_EE_ORIGIN_ZEROCOPY) {
		return true;
	} else {
		return false;
	}
#else
	(void) ms;
	return false;
#endif
}

} // namespace acl

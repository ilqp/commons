#ifndef SOCKET_REQUEST_H
#define SOCKET_REQUEST_H

#include <string>
#include <sstream>

#ifdef __WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#endif

#include "libCom/Buffer.h"

#include <openssl/ssl.h>

// ensure windows compatibility
#ifndef SOCKET_ERROR
#define SOCKET int
#define SOCKET_ERROR (-1)
#endif

/**
 * Platform independent TCP-SSL client
 */
class Request {
protected:
	/**
	 * Hostname to connect to
	 */
	std::string host;

	/**
	 * Port to connect to
	 */
	u_short port;

	/**
	 * Use IPv6 (true) or IPv4 (false)
	 */
	bool ipv6;

	/**
	 * Initializes SSL for connection
	 * @param fd Socket descriptor
	 */
	int initSsl(int fd);

public:
	/**
	 * Creates a Request to host with port
	 * @param host Hostname to connect to
	 * @param port Port to connect to
	 * @param IPv6 Use IPv6 (true) or not (false; default)
	 */
	Request(std::string host, u_short port, bool IPv6=false);

	/**
	 * Free OpenSSL resources
	 */
	~Request();

	/**
	 * Connect to host
	 */
	int init();

	/**
	 * Read from remote into the buffer (greed - as max bytes as available)
	 * @param buffer Buffer receiving the read data
     * @param min Minimum size to read
	 * @return Success (true) or not (false)
	 */
	bool read(Buffer &buffer, const uint32_t min = 0);

    /**
	 * Read at most size bytes from remote into the buffer
	 * @param buffer Buffer receiving the read data
     * @param size Maximum bytes to read
	 * @return >0: Actual bytes read. 0: (clean) shutdown. <0: error occurred
	 */
	int32_t readMax(Buffer &buffer, const uint32_t size);

	/**
	 * Read exactly size bytes from remote into the buffer
	 * @param buffer Buffer receiving the read data
     * @param size Exact count of bytes to read
	 * @return True if exactly bytes have been red, false if not
	 */
	bool readExactly(Buffer &buffer, const uint32_t size);

	/**
	 * Write from buffer to remote
	 * @param buffer Buffer to send to remote
	 * @return Success (true) or not (false)
	 */
	bool write(const Buffer &buffer);

	/**
	 * Closes the underlying socket
	 */
	void close();

	/**
	 * Write a protocol generated class to the request stream
	 * @param pclass the class to write, needs to have T::serialize(const Buffer&)
	 *
	 * @return True on success
	 */
	template<typename T>
	bool writeProtoClass(const T &pclass) {
		Buffer outBuf;
		pclass.serialize(outBuf);
		return write(outBuf);
	}

	/**
	 * Reads a protocol generated class from request stream
	 * @param pclass the protocol generated class to be filled from buffer, needs to have T::deserialize(const Buffer&, uint32_t &missing)
	 *
	 * @return True on success
	 */
	template<typename T>
	bool readProtoClass(T &pclass) {
		Buffer inBuf;
		uint32_t missing = 0;
		// try to deserialize, read missing bytes
		while(!pclass.deserialize(inBuf, missing)) {
			if(missing == 0) // no bytes missing, but class cannot be deserialized => error
				return false;

			readExactly(inBuf, missing);
		}
		return true;
	}

private:
	SSL_CTX *ctx;
	SSL *ssl;
	bool initDone = false;
	SOCKET fd;
};


#endif //SOCKET_REQUEST_H

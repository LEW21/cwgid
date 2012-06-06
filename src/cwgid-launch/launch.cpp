#include "../cwgid/config.h"

#include <iostream>
#include <sys/socket.h>
#include <systemd/sd-daemon.h>
#include <unistd.h>
#include "itoa.h"

#ifdef CWGID_SOCKET_UNIX
#include <sys/un.h>
#endif

#ifdef CWGID_SOCKET_IP
#include <netinet/in.h>
#include "HostPort.h"
#endif

int newServer(const std::string& uri)
{
	int domain = 0;
	sockaddr* address = 0;
	socklen_t addressSize = 0;

	union
	{
#ifdef CWGID_SOCKET_UNIX
		::sockaddr_un sun;
#endif
#ifdef CWGID_SOCKET_IP
		::sockaddr_in6 sin6;
#endif
	};

	if (uri[0] == '/' || uri[0] == '.')
	{
#ifdef CWGID_SOCKET_UNIX
		::memset(&sun, 0, sizeof(::sockaddr_un));

		sun.sun_family = AF_UNIX;

		if (sizeof(sun.sun_path) < (uint) uri.size() + 1)
			throw std::runtime_error("Too long path.");

		::memcpy(sun.sun_path, uri.data(), uri.size());

		domain = PF_UNIX;
		address = (::sockaddr*) &sun;
		addressSize = sizeof(::sockaddr_un);
#else
		throw std::runtime_error("UNIX sockets were disabled during compilation.");
#endif
	}
	else
	{
#ifdef CWGID_SOCKET_IP
		::memset(&sin6, 0, sizeof(::sockaddr_in6));

		sin6.sin6_family = AF_INET6;

		HostPort hp(uri);

		::memcpy(&sin6.sin6_addr, hp.ip, 16);
		sin6.sin6_port = hp.port;

		domain = PF_INET6;
		address = (::sockaddr*) &sin6;
		addressSize = sizeof(::sockaddr_in6);
#else
		throw std::runtime_error("IP sockets were disabled during compilation.");
#endif
	}

	int serverSocket = ::socket(domain, SOCK_STREAM, 0);
	if (serverSocket == -1)
		throw std::runtime_error("Can't open socket.");

	int error = 0;

	error = ::bind(serverSocket, address, addressSize);
	if (error)
		throw std::runtime_error("Can't bind.");

	error = ::listen(serverSocket, 16);
	if (error)
		throw std::runtime_error("Can't listen.");

	return serverSocket;
}

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0] << " [application .so name] [protocol .so name] [local path or ip:port]" << std::endl;
		return 1;
	}

	std::string appName = argv[1];
	std::string protocolName = argv[2];
	std::string uri = argv[3];

	int server = newServer(uri);

	dup2(server, SD_LISTEN_FDS_START);
	setenv("LISTEN_PID", itoa((int) getpid()), 1);
	setenv("LISTEN_FDS", "1", 1);

	std::string self = argv[0];
	std::string search = "cwgid-launch";
	size_t pos = self.find(search);
	if (pos == std::string::npos)
	{
		std::cerr << "cwgid-launch has to be called as cwgid-launch." << std::endl;
		return 1;
	}

	self.replace(pos, search.length(), "cwgid");

	char** newargv = new char*[4];

	newargv[0] = strdup(self.c_str());
	newargv[1] = argv[1];
	newargv[2] = argv[2];
	newargv[3] = 0;

	execvp(newargv[0], newargv);

	std::cerr << "Launching failed." << std::endl;
	return 1;
}

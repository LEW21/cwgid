#include <stdexcept>
#include <thread>
#include <iostream>
#include <cstring>
#include <dlfcn.h>
#include <ext/stdio_filebuf.h>
#include <sys/socket.h>
#include <CWGI/cwgi.h>
#include "../../include/CWGI/cwgid.h"

#define CWGID_SOCKET_UNIX
#define CWGID_SOCKET_IP

#ifdef CWGID_SOCKET_UNIX
#include <sys/un.h>
#endif

#ifdef CWGID_SOCKET_IP
#include <netinet/in.h>
#include "HostPort.h"
#endif

struct IStreamWrapper
{
	std::istream stream;

	IStreamWrapper(int socket)
		: stream(new __gnu_cxx::stdio_filebuf<char>(fdopen(socket, "r"), std::ios_base::in, 1))
	{}

	~IStreamWrapper()
	{
//		streambuf* buf = stream->rdbuf(0);
		auto buf = static_cast<__gnu_cxx::stdio_filebuf<char>*>(stream.rdbuf(0));
		fclose(buf->file());
		delete buf;
	}
};

struct OStreamWrapper
{
	std::ostream stream;

	OStreamWrapper(int socket)
		: stream(new __gnu_cxx::stdio_filebuf<char>(fdopen(socket, "w"), std::ios_base::out, 1))
	{}

	~OStreamWrapper()
	{
//		streambuf* buf = stream->rdbuf(0);
		auto buf = static_cast<__gnu_cxx::stdio_filebuf<char>*>(stream.rdbuf(0));
		fclose(buf->file());
		delete buf;
	}
};

class Library
{
	void* p;

public:
	Library(const std::string& name)
	{
		p = dlopen(name.c_str(), RTLD_LAZY);
		if (!p)
			throw std::runtime_error(dlerror());
	}

	template <class T>
	T resolve(const char* symbol)
	{
		T sym = (T) dlsym(p, symbol);
		if (!sym)
			throw std::runtime_error(dlerror());
		return sym;
	}

	~Library()
	{
		dlclose(p);
	}
};

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

	Library appLib(appName);
	CWGI::ApplicationFactory newApp = appLib.resolve<CWGI::ApplicationFactory>("CWGI_newApplication");

	CWGI::Application* app = newApp();

	Library protocolLib(protocolName);
	CWGId::ProtocolFactory newProtocol = protocolLib.resolve<CWGId::ProtocolFactory>("CWGId_newProtocol");

	CWGId::Protocol* protocol = newProtocol();

	int server = newServer(uri);

	for (;;)
	{
		int connection = ::accept(server, 0, 0);

		if (connection < 0)
			throw std::runtime_error("Can't accept.");

		std::thread thread([=]()
		{
			try
			{
				IStreamWrapper iw(connection);
				OStreamWrapper ow(connection);

				std::auto_ptr<CWGI::Request> request(protocol->newRequest(iw.stream));
				std::auto_ptr<CWGI::Response> response(protocol->newResponse(ow.stream));

				(*app)(*request, *response);
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << std::endl;
			}

			::shutdown(connection, SHUT_RDWR);
 			::close(connection);
		});
		thread.detach();
	}
}

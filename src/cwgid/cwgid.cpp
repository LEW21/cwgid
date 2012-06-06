#include "config.h"

#include <stdexcept>
#include <thread>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <dlfcn.h>
#include <ext/stdio_filebuf.h>
#include <CWGI/cwgi.h>
#include "../../include/CWGI/cwgid.h"
#include <algorithm>
#include <systemd/sd-daemon.h>

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

int safeAtoi(const char* string)
{
	if (!string)
		return 0;

	return atoi(string);
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " [application .so name] [protocol .so name]" << std::endl;
		return 1;
	}

	std::string appName = argv[1];
	std::string protocolName = argv[2];

	Library appLib(appName);
	CWGI::ApplicationFactory newApp = appLib.resolve<CWGI::ApplicationFactory>("CWGI_newApplication");

	Library protocolLib(protocolName);
	CWGId::ProtocolFactory newProtocol = protocolLib.resolve<CWGId::ProtocolFactory>("CWGId_newProtocol");

	CWGId::Protocol* protocol = newProtocol();

	int n = sd_listen_fds(0);

	if (n > 1)
	{
		std::cerr << "Too many file descriptors received." << std::endl;
		return 1;
	}

	if (n < 1)
	{
		std::cerr << "No file descriptors received." << std::endl;
		return 1;
	}

	int server = SD_LISTEN_FDS_START + 0;

	CWGI::Application* app = newApp();

#ifndef CWGID_REF
	int threadsNum = safeAtoi(getenv("CWGID_THREADS"));

	if (!threadsNum)
		threadsNum = std::max(uint(4), std::thread::hardware_concurrency());

	std::thread* threads[threadsNum];

	for (int i = 0; i < threadsNum; ++i)
	{
		threads[i] = new std::thread([=]()
		{
#endif
			for (;;)
			{
				int connection = ::accept(server, 0, 0);

				if (connection < 0)
					throw std::runtime_error("Can't accept.");

				std::function<void()> handler = [=]()
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
				};

				#ifdef CWGID_REF
					std::thread(handler).detach();
				#else
					handler();
				#endif
			}
#ifndef CWGID_REF
		});
	}

	for (int i = 0; i < threadsNum; ++i)
		threads[i]->join();
#endif
}

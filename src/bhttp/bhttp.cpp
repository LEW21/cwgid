#include <string>
#include <stdexcept>
#include <cstring>

#define PROTO BHTTP
#define PROTO_CUSTOM_REQUEST int contentLength;
#include "../proto.h"

namespace BHTTP
{
	void nameToSPDY(std::string& name)
	{
		for (uint i = 0; i < name.size(); ++i)
		{
			if (name[i] == '_')
			{
				name[i] = '-';
			}
			else if (name[i] >= 'A' && name[i] <= 'Z')
			{
				name[i] += -'A' +'a';
			}
		}
	}

	void readline(std::istream& stream, char* buf, int max)
	{
		memset(buf, 0, 1024);
		for (int i = 0; i < max; ++i)
		{
			stream.read(buf+i, 1);
			if (buf[i] == '\r')
			{
				buf[i] = 0;
				--i;
			}

			if (buf[i] == '\n')
			{
				buf[i] = 0;
				--i;
				return;
			}
		}
	}

	CWGI::Request& Request::operator>>(CWGI::Headers& headers)
	{
		char buf[1024];
		readline(stream, buf, 1024);

		char method[1024], path[1024], http[1024];
		if (sscanf(buf, "%s %s %s", method, path, http) < 3)
			throw std::runtime_error("BHTTP: protocol error");

		if (strcmp(http, "HTTP/1.0") != 0 && strcmp(http, "HTTP/1.1") != 0)
			throw std::runtime_error("BHTTP: protocol error");

		headers["method"]  = method;
		headers["path"]    = path;
		headers["version"] = "1.0";
		headers["scheme"]  = "http";

		for (;;)
		{
			readline(stream, buf, 1024);
			std::string line = buf;

			std::cout << line << std::endl << std::endl;

			if (line.empty())
				break;

			int pos = line.find(':');
			std::string name  = line.substr(0, pos);
			std::string value = line.substr(pos+2);

			nameToSPDY(name);

			if (name == "content-length")
				contentLength = atoi(value.c_str());

			if (name != "connection" && name != "keep-alive" && name != "proxy-connection" && name != "transfer-encoding")
			{
				std::string header = name;
				nameToSPDY(header);

				std::string& oldValue = headers[header];
				if (oldValue.empty())
					oldValue = value;
			}
		}
/*
		headers["remote-addr"] = std::move(conn.remoteAddr);
		headers["remote-port"] = std::move(conn.remotePort);
		headers["server-addr"] = std::move(conn.serverAddr);
		headers["server-port"] = std::move(conn.serverPort);
*/
		return *this;
	}

	CWGI::Request& Request::operator>>(std::string& data)
	{
		if (contentLength)
		{
			char buff[contentLength];
			stream.read(buff, contentLength);
			contentLength = 0;

			data = std::string(buff, contentLength);
		}
		else
		{
			data = std::string();
		}

		return *this;
	}

	CWGI::Response& Response::operator<<(const CWGI::Headers& headers)
	{
		for (const std::pair<std::string, std::string>& header : headers)
			stream << header.first << ": " << header.second << "\r\n";

		stream << "\r\n";

		return *this;
	}

	CWGI::Response& Response::operator<<(const std::string& data)
	{
		if (!data.empty())
			stream << data;

		return *this;
	}
}

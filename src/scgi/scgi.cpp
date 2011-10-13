#include <string>
#include <stdexcept>
#include <cstring>

namespace SCGI
{
	struct LiteString
	{
		const char* data;
		uint size;

		LiteString(const char* d, uint s): data(d), size(s) {}

		bool operator==(const char* literal) {return memcmp(data, literal, size) == 0;}
		bool operator!=(const char* literal) {return !operator==(literal);}

		LiteString sub(int pos, int size) const {return LiteString(data + pos, size);}
		LiteString sub(int pos) const {return LiteString(data + pos, size - pos);}

		operator std::string() const {return std::string(data, size);}
	};

	struct HeadersHelper
	{
		int pos;
		int size;
		char* data;

		HeadersHelper(int s, char* d): pos(0), size(s), data(d) {}

		inline LiteString read();

		bool empty() const {return pos >= size;}
	};
}

#define PROTO SCGI
#define PROTO_CUSTOM_REQUEST int contentLength;
#include "../proto.h"

namespace SCGI
{
	LiteString HeadersHelper::read()
	{
		bool found = false;

		int oldPos = pos;

		for (; pos < size; ++pos)
		{
			if (data[pos] == '\0')
			{
				found = true;
				break;
			}
		}

		if (!found)
			throw std::runtime_error("SCGI: protocol error");

		return LiteString(&data[oldPos], pos++ - oldPos);
	}

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

	void nameToHTTP(std::string& name)
	{
		bool firstChar = true;

		for (uint i = 0; i < name.size(); ++i)
		{
			if (name[i] == '_')
			{
				firstChar = true;
				name[i] = '-';
			}
			else if (firstChar)
			{
				if (name[i] >= 'a' && name[i] <= 'z')
				{
					name[i] += +'A' -'a';
				}
				firstChar = false;
			}
			else if (name[i] >= 'A' && name[i] <= 'Z')
			{
				name[i] += -'A' +'a';
			}
		}
	}

	struct SPDYHeaders
	{
		std::string method;
		std::string path;
		std::string version;
		std::string host;
		std::string scheme;
	};

	struct ConnHeaders
	{
		std::string remoteAddr;
		std::string remotePort;
		std::string serverAddr;
		std::string serverPort;
	};

	CWGI::Request& Request::operator>>(CWGI::Headers& headers)
	{
		int size;
		stream >> size;

		char sep;
		stream >> sep;
		if (sep != ':')
			throw std::runtime_error("SCGI: protocol error");

		char buffer[size];
		stream.read(buffer, size);

		stream >> sep;
		if (sep != ',')
			throw std::runtime_error("SCGI: protocol error");

		HeadersHelper helper(size, buffer);

		SPDYHeaders spdy;
		ConnHeaders conn;

		spdy.scheme = "http";

		bool isSPDY = false;

		while (!helper.empty())
		{
			LiteString name = helper.read();
			LiteString value = helper.read();

			if (name == "SPDY")
				isSPDY = true;
			else if (!isSPDY)
			{
				if (name == "REQUEST_METHOD")
					spdy.method = value;
				else if (name == "REQUEST_URI")
					spdy.path = value;
				else if (name == "SERVER_PROTOCOL")
					spdy.version = value;
				else if (name == "HTTP_HOST")
					spdy.host = value;
				else if (name == "HTTPS")
					spdy.scheme = "https";
			}

			if (name == "REMOTE_ADDR")
				conn.remoteAddr = value;
			else if (name == "REMOTE_PORT")
				conn.remotePort = value;
			else if (name == "SERVER_ADDR")
				conn.serverAddr = value;
			else if (name == "SERVER_PORT")
				conn.serverPort = value;
			else if (name == "CONTENT_LENGTH")
			{
				std::string sValue = value;
				headers["content-length"] = sValue;
				contentLength = atoi(sValue.c_str());
			}
			else if (name.sub(0, 5) == "HTTP_")
			{
				name = name.sub(5);
				if (name != "CONNECTION" && name != "KEEP_ALIVE" && name != "PROXY_CONNECTION" && name != "TRANSFER_ENCODING")
				{
					std::string header = name;
					nameToSPDY(header);

					std::string& oldValue = headers[header];
					if (oldValue.empty())
						oldValue = value;
				}
			}
		}

		if (!isSPDY)
		{
			headers["method"]  = std::move(spdy.method);
			headers["path"]    = std::move(spdy.path);
			headers["version"] = std::move(spdy.version);
			headers["host"]    = std::move(spdy.host);
			headers["scheme"]  = std::move(spdy.scheme);
		}

		headers["remote-addr"] = std::move(conn.remoteAddr);
		headers["remote-port"] = std::move(conn.remotePort);
		headers["server-addr"] = std::move(conn.serverAddr);
		headers["server-port"] = std::move(conn.serverPort);

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

#if 0
		char buff[8192];
		int size = 0;

		size = stream.readsome(buff, 8092);
		if (!size)
		{
			stream.read(buff, 1024);
			size = stream.gcount();
			if (size == 1024)
				size += stream.readsome(&buff[1024], 8092-1024);
		}

		if (size)
			data = std::string(buff, size);
		else
			data = std::string();
#endif
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

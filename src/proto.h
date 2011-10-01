#ifndef cwgid_proto_h
#define cwgid_proto_h

#include <CWGI/cwgi.h>
#include "../include/CWGI/cwgid.h"

#ifdef PROTO

#ifndef PROTO_CUSTOM_REQUEST
#define PROTO_CUSTOM_REQUEST
#endif

#ifndef PROTO_CUSTOM_RESPONSE
#define PROTO_CUSTOM_RESPONSE
#endif

namespace PROTO
{
	struct Request: public CWGI::Request
	{
		std::istream& stream;
		PROTO_CUSTOM_REQUEST

		Request(std::istream& s): stream(s) {}

		CWGI::Request& operator>>(CWGI::Headers& headers);
		CWGI::Request& operator>>(std::string& data);
	};

	struct Response: public CWGI::Response
	{
		std::ostream& stream;
		PROTO_CUSTOM_RESPONSE

		Response(std::ostream& s): stream(s) {}

		CWGI::Response& operator<<(const CWGI::Headers& headers);
		CWGI::Response& operator<<(const std::string& data);
	};

	struct Protocol: public CWGId::Protocol
	{
		CWGI::Request*  newRequest(std::istream& stream)  {return new Request(stream);}
		CWGI::Response* newResponse(std::ostream& stream) {return new Response(stream);}
	};
	
	CWGId::Protocol* newProtocol()
	{
		return new PROTO::Protocol();
	}
};

CWGId_ProtocolFactory
{
	return PROTO::newProtocol();
}

#endif

#endif

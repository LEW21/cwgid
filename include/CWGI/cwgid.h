#include <CWGI/cwgi.h>
#include <iostream>

namespace CWGId
{
	struct Protocol
	{
		Protocol();
		virtual ~Protocol();

		virtual CWGI::Request* newRequest(std::istream&) = 0;
		virtual CWGI::Response* newResponse(std::ostream&) = 0;
	};

	typedef Protocol* (*ProtocolFactory)();
}

#define CWGId_ProtocolFactory extern "C" CWGId::Protocol* CWGId_newProtocol()

namespace CWGId
{
	Protocol::Protocol() {}
	Protocol::~Protocol() {}
}

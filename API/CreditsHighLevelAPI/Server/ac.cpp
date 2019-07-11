#include "ac.h"
#include <vector>
#include "common/base58.h"
#include "api/API.h"

general::Address ac::address(const char* sa)
{
	std::vector<unsigned char> evec;
	DecodeBase58(sa, evec);
	//std::string dst(evec.size(), 0);
	//memcpy((void*)dst.c_str(), &(evec[0]), evec.size());
	std::string dst(evec.begin(), evec.end());
	return dst;
}

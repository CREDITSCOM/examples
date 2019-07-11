namespace cpp hlapi.api
namespace java hlapi.api
namespace php hlapi.api
namespace netcore hlapi.api
namespace netcore hlapi.api
namespace js hlapi.api

struct ResultInfo
{
	1: i8 state,
	2: string Message
}

service Api
{
	string Hello()
	ResultInfo GetBalance(1: string publicKey)
}
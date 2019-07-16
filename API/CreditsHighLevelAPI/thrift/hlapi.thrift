namespace cpp hlapi.api
namespace java hlapi.api
namespace php hlapi.api
namespace netcore hlapi.api
namespace netcore hlapi.api
namespace js hlapi.api

struct Amount
{
	1: i32 integral,
	2: i64 fraction,
	3: string message
}

service Api
{
	string Hello()
	Amount GetBalance(1: string publicKey)
	string TransferCoins(1: string publicKey, 2: string privateKey, 3: string targetKey, 4: i32 integral, 5: i64 fraction, 6: double fee)
	string DeploySmartContract(1: string publicKey, 2: string privateKey, 3: string targetKey, 4: string code, 5: double fee)
}
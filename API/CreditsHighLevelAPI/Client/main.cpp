#include <iostream>
#include <memory>
#include <chrono>
#include <ctime>
#include <array>

#include <thrift/stdcxx.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "client.h"

using namespace std;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace hlapi::api;

void pr(const std::string& message) 
{
	std::cout << message << std::endl;
}

int main(int argc, char* argv[])
{
	pr("Example C++ client for Credits HLAPI");

	client cl{ "127.0.0.1", 9099 };
	
	keys ks{
		"35SR5JC6DA9pPd9xYQ7oRvvCgJKK7teLoMwDMBmEN45Q",
		"tvkphPU2Y6svC38FaWuer8zF2F7DsfNkqMsko76uL8Rdv5oztCqmcEwKWi5NbDgtsy7QDNe9vXhkHFQTATNFksc",
		"DXEiDtU7NHz8YdQEzugBXh3oqaVQ6nGHMkFPwLtnvZSG" };

	cl.set_keys(ks);

	pr("Wallet Balance Get");
	cl.wallet_balance();
	pr("");

	pr("Transfer Coins");
	cl.transfer_coins(1, 0, 0.9);
	pr("");

	pr("Deploy Smart Contract");
	cl.deploy_smart_contract("" /* here must be a smart contract code */, 0.9);
	pr("");

}


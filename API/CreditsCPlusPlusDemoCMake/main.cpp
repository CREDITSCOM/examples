#include <iostream>
#include <memory>
#include <chrono>
#include <ctime>
#include <array>

#include <sodium.h>

#include <thrift/stdcxx.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "api/API.h"
#include "keys.h"
#include "client.h"

using namespace std;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace api;
using namespace general;

int main(int argc, char* argv[])
{
	if (argc != 6)
	{
		cout << "Usage: main.exe NodeIpAddress NodePort YourPublicKey YourPrivateKey TargetPublicKey" << std::endl;
		return 1;
	}

	cout << "Credits API Demo C++" << endl;

	auto c = make_unique<client>(argv[1], atoi(argv[2]));
	c->set_keys(argv[3], argv[4], argv[5]);

	c->wallet_balance_get();
	c->transfer_coins(1, 0, 0.9);
}


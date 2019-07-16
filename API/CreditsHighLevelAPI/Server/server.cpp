#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>
#include <thrift/stdcxx.h>

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "hlapi/Api.h"
#include "client.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

class ApiHandler : public hlapi::api::ApiIf {
public:
	ApiHandler() {}

	void Hello(std::string& _return)
	{
		std::cout << "Hello()" << std::endl;
		_return = "Hello";
	}

	void GetBalance(hlapi::api::Amount& _return, const std::string& publicKey)
	{
		try
		{
			std::cout << "GetBalance()" << std::endl;
			
			client cl{ "127.0.0.1", 9090 };
			cl.set_keys(publicKey, "", "");
			auto wbg = cl.wallet_balance_get();

			_return.integral = wbg->balance.integral;
			_return.fraction = wbg->balance.fraction;

			std::cout << std::endl;
			std::cout << "GetBalance is OK" << std::endl;
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			_return.message = ex.what();
		}
	}

	void TransferCoins(std::string& _return, const std::string& publicKey, const std::string& privateKey, const std::string& targetKey, const int32_t integral, const int64_t fraction, const double fee)
	{
		try
		{
			std::cout << "TransferCoins()" << std::endl;

			client cl{ "127.0.0.1", 9090 };
			cl.set_keys(publicKey, privateKey, targetKey);
			cl.transfer_coins(integral, fraction, fee);

			std::cout << std::endl;
			std::cout << "TransferCoins is OK" << std::endl;
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			_return = ex.what();
		}
	}

	void DeploySmartContract(std::string& _return, const std::string& publicKey, const std::string& privateKey, const std::string& targetKey, const std::string& code, const double fee)
	{
		try
		{
			std::cout << "DeploySmartContract()" << std::endl;

			client cl{ "127.0.0.1", 9090 };
			cl.set_keys(publicKey, privateKey, targetKey);
			cl.deploy_smart(code, fee);

			std::cout << std::endl;
			std::cout << "DeploySmartContract is OK" << std::endl;
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			_return = ex.what();
		}
	}
};

class ApiCloneFactory : virtual public hlapi::api::ApiIfFactory {
public:
	virtual ~ApiCloneFactory() {}
	virtual hlapi::api::ApiIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo)
	{
		stdcxx::shared_ptr<TSocket> sock = stdcxx::dynamic_pointer_cast<TSocket>(connInfo.transport);
		cout << "Incoming connection\n";
		cout << "\tSocketInfo: " << sock->getSocketInfo() << "\n";
		cout << "\tPeerHost: " << sock->getPeerHost() << "\n";
		cout << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
		cout << "\tPeerPort: " << sock->getPeerPort() << "\n";
		return new ApiHandler;
	}
	virtual void releaseHandler(hlapi::api::ApiIf* handler) {
		delete handler;
	}
};

int main()
{
	//TThreadedServer server(
	//	stdcxx::make_shared<hlapi::api::ApiProcessorFactory>(stdcxx::make_shared<ApiCloneFactory>()),
	//	stdcxx::make_shared<TServerSocket>(9099), //port
	//	stdcxx::make_shared<TBufferedTransportFactory>(),
	//	stdcxx::make_shared<TBinaryProtocolFactory>());

	// if you don't need per-connection state, do the following instead
	//TThreadedServer server(
	//  stdcxx::make_shared<hlapi::api::ApiProcessor>(stdcxx::make_shared<ApiHandler>()),
	//  stdcxx::make_shared<TServerSocket>(9099), //port
	//  stdcxx::make_shared<TBufferedTransportFactory>(),
	//  stdcxx::make_shared<TBinaryProtocolFactory>());

	/**
	 * Here are some alternate server types...

	// This server only allows one connection at a time, but spawns no threads
	TSimpleServer server(
	  stdcxx::make_shared<ApiProcessor>(stdcxx::make_shared<ApiHandler>()),
	  stdcxx::make_shared<TServerSocket>(9099),
	  stdcxx::make_shared<TBufferedTransportFactory>(),
	  stdcxx::make_shared<TBinaryProtocolFactory>());

	*/

	const int workerCount = 4;

	stdcxx::shared_ptr<ThreadManager> threadManager =
	  ThreadManager::newSimpleThreadManager(workerCount);
	threadManager->threadFactory(
	  stdcxx::make_shared<PlatformThreadFactory>());
	threadManager->start();

	// This server allows "workerCount" connection at a time, and reuses threads
	TThreadPoolServer server(
	  stdcxx::make_shared<hlapi::api::ApiProcessorFactory>(stdcxx::make_shared<ApiCloneFactory>()),
	  stdcxx::make_shared<TServerSocket>(9099),
	  stdcxx::make_shared<TBufferedTransportFactory>(),
	  stdcxx::make_shared<TBinaryProtocolFactory>(),
	  threadManager);
	

	cout << "Starting the server..." << endl;
	server.serve();
	cout << "Done." << endl;
	return 0;
}
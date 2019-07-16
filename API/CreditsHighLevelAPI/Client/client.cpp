#include "client.h"

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

client::client(std::string ip, int port)
{
	m_socket = std::shared_ptr<TSocket>(new TSocket(ip, port));
	m_transport = std::shared_ptr<TTransport>(new TBufferedTransport(m_socket));
	m_protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));
	m_api = std::shared_ptr<hlapi::api::ApiClient>(new hlapi::api::ApiClient(m_protocol));
}

client::~client()
{
	disconnect();
}

void client::set_keys(const keys& ks)
{
	m_keys = ks;
}

void client::wallet_balance()
{
	try
	{
		connect();
		hlapi::api::Amount am;
		m_api->GetBalance(am, m_keys.public_key);
		am.printTo(std::cout);
		disconnect();
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

void client::transfer_coins(int32_t integral, int32_t fraction, double fee_value)
{
	try
	{
		connect();
		std::string message{};
		m_api->TransferCoins(message, m_keys.public_key, m_keys.private_key, m_keys.target_key, integral, fraction, fee_value);
		if (message.length() != 0) std::cout << message << std::endl;
		disconnect();
	}
	catch (const std::exception ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

void client::deploy_smart_contract(std::string code, double fee_value)
{
	try
	{
		connect();
		std::string message{};
		m_api->DeploySmartContract(message, m_keys.public_key, m_keys.private_key, m_keys.target_key, code, fee_value);
		if (message.length() != 0) std::cout << message << std::endl;
		disconnect();
	}
	catch (const std::exception ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

void client::connect()
{
	try
	{
		if (!m_transport->isOpen())
			m_transport->open();
	}
	catch (const std::exception ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

void client::disconnect()
{
	try
	{
		if(m_transport->isOpen())
			m_transport->close();
	}
	catch (const std::exception ex)
	{
		std::cout << ex.what() << std::endl;
	}
}


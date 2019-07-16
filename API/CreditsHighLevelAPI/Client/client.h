#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>

#include <thrift/stdcxx.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "hlapi/API.h"

struct keys
{
	std::string public_key;
	std::string private_key;
	std::string target_key;
};

class client
{
private:
	std::shared_ptr<apache::thrift::transport::TSocket> m_socket;
	std::shared_ptr<apache::thrift::transport::TTransport> m_transport;
	std::shared_ptr<apache::thrift::protocol::TProtocol> m_protocol;
	std::shared_ptr<hlapi::api::ApiClient> m_api;

	keys m_keys;

	void connect();
	void disconnect();

public:
	client(std::string ip, int port);
	~client();

	void set_keys(const keys& ks);

	void wallet_balance();
	void transfer_coins(int32_t integral, int32_t fraction, double fee_value);
	void deploy_smart_contract(std::string code, double fee_value);
};


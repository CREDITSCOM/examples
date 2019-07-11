#pragma once

#include <memory>
#include <sodium.h>
#include <iostream>
#include <string>
#include <algorithm>

#include <thrift/stdcxx.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "api/API.h"
#include "keys.h"

#define SIGNATURE_LEN 64

class client
{
private:
	std::shared_ptr<apache::thrift::transport::TSocket> m_socket;
	std::shared_ptr<apache::thrift::transport::TTransport> m_transport;
	std::shared_ptr<apache::thrift::protocol::TProtocol> m_protocol;
	std::shared_ptr<api::APIClient> m_api;

	std::unique_ptr<keys> m_keys;

	void connect();
	void disconnect();
	short fee(double value);
	std::unique_ptr<api::Transaction> make_transaction_with_smart_contract(std::string code, double fee_value);
	std::unique_ptr<api::Transaction> make_transaction(int32_t integral, int32_t fraction, double fee_value);
	template<class T>
	void cp(std::vector<byte>& arr, T& value, int16_t size, bool reverse);

public:
	client(std::string ip, int port);
	~client();

	void set_keys(const std::string& publicKey, const std::string& privateKey, const std::string& targetKey);

	void wallet_balance_get();
	void transfer_coins(int32_t integral, int32_t fraction, double fee_value);
	void deploy_smart(std::string code, double fee_value);
};


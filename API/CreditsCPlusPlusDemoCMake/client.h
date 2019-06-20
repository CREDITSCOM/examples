#pragma once

#include <memory>
#include <sodium.h>
#include <iostream>

#include <thrift/stdcxx.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "api/API.h"
#include "keys.h"

#define MESSAGE_LEN   86
#define SIGNATURE_LEN 64
#define PUB_KEY_LEN 32
#define PRV_KEY_LEN 64

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
	void cp(unsigned char* src, unsigned char* dst, int offset);
	std::unique_ptr<api::Transaction> make_transaction(int32_t integral, int32_t fraction, double fee_value);

public:
	client(std::string ip, int port);
	~client();

	void set_keys(const std::string& publicKey, const std::string& privateKey, const std::string& targetKey);

	void wallet_balance_get();
	void transfer_coins(int32_t integral, int32_t fraction, double fee_value);
};


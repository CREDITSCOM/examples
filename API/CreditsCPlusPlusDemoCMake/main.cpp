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

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace api;

#define MESSAGE_LEN   86
#define SIGNATURE_LEN 64
#define PUB_KEY_LEN 32
#define PRV_KEY_LEN 64

short Fee(double value)
{
	byte sign = (byte)(value < 0.0 ? 1 : 0); // sign
	int exp;   // exponent
	long frac; // mantissa
	value = abs(value);
	double expf = value == 0.0 ? 0.0 : log10(value);
	int expi = int(expf >= 0 ? expf + 0.5 : expf - 0.5);
	value /= pow(10, expi);
	if (value >= 1.0)
	{
		value *= 0.1;
		++expi;
	}
	exp = expi + 18;
	if (exp < 0 || exp > 28)
	{
		throw "exponent value exp out of range [0, 28]";
	}
	frac = (long)round(value * 1024);
	return (short)(sign * 32768 + exp * 1024 + frac);
}

void cp(unsigned char* src,  unsigned char* dst, int offset)
{
	for (int i = 0; i < 32; i++)
	{
		dst[i + offset] = src[i];
	}
}

int main(int argc, char* argv[])
{
	if (argc != 5)
	{
		std::cout << "Usage: main.exe NodeIpAddress YourPublicKey YourPrivateKey TargetPublicKey" << std::endl;
		return 1;
	}

	std::shared_ptr<TSocket> socket = std::shared_ptr<TSocket>(new TSocket(argv[1], 9090));
	std::shared_ptr<TTransport> transport = std::shared_ptr<TTransport>(new TBufferedTransport(socket));
	std::shared_ptr<TProtocol> protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
	std::shared_ptr<APIClient> api = std::shared_ptr<APIClient>(new APIClient(protocol));

	try
	{
		transport->open();
	}
	catch (...)
	{
		printf("thrift error: failed connect to node\n");
	}

	if (transport->isOpen())
	{
		std::cout << "Transport was opened" << std::endl;

		keys ks(argv[2],
				argv[3],
				argv[4]);

		try
		{
			WalletBalanceGetResult bg_res;
			TransactionFlowResult tr_res;

			api->WalletBalanceGet(bg_res, ks.PublicKeyAddress());
			bg_res.printTo(std::cout);

			WalletTransactionsCountGetResult wtcgr;
			auto& pka = ks.PublicKeyAddress();
			api->WalletTransactionsCountGet(wtcgr, pka);

			Transaction tr;
			tr.id = wtcgr.lastTransactionInnerId + 1;
			tr.source = std::string{ ks.PublicKeyAddress() };
			tr.target = std::string{ ks.TargetPublicKeyAddress() };
			tr.amount.integral = 1;
			tr.amount.fraction = 0;
			tr.fee.commission = Fee(0.9);
			tr.currency = 1;

			unsigned char message[MESSAGE_LEN];
			unsigned char signature[SIGNATURE_LEN];
			unsigned char src[PUB_KEY_LEN];
			unsigned char trg[PUB_KEY_LEN];
			unsigned char prv[PRV_KEY_LEN];

			memcpy(src, (unsigned char*)tr.source.c_str(), 32);
			memcpy(trg, (unsigned char*)tr.target.c_str(), 32);
			memcpy(prv, (unsigned char*)ks.PrivateKeyAddress().c_str(), 64);

			memcpy(message, &tr.id, 6);
			cp(src, message, 6);
			cp(trg, message, 38);
			memcpy(message + 70, &tr.amount.integral, 4);
			memcpy(message + 74, &tr.amount.fraction, 8);
			memcpy(message + 82, &tr.fee.commission, 2);
			message[84] = 1;
			message[85] = 0;

			unsigned long long signatureLen;
			crypto_sign_detached(signature, &signatureLen, message, MESSAGE_LEN, prv);

			if (crypto_sign_verify_detached(signature, message, MESSAGE_LEN, src) != 0) {
				std::cout << "Incorrect signature!" << std::endl;
				return 1;
			}

			tr.signature = std::string{ reinterpret_cast<char*>(signature), SIGNATURE_LEN };

			TransactionFlowResult tfr;
			api->TransactionFlow(tfr, tr);
			tfr.printTo(std::cout);
		}
		catch (...)
		{
			return 1;
		}

		std::cout << std::endl << std::endl;

		transport->close();
	}
}


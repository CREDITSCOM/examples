#include <iostream>
#include <memory>
#include <chrono>
#include <ctime>
#include <array>

#include "ed25519/src/ed25519.h"

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

int main()
{
	std::shared_ptr<TSocket> socket = std::shared_ptr<TSocket>(new TSocket("169.38.89.217", 9090));
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

		keys ks("9onQndywomSUr6iYKA2MS5pERcTJwEuUJys1iKNu13cH",
			   "4xVSMfNdGTdn32QHRoxx7GVUdRCUnqvpF5jDTxkvNzwpDNpiW27taPrQ9QjDacuH9GpU8SJA8XSR9Pb8o2H4GXp1",
			   "H5ptdUUfjJBGiK2X3gN2EzNYxituCUUnXv2tiMdQKP3b");

		try
		{
			WalletBalanceGetResult bg_res;
			TransactionFlowResult tr_res;

			api->WalletBalanceGet(bg_res, ks.PublicKeyBytes());
			bg_res.printTo(std::cout);

			WalletTransactionsCountGetResult wtcgr;
			auto& pka = ks.PublicKeyBytes();
			api->WalletTransactionsCountGet(wtcgr, pka);

			Transaction tr;
			tr.id = wtcgr.lastTransactionInnerId + 1;
			tr.source = ks.PublicKeyBytes();
			tr.target = ks.TargetPublicKeyBytes();
			tr.amount = Amount();
			tr.amount.integral = 1;
			tr.amount.fraction = 0;
			tr.fee = AmountCommission();
			tr.fee.commission = Fee(0.9);
			tr.currency = 1;

			unsigned char message[MESSAGE_LEN];
			unsigned char signature[SIGNATURE_LEN];

			//reinterpret_cast<unsigned char*>((char*)s1.c_str())

			unsigned char* source = reinterpret_cast<unsigned char*>((char*)tr.source.c_str());
			unsigned char* target = reinterpret_cast<unsigned char*>((char*)tr.target.c_str());
			unsigned char* pk = reinterpret_cast<unsigned char*>((char*)ks.PrivateKeyBytes().c_str());

			memcpy(message, &tr.id, 6);
			
			cp(source, message, 6);
			//memcpy(message + 6, &source, 32);
			//memcpy(message + 38, &target, 32);
			cp(target, message, 38);

			memcpy(message + 70, &tr.amount.integral, 4);
			memcpy(message + 74, &tr.amount.fraction, 8);
			memcpy(message + 82, &tr.fee.commission, 2);
			message[84] = 1;
			message[85] = 0;

			//ed25519_sign(signature, message, MESSAGE_LEN, (unsigned char*)ks.PublicKeyBytes().c_str(), (unsigned char*)ks.PrivateKeyBytes().c_str());
			ed25519_sign(signature, message, MESSAGE_LEN, source, pk);
			tr.signature = reinterpret_cast<char*>(signature);

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


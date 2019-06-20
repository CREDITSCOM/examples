#include "client.h"

using namespace std;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace api;
using namespace general;

client::client(string ip, int port)
{
	m_socket = shared_ptr<TSocket>(new TSocket(ip, port));
	m_transport = shared_ptr<TTransport>(new TBufferedTransport(m_socket));
	m_protocol = shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));
	m_api = shared_ptr<APIClient>(new APIClient(m_protocol));
	
	connect();
}

client::~client()
{
	disconnect();
}

void client::set_keys(const std::string& publicKey, const std::string& privateKey, const std::string& targetKey)
{
	m_keys = make_unique<keys>(publicKey.c_str(), privateKey.c_str(), targetKey.c_str());
}

void client::wallet_balance_get()
{
	try
	{
		WalletBalanceGetResult bg_res;
		m_api->WalletBalanceGet(bg_res, m_keys->PublicKeyAddress());
		bg_res.printTo(std::cout);
	}
	catch (const std::exception&)
	{
		throw exception("wallet_balance_get: calling error");
	}
}

void client::transfer_coins(int32_t integral, int32_t fraction, double fee_value)
{
	try
	{
		auto tr = make_transaction(integral, fraction, fee_value);
		TransactionFlowResult tfr;
		m_api->TransactionFlow(tfr, *tr);
		tfr.printTo(std::cout);
	}
	catch (const std::exception ex)
	{
		throw exception(ex.what());
	}
}

void client::connect()
{
	try
	{
		m_transport->open();
	}
	catch (...)
	{
		throw exception("thrift error: failed connect to node");
	}
}

void client::disconnect()
{
	try
	{
		m_transport->close();
	}
	catch (...)
	{
		throw exception("thrift error: failed disconnect from node");
	}
}

short client::fee(double value)
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

void client::cp(unsigned char* src,  unsigned char* dst, int offset)
{
	for (int i = 0; i < 32; i++)
	{
		dst[i + offset] = src[i];
	}
}

std::unique_ptr<api::Transaction> client::make_transaction(int32_t integral, int32_t fraction, double fee_value)
{
	auto tr = make_unique<api::Transaction>();

	WalletTransactionsCountGetResult wtcgr;
	auto& pka = m_keys->PublicKeyAddress();
	m_api->WalletTransactionsCountGet(wtcgr, pka);

	tr->id = wtcgr.lastTransactionInnerId + 1;
	tr->source = std::string{ m_keys->PublicKeyAddress() };
	tr->target = std::string{ m_keys->TargetPublicKeyAddress() };
	tr->amount.integral = integral;
	tr->amount.fraction = fraction;
	tr->fee.commission = fee(fee_value);
	tr->currency = 1;

	unsigned char message[MESSAGE_LEN];
	unsigned char signature[SIGNATURE_LEN];
	unsigned char src[PUB_KEY_LEN];
	unsigned char trg[PUB_KEY_LEN];
	unsigned char prv[PRV_KEY_LEN];

	memcpy(src, (unsigned char*)tr->source.c_str(), 32);
	memcpy(trg, (unsigned char*)tr->target.c_str(), 32);
	memcpy(prv, (unsigned char*)m_keys->PrivateKeyAddress().c_str(), 64);

	memcpy(message, &tr->id, 6);
	cp(src, message, 6);
	cp(trg, message, 38);
	memcpy(message + 70, &tr->amount.integral, 4);
	memcpy(message + 74, &tr->amount.fraction, 8);
	memcpy(message + 82, &tr->fee.commission, 2);
	message[84] = 1;
	message[85] = 0;

	unsigned long long signatureLen;
	crypto_sign_detached(signature, &signatureLen, message, MESSAGE_LEN, prv);

	if (crypto_sign_verify_detached(signature, message, MESSAGE_LEN, src) != 0) {
		throw exception("Incorrect signature!");
	}

	tr->signature = std::string{ reinterpret_cast<char*>(signature), SIGNATURE_LEN };

	return std::move(tr);
}

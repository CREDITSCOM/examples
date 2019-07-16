#include <blake2.h>
#include "client.h"

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace api;
using namespace general;

client::client(std::string ip, int port)
{
	m_socket = std::shared_ptr<TSocket>(new TSocket(ip, port));
	m_transport = std::shared_ptr<TTransport>(new TBufferedTransport(m_socket));
	m_protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));
	m_api = std::shared_ptr<APIClient>(new APIClient(m_protocol));
	
	connect();
}

client::~client()
{
	disconnect();
}

void client::set_keys(const std::string& publicKey, const std::string& privateKey, const std::string& targetKey)
{
	m_keys = std::make_unique<keys>(publicKey.c_str(), privateKey.c_str(), targetKey.c_str());
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
		throw;
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
		throw std::exception(ex.what());
	}
}

void client::deploy_smart(std::string code, double fee_value)
{
	try
	{
		auto tr = make_transaction_with_smart_contract(code, fee_value);
		TransactionFlowResult tfr;

		m_api->TransactionFlow(tfr, *tr);
		tfr.printTo(std::cout);
	}
	catch (const std::exception ex)
	{
		throw std::exception(ex.what());
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
		throw std::exception("thrift error: failed connect to node");
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
		throw std::exception("thrift error: failed disconnect from node");
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

template<class T>
void client::cp(std::vector<byte>& arr, T& value, int16_t size, bool reverse)
{
	auto temp = std::make_unique<byte[]>(size);
	memcpy(&temp[0], &value, size);
	if(reverse)
		std::reverse(&temp[0], &temp[size]);
	std::copy(&temp[0], &temp[size], back_inserter(arr));
}

std::unique_ptr<api::Transaction> client::make_transaction_with_smart_contract(std::string code, double fee_value)
{
	if (code.length() == 0)
		code = "import com.credits.scapi.annotations.*; import com.credits.scapi.v0.*; public class MySmartContract extends SmartContract { public MySmartContract() {} public String hello2(String say) { return \"Hello\" + say; } }";

	auto tr = std::make_unique<api::Transaction>();

	// set true for transfering optional parameters
	tr->__isset.smartContract = true;
	tr->smartContract.__isset.smartContractDeploy = true;

	WalletTransactionsCountGetResult wtcgr;
	auto& pka = m_keys->PublicKeyAddress();
	m_api->WalletTransactionsCountGet(wtcgr, pka);

	tr->id = wtcgr.lastTransactionInnerId + 1;
	tr->source = std::string{ m_keys->PublicKeyAddress() };
	tr->amount.integral = 0;
	tr->amount.fraction = 0;
	tr->fee.commission = fee(fee_value);
	tr->currency = 1;

	std::vector<byte> target;
	copy(&((unsigned char*)tr->source.c_str())[0],
		&((unsigned char*)tr->source.c_str())[tr->source.size()],
		back_inserter(target));
	cp(target, tr->id, 6, false);

	SmartContractCompileResult sccr;
	m_api->SmartContractCompile(sccr, code);
	if (sccr.status.code == 0)
	{
		for (int i = 0; i < sccr.byteCodeObjects.size(); i++)
		{
			copy(&((unsigned char*)sccr.byteCodeObjects[i].byteCode.c_str())[0],
				&((unsigned char*)sccr.byteCodeObjects[i].byteCode.c_str())[sccr.byteCodeObjects[i].byteCode.size()],
				back_inserter(target));
		}
	}
	else
	{
		throw std::exception(sccr.status.message.c_str());
	}

	tr->smartContract.smartContractDeploy.sourceCode = code;
	tr->smartContract.forgetNewState = false;

	byte hash[32];
	blake2s(hash, BLAKE2S_OUTBYTES, target.data(), target.size(), nullptr, 0);
	std::copy(&hash[0], &hash[32], back_inserter(tr->target));

	std::vector<byte> msg;
	cp(msg, tr->id, 6, false);

	copy(&((unsigned char*)tr->source.c_str())[0], &((unsigned char*)tr->source.c_str())[tr->source.size()], back_inserter(msg));
	copy(&((unsigned char*)tr->target.c_str())[0], &((unsigned char*)tr->target.c_str())[tr->target.size()], back_inserter(msg));
	cp(msg, tr->amount.integral, 4, false);
	cp(msg, tr->amount.fraction, 8, false);
	cp(msg, tr->fee.commission, 2, false);
	msg.push_back(1);
	msg.push_back(1);

	std::vector<byte> uf{ 11, 0, 1, 0, 0, 0, 0, 15, 0, 2, 12, 0, 0, 0, 0, 15, 0, 3, 11, 0, 0, 0, 0, 2, 0, 4, 0, 12, 0, 5, 11, 0, 1 };
	int32_t x = (int32_t)code.size();
	cp(uf, x, 4, true);

	copy(&((unsigned char*)code.c_str())[0], &((unsigned char*)code.c_str())[code.size()], back_inserter(uf));

	uf.insert(uf.end(), { 15, 0, 2, 12 });

	x = (int32_t)sccr.byteCodeObjects.size();
	cp(uf, x, 4, true);

	for (size_t i = 0; i < sccr.byteCodeObjects.size(); i++)
	{
		uf.insert(uf.end(), { 11, 0, 1 });

		x = (int32_t)sccr.byteCodeObjects[i].name.size();
		cp(uf, x, 4, true);
		copy(&((unsigned char*)sccr.byteCodeObjects[i].name.c_str())[0],
			&((unsigned char*)sccr.byteCodeObjects[i].name.c_str())[sccr.byteCodeObjects[i].name.size()],
			back_inserter(uf));

		uf.insert(uf.end(), { 11, 0, 2 });

		x = (int32_t)sccr.byteCodeObjects[i].byteCode.size();
		cp(uf, x, 4, true);

		copy(&sccr.byteCodeObjects[i].byteCode[0],
			&sccr.byteCodeObjects[i].byteCode[sccr.byteCodeObjects[i].byteCode.size()], back_inserter(uf));

		ByteCodeObject nbco;
		nbco.name = sccr.byteCodeObjects[i].name;
		nbco.byteCode = sccr.byteCodeObjects[i].byteCode;
		tr->smartContract.smartContractDeploy.byteCodeObjects.push_back(nbco);
		uf.push_back(0);
	}

	uf.insert(uf.end(), { 11, 0, 3, 0, 0, 0, 0, 8, 0, 4, 0, 0, 0, 0, 0 });
	uf.push_back(0);

	x = (int32_t)uf.size();
	cp(msg, x, 4, false);

	copy(uf.begin(), uf.end(), back_inserter(msg));

	unsigned char signature[SIGNATURE_LEN];
	unsigned long long signatureLen;
	crypto_sign_detached(signature, &signatureLen, msg.data(), msg.size(), (unsigned char*)m_keys->PrivateKeyAddress().c_str());

	if (crypto_sign_verify_detached(signature, msg.data(), msg.size(), (unsigned char*)tr->source.c_str()) != 0) {
		throw std::exception("Incorrect signature!");
	}

	tr->signature = std::string(reinterpret_cast<char*>(signature), SIGNATURE_LEN);

	return std::move(tr);
}

std::unique_ptr<api::Transaction> client::make_transaction(int32_t integral, int32_t fraction, double fee_value)
{
	auto tr = std::make_unique<api::Transaction>();

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

	std::vector<byte> msg;

	cp(msg, tr->id, 6, false);
	copy(&((unsigned char*)tr->source.c_str())[0], &((unsigned char*)tr->source.c_str())[tr->source.size()], back_inserter(msg));
	copy(&((unsigned char*)tr->target.c_str())[0], &((unsigned char*)tr->target.c_str())[tr->target.size()], back_inserter(msg));
	cp(msg, tr->amount.integral, 4, false);
	cp(msg, tr->amount.fraction, 8, false);
	cp(msg, tr->fee.commission, 2, false);
	msg.push_back(1);
	msg.push_back(0);

	unsigned char signature[SIGNATURE_LEN];
	unsigned long long signatureLen;
	crypto_sign_detached(signature, &signatureLen, msg.data(), msg.size(), (unsigned char*)m_keys->PrivateKeyAddress().c_str());

	if (crypto_sign_verify_detached(signature, msg.data(), msg.size(), (unsigned char*)tr->source.c_str()) != 0) {
		throw std::exception("Incorrect signature!");
	}

	tr->signature = std::string{ reinterpret_cast<char*>(signature), SIGNATURE_LEN };

	return std::move(tr);
}

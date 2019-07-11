#include "ac.h"
#include "keys.h"
#include "common/base58.h"

keys::keys(const char* publicKey, const char* privateKey, const char* targetPublicKey)
{
	m_publicKeyAddress = ac::address(publicKey);
	m_privateKeyAddress = ac::address(privateKey);
	m_targetPublicKeyAddress = ac::address(targetPublicKey);
}

const general::Address& keys::PublicKeyAddress()
{
	return m_publicKeyAddress;
}

const general::Address& keys::PrivateKeyAddress()
{
	return m_privateKeyAddress;
}

const general::Address& keys::TargetPublicKeyAddress()
{
	return m_targetPublicKeyAddress;
}

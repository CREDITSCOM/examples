#include "ac.h"
#include "keys.h"
#include "common/base58.h"

keys::keys(const char* publicKey, const char* privateKey, const char* targetPublicKey)
{
	m_publicKey = publicKey;
	m_privateKey = privateKey;
	m_targetPublicKey = targetPublicKey;

	m_publicKeyBytes = ac::address(m_publicKey);
	m_privateKeyBytes = ac::address(m_privateKey);
	m_targetPublicKeyBytes = ac::address(m_targetPublicKey);
}

const char* keys::PublicKey()
{
	return m_publicKey;
}

const char* keys::PrivateKey()
{
	return m_privateKey;
}

const char* keys::TargetPublicKey()
{
	return m_targetPublicKey;
}

const general::Address& keys::PublicKeyBytes()
{
	return m_publicKeyBytes;
}

const general::Address& keys::PrivateKeyBytes()
{
	return m_privateKeyBytes;
}

const general::Address& keys::TargetPublicKeyBytes()
{
	return m_targetPublicKeyBytes;
}

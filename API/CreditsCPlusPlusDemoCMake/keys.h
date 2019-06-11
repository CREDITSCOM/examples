#pragma once

#include "api/API.h"

class keys
{
	general::Address m_publicKeyAddress;
	general::Address m_privateKeyAddress;
	general::Address m_targetPublicKeyAddress;

public:
	keys(const char* publicKey, const char* privateKey, const char* targetPublicKey);

	const char* PublicKey();
	const char* PrivateKey();
	const char* TargetPublicKey();
	const general::Address& PublicKeyAddress();
	const general::Address& PrivateKeyAddress();
	const general::Address& TargetPublicKeyAddress();
};


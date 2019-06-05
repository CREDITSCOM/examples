#pragma once

#include "api/API.h"

class keys
{
	const char* m_publicKey;
	const char* m_privateKey;
	const char* m_targetPublicKey;
	general::Address m_publicKeyBytes;
	general::Address m_privateKeyBytes;
	general::Address m_targetPublicKeyBytes;

public:
	keys(const char* publicKey, const char* privateKey, const char* targetPublicKey);

	const char* PublicKey();
	const char* PrivateKey();
	const char* TargetPublicKey();
	const general::Address& PublicKeyBytes();
	const general::Address& PrivateKeyBytes();
	const general::Address& TargetPublicKeyBytes();
};


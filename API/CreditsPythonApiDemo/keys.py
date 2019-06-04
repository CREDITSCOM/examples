import base58


class Keys:
    def __init__(self, public_key, private_key, target_public_key):
        self.public_key = public_key
        self.private_key = private_key
        self.target_public_key = target_public_key
        self.public_key_bytes = base58.b58decode(public_key)
        self.private_key_bytes = base58.b58decode(private_key)
        self.target_public_key_bytes = base58.b58decode(target_public_key)


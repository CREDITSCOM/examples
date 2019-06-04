import sys
from keys import Keys
from clientex import ClientEx


def main():
    if len(sys.argv) != 5:
        print('Welcome to the Credits API Python Demo')
        print('Usage: app.py NodeIpAddress:Port YourPublicKey YourPrivateKey TargetPublicKey')
        print('')
        return

    keys = Keys(sys.argv[2], sys.argv[3], sys.argv[4])

    try:
        client = ClientEx(sys.argv[1].split(':'))
        print(client.wallet_balance_get(keys.public_key_bytes))
        client.execute_transaction(keys)
        client.close()

    except:
        print("Oops. Unexpected error.")


if __name__ == '__main__':
    main()

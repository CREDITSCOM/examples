# run bash
#sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=test_db,target=/credits/test_db --mount source=test_keys,target=/credits/test_keys credits_testnet:latest bash

# run node
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=test_db,target=/credits/test_db --mount source=test_keys,target=/credits/test_keys credits_testnet:latest

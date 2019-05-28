# run bash
#sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=main_db,target=/credits/main_db --mount source=main_keys,target=/credits/main_keys credits_mainnet:latest bash

# run node
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=main_db,target=/credits/main_db --mount source=main_keys,target=/credits/main_keys credits_mainnet:latest

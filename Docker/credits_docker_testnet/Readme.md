# credits_docker_mainnet
Using Credits (version 4_2_410) in Docker
https://developers.credits.com/en/Articles/a_Using_Credits_blockchain_software_in_Docker

## How to build
### You can run build.sh or run step by step commands below:
<p>wget https://credits.com/Content/file_users/Credits_Node_linux_x64_ver_4.2.412.3_test.tar.gz -P build</p> <p>Credits_Node_linux_x64_ver_4.2.412.3_test.tar.gz</p>
<p>tar -xvzf build/Credits_Node_linux_x64_ver_4.2.412.3_test.tar.gz --strip-components 1 -C build</p>
<p>rm build/Credits_Node_linux_x64_ver_4.2.412.3_test.tar.gz</p>

<p>g++ -pthread source/runner.cpp  -o build/runner</p>
<p>cp common/* build/</p>

<p>sudo docker build -t credits_testnet:4.2.412.3 .</p>
<p>sudo docker volume create test_db</p>
<p>sudo docker volume create test_keys</p>

### some helpers
sudo docker run -d -p 9000:9000 -v /var/run/docker.sock:/var/run/docker.sock -v portainer_data:/data portainer/portainer

### run bash
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=test_db,target=/credits/test_db --mount source=test_keys,target=/credits/test_keys credits_testnet:4.2.412.3 bash

### run local repo node
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=test_db,target=/credits/test_db --mount source=test_keys,target=/credits/test_keys credits_testnet:4.2.412.3

### run node docker repo
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=test_db,target=/credits/test_db --mount source=test_keys,target=/credits/test_keys pvl1175/credits_testnet:4.2.412.3

### additional example
./client --db-path test_db/ --public-key-file test_keys/public.txt --private-key-file test_keys/private.txt


##How to run
### You can run run.sh or run step by step commands below:
### run bash
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=test_db,target=/credits/test_db --mount source=test_keys,target=/credits/test_keys credits_testnet:latest bash

### run node
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=test_db,target=/credits/test_db --mount source=test_keys,target=/credits/test_keys credits_testnet:latest

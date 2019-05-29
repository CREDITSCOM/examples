# credits_docker_mainnet

Using Credits (version 4_2_410) in Docker
https://developers.credits.com/en/Articles/a_Using_Credits_blockchain_software_in_Docker

## How to build

#### You can run build.sh or run step by step commands below:

<p>wget https://credits.com/Content/file_users/Credits_Network_for_Linux_x64_4_2_410.tar.gz -P build Credits_Network_for_Linux_x64_4_2_410.tar.gz</p>
<p>tar -xvzf build/Credits_Network_for_Linux_x64_4_2_410.tar.gz --strip-components 1 -C build</p>
<p>rm build/Credits_Network_for_Linux_x64_4_2_410.tar.gz</p>

<p>g++ -pthread source/runner.cpp  -o build/runner</p>
<p>cp common/* build/</p>

<p>sudo docker build -t credits_mainnet:4.2.410 .</p>
<p>sudo docker volume create main_db</p>
<p>sudo docker volume create main_keys</p>

### some helpers
sudo docker run -d -p 9000:9000 -v /var/run/docker.sock:/var/run/docker.sock -v portainer_data:/data portainer/portainer

### run bash
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=main_db,target=/credits/main_db --mount source=main_keys,target=/credits/main_keys credits_mainnet:4.2.410 bash

### run local repo node
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=main_db,target=/credits/main_db --mount source=main_keys,target=/credits/main_keys credits_mainnet:4.2.410

### run node docker repo
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=main_db,target=/credits/main_db --mount source=main_keys,target=/credits/main_keys pvl1175/credits_mainnet:4.2.410

### additional example
./client --db-path main_db/ --public-key-file main_keys/public.txt --private-key-file main_keys/private.txt

## How to run
### You can run run.sh or run step by step commands below:

### run bash
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=main_db,target=/credits/main_db --mount source=main_keys,target=/credits/main_keys credits_mainnet:latest bash

### run node
sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=main_db,target=/credits/main_db --mount source=main_keys,target=/credits/main_keys credits_mainnet:latest


wget https://credits.com/Content/file_users/Credits_Node_linux_x64_ver_4.2.416_testnet.tar.gz -P build Credits_Node_linux_x64_ver_4.2.416_testnet.tar.gz
tar -xvzf Credits_Node_linux_x64_ver_4.2.416_testnet.tar.gz --strip-components 1 -C build
rm build/Credits_Node_linux_x64_ver_4.2.416_testnet.tar.gz

g++ -pthread source/runner.cpp  -o build/runner
cp common/* build/

sudo docker build -t credits_testnet:4.2.416 .
sudo docker volume create test_db
sudo docker volume create test_keys

# some helpers
#sudo docker run -d -p 9000:9000 -v /var/run/docker.sock:/var/run/docker.sock -v portainer_data:/data portainer/portainer

# run bash
#sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=test_db,target=/credits/test_db --mount source=test_keys,target=/credits/test_keys credits_testnet:4.2.416 bash

# run local repo node
#sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=test_db,target=/credits/test_db --mount source=test_keys,target=/credits/test_keys credits_testnet:4.2.416

# run node docker repo
#sudo docker run -it -p 6000:6000 -p 9090:9090 --mount source=test_db,target=/credits/test_db --mount source=test_keys,target=/credits/test_keys pvl1175/credits_testnet:4.2.416
# additional example
#./client --db-path test_db/ --public-key-file test_keys/public.txt --private-key-file test_keys/private.txt

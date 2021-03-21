#!/usr/bin/env bash

# Update the system
sudo apt-get update
sudo apt-get -y upgrade 
sudo apt-get -y dist-upgrade

# Remove Google Cloud SDK
sudo apt-get purge google-cloud-sdk -y

# Install Python3.8
cd
sudo apt-get install build-essential zlib1g-dev libncurses5-dev libgdbm-dev libnss3-dev libssl-dev libsqlite3-dev libreadline-dev libffi-dev curl libbz2-dev liblzma-dev
curl -O https://www.python.org/ftp/python/3.8.8/Python-3.8.8.tgz
tar xzvf Python-3.8.8.tgz
cd Python-3.8.8
./configure --enable-optimizations
make -j2
sudo make install
cd

# Install Docker
sudo apt-get install -y apt-transport-https ca-certificates curl gnupg lsb-release
curl -fsSL https://download.docker.com/linux/debian/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
echo "deb [arch=amd64 signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/debian $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update
sudo apt-get install -y docker-ce docker-ce-cli containerd.io
sudo usermod -aG docker $USER

# Download and install Fuzzbench environment
git clone https://github.com/google/fuzzbench
cd fuzzbench
git submodule update --init
sudo apt-get install -y rsync build-essential python3-dev python3-venv
make install-dependencies
source .venv/bin/activate
make presubmit
deactivate
cd

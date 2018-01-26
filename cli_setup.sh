#!/bin/sh
sudo apt-get update -y

sudo rm /boot/grub/menu.lst
sudo update-grub-legacy-ec2 -y
sudo apt-get dist-upgrade -qq -y
sudo apt-get upgrade -y

sudo apt-get install -y build-essential tcl g++ python-pip python3-pip redis-server unzip zip
pip install --upgrade pip
sudo pip3 install redis trueskill

mkdir bots
mkdir backup

sudo service redis stop

sudo sed -i 's/bind 127.0.0.1/bind 0.0.0.0/' /etc/redis/redis.conf

wget https://halite.io/assets/downloads/Halite2_Linux-x64.zip
unzip Halite2_Linux-x64.zip
rm Halite2_Linux-x64.zip

redis-server --daemonize yes

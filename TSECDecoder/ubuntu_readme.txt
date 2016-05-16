1. install libuv
  a. sudo apt-get install make automake libtool curl unzip
  b. wget https://github.com/libuv/libuv/archive/v1.x.zip
  c. sudo unzip v1.x.zip -d /usr/local/src
  d. cd /usr/local/src/libuv-1.x/
  e. sudo sh autogen.sh
  f. sudo ./configure
  g. sudo make
  h. sudo make install
  i. sudo rm -rf /usr/local/src/libuv-1.8.0 && cd ~/
  j. sudo ldconfig

2. install cmake 3.5
  a. wget https://cmake.org/files/v3.5/cmake-3.5.2-Linux-x86_64.sh
  b. sudo cmake-3.5.2-Linux-x86_64.sh --prefix=/opt/cmake
  c. sudo ln -s /opt/cmake/cmake-3.5.2-Linux-x86_64/bin/cmake /usr/local/bin/cmake

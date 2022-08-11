# change directory to the scripts parent directory
cd $( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/..


mkdir build && cd build # create build directory
cmake .. # create Makefile
cmake --build . -j4 # build project


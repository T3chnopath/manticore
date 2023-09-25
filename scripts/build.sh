rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TARGET=$1 -DSUBTREE=FALSE ..
make
old_dir=`pwd`

mkdir -p thirdparty/install
cd thirdparty

rm -fr dige
git clone https://github.com/matt-42/dige.git
cd dige && mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=../../install ..
make -j8 install

cd $old_dir

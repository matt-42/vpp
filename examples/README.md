
# Compilation

```
git clone https://github.com/matt-42/vpp.git
cd vpp
./install.sh ./vpp_install
cd examples
mkdir build
cd build
cmake .. -DIOD_INCLUDE_DIR=../../vpp_install
make -j4
```

Replace vpp_install with your prefered vpp install location.

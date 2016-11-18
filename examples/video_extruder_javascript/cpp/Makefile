all: main.js

main.js: main.cc
	emcc -O3  -std=c++1z -I ~/projects/vpp -I ~/projects/iod -I /usr/include/eigen3  main.cc -o main.js  -s TOTAL_MEMORY=33554432  -s EXPORTED_FUNCTIONS="['_update', '_get_display_buffer', '_set_threshold']"

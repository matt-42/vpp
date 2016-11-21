all: main.js

main.js: main.cc Makefile
	emcc -O3 -DNDEBUG -std=c++1z -I ~/projects/vpp -I ~/projects/iod -I /usr/include/eigen3  main.cc -o main.js   -s EXPORTED_FUNCTIONS="['_update', '_get_display_buffer', '_set_threshold']" -s TOTAL_MEMORY=163554432
# -s USE_PTHREADS=1 
# -s ALLOW_MEMORY_GROWTH=1

# -s ASSERTIONS=2  -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=1

clean:
	rm main.js main.js.mem

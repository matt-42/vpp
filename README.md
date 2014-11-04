Video++
=============

Video++ is a video and image processing library that takes advantage
of the C++14 standard to ease the writing of fast parallel real-time
video and image processing. The idea behind Video++ performance is to
generate via meta-programming the most static code possible such that the
compiler enable optimizations like loop vectorizing. Its main features are:

  - Generic N-dimentional image containers.
  - A growing set of image processing algorithms.
  - Tools to write image processing algorithms for multicore SIMD CPU processors.
  - An embeded language to evalute image expressions.

Tested compilers : **G++ 4.9.1, Clang++ 3.5.0**

**Since Video++ relies on C++14 features, only compilers fully supporting this standard are able to
compile the library.**


## Getting Started

Video++ is a header-only library, To start coding, include the vpp.hh header to a C++ source file:

```c++
#include <vpp/vpp.hh>

int main()
{
  using namespace vpp; // Optional
}
```

Then, tell the compiler where to find the folder vpp by setting the include search path :

```sh
gcc -I __path_to_vpp__ main.cc
```

If you use the parallel constructs of Video++ and need to target multiple cores, activate the OpenMP optimizations :
```sh
gcc -I __path_to_vpp__ main.cc -fopenmp -lgomp
```

## Overview of the Library

### Image Containers

```image2d<T>``` is a 2d image container with value of type
T. Assigning A to B shares the A's data with B. ```clone``` clones images.
An image buffer are automatically freed when no container references it anymore.

```c++
int main()
{
  using namespace vpp; // Skip the vpp:: prefix.

  // Allocate an 100 rows x 200 cols image of int.
  image2d<int> img(100, 200);

  // Access to the value of the first pixel.
  int pixel0 = img(0,0);

  // Assignment shares the data between two containers.
  image2d<int> img2 = img;

  // img2 and img now share the same buffer :
  img2(0,0) = 42;
  assert(img(0,0) == 42);

  // Clone a image to duplicate its data.
  image2d<int> img3 = clone(img);
  // img3 holds its own separate buffer.

  image2d<int> img4(100, 100, _Aligned = 32); // The begining of each row is aligned on 128 bytes.
  image2d<int> img5(100, 100, _Border = 10); // A border of 10 pixels surround the image pixels.

  // The allocated images are automatically freed at the end of the scope.
}
```

### Image Expressions

One of the most powerful feature of the library is the ability to
evaluate complex images expressions via the function ```eval```. The
generated code spans one thread per processor core. Depending on your
compiler, there is a good chance that the loops will be optimized with
SIMD vector instructions.

```c++

// evaluate A+B*2 pixel by pixels and store the result in a new image C.
// A and B must have the same domain.
auto C = eval(_V(A) + _V(B) * 2);

// Or assign an existing image.
eval(_V(C) = _V(A) * _V(B));

// Compute statistics such as the minimum of an expression over all the domain of some images.
auto min_value = eval(_Min(_V(A) + _V(B)));

// Use statistics to generate a new normalized image.
auto C_normalized = eval(_V(C) * 1.f / _Max(C));

// Find the position of the minimum value of an expression.
auto argmin = eval(_Argmin(_V(A) + _V(B)));
```

### Vector Types

Video++ vector types are aliases to eigen3 vectors, providing fast linear algebra. The syntax
is ```v{T}{N}``` such as

 - T is one the following : *char, short, int, float, double, uchar, ushort, uint*.
 - N is an integer in [0, 4].

For exemple, the type ```image2d<vuchar4>``` can handle an image of RGBA 8-bit.

### Sequential and parallel Kernels

The ```pixel_wise```, ```block_wise```, ```row_wise```, and ```col_wise``` constructs
allow to run kernel on pixel, rows, and columns. They take three kind of inputs:

```c++
pixel_wise(R1, ..., RN)(O1, ..., ON) | F
```
   - RX: The ranges
     images, sub images, set of coordinates or views on neighborhs.

   - OX: Dependencies and multithreading
     Their is 4 possible dependencies between computation of pixel
       - Row forward: the computation of the i th pixel of row X depend
         on the computation of the i-1 th pixel of the same row
       - Row backward: the computation of the i-1th pixel of row X depend
         on the computation of the i th pixel of the same row
       - Col forward: the computation of the i th pixel of col X depend
         on the computation of the i-1 th pixel of the same col
       - Col backward: the computation of the i-1th pixel of col X depend
         on the computation of the i th pixel of the same col

      To satisfy dependencies, video++ iterates on the appropriate direction, and
      disable multithreading if the combination of dependencies (for example row forward
      + col forward) does not allows it.

      You can also explicitely disable multithreading by adding _No_thread to the
      list.

      Note: the program must be compiled with ```-fopenmp -lgomp``` to enable the
      multi-threading optimizations.

   - F: The kernel
     The kernel is a lambda function taking as argument the elements of each range RX.
     If F returns a value, pixel_wise create a new image and fill it with those values.


The following snippet uses pixel_wise to add two images:

```c++
pixel_wise(A, B, C) | [] (int& a, int& b, int& c) { a = b + c; };
```

When there is no dependencies between the computations of two pixels,
the parallel version can speedup the execution of kernel on multi-core
architectures. 

The ```row_wise```, ```col_wise``` and ```block_wise``` routines
execute kernel working on an entire sub division of the image domain.

```c++
// Compute the sum of each row of A in its first column.
image2d<int> A(100, 100);

row_wise(A) | [&] (auto& row) { row(0,0) = sum(row); };
```


### Accessing Rectangular Neighborhood

Video++ provide a fast access to rectangular pixel neighboors:

```c++
// A parallel implementation of a 3x3 box_filter using Video++.

image2d<int> A(1000, 1000);
image2d<int> B(A.domain());

auto BN = box_nbh2d<int, 3, 3>(A);

// Parallel Loop over pixels of in and out.
pixel_wise(A, BN) | [&] (int& a, auto& b_nbh) {
  int sum = 0;

  // Loop over neighbors wrt nbh to compute a sum.
  b_nbh.forall([&] (int& n) { sum += n; });

  // Write the sum to the output image.
  b = sum / 3;
};
```

### Interoperability with OpenCV images

The header ```#include <vpp/opencv_bridge.hh>``` (not included by
default) provides conversions to and from OpenCV image types. It
allows to run video++ code on OpenCV images and to run OpenCV code on
video++ images, without cloning the pixel buffer.  Ownership of the buffer
will switch to OpenCV or Video++ depending of the order of the
destructor calls.

```c++
// Load JPG image in a vpp image using OpenCV imread.
image2d<vuchar3> img = from_opencv<vuchar3>(cv::imread("image.jpg"));

// Write a vpp image using OpenCV imwrite.
cv::imwrite("in.jpg", to_opencv(img));
```

Note: Since it is not possible to opencv to free video++ images, an
opencv image must not be the last one to hold a video++ image.

## Contributing

Contributions are welcome. Do not hesitate to fill issues, send pull
requests, or discuss by email at matthieu.garrigues@gmail.com.

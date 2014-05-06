Video++
=============

Video++ is a video and image processing library that takes advantage of the C++11 and C++14 standard to ease the writing of fast parallel real-time video and image processing.


```c++
// A fast parallel implementation of a box_filter using Video++.

image2d<int> A(1000, 1000);
image2d<int> B(A.domain());

auto nbh = make_window(A, { {0, -1}, {0, 0}, {0, 1}, });
pixel_wise(A, B) << [&] (auto& a, auto& b) {
  vint3 sum = vint3::Zero();

  // Loop over in's neighboords wrt nbh to compute a sum.
  for (vuchar3& n : nbh(a)) sum += n.cast<int>();

  // Write the sum to the output image.
  b = (sum / 3).cast<unsigned char>();
};
```

Supported compiler : **GCC 4.9**

**Since Video++ rely on C++14 features, only compilers supporting this standard are able to
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

If you use the parallel constructs of Video++, you also want to activate the OpenMP optimizations :
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


  // The two allocated buffers are automatically freed at the end of the scope.
}


```

### Vector Types

Video++ vector types are aliases to eigen3 vectors, providing fast linear algebra. The syntax
is ```v{T}{N}``` such as

 - T is one the following : *char, short, int, float, double, uchar, ushort, uint*.
 - N is an integer in [0, 4].

For exemple, the type ```image2d<vuchar4>``` can handle an image of RGBA 8-bit.

### Sequential and Single-Thread Kernels

```c++
// Iterate on the 2d coordinates of the image domain.
image2d<int> img(100, 100);
for (vint2& p : img.domain())
  img(p) = 42; // write 42 to the pixel with 2d coordinate p;

// Iterate on the img's pixel buffer (faster than the previous iteration).
image2d<int> img(100, 100);
for (int& pix : img)
  pix = 42; // Set the pixel value to 42.

```

### Parallel Multi-Thread Kernels


#### Pixel Wise Parallel Kernels

The ```pixel_wise``` construct allows to spread the execution of a
kernel to all the available cpu cores. Since pixel_wise execute the
kernel in parallel, there should not be any dependencies between the
computation of two pixels.

```c++
image2d<int> A(100, 100);
image2d<int> B(100, 100);
image2d<int> C(100, 100);

pixel_wise(A, B, C) << [] (int& a, int& b, int& c) {
  a = b + c;
};

```

#### Row Wise Parallel Kernels

```c++
// Compute the sum of each row of A into the sums, such as
// sums[i] is the sum of the ith row;
image2d<int> A(100, 100);
std::vector<int> sums(A.nrows(), 0);
int ncols = A.ncols();

row_wise(A, A.domain()) << [=] (int& row_start, vin2 coord) {
  int sum = 0;
  int* cur = &row_start; // Iterator.
  int* end = &row_start + ncols; // End of row.
  while (cur != end) counter += *(cur++); // Loop over the row.
  sums[coord[0]] = sum; // Write the result.
};

```

#### Col Wise Parallel Kernels

Similar to ```row_wise```, ```col_wise``` allow to paralelize col wise
kernels.


### Accessing the neighbors pixels

Video++ provide a fast access to pixel neighboors. By precomputing the
offset of a neighborhood expressed in 2D coordinates, it suppresses the
need of 2d coordinate arithmetic inside the kernel loop. Instead, it
uses a fast pointer arithmetic to iterate on the neighborhood.


```c++

// A parallel implementation of a box_filter using video++.

image2d<int> A(1000, 1000);
image2d<int> B(in.domain());

auto nbh = make_window(A, { {0, -1}, {0, 0}, {0, 1} });

// Parallel Loop over pixels of in and out.
pixel_wise(A, B) << [&] (auto& a, auto& b) {
  vint3 sum = vint3::Zero();

  // Loop over in's neighboords wrt nbh to compute a sum.
  for (vuchar3& n : nbh(a)) sum += n.cast<int>();

  // Write the sum to the output image.
  b = (sum / 3).cast<unsigned char>();
};
```

### Interoperatiliby with OpenCV images

The header ```#include <vpp/opencv_bridge.hh>``` (not included by
default) provides conversions to and from OpenCV image types. It
allows to run video++ code on OpenCV images and to run OpenCV code on
video++ images, without cloning the pixel buffer.  Ownership of the buffer
will switch to OpenCV or video++ depending of the order of the
destructor calls.

```c++
// Load JPG image in a vpp image using OpenCV imread.
image2d<vuchar3> img = from_opencv<vuchar3>(cv::imread("image.jpg"));

// Write a vpp image using OpenCV imwrite.
cv::imwrite("in.jpg", to_opencv(img));
```


## Contributing

Contrubutions are welcome. Do not hesitate to fill issues, send pull
requests, or discuss by email at matthieu.garrigues@gmail.com.

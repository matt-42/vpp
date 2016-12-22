[![Travis build status](https://travis-ci.org/matt-42/vpp.svg?branch=phd_work)](https://travis-ci.org/matt-42/vpp)

Video++
=============

Video++ is a video and image processing library taking advantage of
the C++14 standard to ease the writing of fast video and image
processing applications. The idea behind Video++ performance is to
generate via meta-programming code that the compiler can easily 
optimize. Its main features are:

  - Generic N-dimensional image containers.
  - A growing set of image processing algorithms.
  - Zero-cost abstractions to easily write image processing algorithms for multicore SIMD processors.
  - An embedded language to evaluate image expressions.

Tested compilers: **G++6, Clang++ 3.8.0**
Dependencies:
  - C++14
  - [Eigen 3](http://eigen.tuxfamily.org)
  - [Boost](http://www.boost.org/)
  - [the iod library](http://github.com/matt-42/iod) installed by the install.sh script

Video++ is free of use (BSD). If you you are using Video++ in
research related work, you are welcome to cite us :

```
@inproceedings{garrigues2014video++,
  title={Video++, a modern image and video processing C++ framework},
  author={Garrigues, Matthieu and Manzanera, Antoine},
  booktitle={Design and Architectures for Signal and Image Processing (DASIP), 2014 Conference on},
  pages={1--6},
  year={2014},
  organization={IEEE}
}
```

## Getting Started

### Installation

Before installing Video++, you need to get Eigen3 and Boost on your
system. Your package managers will probably help you to install these
libraries. Then, use the install.sh script to install iod and vpp :

```
git clone https://github.com/matt-42/vpp.git
cd vpp
./install.sh your_install_prefix # Install iod and vpp in a given prefix
```

Video++ is a header-only library, so you just need include the vpp.hh
header to access all the basic features of the library:

```c++
#include <vpp/vpp.hh>
```

If you use the parallel constructs of Video++ and need to target multiple cores, activate the OpenMP optimizations:
```sh
g++ -I __your_isntall_prefix__ main.cc -fopenmp -lgomp
```

## Image Containers

The generic container imageNd<V, N> represents a dense N-dimensional
rectangle set of pixels with values of type V. For convenience,
image1d<V>, image2d<V>, image3d<V> are respectively aliases to
imageNd<V, 1>, imageNd<V, 2>, and imageNd<V, 3>.

```c++
// Allocates a 100 rows x 200 columns 2d image of integers.
image2d<int> A(100, 200);
```

These types provide accesses to the pixel buffer and to other piece of
information useful to process the image. In contrast to std::vector,
assigning an image to the other does not copy the data, but share them
so no accidental expensive deep copy happen.

```c++
image2d<int> B = A; // B now points to A's data.
```

### Image options

#### _border and _aligned

Image constructors also take special image options. ```_border``` (default: 0) set
the border surrounding the pixels so filter accessing neighborhood
access to valid pixels when traversing image borders. ```_aligned``` (default: 16 bytes)
set the alignment in bytes of the beginning of the first pixel of each row
(border excluded) to enable aligned SIMD memory instructions.

```c++
// Allocates a 100x100 image with a border of 3 pixels
// and rows aligned on 32 bytes (best for 256 bits SIMD
// units like AVX2).
image2d<int> C(100, 100, _border = 3, _aligned = 32);
```

#### _data and _pitch

To use image buffers allocated by external libraries, the ```_data``` and
```_pitch``` options respectively pass the pointer to the first
pixel of the image and the distance in bytes between the first pixels
of two consecutive rows. When setting these options, video++ is not
responsible of freeing the data buffer.


```c++
// Wraps an external images of 100x100 pixels, with a pitch
// of 1000 bytes.
image2d<int> C(100, 100, _data = my_data_ptr, _pitch = 1000);
```

### OpenCV interoperability

The header ```vpp/utils/opencv_bridge.hh``` (not included by
```vpp/vpp.hh```) provides explicit conversions to and from OpenCV image types. It
allows running video++ algorithms on OpenCV images and to run OpenCV algorithms on
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
OpenCV image must not be the last one to hold a video++ image.


## Vector Types

Video++ vector types are aliases to eigen3 vectors, providing fast linear algebra. The syntax
is ```v{T}{N}``` such as

 - T is one the following : *char, short, int, float, double, uchar, ushort, uint*.
 - N is an integer in [0, 4].

For example, the type ```image2d<vuchar4>``` can handle an image of RGBA 8-bit.



## Pixel Wise Kernels

The ```pixel_wise``` construct allows to easily and efficiently map
simple kernel functions on all the pixels of a set of images. It relies
on OpenMP to spread the processing on all the available cores, and
SIMD-vectorize the processing of rows when possible. If the kernel
function returns a value, ```pixel_wise``` will return a newly
allocated image filled with the results of the kernel. Given A and B
two 2d images of integers:

```c++
// Compute C, the parallel pixel wise sum of A and B.
auto C = pixel_wise(A, B) | [] (int& a, int& b) { return a + b; };

// Compute the average of the c4 neighborhood.
pixel_wise(relative_access(A), B) | [] (auto a, int& b) {
  b = (a(-1, 0) + a(1, 0) + a(0, 1) + a(0, -1)) / 4;
};
```

### Kernel arguments

Given ```pixel_wise(I_0, ..., I_N) | my_kernel```, ```pixel_wise``` will
traverse the domain of I_0 and for every pixel location P call
```my_kernel(I_0(P), ..., I_N(P))``` with ```I_N(P)``` a reference to the
pixel of I_N located at location P.

### Parallelism and traversal directions

By default, ```pixel_wise``` traverses the image from left to right
and top to bottom and spread the processing of different row on
different cores. The following options allow to change this defaut behavior:

  - **_no_thread**: disable the parallelism.

  - **_left_to_right**: Traverse each row from left to right.

  - **_right_to_left**: Traverse each row from right to left.

  - **_top_to_bottom**: Traverse the rows from top to bottom.

  - **_bottom_to_top**: Traverse the rows from bottom to top.

### Accessing Neighborhood

Video++ provides fast access to rectangular pixel neighbors inside
pixel_wise kernels. The speed of access mainly comes from the direct
2d indexing of the neighborhood, and the ability of the compiler to
vectorize these accesses using SIMD vector extensions. To stay
portable, no explicit SIMD optimization is done by video++. On the
other hand, the generated code was designed to be trivial for the
compiler to vectorize if the kernel does not contain conditional
branching.

The accesses are expressed in relative coordinates. For example,
```nbh(1,1)``` accesses the bottom right neighbor of the current
pixel, and ```nbh(0,0)``` refers to the current pixel.

```c++
// Define a 3x3 neighborhood accessor on image B.
// Note that for performance reasons, the boundaries
// of the neighborhood are static.

pixel_wise(S, relative_access(B)) | [&] (auto& s, auto b) {

  // Average the pixels in the c4 neighborhood.
  s = (b(-1,0) + b(0, -1) +
      b(1,0) + b(0,1)) / 4;
};
```

## Block Wise Kernels

The```block_wise``` construct allows to map a function on every cell
of a given grid. This construct is similar to pixel_wise, but instead
of processing one pixel, the block_wise kernel processes a given cell.

```c++
// Given a grid with cell of size 10x10 pixels.
block_wise(vint2{10, 10}, A, B, C, A.domain()) |
  [] (auto a, auto b, auto c, box2d box)
  {
  
    // a, b and c are sub images representing A, B
    // and C at the current cell.
    // All the algorithms of the library work on sub images.

    // The box argument is the cell representation of A.domain() and holds
    // the coordinates of the current cell. box.p1() and box.p2() are
    // respectively the first and the last pixel of the cell.
};
```

### Row Wise and Col Wise Kernels

```row_wise``` and ```col_wise``` are shortcuts to process the image by row or by column.
For example, the following compute row wise sums of a given image:

```c++
std::vector<int> sums(A.nrows(), 0);
row_wise(A, A.domain()) | [] (auto& a, vint2 coord)
{
  sums[coord[0]] += a;
};
```

## Contributing

Contributions are welcome. Do not hesitate to fill issues, send pull
requests, or send me emails at matthieu.garrigues@gmail.com.

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

Tested compilers: **G++ 4.9.1, Clang++ 3.5.0**
Dependencies:
  - [the iod library](http://github.com/matt-42/iod)
  - [Eigen 3](http://eigen.tuxfamily.org)
  - [Boost](http://www.boost.org/)


**Because Video++ relies on C++14 features, only compilers fully supporting this standard are able to
compile the library.**

Video++ is free of use (BSD). But if you you are using Video++ in research related work, you are more than
welcome to cite us :

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
```
while being much shorter, runs as fast as the following OpenMP optimized code:

```c++
image2d<int> C(A.domain());
int nr = A.nrows();
int nc = A.nrows();

#pragma omp parallel for
for (int r = 0; r < nr; r++)
{
  int* curA = &A(r, 0);
  int* curB = &B(r, 0);
  int* curC = &C(r, 0);

  for (int i = 0; i < nc; i++)
    curA[i] = curB[i] + curC[i];
}
```

### Kernel arguments

Given ```pixel_wise(I_0, ..., I_N) | my_kernel```, ```pixel_wise``` will
traverse the definition domain and for every pixel location P call
```my_kernel(I_0(P), ..., I_N(P))``` with ```I_N(P)``` a reference to the
pixel of I_N located at location P.

### Parallelism and computational dependencies

By default, ```pixel_wise``` generates a parallel execution of the
kernel processing a batch of image row per processor core. However,
some recursive pixel wise kernels imply a dependency between the
computation of neighbor pixels. The ```pixel_wise``` construct let you
express these constraints. For example, the following set the
```_col_backward``` dependency to integrate pixel values along the
columns, from the bottom to the top of the image:

```c++
pixel_wise(A_nbh)(_col_backward) |
  [] (auto& nbh) { nbh(0,0) += nbh(1, 0); }
```

The following are valid options:

  - **_row_forward**: the computation of the i th pixel of row X depends
    on the computation of the i-1 th pixel of the same row.

  - **_row_backward**: the computation of the i-1th pixel of row X depends
    on the computation of the i th pixel of the same row.

  - **_col_forward**: the computation of the i th pixel of col X depends
    on the computation of the i-1 th pixel of the same col.

  - **_col_backward**: the computation of the i-1th pixel of col X depends
    on the computation of the i th pixel of the same col.

  - **_no_thread**: disable the parallelism without specifying a dependency.

To take these constraints into account, video++ change the way
pixel_wise iterate on images while keeping as much parallelism as
possible. For example, if **_col_backward** is specified, each thread
will process a batch of consecutive rows, in order to leverage
multiple cores and SIMD instructions.

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

auto BN = box_nbh2d<int, 3, 3>(B);

pixel_wise(S, BN) | [&] (auto& s, auto& b_nbh) {

  // Average the pixels in the c4 neighborhood.
  s = (b_nbh(-1,0) + b_nbh(0, -1) +
      b_nbh(1,0) + b_nbh(0,1)) / 4;
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


## 2D Image Expressions

**Warning: This feature is experimental and is currently very slow to compile.**


Video++ embeds into C++14 a domain specific language allowing to build
image expressions that will be evaluated using ```eval```. While
leveraging the power of multi-core SIMD architectures, it enables the developer
to shorten tens of lines of code into one line, easy to write and to
read.

For example, the following expression formulates the an operation on the
pixels of two images using the ```_v``` accessor:

```c++
_v(A) + _v(B) * 2
```

The ```eval``` routine actually runs this expression against each
pixels of A and B and returns the resulting image such that, given two
images A and B with same dimensions and pixels with type ```pixel_type```:

```c++
auto C = eval(_v(A) + _v(B) * 2);
```
is equivalent to

```c++
image2d<pixel_type> C(A.domain());
for (int r = 0; r < A.nrows(); r++)
for (int c = 0; c < A.ncols(); c++)
  C(r, c) = A(r, c) + B(r, c) * 2;
```

Placeholders can also be used to refer to images passed as arguments
to eval. Using placeholders, the use of _v to access pixels is not required:

```c++
auto C = eval(A, B, _1 + _2 * 2);
```

```eval``` generates code spanning one thread per processor
core. Depending on your compiler, there is a good chance that the
loops will be optimized with SIMD vector instructions.

Note that no technical challenge but some time constraints prevented
us to implement N-dimensional image expressions.

The following explains the different types of valid expressions.

### Assignments

In many cases, creating new images when evaluating an expression is not
needed and can affect the performances of an algorithm. To avoid this
extra image creation, assignments allows storing the result of an
expression in an existing image as in the following. Given A, B and C
three images of the same dimensions:

```c++
// No image allocation here:
eval(_v(C) = _v(A) + _v(B) * 2);
```

### Conditional branching

The language of image expression support conditional branching with a
construct similar to the ?: ternary operator of C++:

```c++
eval(_v(C) = _if(_v(A) > _v(B))
                (_v(A))
                (_v(B))
   );
```

### Global expressions

Image expressions are not limited to pixel wise operations. Global
expressions integrate information over the whole definition domain.

#### Sum

Sum an expression over the whole definition domain:

```c++
int sum = eval(_sum(_v(A) + _v(B)));
```

#### Min, Max

Find the minimum/maximum value of an expression:

```c++
auto min_value = eval(_min(_v(A) + _v(B)));
auto max_value = eval(_max(_v(A) + _v(B)));
```

#### Argmin, Argmax

Find the position (row, column) of the minimum/maximum value of an expression:

```c++
vint2 argmin = eval(_argmin(_v(A) + _v(B)));
vint2 argmax = eval(_argmax(_v(A) + _v(B)));
```

### Mixing global expression in pixel wise image expressions

In opposition to pixel wise image expressions, global expressions are
evaluated only once per expression. However, it is possible to include
them in pixel wise image expression without impacting
performances. The eval function first traverses the expression
abstract syntax tree (AST), replace them with their actual value and
then finally launch the pixel wise evaluation. As a result, the global
expressions are still evaluated once:

```c++

// Normalize pixel values and generate an image of float.
auto C_normalized = eval(_v(C) * 1.f / _max(C));

// Note: The multiplication with 1.f allows to force conversion from int
// pixel to float pixel values.
```

### Limitations

Because embedded domain specific languages such as image expressions involve
heavy C++ meta-programming, compilation time and compiler memory
consumption can increase significantly with the use of long and
complex image expressions.

## Contributing

Contributions are welcome. Do not hesitate to fill issues, send pull
requests, or send me emails at matthieu.garrigues@gmail.com.

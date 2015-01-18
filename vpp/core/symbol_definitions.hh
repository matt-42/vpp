//  Convert the symbol file into a C++ header using sed, or another text processing tool
//  with the equivalent command:
//  sed -e 's/^\([a-zA-Z1-9_]\)\([a-zA-Z1-9_]*\)/#ifndef IOD_SYMBOL__\U\1\L\2\E\n\#define IOD_SYMBOL__\U\1\L\2\E\n    iod_define_symbol(\1\2, _\U\1\L\2\E)\n#endif/' symbols.sb > symbol_definitions.hh 

#ifndef IOD_SYMBOL__tie_arguments
#define IOD_SYMBOL__tie_arguments
    iod_define_symbol(tie_arguments, _tie_arguments)
#endif
#ifndef IOD_SYMBOL__row_forward
#define IOD_SYMBOL__row_forward
    iod_define_symbol(row_forward, _row_forward)
#endif
#ifndef IOD_SYMBOL__row_backward
#define IOD_SYMBOL__row_backward
    iod_define_symbol(row_backward, _row_backward)
#endif
#ifndef IOD_SYMBOL__col_forward
#define IOD_SYMBOL__col_forward
    iod_define_symbol(col_forward, _col_forward)
#endif
#ifndef IOD_SYMBOL__col_backward
#define IOD_SYMBOL__col_backward
    iod_define_symbol(col_backward, _col_backward)
#endif
#ifndef IOD_SYMBOL__mem_forward
#define IOD_SYMBOL__mem_forward
    iod_define_symbol(mem_forward, _mem_forward)
#endif
#ifndef IOD_SYMBOL__mem_backward
#define IOD_SYMBOL__mem_backward
    iod_define_symbol(mem_backward, _mem_backward)
#endif
#ifndef IOD_SYMBOL__serial
#define IOD_SYMBOL__serial
    iod_define_symbol(serial, _serial)
#endif
#ifndef IOD_SYMBOL__block_size
#define IOD_SYMBOL__block_size
    iod_define_symbol(block_size, _block_size)
#endif
#ifndef IOD_SYMBOL__no_threads
#define IOD_SYMBOL__no_threads
    iod_define_symbol(no_threads, _no_threads)
#endif
#ifndef IOD_SYMBOL__aligned
#define IOD_SYMBOL__aligned
    iod_define_symbol(aligned, _aligned)
#endif
#ifndef IOD_SYMBOL__border
#define IOD_SYMBOL__border
    iod_define_symbol(border, _border)
#endif
#ifndef IOD_SYMBOL__data
#define IOD_SYMBOL__data
    iod_define_symbol(data, _data)
#endif
#ifndef IOD_SYMBOL__pitch
#define IOD_SYMBOL__pitch
    iod_define_symbol(pitch, _pitch)
#endif

#ifndef IOD_SYMBOL__v
#define IOD_SYMBOL__v
    iod_define_symbol(V, _v)
#endif

#ifndef IOD_SYMBOL__sum
#define IOD_SYMBOL__sum
    iod_define_symbol(sum, _sum)
#endif
#ifndef IOD_SYMBOL__avg
#define IOD_SYMBOL__avg
    iod_define_symbol(avg, _avg)
#endif
#ifndef IOD_SYMBOL__min
#define IOD_SYMBOL__min
    iod_define_symbol(min, _min)
#endif
#ifndef IOD_SYMBOL__max
#define IOD_SYMBOL__max
    iod_define_symbol(max, _max)
#endif
#ifndef IOD_SYMBOL__stddev
#define IOD_SYMBOL__stddev
    iod_define_symbol(stddev, _stddev)
#endif
#ifndef IOD_SYMBOL__argmin
#define IOD_SYMBOL__argmin
    iod_define_symbol(argmin, _argmin)
#endif
#ifndef IOD_SYMBOL__argmax
#define IOD_SYMBOL__argmax
    iod_define_symbol(argmax, _argmax)
#endif
#ifndef IOD_SYMBOL__if
#define IOD_SYMBOL__if
    iod_define_symbol(_if, _if)
#endif

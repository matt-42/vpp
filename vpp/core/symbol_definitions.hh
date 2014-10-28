//  Convert the symbol file into a C++ header using sed, or another text processing tool
//  with the equivalent command:
//  sed -e 's/^\([a-zA-Z1-9_]\)\([a-zA-Z1-9_]*\)/#ifndef IOD_SYMBOL__\U\1\L\2\E\n\#define IOD_SYMBOL__\U\1\L\2\E\n    iod_define_symbol(\1\2, _\U\1\L\2\E)\n#endif/' symbols.sb > symbol_definitions.hh 

#ifndef IOD_SYMBOL__Tie_arguments
#define IOD_SYMBOL__Tie_arguments
    iod_define_symbol(tie_arguments, _Tie_arguments)
#endif
#ifndef IOD_SYMBOL__Row_forward
#define IOD_SYMBOL__Row_forward
    iod_define_symbol(row_forward, _Row_forward)
#endif
#ifndef IOD_SYMBOL__Row_backward
#define IOD_SYMBOL__Row_backward
    iod_define_symbol(row_backward, _Row_backward)
#endif
#ifndef IOD_SYMBOL__Col_forward
#define IOD_SYMBOL__Col_forward
    iod_define_symbol(col_forward, _Col_forward)
#endif
#ifndef IOD_SYMBOL__Col_backward
#define IOD_SYMBOL__Col_backward
    iod_define_symbol(col_backward, _Col_backward)
#endif
#ifndef IOD_SYMBOL__Mem_forward
#define IOD_SYMBOL__Mem_forward
    iod_define_symbol(mem_forward, _Mem_forward)
#endif
#ifndef IOD_SYMBOL__Mem_backward
#define IOD_SYMBOL__Mem_backward
    iod_define_symbol(mem_backward, _Mem_backward)
#endif
#ifndef IOD_SYMBOL__Serial
#define IOD_SYMBOL__Serial
    iod_define_symbol(serial, _Serial)
#endif
#ifndef IOD_SYMBOL__Block_size
#define IOD_SYMBOL__Block_size
    iod_define_symbol(block_size, _Block_size)
#endif
#ifndef IOD_SYMBOL__No_threads
#define IOD_SYMBOL__No_threads
    iod_define_symbol(no_threads, _No_threads)
#endif
#ifndef IOD_SYMBOL__Aligned
#define IOD_SYMBOL__Aligned
    iod_define_symbol(aligned, _Aligned)
#endif
#ifndef IOD_SYMBOL__Border
#define IOD_SYMBOL__Border
    iod_define_symbol(border, _Border)
#endif
#ifndef IOD_SYMBOL__Data
#define IOD_SYMBOL__Data
    iod_define_symbol(data, _Data)
#endif
#ifndef IOD_SYMBOL__Pitch
#define IOD_SYMBOL__Pitch
    iod_define_symbol(pitch, _Pitch)
#endif

#ifndef IOD_SYMBOL__V
#define IOD_SYMBOL__V
    iod_define_symbol(V, _V)
#endif

#ifndef IOD_SYMBOL__Sum
#define IOD_SYMBOL__Sum
    iod_define_symbol(sum, _Sum)
#endif
#ifndef IOD_SYMBOL__Avg
#define IOD_SYMBOL__Avg
    iod_define_symbol(avg, _Avg)
#endif
#ifndef IOD_SYMBOL__Min
#define IOD_SYMBOL__Min
    iod_define_symbol(min, _Min)
#endif
#ifndef IOD_SYMBOL__Max
#define IOD_SYMBOL__Max
    iod_define_symbol(max, _Max)
#endif
#ifndef IOD_SYMBOL__Stddev
#define IOD_SYMBOL__Stddev
    iod_define_symbol(stddev, _Stddev)
#endif
#ifndef IOD_SYMBOL__Argmin
#define IOD_SYMBOL__Argmin
    iod_define_symbol(argmin, _Argmin)
#endif
#ifndef IOD_SYMBOL__Argmax
#define IOD_SYMBOL__Argmax
    iod_define_symbol(argmax, _Argmax)
#endif

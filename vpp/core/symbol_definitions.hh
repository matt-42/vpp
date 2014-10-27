//  Convert the symbol file into a C++ header using sed, or another text processing tool
//  with the equivalent command:
//  sed -e 's/^\([a-zA-Z1-9_]\)\([a-zA-Z1-9_]*\)/#ifndef IOD_SYMBOL__\U\1\L\2\n\#define IOD_SYMBOL__\U\1\L\2\n    iod_define_symbol(\1\2, _\U\1\L\2)\n#endif/' symbols.sb > symbol_definitions.hh

#ifndef IOD_SYMBOL__Tie_arguments
#define iod_symbol__Tie_arguments
    iod_define_symbol(tie_arguments, _Tie_arguments)
#endif
#ifndef IOD_SYMBOL__Row_forward
#define iod_symbol__Row_forward
    iod_define_symbol(row_forward, _Row_forward)
#endif
#ifndef IOD_SYMBOL__Row_backward
#define iod_symbol__Row_backward
    iod_define_symbol(row_backward, _Row_backward)
#endif
#ifndef IOD_SYMBOL__Col_forward
#define iod_symbol__Col_forward
    iod_define_symbol(col_forward, _Col_forward)
#endif
#ifndef IOD_SYMBOL__Col_backward
#define iod_symbol__Col_backward
    iod_define_symbol(col_backward, _Col_backward)
#endif
#ifndef IOD_SYMBOL__Mem_forward
#define iod_symbol__Mem_forward
    iod_define_symbol(mem_forward, _Mem_forward)
#endif
#ifndef IOD_SYMBOL__Mem_backward
#define iod_symbol__Mem_backward
    iod_define_symbol(mem_backward, _Mem_backward)
#endif
#ifndef IOD_SYMBOL__Serial
#define iod_symbol__Serial
    iod_define_symbol(serial, _Serial)
#endif
#ifndef IOD_SYMBOL__Block_size
#define iod_symbol__Block_size
    iod_define_symbol(block_size, _Block_size)
#endif
#ifndef IOD_SYMBOL__No_threads
#define iod_symbol__No_threads
    iod_define_symbol(no_threads, _No_threads)
#endif
#ifndef IOD_SYMBOL__Aligned
#define iod_symbol__Aligned
    iod_define_symbol(aligned, _Aligned)
#endif
#ifndef IOD_SYMBOL__Border
#define iod_symbol__Border
    iod_define_symbol(border, _Border)
#endif
#ifndef IOD_SYMBOL__Data
#define iod_symbol__Data
    iod_define_symbol(data, _Data)
#endif
#ifndef IOD_SYMBOL__Pitch
#define iod_symbol__Pitch
    iod_define_symbol(pitch, _Pitch)
#endif

#ifndef IOD_SYMBOL__V
#define iod_symbol__V
    iod_define_symbol(v, _V)
#endif
#pragma once

#include <iod/symbol.hh>

// iod symbols for video++.

//  Convert the symbol file into a C++ header using sed, or another text processing tool
//  with the equivalent command:
//  sed -e 's/^\([a-zA-Z1-9_]\+\)/#ifndef IOD_SYMBOL_\1\n\#define IOD_SYMBOL_\1\n    iod_define_symbol(\1)\n#endif/' symbols.sb > symbols.hh

#ifndef IOD_SYMBOL_tie_arguments
#define IOD_SYMBOL_tie_arguments
    iod_define_symbol(tie_arguments)
#endif
#ifndef IOD_SYMBOL_row_forward
#define IOD_SYMBOL_row_forward
    iod_define_symbol(row_forward)
#endif
#ifndef IOD_SYMBOL_row_backward
#define IOD_SYMBOL_row_backward
    iod_define_symbol(row_backward)
#endif
#ifndef IOD_SYMBOL_col_forward
#define IOD_SYMBOL_col_forward
    iod_define_symbol(col_forward)
#endif
#ifndef IOD_SYMBOL_col_backward
#define IOD_SYMBOL_col_backward
    iod_define_symbol(col_backward)
#endif
#ifndef IOD_SYMBOL_mem_forward
#define IOD_SYMBOL_mem_forward
    iod_define_symbol(mem_forward)
#endif
#ifndef IOD_SYMBOL_mem_backward
#define IOD_SYMBOL_mem_backward
    iod_define_symbol(mem_backward)
#endif
#ifndef IOD_SYMBOL_serial
#define IOD_SYMBOL_serial
    iod_define_symbol(serial)
#endif
#ifndef IOD_SYMBOL_block_size
#define IOD_SYMBOL_block_size
    iod_define_symbol(block_size)
#endif
#ifndef IOD_SYMBOL_no_threads
#define IOD_SYMBOL_no_threads
    iod_define_symbol(no_threads)
#endif

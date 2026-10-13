#!/usr/bin/env python

"""
The schedule information is taken from SiFive P600 LLVM schedule model
https://github.com/llvm/llvm-project/blob/llvmorg-19.1.3/llvm/lib/Target/RISCV/RISCVSchedSiFiveP600.td
"""

import os
import json

MAVIS = os.environ.get('MAVIS', "../../mavis/")

SUPPORTED_VECTOR_INSTS = {
# Vector Config Setting Instructions
    "vsetvli"  : {"pipe" : "vset", "latency" : 1},
    "vsetvl"   : {"pipe" : "vset", "latency" : 1},
    "vsetivli" : {"pipe" : "vset", "latency" : 1},

# Vector Loads and Stores: Vector Unit-Stride Instructions
    "vle8.v"  : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vle16.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 16},
    "vle32.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 32},
    "vle64.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 64},
    "vse8.v"  : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vse16.v" : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 16},
    "vse32.v" : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 32},
    "vse64.v" : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 64},
    "vlm.v"   : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vsm.v"   : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},

# Vector Loads and Stores: Vector Strided Instructions
    "vlse8.v"  : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vlse16.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 16},
    "vlse32.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 32},
    "vlse64.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 64},
    "vsse8.v"  : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vsse16.v" : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 16},
    "vsse32.v" : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 32},
    "vsse64.v" : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 64},

# Vector Loads and Stores: Vector Indexed Instructions
    "vluxei8.v"  : {"pipe" : "vload", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 8},
    "vluxei16.v" : {"pipe" : "vload", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 16},
    "vluxei32.v" : {"pipe" : "vload", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 32},
    "vluxei64.v" : {"pipe" : "vload", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 64},
    "vloxei8.v"  : {"pipe" : "vload", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 8},
    "vloxei16.v" : {"pipe" : "vload", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 16},
    "vloxei32.v" : {"pipe" : "vload", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 32},
    "vloxei64.v" : {"pipe" : "vload", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 64},
    "vsuxei8.v"  : {"pipe" : "vstore", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 8},
    "vsuxei16.v" : {"pipe" : "vstore", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 16},
    "vsuxei32.v" : {"pipe" : "vstore", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 32},
    "vsuxei64.v" : {"pipe" : "vstore", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 64},
    "vsoxei8.v"  : {"pipe" : "vstore", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 8},
    "vsoxei16.v" : {"pipe" : "vstore", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 16},
    "vsoxei32.v" : {"pipe" : "vstore", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 32},
    "vsoxei64.v" : {"pipe" : "vstore", "uop_gen" : "INDEX_LDST", "latency" : 1, "eew" : 64},

# Vector Loads and Stores: Unit-stride Fault-Only-First Loads
    "vle8ff.v"  : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vle16ff.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 16},
    "vle32ff.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 32},
    "vle64ff.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 64},

# Vector Loads and Stores: Vector Load/Store Segment Instructions
# Encoded in Mavis as other vector load and store instructions with the NF field set to 2+

# Vector Loads and Stores: Vector Load/Store Whole Register Instructions
    "vl1re8.v"  : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vl1re16.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 16},
    "vl1re32.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 32},
    "vl1re64.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 64},
    "vl2re8.v"  : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vl2re16.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 16},
    "vl2re32.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 32},
    "vl2re64.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 64},
    "vl4re8.v"  : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vl4re16.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 16},
    "vl4re32.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 32},
    "vl4re64.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 64},
    "vl8re8.v"  : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vl8re16.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 16},
    "vl8re32.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 32},
    "vl8re64.v" : {"pipe" : "vload", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 64},
    "vs1r.v"    : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vs2r.v"    : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vs4r.v"    : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},
    "vs8r.v"    : {"pipe" : "vstore", "uop_gen" : "ELEMENTWISE", "latency" : 1, "eew" : 8},

# Vector Integer Arithmetic Instructions: Vector Single-Width Integer Add and Subtract
    "vadd.vv"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vadd.vx"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vadd.vi"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsub.vv"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsub.vx"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vrsub.vi" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vrsub.vx" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Integer Arithmetic Instructions: Vector Widening Integer Add/Subtract
    "vwaddu.vv" : {"pipe" : "vint", "uop_gen" : "WIDENING", "latency" : 1},
    "vwaddu.vx" : {"pipe" : "vint", "uop_gen" : "WIDENING", "latency" : 1},
    "vwsubu.vv" : {"pipe" : "vint", "uop_gen" : "WIDENING", "latency" : 1},
    "vwsubu.vx" : {"pipe" : "vint", "uop_gen" : "WIDENING", "latency" : 1},
    "vwadd.vv"  : {"pipe" : "vint", "uop_gen" : "WIDENING", "latency" : 1},
    "vwadd.vx"  : {"pipe" : "vint", "uop_gen" : "WIDENING", "latency" : 1},
    "vwsub.vv"  : {"pipe" : "vint", "uop_gen" : "WIDENING", "latency" : 1},
    "vwsub.vx"  : {"pipe" : "vint", "uop_gen" : "WIDENING", "latency" : 1},
    "vwaddu.wv" : {"pipe" : "vint", "uop_gen" : "WIDENING_MIXED", "latency" : 1},
    "vwaddu.wx" : {"pipe" : "vint", "uop_gen" : "WIDENING_MIXED", "latency" : 1},
    "vwsubu.wv" : {"pipe" : "vint", "uop_gen" : "WIDENING_MIXED", "latency" : 1},
    "vwsubu.wx" : {"pipe" : "vint", "uop_gen" : "WIDENING_MIXED", "latency" : 1},
    "vwadd.wv"  : {"pipe" : "vint", "uop_gen" : "WIDENING_MIXED", "latency" : 1},
    "vwadd.wx"  : {"pipe" : "vint", "uop_gen" : "WIDENING_MIXED", "latency" : 1},
    "vwsub.wv"  : {"pipe" : "vint", "uop_gen" : "WIDENING_MIXED", "latency" : 1},
    "vwsub.wx"  : {"pipe" : "vint", "uop_gen" : "WIDENING_MIXED", "latency" : 1},

# Vector Integer Arithmetic Instructions: Vector Integer Extension
    "vzext.vf2" : {"pipe" : "vint", "uop_gen" : "INT_EXT", "latency" : 1},
    "vsext.vf2" : {"pipe" : "vint", "uop_gen" : "INT_EXT", "latency" : 1},
    "vzext.vf4" : {"pipe" : "vint", "uop_gen" : "INT_EXT", "latency" : 1},
    "vsext.vf4" : {"pipe" : "vint", "uop_gen" : "INT_EXT", "latency" : 1},
    "vzext.vf8" : {"pipe" : "vint", "uop_gen" : "INT_EXT", "latency" : 1},
    "vsext.vf8" : {"pipe" : "vint", "uop_gen" : "INT_EXT", "latency" : 1},

# Vector Integer Arithmetic Instructions: Vector Integer Add-with-Carry/Subtract-with-Borrow Instructions
# FIXME: Requires Mavis fix to include vector mask
    "vadc.vvm"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vadc.vxm"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vadc.vim"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmadc.vvm" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmadc.vxm" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmadc.vim" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmadc.vv"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmadc.vx"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmadc.vi"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsbc.vvm"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsbc.vxm"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmsbc.vvm" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmsbc.vxm" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmsbc.vv"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmsbc.vx"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Integer Arithmetic Instructions: Vector Bitwise Logical Instructions
    "vand.vv" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vand.vx" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vand.vi" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vor.vv"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vor.vx"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vor.vi"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vxor.vv" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vxor.vx" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vxor.vi" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Integer Arithmetic Instructions: Vector Single-Width Shift Instructions
    "vsll.vv" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsll.vx" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsll.vi" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsrl.vv" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsrl.vx" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsrl.vi" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsra.vv" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsra.vx" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vsra.vi" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Integer Arithmetic Instructions: Vector Narrowing Integer Right Shift Instructions
    "vnsrl.wv" : {"pipe" : "vint", "uop_gen" : "NARROWING", "latency" : 1},
    "vnsrl.wx" : {"pipe" : "vint", "uop_gen" : "NARROWING", "latency" : 1},
    "vnsrl.wi" : {"pipe" : "vint", "uop_gen" : "NARROWING", "latency" : 1},
    "vnsra.wv" : {"pipe" : "vint", "uop_gen" : "NARROWING", "latency" : 1},
    "vnsra.wx" : {"pipe" : "vint", "uop_gen" : "NARROWING", "latency" : 1},
    "vnsra.wi" : {"pipe" : "vint", "uop_gen" : "NARROWING", "latency" : 1},

# Vector Integer Arithmetic Instructions: Vector Integer Compare Instructions
    "vmseq.vv"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmseq.vx"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmseq.vi"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsne.vv"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsne.vx"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsne.vi"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsltu.vv" : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsltu.vx" : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmslt.vv"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmslt.vx"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsleu.vv" : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsleu.vx" : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsleu.vi" : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsle.vv"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsle.vx"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsle.vi"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsgtu.vx" : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsgtu.vi" : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsgt.vx"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},
    "vmsgt.vi"  : {"pipe" : "vint", "uop_gen" : "SINGLE_DEST", "latency" : 1},

# Vector Integer Arithmetic Instructions: Vector Integer Min/Max Instructions
    "vminu.vv" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vminu.vx" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmin.vv"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmin.vx"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmaxu.vv" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmaxu.vx" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmax.vv"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmax.vx"  : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Integer Arithmetic Instructions: Vector Single-Width Integer Multiply Instructions
    "vmul.vv"    : {"pipe" : "vmul", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vmul.vx"    : {"pipe" : "vmul", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vmulhu.vx"  : {"pipe" : "vmul", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vmulhu.vv"  : {"pipe" : "vmul", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vmulh.vv"   : {"pipe" : "vmul", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vmulh.vx"   : {"pipe" : "vmul", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vmulhsu.vv" : {"pipe" : "vmul", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vmulhsu.vx" : {"pipe" : "vmul", "uop_gen" : "ELEMENTWISE", "latency" : 3},

# Vector Integer Arithmetic Instructions: Vector Integer Divide Instructions
    "vdiv.vv"  : {"pipe" : "vdiv", "uop_gen" : "ELEMENTWISE", "latency" : 23},
    "vdiv.vx"  : {"pipe" : "vdiv", "uop_gen" : "ELEMENTWISE", "latency" : 23},
    "vdivu.vv" : {"pipe" : "vdiv", "uop_gen" : "ELEMENTWISE", "latency" : 23},
    "vdivu.vx" : {"pipe" : "vdiv", "uop_gen" : "ELEMENTWISE", "latency" : 23},
    "vremu.vv" : {"pipe" : "vdiv", "uop_gen" : "ELEMENTWISE", "latency" : 23},
    "vremu.vx" : {"pipe" : "vdiv", "uop_gen" : "ELEMENTWISE", "latency" : 23},
    "vrem.vv"  : {"pipe" : "vdiv", "uop_gen" : "ELEMENTWISE", "latency" : 23},
    "vrem.vx"  : {"pipe" : "vdiv", "uop_gen" : "ELEMENTWISE", "latency" : 23},

# Vector Integer Arithmetic Instructions: Vector Widening Integer Multiply Instructions
    "vwmul.vv"   : {"pipe" : "vmul", "uop_gen" : "WIDENING", "latency" : 3},
    "vwmul.vx"   : {"pipe" : "vmul", "uop_gen" : "WIDENING", "latency" : 3},
    "vwmulu.vv"  : {"pipe" : "vmul", "uop_gen" : "WIDENING", "latency" : 3},
    "vwmulu.vx"  : {"pipe" : "vmul", "uop_gen" : "WIDENING", "latency" : 3},
    "vwmulsu.vv" : {"pipe" : "vmul", "uop_gen" : "WIDENING", "latency" : 3},
    "vwmulsu.vx" : {"pipe" : "vmul", "uop_gen" : "WIDENING", "latency" : 3},

# Vector Integer Arithmetic Instructions: Vector Single-Width Integer Multiply-Add Instructions
     "vmacc.vv"  : {"pipe" : "vmul", "uop_gen" : "MAC", "latency" : 3},
     "vmacc.vx"  : {"pipe" : "vmul", "uop_gen" : "MAC", "latency" : 3},
     "vnmsac.vv" : {"pipe" : "vmul", "uop_gen" : "MAC", "latency" : 3},
     "vnmsac.vx" : {"pipe" : "vmul", "uop_gen" : "MAC", "latency" : 3},
     "vmadd.vv"  : {"pipe" : "vmul", "uop_gen" : "MAC", "latency" : 3},
     "vmadd.vx"  : {"pipe" : "vmul", "uop_gen" : "MAC", "latency" : 3},
     "vnmsub.vv" : {"pipe" : "vmul", "uop_gen" : "MAC", "latency" : 3},
     "vnmsub.vx" : {"pipe" : "vmul", "uop_gen" : "MAC", "latency" : 3},

# Vector Integer Arithmetic Instructions: Vector Widening Integer Multiply-Add Instructions
     "vwmaccu.vv"  : {"pipe" : "vmul", "uop_gen" : "MAC_WIDE", "latency" : 3},
     "vwmaccu.vx"  : {"pipe" : "vmul", "uop_gen" : "MAC_WIDE", "latency" : 3},
     "vwmacc.vv"   : {"pipe" : "vmul", "uop_gen" : "MAC_WIDE", "latency" : 3},
     "vwmacc.vx"   : {"pipe" : "vmul", "uop_gen" : "MAC_WIDE", "latency" : 3},
     "vwmaccsu.vv" : {"pipe" : "vmul", "uop_gen" : "MAC_WIDE", "latency" : 3},
     "vwmaccsu.vx" : {"pipe" : "vmul", "uop_gen" : "MAC_WIDE", "latency" : 3},
     "vwmaccus.vx" : {"pipe" : "vmul", "uop_gen" : "MAC_WIDE", "latency" : 3},

# Vector Integer Arithmetic Instructions: Vector Integer Merge Instructions
# FIXME: Requires Mavis fix to include vector mask
    "vmerge.vvm" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmerge.vxm" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmerge.vim" : {"pipe" : "vint", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Integer Arithmetic Instructions: Vector Integer Move Instructions
    "vmv.v.v" : {"pipe" : "vmv", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmv.v.x" : {"pipe" : "vmv", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmv.v.i" : {"pipe" : "vmv", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Fixed-Point Arithmetic Instructions: Vector Single-Width Saturating Add and Subtract
    "vsaddu.vv" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vsaddu.vx" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vsaddu.vi" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vsadd.vv"  : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vsadd.vx"  : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vsadd.vi"  : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vssubu.vv" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vssubu.vx" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vssub.vv"  : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vssub.vx"  : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},

# Vector Fixed-Point Arithmetic Instructions: Vector Single-Width Averaging Add and Subtract
    "vaaddu.vv" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vaaddu.vx" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vaadd.vv"  : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vaadd.vx"  : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vasubu.vv" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vasubu.vx" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vasub.vv"  : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vasub.vx"  : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},

# Vector Fixed-Point Arithmetic Instructions: Vector Single-Width Fractional Multiply with Rounding and Saturation
    "vsmul.vx" : {"pipe" : "vmul", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vsmul.vv" : {"pipe" : "vmul", "uop_gen" : "ELEMENTWISE", "latency" : 3},

# Vector Fixed-Point Arithmetic Instructions: Vector Single-Width Scaling Shift Instructions
    "vssrl.vv" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vssrl.vx" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vssrl.vi" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vssra.vv" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vssra.vx" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vssra.vi" : {"pipe" : "vfixed", "uop_gen" : "ELEMENTWISE", "latency" : 6},

# Vector Fixed-Point Arithmetic Instructions: Vector Narrowing Fixed-Point Clip Instructions
    "vnclipu.wv" : {"pipe" : "vfixed", "uop_gen" : "NARROWING", "latency" : 2},
    "vnclipu.wx" : {"pipe" : "vfixed", "uop_gen" : "NARROWING", "latency" : 2},
    "vnclipu.wi" : {"pipe" : "vfixed", "uop_gen" : "NARROWING", "latency" : 2},
    "vnclip.wv"  : {"pipe" : "vfixed", "uop_gen" : "NARROWING", "latency" : 2},
    "vnclip.wx"  : {"pipe" : "vfixed", "uop_gen" : "NARROWING", "latency" : 2},
    "vnclip.wi"  : {"pipe" : "vfixed", "uop_gen" : "NARROWING", "latency" : 2},

# Vector Floating-Point Instructions: Vector Single-Width Floating-Point Add/Subtract Instructions
    "vfadd.vv"  : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfadd.vf"  : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfsub.vv"  : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfsub.vf"  : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfrsub.vf" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 6},

# Vector Floating-Point Instructions: Vector Widening Floating-Point Add/Subtract Instructions
    "vfwadd.vv" : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwadd.vf" : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwsub.vv" : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwsub.vf" : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwadd.wv" : {"pipe" : "vfloat", "uop_gen" : "WIDENING_MIXED", "latency" : 6},
    "vfwadd.wf" : {"pipe" : "vfloat", "uop_gen" : "WIDENING_MIXED", "latency" : 6},
    "vfwsub.wv" : {"pipe" : "vfloat", "uop_gen" : "WIDENING_MIXED", "latency" : 6},
    "vfwsub.wf" : {"pipe" : "vfloat", "uop_gen" : "WIDENING_MIXED", "latency" : 6},

# Vector Floating-Point Instructions: Vector Single-Width Floating-Point Multiply/Divide Instructions
    "vfmul.vv" : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfmul.vf" : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfdiv.vv" : {"pipe" : "vfdiv", "uop_gen" : "ELEMENTWISE", "latency" : 25},
    "vfdiv.vf" : {"pipe" : "vfdiv", "uop_gen" : "ELEMENTWISE", "latency" : 25},
    "vfrdiv.vf" : {"pipe" : "vfdiv", "uop_gen" : "ELEMENTWISE", "latency" : 25},

# Vector Floating-Point Instructions: Vector Widening Floating-Point Multiply
    "vfwmul.vv" : {"pipe" : "vfmul", "uop_gen" : "WIDENING", "latency" : 3},
    "vfwmul.vf" : {"pipe" : "vfmul", "uop_gen" : "WIDENING", "latency" : 3},

# Vector Floating-Point Instructions: Vector Single-Width Floating-Point Fused Multiply-Add Instructions
    "vfmacc.vv"  : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfmacc.vf"  : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfnmacc.vv" : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfnmacc.vf" : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfmsac.vv"  : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfmsac.vf"  : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfnmsac.vv" : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfnmsac.vf" : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfmadd.vv"  : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfmadd.vf"  : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfnmadd.vv" : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfnmadd.vf" : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfmsub.vv"  : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfmsub.vf"  : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfnmsub.vv" : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},
    "vfnmsub.vf" : {"pipe" : "vfmul", "uop_gen" : "ELEMENTWISE", "latency" : 6},

# Vector Floating-Point Instructions: Vector Widening Floating-Point Fused Multiply-Add Instructions
    "vfwmacc.vv"  : {"pipe" : "vfmul", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwmacc.vf"  : {"pipe" : "vfmul", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwnmacc.vv" : {"pipe" : "vfmul", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwnmacc.vf" : {"pipe" : "vfmul", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwmsac.vv"  : {"pipe" : "vfmul", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwmsac.vf"  : {"pipe" : "vfmul", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwnmsac.vv" : {"pipe" : "vfmul", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwnmsac.vf" : {"pipe" : "vfmul", "uop_gen" : "WIDENING", "latency" : 6},

# Vector Floating-Point Instructions: Vector Floating-Point Square-Root Instruction
    "vfsqrt.v" : {"pipe" : "vfdiv", "uop_gen" : "ELEMENTWISE", "latency" : 25},

# TODO: support variable length latency
# Vector Floating-Point Instructions: Vector Floating-Point Reciprocal Square-Root Estimate Instruction
    "vfrsqrt7.v" : {"pipe" : "vfdiv", "uop_gen" : "ELEMENTWISE", "latency" : 2},

# Vector Floating-Point Instructions: Vector Floating-Point Reciprocal Estimate Instruction
    "vfrec7.v" : {"pipe" : "vfdiv", "uop_gen" : "ELEMENTWISE", "latency" : 2},

# Vector Floating-Point Instructions: Vector Floating-Point MIN/MAX Instructions
    "vfmin.vv" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vfmin.vf" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vfmax.vv" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vfmax.vf" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Floating-Point Instructions: Vector Floating-Point Sign-Injection Instructions
    "vfsgnj.vv"  : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vfsgnj.vf"  : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vfsgnjn.vv" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vfsgnjn.vf" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vfsgnjx.vv" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vfsgnjx.vf" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Floating-Point Instructions: Vector Floating-Point Compare Instructions
    "vmfeq.vv"  : {"pipe" : "vfloat", "uop_gen" : "SINGLE_DEST", "latency" : 2},
    "vmfeq.vf"  : {"pipe" : "vfloat", "uop_gen" : "SINGLE_DEST", "latency" : 2},
    "vmfne.vv"  : {"pipe" : "vfloat", "uop_gen" : "SINGLE_DEST", "latency" : 2},
    "vmfne.vf"  : {"pipe" : "vfloat", "uop_gen" : "SINGLE_DEST", "latency" : 2},
    "vmflt.vv"  : {"pipe" : "vfloat", "uop_gen" : "SINGLE_DEST", "latency" : 2},
    "vmflt.vf"  : {"pipe" : "vfloat", "uop_gen" : "SINGLE_DEST", "latency" : 2},
    "vmfle.vv"  : {"pipe" : "vfloat", "uop_gen" : "SINGLE_DEST", "latency" : 2},
    "vmfle.vf"  : {"pipe" : "vfloat", "uop_gen" : "SINGLE_DEST", "latency" : 2},
    "vmfgt.vf"  : {"pipe" : "vfloat", "uop_gen" : "SINGLE_DEST", "latency" : 2},
    "vmfge.vf"  : {"pipe" : "vfloat", "uop_gen" : "SINGLE_DEST", "latency" : 2},

# Vector Floating-Point Instructions: Vector Floating-Point Classify Instruction
    "vfclass.v" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Floating-Point Instructions: Vector Floating-Point Merge Instruction
    "vfmerge.vfm" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Floating-Point Instructions: Vector Floating-Point Move Instruction
    "vfmv.v.f" : {"pipe" : "vmv", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Floating-Point Instructions: Single-Width Floating-Point/Integer Type-Convert Instructions
    "vfcvt.xu.f.v"     : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfcvt.x.f.v"      : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfcvt.rtz.xu.f.v" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfcvt.rtz.x.f.v"  : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfcvt.f.xu.v"     : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfcvt.f.x.v"      : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},

# Vector Floating-Point Instructions: Widening Floating-Point/Integer Type-Convert Instructions
    "vfwcvt.xu.f.v"     : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwcvt.x.f.v"      : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwcvt.rtz.xu.f.v" : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwcvt.rtz.x.f.v"  : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwcvt.f.xu.v"     : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwcvt.f.x.v"      : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},
    "vfwcvt.f.f.v"      : {"pipe" : "vfloat", "uop_gen" : "WIDENING", "latency" : 6},

# Vector Floating-Point Instructions: Narrowing Floating-Point/Integer Type-Convert Instructions
    "vfncvt.xu.f.w"     : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfncvt.x.f.w"      : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfncvt.rtz.xu.f.w" : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfncvt.rtz.x.f.w"  : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfncvt.f.xu.w"     : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfncvt.f.x.w"      : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfncvt.f.f.w"      : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},
    "vfncvt.rod.f.f.w"  : {"pipe" : "vfloat", "uop_gen" : "ELEMENTWISE", "latency" : 3},

# Vector Reduction Operations: Vector Single-Width Integer Reduction Instructions
    "vredsum.vs"        : {"pipe" : "vint", "uop_gen" : "RED", "latency" : 2},
    "vredmaxu.vs"       : {"pipe" : "vint", "uop_gen" : "RED", "latency" : 2},
    "vredmax.vs"        : {"pipe" : "vint", "uop_gen" : "RED", "latency" : 2},
    "vredminu.vs"       : {"pipe" : "vint", "uop_gen" : "RED", "latency" : 2},
    "vredmin.vs"        : {"pipe" : "vint", "uop_gen" : "RED", "latency" : 2},
    "vredand.vs"        : {"pipe" : "vint", "uop_gen" : "RED", "latency" : 2},
    "vredor.vs"         : {"pipe" : "vint", "uop_gen" : "RED", "latency" : 2},
    "vredxor.vs"        : {"pipe" : "vint", "uop_gen" : "RED", "latency" : 2},

# Vector Reduction Operations: Vector Widening Integer Reduction Instructions
    "vwredsumu.vs"      : {"pipe" : "vint", "uop_gen" : "RED_WIDE", "latency" : 2},
    "vwredsum.vs"       : {"pipe" : "vint", "uop_gen" : "RED_WIDE", "latency" : 2},

# Vector Reduction Operations: Vector Single-Width Floating-Point Reduction Instructions
    "vfredosum.vs"      : {"pipe" : "vfloat", "uop_gen" : "RED", "latency" : 2},
    "vfredusum.vs"      : {"pipe" : "vfloat", "uop_gen" : "RED", "latency" : 2},
    "vfredmax.vs"       : {"pipe" : "vfloat", "uop_gen" : "RED", "latency" : 2},
    "vfredmin.vs"       : {"pipe" : "vfloat", "uop_gen" : "RED", "latency" : 2},

# Vector Reduction Operations: Vector Widening Floating-Point Reduction Instructions
    "vfwredosum.vs"     : {"pipe" : "vfloat", "uop_gen" : "RED_WIDE", "latency" : 2},
    "vfwredusum.vs"     : {"pipe" : "vfloat", "uop_gen" : "RED_WIDE", "latency" : 2},

# Vector Mask Instructions: Vector Mask-Register Logical Instructions
    "vmandn.mm" : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},
    "vmand.mm"  : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},
    "vmor.mm"   : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},
    "vmxor.mm"  : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},
    "vmorn.mm"  : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},
    "vmnand.mm" : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},
    "vmnor.mm"  : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},
    "vmxnor.mm" : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},

# Vector Mask Instructions: Vector count population in mask vcpop.m
    "vcpop.m" : {"pipe" : "v2s", "uop_gen" : "NONE", "latency" : 1},

# Vector Mask Instructions: vfirst find-first-set mask bit
    "vfirst.m" : {"pipe" : "v2s", "uop_gen" : "NONE", "latency" : 1},

# Vector Mask Instructions: vmsbf.m set-before-first mask bit
    "vmsbf.m" : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},

# Vector Mask Instructions: vmsif.m set-including-first mask bit
    "vmsif.m" : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},

# Vector Mask Instructions: vmsof.m set-only-first mask bit
    "vmsof.m" : {"pipe" : "vmask", "uop_gen" : "NONE", "latency" : 1},

# Vector Mask Instructions: Vector Iota Instruction
    "viota.m" : {"pipe" : "vmask", "uop_gen" : "SINGLE_SRC", "latency" : 1},

# Vector Mask Instructions: Vector Element Index Instruction
    "vid.v" : {"pipe" : "vmask", "uop_gen" : "ELEMENTWISE", "latency" : 1},

# Vector Permutation Instructions: Integer Scalar Move Instructions
    "vmv.x.s" : {"pipe" : "v2s", "uop_gen" : "NONE", "latency" : 1},
    "vmv.s.x" : {"pipe" : "vmv", "uop_gen" : "NONE", "latency" : 1},

# Vector Permutation Instructions: Floating-Point Scalar Move Instructions
    "vfmv.f.s" : {"pipe" : "v2s", "uop_gen" : "NONE", "latency" : 1},
    "vfmv.s.f" : {"pipe" : "vmv", "uop_gen" : "NONE", "latency" : 1},

# Vector Permutation Instructions: Vector Slide Instructions
    "vslideup.vx"    : {"pipe" : "vpermute", "uop_gen" : "SLIDE",    "latency" : 6},
    "vslideup.vi"    : {"pipe" : "vpermute", "uop_gen" : "SLIDE",    "latency" : 6},
    "vslidedown.vx"  : {"pipe" : "vpermute", "uop_gen" : "SLIDE",    "latency" : 6},
    "vslidedown.vi"  : {"pipe" : "vpermute", "uop_gen" : "SLIDE",    "latency" : 6},
    "vslide1up.vx"   : {"pipe" : "vint",     "uop_gen" : "SLIDE1UP",   "latency" : 1},
    "vfslide1up.vf"  : {"pipe" : "vfloat",   "uop_gen" : "SLIDE1UP",   "latency" : 1},
    "vslide1down.vx" : {"pipe" : "vint",     "uop_gen" : "SLIDE1DOWN", "latency" : 1},
    "vfslide1down.vf": {"pipe" : "vfloat",   "uop_gen" : "SLIDE1DOWN", "latency" : 1},

# Vector Permutation Instructions: Vector Register Gather Instructions
    "vrgather.vv"    : {"pipe" : "vpermute", "uop_gen" : "PERMUTE", "latency" : 6},
    "vrgatherei16.vv": {"pipe" : "vpermute", "uop_gen" : "PERMUTE", "latency" : 6, "eew" : 16},
    "vrgather.vx"    : {"pipe" : "vpermute", "uop_gen" : "PERMUTE", "latency" : 6},
    "vrgather.vi"    : {"pipe" : "vpermute", "uop_gen" : "PERMUTE", "latency" : 6},

# Vector Permutation Instructions: Vector Compress Instruction
    "vcompress.vm" : {"pipe" : "vpermute", "uop_gen" : "PERMUTE", "latency" : 6},

# Vector Permutation Instructions: Whole Vector Register Move
    "vmv1r.v" : {"pipe" : "vmv", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmv2r.v" : {"pipe" : "vmv", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmv4r.v" : {"pipe" : "vmv", "uop_gen" : "ELEMENTWISE", "latency" : 1},
    "vmv8r.v" : {"pipe" : "vmv", "uop_gen" : "ELEMENTWISE", "latency" : 1},
}


def write_uarch_json(insts, filename):
    # Get a list of all vector insts from Mavis
    mavis_rv64v_json = []
    with open(MAVIS+"json/isa_rv64zve32f.json", "r") as f:
        mavis_rv64v_json.extend(json.load(f))
    with open(MAVIS+"json/isa_rv64zve32x.json", "r") as f:
        mavis_rv64v_json.extend(json.load(f))
    with open(MAVIS+"json/isa_rv64zve64d.json", "r") as f:
        mavis_rv64v_json.extend(json.load(f))
    with open(MAVIS+"json/isa_rv64zve64x.json", "r") as f:
        mavis_rv64v_json.extend(json.load(f))

    # Sort alphabetically by mnemonic
    mavis_rv64v_json.sort(key=lambda opcode: opcode["mnemonic"])

    # Construct the uarch json
    uarch_json = []
    for opcode in mavis_rv64v_json:
        mnemonic = opcode["mnemonic"]

        # Assume inst is unsupported
        opcode_entry = {
            "mnemonic" : mnemonic,
            "pipe" : "?",
            "uop_gen" : "NONE",
            "latency" : 0
        }

        # If it is supported, get the pipe and latency
        if mnemonic in insts.keys():
            opcode_entry["mnemonic"] = mnemonic
            opcode_entry["pipe"] = insts[mnemonic]["pipe"]
            if(insts[mnemonic].get("uop_gen")):
                opcode_entry["uop_gen"] = insts[mnemonic]["uop_gen"]
            opcode_entry["latency"] = insts[mnemonic]["latency"]
            if(insts[mnemonic].get("eew")):
                opcode_entry["eew"] = insts[mnemonic]["eew"]

        uarch_json.append(opcode_entry)

    # Write the json to the file
    with open(filename, "w") as f:
        print("Writing rv64v uarch json to "+filename)
        json.dump(uarch_json, f, indent=4)


def main():
    write_uarch_json(SUPPORTED_VECTOR_INSTS, "olympia_uarch_rv64v.json")


if __name__ == "__main__":
    main()

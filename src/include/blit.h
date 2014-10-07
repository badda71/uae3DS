#define blit_func_0x0(srca,srcb,srcc) return 0
#define blit_func_0x1(srca,srcb,srcc) return ~(srca | srcb | srcc)
#define blit_func_0x2(srca,srcb,srcc) return (srcc & ~(srca | srcb))
#define blit_func_0x3(srca,srcb,srcc) return ~(srca | srcb)
#define blit_func_0x4(srca,srcb,srcc) return (srcb & ~(srca | srcc))
#define blit_func_0x5(srca,srcb,srcc) return ~(srca | srcc)
#define blit_func_0x6(srca,srcb,srcc) return (~srca & (srcb ^ srcc))
#define blit_func_0x7(srca,srcb,srcc) return ~(srca | (srcb & srcc))
#define blit_func_0x8(srca,srcb,srcc) return (~srca & srcb & srcc)
#define blit_func_0x9(srca,srcb,srcc) return ~(srca | (srcb ^ srcc))
#define blit_func_0xa(srca,srcb,srcc) return (~srca & srcc)
#define blit_func_0xb(srca,srcb,srcc) return ~(srca | (srcb & ~srcc))
#define blit_func_0xc(srca,srcb,srcc) return (~srca & srcb)
#define blit_func_0xd(srca,srcb,srcc) return ~(srca | (~srcb & srcc))
#define blit_func_0xe(srca,srcb,srcc) return (~srca & (srcb | srcc))
#define blit_func_0xf(srca,srcb,srcc) return ~srca
#define blit_func_0x10(srca,srcb,srcc) return (srca & ~(srcb | srcc))
#define blit_func_0x11(srca,srcb,srcc) return ~(srcb | srcc)
#define blit_func_0x12(srca,srcb,srcc) return (~srcb & (srca ^ srcc))
#define blit_func_0x13(srca,srcb,srcc) return ~(srcb | (srca & srcc))
#define blit_func_0x14(srca,srcb,srcc) return (~srcc & (srca ^ srcb))
#define blit_func_0x15(srca,srcb,srcc) return ~(srcc | (srca & srcb))
#define blit_func_0x16(srca,srcb,srcc) return (srca ^ ((srca & srcb) | (srcb ^ srcc)))
#define blit_func_0x17(srca,srcb,srcc) return ~(srca ^ ((srca ^ srcb) & (srca ^ srcc)))
#define blit_func_0x18(srca,srcb,srcc) return ((srca ^ srcb) & (srca ^ srcc))
#define blit_func_0x19(srca,srcb,srcc) return (srcb ^ (~srcc | (srca & srcb)))
#define blit_func_0x1a(srca,srcb,srcc) return (srca ^ (srcc | (srca & srcb)))
#define blit_func_0x1b(srca,srcb,srcc) return (srca ^ (srcc | ~(srca ^ srcb)))
#define blit_func_0x1c(srca,srcb,srcc) return (srca ^ (srcb | (srca & srcc)))
#define blit_func_0x1d(srca,srcb,srcc) return (srca ^ (srcb | ~(srca ^ srcc)))
#define blit_func_0x1e(srca,srcb,srcc) return (srca ^ (srcb | srcc))
#define blit_func_0x1f(srca,srcb,srcc) return ~(srca & (srcb | srcc))
#define blit_func_0x20(srca,srcb,srcc) return (srca & ~srcb & srcc)
#define blit_func_0x21(srca,srcb,srcc) return ~(srcb | (srca ^ srcc))
#define blit_func_0x22(srca,srcb,srcc) return (~srcb & srcc)
#define blit_func_0x23(srca,srcb,srcc) return ~(srcb | (srca & ~srcc))
#define blit_func_0x24(srca,srcb,srcc) return ((srca ^ srcb) & (srcb ^ srcc))
#define blit_func_0x25(srca,srcb,srcc) return (srca ^ (~srcc | (srca & srcb)))
#define blit_func_0x26(srca,srcb,srcc) return (srcb ^ (srcc | (srca & srcb)))
#define blit_func_0x27(srca,srcb,srcc) return ~(srca ^ (srcc & (srca ^ srcb)))
#define blit_func_0x28(srca,srcb,srcc) return (srcc & (srca ^ srcb))
#define blit_func_0x29(srca,srcb,srcc) return ~(srca ^ srcb ^ (srcc | (srca & srcb)))
#define blit_func_0x2a(srca,srcb,srcc) return (srcc & ~(srca & srcb))
#define blit_func_0x2b(srca,srcb,srcc) return ~(srca ^ ((srca ^ srcb) & (srcb ^ srcc)))
#define blit_func_0x2c(srca,srcb,srcc) return (srcb ^ (srca & (srcb | srcc)))
#define blit_func_0x2d(srca,srcb,srcc) return (srca ^ (srcb | ~srcc))
#define blit_func_0x2e(srca,srcb,srcc) return (srca ^ (srcb | (srca ^ srcc)))
#define blit_func_0x2f(srca,srcb,srcc) return ~(srca & (srcb | ~srcc))
#define blit_func_0x30(srca,srcb,srcc) return (srca & ~srcb)
#define blit_func_0x31(srca,srcb,srcc) return ~(srcb | (~srca & srcc))
#define blit_func_0x32(srca,srcb,srcc) return (~srcb & (srca | srcc))
#define blit_func_0x33(srca,srcb,srcc) return ~srcb
#define blit_func_0x34(srca,srcb,srcc) return (srcb ^ (srca | (srcb & srcc)))
#define blit_func_0x35(srca,srcb,srcc) return (srcb ^ (srca | ~(srcb ^ srcc)))
#define blit_func_0x36(srca,srcb,srcc) return (srcb ^ (srca | srcc))
#define blit_func_0x37(srca,srcb,srcc) return ~(srcb & (srca | srcc))
#define blit_func_0x38(srca,srcb,srcc) return (srca ^ (srcb & (srca | srcc)))
#define blit_func_0x39(srca,srcb,srcc) return (srcb ^ (srca | ~srcc))
#define blit_func_0x3a(srca,srcb,srcc) return (srcb ^ (srca | (srcb ^ srcc)))
#define blit_func_0x3b(srca,srcb,srcc) return ~(srcb & (srca | ~srcc))
#define blit_func_0x3c(srca,srcb,srcc) return (srca ^ srcb)
#define blit_func_0x3d(srca,srcb,srcc) return (srca ^ (srcb | ~(srca | srcc)))
#define blit_func_0x3e(srca,srcb,srcc) return (srca ^ (srcb | (srca ^ (srca | srcc))))
#define blit_func_0x3f(srca,srcb,srcc) return ~(srca & srcb)
#define blit_func_0x40(srca,srcb,srcc) return (srca & srcb & ~srcc)
#define blit_func_0x41(srca,srcb,srcc) return ~(srcc | (srca ^ srcb))
#define blit_func_0x42(srca,srcb,srcc) return ((srca ^ srcc) & (srcb ^ srcc))
#define blit_func_0x43(srca,srcb,srcc) return (srca ^ (~srcb | (srca & srcc)))
#define blit_func_0x44(srca,srcb,srcc) return (srcb & ~srcc)
#define blit_func_0x45(srca,srcb,srcc) return ~(srcc | (srca & ~srcb))
#define blit_func_0x46(srca,srcb,srcc) return (srcc ^ (srcb | (srca & srcc)))
#define blit_func_0x47(srca,srcb,srcc) return ~(srca ^ (srcb & (srca ^ srcc)))
#define blit_func_0x48(srca,srcb,srcc) return (srcb & (srca ^ srcc))
#define blit_func_0x49(srca,srcb,srcc) return ~(srca ^ srcc ^ (srcb | (srca & srcc)))
#define blit_func_0x4a(srca,srcb,srcc) return (srcc ^ (srca & (srcb | srcc)))
#define blit_func_0x4b(srca,srcb,srcc) return (srca ^ (~srcb | srcc))
#define blit_func_0x4c(srca,srcb,srcc) return (srcb & ~(srca & srcc))
#define blit_func_0x4d(srca,srcb,srcc) return (srca ^ ((srca ^ srcb) | ~(srca ^ srcc)))
#define blit_func_0x4e(srca,srcb,srcc) return (srca ^ (srcc | (srca ^ srcb)))
#define blit_func_0x4f(srca,srcb,srcc) return ~(srca & (~srcb | srcc))
#define blit_func_0x50(srca,srcb,srcc) return (srca & ~srcc)
#define blit_func_0x51(srca,srcb,srcc) return ~(srcc | (~srca & srcb))
#define blit_func_0x52(srca,srcb,srcc) return (srcc ^ (srca | (srcb & srcc)))
#define blit_func_0x53(srca,srcb,srcc) return ~(srcb ^ (srca & (srcb ^ srcc)))
#define blit_func_0x54(srca,srcb,srcc) return (~srcc & (srca | srcb))
#define blit_func_0x55(srca,srcb,srcc) return ~srcc
#define blit_func_0x56(srca,srcb,srcc) return (srcc ^ (srca | srcb))
#define blit_func_0x57(srca,srcb,srcc) return ~(srcc & (srca | srcb))
#define blit_func_0x58(srca,srcb,srcc) return (srca ^ (srcc & (srca | srcb)))
#define blit_func_0x59(srca,srcb,srcc) return (srcc ^ (srca | ~srcb))
#define blit_func_0x5a(srca,srcb,srcc) return (srca ^ srcc)
#define blit_func_0x5b(srca,srcb,srcc) return (srca ^ (srcc | ~(srca | srcb)))
#define blit_func_0x5c(srca,srcb,srcc) return (srcc ^ (srca | (srcb ^ srcc)))
#define blit_func_0x5d(srca,srcb,srcc) return ~(srcc & (srca | ~srcb))
#define blit_func_0x5e(srca,srcb,srcc) return (srca ^ (srcc | (srca ^ (srca | srcb))))
#define blit_func_0x5f(srca,srcb,srcc) return ~(srca & srcc)
#define blit_func_0x60(srca,srcb,srcc) return (srca & (srcb ^ srcc))
#define blit_func_0x61(srca,srcb,srcc) return ~(srcb ^ srcc ^ (srca | (srcb & srcc)))
#define blit_func_0x62(srca,srcb,srcc) return (srcc ^ (srcb & (srca | srcc)))
#define blit_func_0x63(srca,srcb,srcc) return (srcb ^ (~srca | srcc))
#define blit_func_0x64(srca,srcb,srcc) return (srcb ^ (srcc & (srca | srcb)))
#define blit_func_0x65(srca,srcb,srcc) return (srcc ^ (~srca | srcb))
#define blit_func_0x66(srca,srcb,srcc) return (srcb ^ srcc)
#define blit_func_0x67(srca,srcb,srcc) return (srcb ^ (srcc | ~(srca | srcb)))
#define blit_func_0x68(srca,srcb,srcc) return ((srca & srcb) ^ (srcc & (srca | srcb)))
#define blit_func_0x69(srca,srcb,srcc) return ~(srca ^ srcb ^ srcc)
#define blit_func_0x6a(srca,srcb,srcc) return (srcc ^ (srca & srcb))
#define blit_func_0x6b(srca,srcb,srcc) return ~(srca ^ srcb ^ (srcc & (srca | srcb)))
#define blit_func_0x6c(srca,srcb,srcc) return (srcb ^ (srca & srcc))
#define blit_func_0x6d(srca,srcb,srcc) return ~(srca ^ srcc ^ (srcb & (srca | srcc)))
#define blit_func_0x6e(srca,srcb,srcc) return ((~srca & srcb) | (srcb ^ srcc))
#define blit_func_0x6f(srca,srcb,srcc) return (~srca | (srcb ^ srcc))
#define blit_func_0x70(srca,srcb,srcc) return (srca & ~(srcb & srcc))
#define blit_func_0x71(srca,srcb,srcc) return ~(srca ^ ((srca ^ srcb) | (srca ^ srcc)))
#define blit_func_0x72(srca,srcb,srcc) return (srcb ^ (srcc | (srca ^ srcb)))
#define blit_func_0x73(srca,srcb,srcc) return ~(srcb & (~srca | srcc))
#define blit_func_0x74(srca,srcb,srcc) return (srcc ^ (srcb | (srca ^ srcc)))
#define blit_func_0x75(srca,srcb,srcc) return ~(srcc & (~srca | srcb))
#define blit_func_0x76(srca,srcb,srcc) return (srcb ^ (srcc | (srca ^ (srca & srcb))))
#define blit_func_0x77(srca,srcb,srcc) return ~(srcb & srcc)
#define blit_func_0x78(srca,srcb,srcc) return (srca ^ (srcb & srcc))
#define blit_func_0x79(srca,srcb,srcc) return ~(srcb ^ srcc ^ (srca & (srcb | srcc)))
#define blit_func_0x7a(srca,srcb,srcc) return ((srca & ~srcb) | (srca ^ srcc))
#define blit_func_0x7b(srca,srcb,srcc) return (~srcb | (srca ^ srcc))
#define blit_func_0x7c(srca,srcb,srcc) return ((srca ^ srcb) | (srca & ~srcc))
#define blit_func_0x7d(srca,srcb,srcc) return (~srcc | (srca ^ srcb))
#define blit_func_0x7e(srca,srcb,srcc) return ((srca ^ srcb) | (srca ^ srcc))
#define blit_func_0x7f(srca,srcb,srcc) return ~(srca & srcb & srcc)
#define blit_func_0x80(srca,srcb,srcc) return (srca & srcb & srcc)
#define blit_func_0x81(srca,srcb,srcc) return ~((srca ^ srcb) | (srca ^ srcc))
#define blit_func_0x82(srca,srcb,srcc) return (srcc & ~(srca ^ srcb))
#define blit_func_0x83(srca,srcb,srcc) return (srca ^ (~srcb | (srca & ~srcc)))
#define blit_func_0x84(srca,srcb,srcc) return (srcb & ~(srca ^ srcc))
#define blit_func_0x85(srca,srcb,srcc) return (srca ^ (~srcc | (srca & ~srcb)))
#define blit_func_0x86(srca,srcb,srcc) return (srcb ^ srcc ^ (srca & (srcb | srcc)))
#define blit_func_0x87(srca,srcb,srcc) return ~(srca ^ (srcb & srcc))
#define blit_func_0x88(srca,srcb,srcc) return (srcb & srcc)
#define blit_func_0x89(srca,srcb,srcc) return (srcb ^ (~srcc & (~srca | srcb)))
#define blit_func_0x8a(srca,srcb,srcc) return (srcc & (~srca | srcb))
#define blit_func_0x8b(srca,srcb,srcc) return (srca ^ (~srcb | (srca ^ srcc)))
#define blit_func_0x8c(srca,srcb,srcc) return (srcb & (~srca | srcc))
#define blit_func_0x8d(srca,srcb,srcc) return (srca ^ (~srcc | (srca ^ srcb)))
#define blit_func_0x8e(srca,srcb,srcc) return (srca ^ ((srca ^ srcb) | (srca ^ srcc)))
#define blit_func_0x8f(srca,srcb,srcc) return (~srca | (srcb & srcc))
#define blit_func_0x90(srca,srcb,srcc) return (srca & ~(srcb ^ srcc))
#define blit_func_0x91(srca,srcb,srcc) return (srcb ^ (~srcc | (~srca & srcb)))
#define blit_func_0x92(srca,srcb,srcc) return (srca ^ srcc ^ (srcb & (srca | srcc)))
#define blit_func_0x93(srca,srcb,srcc) return ~(srcb ^ (srca & srcc))
#define blit_func_0x94(srca,srcb,srcc) return (srca ^ srcb ^ (srcc & (srca | srcb)))
#define blit_func_0x95(srca,srcb,srcc) return ~(srcc ^ (srca & srcb))
#define blit_func_0x96(srca,srcb,srcc) return (srca ^ srcb ^ srcc)
#define blit_func_0x97(srca,srcb,srcc) return (srca ^ srcb ^ (srcc | ~(srca | srcb)))
#define blit_func_0x98(srca,srcb,srcc) return (srcb ^ (~srcc & (srca | srcb)))
#define blit_func_0x99(srca,srcb,srcc) return ~(srcb ^ srcc)
#define blit_func_0x9a(srca,srcb,srcc) return (srcc ^ (srca & ~srcb))
#define blit_func_0x9b(srca,srcb,srcc) return ~(srcb ^ (srcc & (srca | srcb)))
#define blit_func_0x9c(srca,srcb,srcc) return (srcb ^ (srca & ~srcc))
#define blit_func_0x9d(srca,srcb,srcc) return ~(srcc ^ (srcb & (srca | srcc)))
#define blit_func_0x9e(srca,srcb,srcc) return (srcb ^ srcc ^ (srca | (srcb & srcc)))
#define blit_func_0x9f(srca,srcb,srcc) return ~(srca & (srcb ^ srcc))
#define blit_func_0xa0(srca,srcb,srcc) return (srca & srcc)
#define blit_func_0xa1(srca,srcb,srcc) return (srca ^ (~srcc & (srca | ~srcb)))
#define blit_func_0xa2(srca,srcb,srcc) return (srcc & (srca | ~srcb))
#define blit_func_0xa3(srca,srcb,srcc) return (srcb ^ (~srca | (srcb ^ srcc)))
#define blit_func_0xa4(srca,srcb,srcc) return (srca ^ (~srcc & (srca | srcb)))
#define blit_func_0xa5(srca,srcb,srcc) return ~(srca ^ srcc)
#define blit_func_0xa6(srca,srcb,srcc) return (srcc ^ (~srca & srcb))
#define blit_func_0xa7(srca,srcb,srcc) return ~(srca ^ (srcc & (srca | srcb)))
#define blit_func_0xa8(srca,srcb,srcc) return (srcc & (srca | srcb))
#define blit_func_0xa9(srca,srcb,srcc) return ~(srcc ^ (srca | srcb))
#define blit_func_0xaa(srca,srcb,srcc) return srcc
#define blit_func_0xab(srca,srcb,srcc) return (srcc | ~(srca | srcb))
#define blit_func_0xac(srca,srcb,srcc) return (srcb ^ (srca & (srcb ^ srcc)))
#define blit_func_0xad(srca,srcb,srcc) return ~(srcc ^ (srca | (srcb & srcc)))
#define blit_func_0xae(srca,srcb,srcc) return (srcc | (~srca & srcb))
#define blit_func_0xaf(srca,srcb,srcc) return (~srca | srcc)
#define blit_func_0xb0(srca,srcb,srcc) return (srca & (~srcb | srcc))
#define blit_func_0xb1(srca,srcb,srcc) return ~(srca ^ (srcc | (srca ^ srcb)))
#define blit_func_0xb2(srca,srcb,srcc) return (srca ^ ((srca ^ srcc) & (srcb ^ srcc)))
#define blit_func_0xb3(srca,srcb,srcc) return (~srcb | (srca & srcc))
#define blit_func_0xb4(srca,srcb,srcc) return (srca ^ (srcb & ~srcc))
#define blit_func_0xb5(srca,srcb,srcc) return ~(srcc ^ (srca & (srcb | srcc)))
#define blit_func_0xb6(srca,srcb,srcc) return (srca ^ srcc ^ (srcb | (srca & srcc)))
#define blit_func_0xb7(srca,srcb,srcc) return ~(srcb & (srca ^ srcc))
#define blit_func_0xb8(srca,srcb,srcc) return (srca ^ (srcb & (srca ^ srcc)))
#define blit_func_0xb9(srca,srcb,srcc) return ~(srcc ^ (srcb | (srca & srcc)))
#define blit_func_0xba(srca,srcb,srcc) return (srcc | (srca & ~srcb))
#define blit_func_0xbb(srca,srcb,srcc) return (~srcb | srcc)
#define blit_func_0xbc(srca,srcb,srcc) return ((srca ^ srcb) | (srca & srcc))
#define blit_func_0xbd(srca,srcb,srcc) return ((srca ^ srcb) | ~(srca ^ srcc))
#define blit_func_0xbe(srca,srcb,srcc) return (srcc | (srca ^ srcb))
#define blit_func_0xbf(srca,srcb,srcc) return (srcc | ~(srca & srcb))
#define blit_func_0xc0(srca,srcb,srcc) return (srca & srcb)
#define blit_func_0xc1(srca,srcb,srcc) return (srca ^ (~srcb & (srca | ~srcc)))
#define blit_func_0xc2(srca,srcb,srcc) return (srca ^ (~srcb & (srca | srcc)))
#define blit_func_0xc3(srca,srcb,srcc) return ~(srca ^ srcb)
#define blit_func_0xc4(srca,srcb,srcc) return (srcb & (srca | ~srcc))
#define blit_func_0xc5(srca,srcb,srcc) return ~(srcb ^ (srca | (srcb ^ srcc)))
#define blit_func_0xc6(srca,srcb,srcc) return (srcb ^ (~srca & srcc))
#define blit_func_0xc7(srca,srcb,srcc) return ~(srca ^ (srcb & (srca | srcc)))
#define blit_func_0xc8(srca,srcb,srcc) return (srcb & (srca | srcc))
#define blit_func_0xc9(srca,srcb,srcc) return ~(srcb ^ (srca | srcc))
#define blit_func_0xca(srca,srcb,srcc) return (srcc ^ (srca & (srcb ^ srcc)))
#define blit_func_0xcb(srca,srcb,srcc) return ~(srcb ^ (srca | (srcb & srcc)))
#define blit_func_0xcc(srca,srcb,srcc) return srcb
#define blit_func_0xcd(srca,srcb,srcc) return (srcb | ~(srca | srcc))
#define blit_func_0xce(srca,srcb,srcc) return (srcb | (~srca & srcc))
#define blit_func_0xcf(srca,srcb,srcc) return (~srca | srcb)
#define blit_func_0xd0(srca,srcb,srcc) return (srca & (srcb | ~srcc))
#define blit_func_0xd1(srca,srcb,srcc) return ~(srca ^ (srcb | (srca ^ srcc)))
#define blit_func_0xd2(srca,srcb,srcc) return (srca ^ (~srcb & srcc))
#define blit_func_0xd3(srca,srcb,srcc) return ~(srcb ^ (srca & (srcb | srcc)))
#define blit_func_0xd4(srca,srcb,srcc) return (srca ^ ((srca ^ srcb) & (srcb ^ srcc)))
#define blit_func_0xd5(srca,srcb,srcc) return (~srcc | (srca & srcb))
#define blit_func_0xd6(srca,srcb,srcc) return (srca ^ srcb ^ (srcc | (srca & srcb)))
#define blit_func_0xd7(srca,srcb,srcc) return ~(srcc & (srca ^ srcb))
#define blit_func_0xd8(srca,srcb,srcc) return (srca ^ (srcc & (srca ^ srcb)))
#define blit_func_0xd9(srca,srcb,srcc) return ~(srcb ^ (srcc | (srca & srcb)))
#define blit_func_0xda(srca,srcb,srcc) return ((srca & srcb) | (srca ^ srcc))
#define blit_func_0xdb(srca,srcb,srcc) return ~((srca ^ srcb) & (srcb ^ srcc))
#define blit_func_0xdc(srca,srcb,srcc) return (srcb | (srca & ~srcc))
#define blit_func_0xdd(srca,srcb,srcc) return (srcb | ~srcc)
#define blit_func_0xde(srca,srcb,srcc) return (srcb | (srca ^ srcc))
#define blit_func_0xdf(srca,srcb,srcc) return (srcb | ~(srca & srcc))
#define blit_func_0xe0(srca,srcb,srcc) return (srca & (srcb | srcc))
#define blit_func_0xe1(srca,srcb,srcc) return ~(srca ^ (srcb | srcc))
#define blit_func_0xe2(srca,srcb,srcc) return (srcc ^ (srcb & (srca ^ srcc)))
#define blit_func_0xe3(srca,srcb,srcc) return ~(srca ^ (srcb | (srca & srcc)))
#define blit_func_0xe4(srca,srcb,srcc) return (srcb ^ (srcc & (srca ^ srcb)))
#define blit_func_0xe5(srca,srcb,srcc) return ~(srca ^ (srcc | (srca & srcb)))
#define blit_func_0xe6(srca,srcb,srcc) return ((srca & srcb) | (srcb ^ srcc))
#define blit_func_0xe7(srca,srcb,srcc) return ~((srca ^ srcb) & (srca ^ srcc))
#define blit_func_0xe8(srca,srcb,srcc) return (srca ^ ((srca ^ srcb) & (srca ^ srcc)))
#define blit_func_0xe9(srca,srcb,srcc) return (srca ^ srcb ^ (~srcc | (srca & srcb)))
#define blit_func_0xea(srca,srcb,srcc) return (srcc | (srca & srcb))
#define blit_func_0xeb(srca,srcb,srcc) return (srcc | ~(srca ^ srcb))
#define blit_func_0xec(srca,srcb,srcc) return (srcb | (srca & srcc))
#define blit_func_0xed(srca,srcb,srcc) return (srcb | ~(srca ^ srcc))
#define blit_func_0xee(srca,srcb,srcc) return (srcb | srcc)
#define blit_func_0xef(srca,srcb,srcc) return (~srca | srcb | srcc)
#define blit_func_0xf0(srca,srcb,srcc) return srca
#define blit_func_0xf1(srca,srcb,srcc) return (srca | ~(srcb | srcc))
#define blit_func_0xf2(srca,srcb,srcc) return (srca | (~srcb & srcc))
#define blit_func_0xf3(srca,srcb,srcc) return (srca | ~srcb)
#define blit_func_0xf4(srca,srcb,srcc) return (srca | (srcb & ~srcc))
#define blit_func_0xf5(srca,srcb,srcc) return (srca | ~srcc)
#define blit_func_0xf6(srca,srcb,srcc) return (srca | (srcb ^ srcc))
#define blit_func_0xf7(srca,srcb,srcc) return (srca | ~(srcb & srcc))
#define blit_func_0xf8(srca,srcb,srcc) return (srca | (srcb & srcc))
#define blit_func_0xf9(srca,srcb,srcc) return (srca | ~(srcb ^ srcc))
#define blit_func_0xfa(srca,srcb,srcc) return (srca | srcc)
#define blit_func_0xfb(srca,srcb,srcc) return (srca | ~srcb | srcc)
#define blit_func_0xfc(srca,srcb,srcc) return (srca | srcb)
#define blit_func_0xfd(srca,srcb,srcc) return (srca | srcb | ~srcc)
#define blit_func_0xfe(srca,srcb,srcc) return (srca | srcb | srcc)
#define blit_func_0xff(srca,srcb,srcc) return 0xFFFFFFFF


#ifndef USE_BLIT_FUNC

#define blit_func_func(mt) \
uae_u32 blit_func_tbl_##mt(uae_u32 srca, uae_u32 srcb, uae_u32 srcc) { \
blit_func_##mt(srca,srcb,srcc); \
}

blit_func_func(0x0)
blit_func_func(0x1)
blit_func_func(0x2)
blit_func_func(0x3)
blit_func_func(0x4)
blit_func_func(0x5)
blit_func_func(0x6)
blit_func_func(0x7)
blit_func_func(0x8)
blit_func_func(0x9)
blit_func_func(0xa)
blit_func_func(0xb)
blit_func_func(0xc)
blit_func_func(0xd)
blit_func_func(0xe)
blit_func_func(0xf)
blit_func_func(0x10)
blit_func_func(0x11)
blit_func_func(0x12)
blit_func_func(0x13)
blit_func_func(0x14)
blit_func_func(0x15)
blit_func_func(0x16)
blit_func_func(0x17)
blit_func_func(0x18)
blit_func_func(0x19)
blit_func_func(0x1a)
blit_func_func(0x1b)
blit_func_func(0x1c)
blit_func_func(0x1d)
blit_func_func(0x1e)
blit_func_func(0x1f)
blit_func_func(0x20)
blit_func_func(0x21)
blit_func_func(0x22)
blit_func_func(0x23)
blit_func_func(0x24)
blit_func_func(0x25)
blit_func_func(0x26)
blit_func_func(0x27)
blit_func_func(0x28)
blit_func_func(0x29)
blit_func_func(0x2a)
blit_func_func(0x2b)
blit_func_func(0x2c)
blit_func_func(0x2d)
blit_func_func(0x2e)
blit_func_func(0x2f)
blit_func_func(0x30)
blit_func_func(0x31)
blit_func_func(0x32)
blit_func_func(0x33)
blit_func_func(0x34)
blit_func_func(0x35)
blit_func_func(0x36)
blit_func_func(0x37)
blit_func_func(0x38)
blit_func_func(0x39)
blit_func_func(0x3a)
blit_func_func(0x3b)
blit_func_func(0x3c)
blit_func_func(0x3d)
blit_func_func(0x3e)
blit_func_func(0x3f)
blit_func_func(0x40)
blit_func_func(0x41)
blit_func_func(0x42)
blit_func_func(0x43)
blit_func_func(0x44)
blit_func_func(0x45)
blit_func_func(0x46)
blit_func_func(0x47)
blit_func_func(0x48)
blit_func_func(0x49)
blit_func_func(0x4a)
blit_func_func(0x4b)
blit_func_func(0x4c)
blit_func_func(0x4d)
blit_func_func(0x4e)
blit_func_func(0x4f)
blit_func_func(0x50)
blit_func_func(0x51)
blit_func_func(0x52)
blit_func_func(0x53)
blit_func_func(0x54)
blit_func_func(0x55)
blit_func_func(0x56)
blit_func_func(0x57)
blit_func_func(0x58)
blit_func_func(0x59)
blit_func_func(0x5a)
blit_func_func(0x5b)
blit_func_func(0x5c)
blit_func_func(0x5d)
blit_func_func(0x5e)
blit_func_func(0x5f)
blit_func_func(0x60)
blit_func_func(0x61)
blit_func_func(0x62)
blit_func_func(0x63)
blit_func_func(0x64)
blit_func_func(0x65)
blit_func_func(0x66)
blit_func_func(0x67)
blit_func_func(0x68)
blit_func_func(0x69)
blit_func_func(0x6a)
blit_func_func(0x6b)
blit_func_func(0x6c)
blit_func_func(0x6d)
blit_func_func(0x6e)
blit_func_func(0x6f)
blit_func_func(0x70)
blit_func_func(0x71)
blit_func_func(0x72)
blit_func_func(0x73)
blit_func_func(0x74)
blit_func_func(0x75)
blit_func_func(0x76)
blit_func_func(0x77)
blit_func_func(0x78)
blit_func_func(0x79)
blit_func_func(0x7a)
blit_func_func(0x7b)
blit_func_func(0x7c)
blit_func_func(0x7d)
blit_func_func(0x7e)
blit_func_func(0x7f)
blit_func_func(0x80)
blit_func_func(0x81)
blit_func_func(0x82)
blit_func_func(0x83)
blit_func_func(0x84)
blit_func_func(0x85)
blit_func_func(0x86)
blit_func_func(0x87)
blit_func_func(0x88)
blit_func_func(0x89)
blit_func_func(0x8a)
blit_func_func(0x8b)
blit_func_func(0x8c)
blit_func_func(0x8d)
blit_func_func(0x8e)
blit_func_func(0x8f)
blit_func_func(0x90)
blit_func_func(0x91)
blit_func_func(0x92)
blit_func_func(0x93)
blit_func_func(0x94)
blit_func_func(0x95)
blit_func_func(0x96)
blit_func_func(0x97)
blit_func_func(0x98)
blit_func_func(0x99)
blit_func_func(0x9a)
blit_func_func(0x9b)
blit_func_func(0x9c)
blit_func_func(0x9d)
blit_func_func(0x9e)
blit_func_func(0x9f)
blit_func_func(0xa0)
blit_func_func(0xa1)
blit_func_func(0xa2)
blit_func_func(0xa3)
blit_func_func(0xa4)
blit_func_func(0xa5)
blit_func_func(0xa6)
blit_func_func(0xa7)
blit_func_func(0xa8)
blit_func_func(0xa9)
blit_func_func(0xaa)
blit_func_func(0xab)
blit_func_func(0xac)
blit_func_func(0xad)
blit_func_func(0xae)
blit_func_func(0xaf)
blit_func_func(0xb0)
blit_func_func(0xb1)
blit_func_func(0xb2)
blit_func_func(0xb3)
blit_func_func(0xb4)
blit_func_func(0xb5)
blit_func_func(0xb6)
blit_func_func(0xb7)
blit_func_func(0xb8)
blit_func_func(0xb9)
blit_func_func(0xba)
blit_func_func(0xbb)
blit_func_func(0xbc)
blit_func_func(0xbd)
blit_func_func(0xbe)
blit_func_func(0xbf)
blit_func_func(0xc0)
blit_func_func(0xc1)
blit_func_func(0xc2)
blit_func_func(0xc3)
blit_func_func(0xc4)
blit_func_func(0xc5)
blit_func_func(0xc6)
blit_func_func(0xc7)
blit_func_func(0xc8)
blit_func_func(0xc9)
blit_func_func(0xca)
blit_func_func(0xcb)
blit_func_func(0xcc)
blit_func_func(0xcd)
blit_func_func(0xce)
blit_func_func(0xcf)
blit_func_func(0xd0)
blit_func_func(0xd1)
blit_func_func(0xd2)
blit_func_func(0xd3)
blit_func_func(0xd4)
blit_func_func(0xd5)
blit_func_func(0xd6)
blit_func_func(0xd7)
blit_func_func(0xd8)
blit_func_func(0xd9)
blit_func_func(0xda)
blit_func_func(0xdb)
blit_func_func(0xdc)
blit_func_func(0xdd)
blit_func_func(0xde)
blit_func_func(0xdf)
blit_func_func(0xe0)
blit_func_func(0xe1)
blit_func_func(0xe2)
blit_func_func(0xe3)
blit_func_func(0xe4)
blit_func_func(0xe5)
blit_func_func(0xe6)
blit_func_func(0xe7)
blit_func_func(0xe8)
blit_func_func(0xe9)
blit_func_func(0xea)
blit_func_func(0xeb)
blit_func_func(0xec)
blit_func_func(0xed)
blit_func_func(0xee)
blit_func_func(0xef)
blit_func_func(0xf0)
blit_func_func(0xf1)
blit_func_func(0xf2)
blit_func_func(0xf3)
blit_func_func(0xf4)
blit_func_func(0xf5)
blit_func_func(0xf6)
blit_func_func(0xf7)
blit_func_func(0xf8)
blit_func_func(0xf9)
blit_func_func(0xfa)
blit_func_func(0xfb)
blit_func_func(0xfc)
blit_func_func(0xfd)
blit_func_func(0xfe) 
blit_func_func(0xff)

typedef uae_u32 (*blit_func_tbl_t)(uae_u32, uae_u32, uae_u32);
blit_func_tbl_t blit_func_tbl[0x100]= { 
 blit_func_tbl_0x0, blit_func_tbl_0x1, blit_func_tbl_0x2, blit_func_tbl_0x3,
 blit_func_tbl_0x4, blit_func_tbl_0x5, blit_func_tbl_0x6, blit_func_tbl_0x7,
 blit_func_tbl_0x8, blit_func_tbl_0x9, blit_func_tbl_0xa, blit_func_tbl_0xb,
 blit_func_tbl_0xc, blit_func_tbl_0xd, blit_func_tbl_0xe, blit_func_tbl_0xf,
 blit_func_tbl_0x10, blit_func_tbl_0x11, blit_func_tbl_0x12, blit_func_tbl_0x13,
 blit_func_tbl_0x14, blit_func_tbl_0x15, blit_func_tbl_0x16, blit_func_tbl_0x17,
 blit_func_tbl_0x18, blit_func_tbl_0x19, blit_func_tbl_0x1a, blit_func_tbl_0x1b,
 blit_func_tbl_0x1c, blit_func_tbl_0x1d, blit_func_tbl_0x1e, blit_func_tbl_0x1f,
 blit_func_tbl_0x20, blit_func_tbl_0x21, blit_func_tbl_0x22, blit_func_tbl_0x23,
 blit_func_tbl_0x24, blit_func_tbl_0x25, blit_func_tbl_0x26, blit_func_tbl_0x27,
 blit_func_tbl_0x28, blit_func_tbl_0x29, blit_func_tbl_0x2a, blit_func_tbl_0x2b,
 blit_func_tbl_0x2c, blit_func_tbl_0x2d, blit_func_tbl_0x2e, blit_func_tbl_0x2f,
 blit_func_tbl_0x30, blit_func_tbl_0x31, blit_func_tbl_0x32, blit_func_tbl_0x33,
 blit_func_tbl_0x34, blit_func_tbl_0x35, blit_func_tbl_0x36, blit_func_tbl_0x37,
 blit_func_tbl_0x38, blit_func_tbl_0x39, blit_func_tbl_0x3a, blit_func_tbl_0x3b,
 blit_func_tbl_0x3c, blit_func_tbl_0x3d, blit_func_tbl_0x3e, blit_func_tbl_0x3f,
 blit_func_tbl_0x40, blit_func_tbl_0x41, blit_func_tbl_0x42, blit_func_tbl_0x43,
 blit_func_tbl_0x44, blit_func_tbl_0x45, blit_func_tbl_0x46, blit_func_tbl_0x47,
 blit_func_tbl_0x48, blit_func_tbl_0x49, blit_func_tbl_0x4a, blit_func_tbl_0x4b,
 blit_func_tbl_0x4c, blit_func_tbl_0x4d, blit_func_tbl_0x4e, blit_func_tbl_0x4f,
 blit_func_tbl_0x50, blit_func_tbl_0x51, blit_func_tbl_0x52, blit_func_tbl_0x53,
 blit_func_tbl_0x54, blit_func_tbl_0x55, blit_func_tbl_0x56, blit_func_tbl_0x57,
 blit_func_tbl_0x58, blit_func_tbl_0x59, blit_func_tbl_0x5a, blit_func_tbl_0x5b,
 blit_func_tbl_0x5c, blit_func_tbl_0x5d, blit_func_tbl_0x5e, blit_func_tbl_0x5f,
 blit_func_tbl_0x60, blit_func_tbl_0x61, blit_func_tbl_0x62, blit_func_tbl_0x63,
 blit_func_tbl_0x64, blit_func_tbl_0x65, blit_func_tbl_0x66, blit_func_tbl_0x67,
 blit_func_tbl_0x68, blit_func_tbl_0x69, blit_func_tbl_0x6a, blit_func_tbl_0x6b,
 blit_func_tbl_0x6c, blit_func_tbl_0x6d, blit_func_tbl_0x6e, blit_func_tbl_0x6f,
 blit_func_tbl_0x70, blit_func_tbl_0x71, blit_func_tbl_0x72, blit_func_tbl_0x73,
 blit_func_tbl_0x74, blit_func_tbl_0x75, blit_func_tbl_0x76, blit_func_tbl_0x77,
 blit_func_tbl_0x78, blit_func_tbl_0x79, blit_func_tbl_0x7a, blit_func_tbl_0x7b,
 blit_func_tbl_0x7c, blit_func_tbl_0x7d, blit_func_tbl_0x7e, blit_func_tbl_0x7f,
 blit_func_tbl_0x80, blit_func_tbl_0x81, blit_func_tbl_0x82, blit_func_tbl_0x83,
 blit_func_tbl_0x84, blit_func_tbl_0x85, blit_func_tbl_0x86, blit_func_tbl_0x87,
 blit_func_tbl_0x88, blit_func_tbl_0x89, blit_func_tbl_0x8a, blit_func_tbl_0x8b,
 blit_func_tbl_0x8c, blit_func_tbl_0x8d, blit_func_tbl_0x8e, blit_func_tbl_0x8f,
 blit_func_tbl_0x90, blit_func_tbl_0x91, blit_func_tbl_0x92, blit_func_tbl_0x93,
 blit_func_tbl_0x94, blit_func_tbl_0x95, blit_func_tbl_0x96, blit_func_tbl_0x97,
 blit_func_tbl_0x98, blit_func_tbl_0x99, blit_func_tbl_0x9a, blit_func_tbl_0x9b,
 blit_func_tbl_0x9c, blit_func_tbl_0x9d, blit_func_tbl_0x9e, blit_func_tbl_0x9f,
 blit_func_tbl_0xa0, blit_func_tbl_0xa1, blit_func_tbl_0xa2, blit_func_tbl_0xa3,
 blit_func_tbl_0xa4, blit_func_tbl_0xa5, blit_func_tbl_0xa6, blit_func_tbl_0xa7,
 blit_func_tbl_0xa8, blit_func_tbl_0xa9, blit_func_tbl_0xaa, blit_func_tbl_0xab,
 blit_func_tbl_0xac, blit_func_tbl_0xad, blit_func_tbl_0xae, blit_func_tbl_0xaf,
 blit_func_tbl_0xb0, blit_func_tbl_0xb1, blit_func_tbl_0xb2, blit_func_tbl_0xb3,
 blit_func_tbl_0xb4, blit_func_tbl_0xb5, blit_func_tbl_0xb6, blit_func_tbl_0xb7,
 blit_func_tbl_0xb8, blit_func_tbl_0xb9, blit_func_tbl_0xba, blit_func_tbl_0xbb,
 blit_func_tbl_0xbc, blit_func_tbl_0xbd, blit_func_tbl_0xbe, blit_func_tbl_0xbf,
 blit_func_tbl_0xc0, blit_func_tbl_0xc1, blit_func_tbl_0xc2, blit_func_tbl_0xc3,
 blit_func_tbl_0xc4, blit_func_tbl_0xc5, blit_func_tbl_0xc6, blit_func_tbl_0xc7,
 blit_func_tbl_0xc8, blit_func_tbl_0xc9, blit_func_tbl_0xca, blit_func_tbl_0xcb,
 blit_func_tbl_0xcc, blit_func_tbl_0xcd, blit_func_tbl_0xce, blit_func_tbl_0xcf,
 blit_func_tbl_0xd0, blit_func_tbl_0xd1, blit_func_tbl_0xd2, blit_func_tbl_0xd3,
 blit_func_tbl_0xd4, blit_func_tbl_0xd5, blit_func_tbl_0xd6, blit_func_tbl_0xd7,
 blit_func_tbl_0xd8, blit_func_tbl_0xd9, blit_func_tbl_0xda, blit_func_tbl_0xdb,
 blit_func_tbl_0xdc, blit_func_tbl_0xdd, blit_func_tbl_0xde, blit_func_tbl_0xdf,
 blit_func_tbl_0xe0, blit_func_tbl_0xe1, blit_func_tbl_0xe2, blit_func_tbl_0xe3,
 blit_func_tbl_0xe4, blit_func_tbl_0xe5, blit_func_tbl_0xe6, blit_func_tbl_0xe7,
 blit_func_tbl_0xe8, blit_func_tbl_0xe9, blit_func_tbl_0xea, blit_func_tbl_0xeb,
 blit_func_tbl_0xec, blit_func_tbl_0xed, blit_func_tbl_0xee, blit_func_tbl_0xef,
 blit_func_tbl_0xf0, blit_func_tbl_0xf1, blit_func_tbl_0xf2, blit_func_tbl_0xf3,
 blit_func_tbl_0xf4, blit_func_tbl_0xf5, blit_func_tbl_0xf6, blit_func_tbl_0xf7,
 blit_func_tbl_0xf8, blit_func_tbl_0xf9, blit_func_tbl_0xfa, blit_func_tbl_0xfb,
 blit_func_tbl_0xfc, blit_func_tbl_0xfd, blit_func_tbl_0xfe, blit_func_tbl_0xff
};

#define blit_func(srca,srcb,srcc,mt) (blit_func_tbl[mt])(srca,srcb,srcc)

#else

#define blit_func_case(srca,srcb,srcc,mt) case mt: blit_func_##mt(srca,srcb,srcc)

uae_u32 blit_func(uae_u32 srca, uae_u32 srcb, uae_u32 srcc, uae_u8 mt)
{
switch(mt){
blit_func_case(srca,srcb,srcc,0x0);
blit_func_case(srca,srcb,srcc,0x1);
blit_func_case(srca,srcb,srcc,0x2);
blit_func_case(srca,srcb,srcc,0x3);
blit_func_case(srca,srcb,srcc,0x4); 
blit_func_case(srca,srcb,srcc,0x5);
blit_func_case(srca,srcb,srcc,0x6); 
blit_func_case(srca,srcb,srcc,0x7);
blit_func_case(srca,srcb,srcc,0x8);
blit_func_case(srca,srcb,srcc,0x9);
blit_func_case(srca,srcb,srcc,0xa);
blit_func_case(srca,srcb,srcc,0xb);
blit_func_case(srca,srcb,srcc,0xc);
blit_func_case(srca,srcb,srcc,0xd);
blit_func_case(srca,srcb,srcc,0xe);
blit_func_case(srca,srcb,srcc,0xf);
blit_func_case(srca,srcb,srcc,0x10);
blit_func_case(srca,srcb,srcc,0x11);
blit_func_case(srca,srcb,srcc,0x12);
blit_func_case(srca,srcb,srcc,0x13);
blit_func_case(srca,srcb,srcc,0x14);
blit_func_case(srca,srcb,srcc,0x15);
blit_func_case(srca,srcb,srcc,0x16);
blit_func_case(srca,srcb,srcc,0x17);
blit_func_case(srca,srcb,srcc,0x18);
blit_func_case(srca,srcb,srcc,0x19);
blit_func_case(srca,srcb,srcc,0x1a);
blit_func_case(srca,srcb,srcc,0x1b);
blit_func_case(srca,srcb,srcc,0x1c);
blit_func_case(srca,srcb,srcc,0x1d);
blit_func_case(srca,srcb,srcc,0x1e);
blit_func_case(srca,srcb,srcc,0x1f);
blit_func_case(srca,srcb,srcc,0x20);
blit_func_case(srca,srcb,srcc,0x21);
blit_func_case(srca,srcb,srcc,0x22);
blit_func_case(srca,srcb,srcc,0x23);
blit_func_case(srca,srcb,srcc,0x24);
blit_func_case(srca,srcb,srcc,0x25);
blit_func_case(srca,srcb,srcc,0x26);
blit_func_case(srca,srcb,srcc,0x27);
blit_func_case(srca,srcb,srcc,0x28);
blit_func_case(srca,srcb,srcc,0x29);
blit_func_case(srca,srcb,srcc,0x2a);
blit_func_case(srca,srcb,srcc,0x2b);
blit_func_case(srca,srcb,srcc,0x2c);
blit_func_case(srca,srcb,srcc,0x2d);
blit_func_case(srca,srcb,srcc,0x2e);
blit_func_case(srca,srcb,srcc,0x2f);
blit_func_case(srca,srcb,srcc,0x30);
blit_func_case(srca,srcb,srcc,0x31);
blit_func_case(srca,srcb,srcc,0x32);
blit_func_case(srca,srcb,srcc,0x33);
blit_func_case(srca,srcb,srcc,0x34);
blit_func_case(srca,srcb,srcc,0x35);
blit_func_case(srca,srcb,srcc,0x36);
blit_func_case(srca,srcb,srcc,0x37);
blit_func_case(srca,srcb,srcc,0x38);
blit_func_case(srca,srcb,srcc,0x39);
blit_func_case(srca,srcb,srcc,0x3a);
blit_func_case(srca,srcb,srcc,0x3b);
blit_func_case(srca,srcb,srcc,0x3c);
blit_func_case(srca,srcb,srcc,0x3d);
blit_func_case(srca,srcb,srcc,0x3e);
blit_func_case(srca,srcb,srcc,0x3f);
blit_func_case(srca,srcb,srcc,0x40);
blit_func_case(srca,srcb,srcc,0x41);
blit_func_case(srca,srcb,srcc,0x42);
blit_func_case(srca,srcb,srcc,0x43);
blit_func_case(srca,srcb,srcc,0x44);
blit_func_case(srca,srcb,srcc,0x45);
blit_func_case(srca,srcb,srcc,0x46);
blit_func_case(srca,srcb,srcc,0x47);
blit_func_case(srca,srcb,srcc,0x48);
blit_func_case(srca,srcb,srcc,0x49);
blit_func_case(srca,srcb,srcc,0x4a);
blit_func_case(srca,srcb,srcc,0x4b);
blit_func_case(srca,srcb,srcc,0x4c);
blit_func_case(srca,srcb,srcc,0x4d);
blit_func_case(srca,srcb,srcc,0x4e);
blit_func_case(srca,srcb,srcc,0x4f);
blit_func_case(srca,srcb,srcc,0x50);
blit_func_case(srca,srcb,srcc,0x51);
blit_func_case(srca,srcb,srcc,0x52);
blit_func_case(srca,srcb,srcc,0x53);
blit_func_case(srca,srcb,srcc,0x54);
blit_func_case(srca,srcb,srcc,0x55);
blit_func_case(srca,srcb,srcc,0x56);
blit_func_case(srca,srcb,srcc,0x57);
blit_func_case(srca,srcb,srcc,0x58);
blit_func_case(srca,srcb,srcc,0x59);
blit_func_case(srca,srcb,srcc,0x5a);
blit_func_case(srca,srcb,srcc,0x5b);
blit_func_case(srca,srcb,srcc,0x5c);
blit_func_case(srca,srcb,srcc,0x5d);
blit_func_case(srca,srcb,srcc,0x5e);
blit_func_case(srca,srcb,srcc,0x5f);
blit_func_case(srca,srcb,srcc,0x60);
blit_func_case(srca,srcb,srcc,0x61);
blit_func_case(srca,srcb,srcc,0x62);
blit_func_case(srca,srcb,srcc,0x63);
blit_func_case(srca,srcb,srcc,0x64);
blit_func_case(srca,srcb,srcc,0x65);
blit_func_case(srca,srcb,srcc,0x66);
blit_func_case(srca,srcb,srcc,0x67);
blit_func_case(srca,srcb,srcc,0x68);
blit_func_case(srca,srcb,srcc,0x69);
blit_func_case(srca,srcb,srcc,0x6a);
blit_func_case(srca,srcb,srcc,0x6b);
blit_func_case(srca,srcb,srcc,0x6c);
blit_func_case(srca,srcb,srcc,0x6d);
blit_func_case(srca,srcb,srcc,0x6e);
blit_func_case(srca,srcb,srcc,0x6f);
blit_func_case(srca,srcb,srcc,0x70);
blit_func_case(srca,srcb,srcc,0x71);
blit_func_case(srca,srcb,srcc,0x72);
blit_func_case(srca,srcb,srcc,0x73);
blit_func_case(srca,srcb,srcc,0x74);
blit_func_case(srca,srcb,srcc,0x75);
blit_func_case(srca,srcb,srcc,0x76);
blit_func_case(srca,srcb,srcc,0x77);
blit_func_case(srca,srcb,srcc,0x78);
blit_func_case(srca,srcb,srcc,0x79);
blit_func_case(srca,srcb,srcc,0x7a);
blit_func_case(srca,srcb,srcc,0x7b);
blit_func_case(srca,srcb,srcc,0x7c);
blit_func_case(srca,srcb,srcc,0x7d);
blit_func_case(srca,srcb,srcc,0x7e);
blit_func_case(srca,srcb,srcc,0x7f);
blit_func_case(srca,srcb,srcc,0x80);
blit_func_case(srca,srcb,srcc,0x81);
blit_func_case(srca,srcb,srcc,0x82);
blit_func_case(srca,srcb,srcc,0x83);
blit_func_case(srca,srcb,srcc,0x84);
blit_func_case(srca,srcb,srcc,0x85);
blit_func_case(srca,srcb,srcc,0x86);
blit_func_case(srca,srcb,srcc,0x87);
blit_func_case(srca,srcb,srcc,0x88);
blit_func_case(srca,srcb,srcc,0x89);
blit_func_case(srca,srcb,srcc,0x8a);
blit_func_case(srca,srcb,srcc,0x8b);
blit_func_case(srca,srcb,srcc,0x8c);
blit_func_case(srca,srcb,srcc,0x8d);
blit_func_case(srca,srcb,srcc,0x8e);
blit_func_case(srca,srcb,srcc,0x8f);
blit_func_case(srca,srcb,srcc,0x90);
blit_func_case(srca,srcb,srcc,0x91);
blit_func_case(srca,srcb,srcc,0x92);
blit_func_case(srca,srcb,srcc,0x93);
blit_func_case(srca,srcb,srcc,0x94);
blit_func_case(srca,srcb,srcc,0x95);
blit_func_case(srca,srcb,srcc,0x96);
blit_func_case(srca,srcb,srcc,0x97);
blit_func_case(srca,srcb,srcc,0x98);
blit_func_case(srca,srcb,srcc,0x99);
blit_func_case(srca,srcb,srcc,0x9a);
blit_func_case(srca,srcb,srcc,0x9b);
blit_func_case(srca,srcb,srcc,0x9c);
blit_func_case(srca,srcb,srcc,0x9d);
blit_func_case(srca,srcb,srcc,0x9e);
blit_func_case(srca,srcb,srcc,0x9f);
blit_func_case(srca,srcb,srcc,0xa0);
blit_func_case(srca,srcb,srcc,0xa1);
blit_func_case(srca,srcb,srcc,0xa2);
blit_func_case(srca,srcb,srcc,0xa3);
blit_func_case(srca,srcb,srcc,0xa4);
blit_func_case(srca,srcb,srcc,0xa5);
blit_func_case(srca,srcb,srcc,0xa6);
blit_func_case(srca,srcb,srcc,0xa7);
blit_func_case(srca,srcb,srcc,0xa8);
blit_func_case(srca,srcb,srcc,0xa9);
blit_func_case(srca,srcb,srcc,0xaa);
blit_func_case(srca,srcb,srcc,0xab);
blit_func_case(srca,srcb,srcc,0xac);
blit_func_case(srca,srcb,srcc,0xad);
blit_func_case(srca,srcb,srcc,0xae);
blit_func_case(srca,srcb,srcc,0xaf);
blit_func_case(srca,srcb,srcc,0xb0);
blit_func_case(srca,srcb,srcc,0xb1);
blit_func_case(srca,srcb,srcc,0xb2);
blit_func_case(srca,srcb,srcc,0xb3);
blit_func_case(srca,srcb,srcc,0xb4);
blit_func_case(srca,srcb,srcc,0xb5);
blit_func_case(srca,srcb,srcc,0xb6);
blit_func_case(srca,srcb,srcc,0xb7);
blit_func_case(srca,srcb,srcc,0xb8);
blit_func_case(srca,srcb,srcc,0xb9);
blit_func_case(srca,srcb,srcc,0xba);
blit_func_case(srca,srcb,srcc,0xbb);
blit_func_case(srca,srcb,srcc,0xbc);
blit_func_case(srca,srcb,srcc,0xbd);
blit_func_case(srca,srcb,srcc,0xbe);
blit_func_case(srca,srcb,srcc,0xbf);
blit_func_case(srca,srcb,srcc,0xc0);
blit_func_case(srca,srcb,srcc,0xc1);
blit_func_case(srca,srcb,srcc,0xc2);
blit_func_case(srca,srcb,srcc,0xc3);
blit_func_case(srca,srcb,srcc,0xc4);
blit_func_case(srca,srcb,srcc,0xc5);
blit_func_case(srca,srcb,srcc,0xc6);
blit_func_case(srca,srcb,srcc,0xc7);
blit_func_case(srca,srcb,srcc,0xc8);
blit_func_case(srca,srcb,srcc,0xc9);
blit_func_case(srca,srcb,srcc,0xca);
blit_func_case(srca,srcb,srcc,0xcb);
blit_func_case(srca,srcb,srcc,0xcc);
blit_func_case(srca,srcb,srcc,0xcd);
blit_func_case(srca,srcb,srcc,0xce);
blit_func_case(srca,srcb,srcc,0xcf);
blit_func_case(srca,srcb,srcc,0xd0);
blit_func_case(srca,srcb,srcc,0xd1);
blit_func_case(srca,srcb,srcc,0xd2);
blit_func_case(srca,srcb,srcc,0xd3);
blit_func_case(srca,srcb,srcc,0xd4);
blit_func_case(srca,srcb,srcc,0xd5);
blit_func_case(srca,srcb,srcc,0xd6);
blit_func_case(srca,srcb,srcc,0xd7); 
blit_func_case(srca,srcb,srcc,0xd8);
blit_func_case(srca,srcb,srcc,0xd9);
blit_func_case(srca,srcb,srcc,0xda);
blit_func_case(srca,srcb,srcc,0xdb);
blit_func_case(srca,srcb,srcc,0xdc);
blit_func_case(srca,srcb,srcc,0xdd);
blit_func_case(srca,srcb,srcc,0xde);
blit_func_case(srca,srcb,srcc,0xdf);
blit_func_case(srca,srcb,srcc,0xe0);
blit_func_case(srca,srcb,srcc,0xe1);
blit_func_case(srca,srcb,srcc,0xe2);
blit_func_case(srca,srcb,srcc,0xe3);
blit_func_case(srca,srcb,srcc,0xe4);
blit_func_case(srca,srcb,srcc,0xe5);
blit_func_case(srca,srcb,srcc,0xe6);
blit_func_case(srca,srcb,srcc,0xe7);
blit_func_case(srca,srcb,srcc,0xe8);
blit_func_case(srca,srcb,srcc,0xe9);
blit_func_case(srca,srcb,srcc,0xea);
blit_func_case(srca,srcb,srcc,0xeb);
blit_func_case(srca,srcb,srcc,0xec);
blit_func_case(srca,srcb,srcc,0xed);
blit_func_case(srca,srcb,srcc,0xee);
blit_func_case(srca,srcb,srcc,0xef);
blit_func_case(srca,srcb,srcc,0xf0);
blit_func_case(srca,srcb,srcc,0xf1);
blit_func_case(srca,srcb,srcc,0xf2);
blit_func_case(srca,srcb,srcc,0xf3);
blit_func_case(srca,srcb,srcc,0xf4);
blit_func_case(srca,srcb,srcc,0xf5);
blit_func_case(srca,srcb,srcc,0xf6);
blit_func_case(srca,srcb,srcc,0xf7);
blit_func_case(srca,srcb,srcc,0xf8);
blit_func_case(srca,srcb,srcc,0xf9);
blit_func_case(srca,srcb,srcc,0xfa);
blit_func_case(srca,srcb,srcc,0xfb);
blit_func_case(srca,srcb,srcc,0xfc);
blit_func_case(srca,srcb,srcc,0xfd); 
blit_func_case(srca,srcb,srcc,0xfe);
blit_func_case(srca,srcb,srcc,0xff);
}
return 0;
}

#endif


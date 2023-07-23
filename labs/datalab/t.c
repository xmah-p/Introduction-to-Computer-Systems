#include <stdio.h>



int howManyBits(int x) {
    // 其实就是返回 最高有效位+1 因此负数和正数是一样的
    // 用二分法找
    int t = x ^ (x >> 31);    // 负数取反 正数不变
    int b16, b8, b4, b2, b1;
    b16 = (!(!(t >> 16))) << 4;    // 高16位非空 ? 16 : 0
    t = t >> b16;
    b8 = !(!(t >> 8)) << 3;
    t = t >> b8;
    b4 = !(!(t >> 4)) << 2;
    t = t >> b4;
    b2 = !(!(t >> 2)) << 1;
    t = t >> b2;
    b1 = !(!(t >> 1));
    printf("b1: %d\nb2: %d\nb4: %d\nb8: %d\nb16: %d\n", b1, b2, b4, b8, b16);
    return b1 + b2 + b4 + b8 + b16 + 2;
}

void fshow(unsigned uf) {
    
}


int main() {
    howManyBits(0x80000000);
}
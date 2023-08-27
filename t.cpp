#include <stdio.h>

#include <initializer_list>


enum { IRRMOVQ, RMMOVQ, MRMOVQ, OPQ, PUSHQ, POPQ, CMOVXX };

void scf(const char* str, int* arr) { sscanf(str, "%d %d", arr, arr + 1); }

int main() {
    int arr[2];
    scf("1 2", arr);
    printf("%d %d\n", arr[0], arr[1]);

    std::initializer_list<int> need_regids = {IRRMOVQ, RMMOVQ, MRMOVQ, OPQ,
                                              PUSHQ,   POPQ,   CMOVXX};
}

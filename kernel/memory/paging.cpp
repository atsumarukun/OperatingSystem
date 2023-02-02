/*
@file paging.cpp

ページングの設定ファイル.
*/

#include <array>
#include <stdint.h>

using namespace std;

namespace  {
    const uint64_t PageSize4K = 4096;
    const uint64_t PageSize2M = 512 * PageSize4K;
    const uint64_t PageSize1G = 512 * PageSize2M;

    alignas(PageSize4K) array<uint64_t, 1> pml4_table;
    alignas(PageSize4K) array<uint64_t, 64> pdp_table;
    alignas(PageSize4K) array<array<uint64_t, 512>, 64> page_directory;
}

void MakePageTable() {
    pml4_table[0] = (uint64_t) &pdp_table[0] | 0x003;
    for (int i = 0; i < 64; i++) {
        pdp_table[i] = (uint64_t) &page_directory[i] | 0x003;
        for (int j = 0; j < 512; j++) {
            page_directory[i][j] = i * PageSize1G + j * PageSize2M | 0x083;
        }
    }
    uintptr_t pml4_table_address = (uintptr_t) &pml4_table[0];
    __asm__ volatile("movq %%rsi, %%cr3":: "S"(pml4_table_address): "cr3");
}
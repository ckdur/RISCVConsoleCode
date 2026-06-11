#include <main.h>
#include <kprintf/kprintf.h>
#include <clkutils/clkutils.h>

volatile uint32_t* pusher = (uint32_t*)0x0;
volatile uint32_t* bls12381_ctrl = (uint32_t*)0x0;
volatile uint32_t* bls12381_imem = (uint32_t*)0x0;
volatile uint32_t* bls12381_omem = (uint32_t*)0x0;

uint32_t vec [0x17][12] = {
   {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
   {0x05feee15, 0xc6801965, 0xb4e45849, 0xbcb45328, 0x9b88b47b, 0x0c7aed40, 0x98cf2d5f, 0x094f09db, 0xe1540001, 0x4eac0000, 0x46010000, 0x00005555, },
   {0x13f10530, 0xdb01638f, 0xe47cd408, 0x51dc8c38, 0x8179deba, 0xaeb8f988, 0x5b547b32, 0x82528a06, 0x52980012, 0x4d680003, 0xd40e0000, 0x0004aaa6, },
   {0x0845b458, 0x45dfaaa0, 0xd87b351a, 0x63acc2b9, 0x935716f2, 0x382fdf44, 0xbfad4663, 0x4b161567, 0x432dfbd7, 0xd1a74fcf, 0x7df9b4f4, 0x4b544ca6, },
   {0x0845b458, 0x45dfaaa0, 0xd87b351a, 0x63acc2b9, 0x935716f2, 0x382fdf44, 0xbfad4663, 0x4b161567, 0x432dfbd7, 0xd1a74fcf, 0x7df9b4f4, 0x4b544ca6, },
   {0x045e667f, 0x3d4f69b0, 0xacffe774, 0x141ecf64, 0xa0fa5f49, 0xf4cc2795, 0xbfe321ed, 0xb8f90316, 0x7ca2abf2, 0x2ccbd062, 0x5820f2aa, 0x56632fe2, },
   {0x15a2ab6a, 0xfc307ce9, 0x9e1bc042, 0x2f2cdd72, 0xc37cec3a, 0xfeb8eb29, 0xa74db0b3, 0x3db7f30d, 0xa209540c, 0x84882f9d, 0x61de0d55, 0xa99c7ac9, },
   {0x195d9d54, 0xd4fc7a6e, 0x4454d0c9, 0x773da949, 0x8c2a0585, 0xe4e5726c, 0x045bb959, 0x9dee4725, 0x2034e449, 0x417a34ba, 0x73e0d93c, 0x90ce5e0e, },
   {0x135eaf3f, 0x0e7c6108, 0x8f70787f, 0xba895620, 0xf0a1510a, 0xd86a852b, 0x6b8c8bfa, 0x949f3d49, 0x3ee0e447, 0xf2ce34ba, 0x2ddfd93c, 0x90ce08b9, },
   {0x0ca41ad7, 0x832f1451, 0x857b1c8e, 0x77cb921e, 0x3451763c, 0x2cfc06da, 0x7f906851, 0x040f187d, 0xbfd0a7c9, 0xfe732031, 0xd61aa79e, 0xa1b77c88, },
   {0x0d5cf712, 0xb650d248, 0xc5a08b27, 0xcb801ab9, 0x3025d548, 0xc6890be4, 0xe7a06a4f, 0xf2a1dda6, 0x5edb5834, 0xb2e0dfcd, 0xe3e45861, 0x5e482e23, },
   {0x16503c43, 0xa8c10a29, 0x030775e5, 0xb0d089d1, 0x917fc342, 0x53993e95, 0xd48b8b9a, 0xc182022f, 0x7a9ec9f1, 0x4c2ceddb, 0x03821e4b, 0xdb1dc4eb, },
   {0x06bd530f, 0x94e65a09, 0xdbfd9eae, 0x8cfc03a3, 0xa8afda94, 0xc514073a, 0x9e69b537, 0xf3f11f8d, 0x11d29c0e, 0x20f393f5, 0xb66c2301, 0xa3fb2d6f, },
   {0x06f3f7ab, 0x1a8e0a67, 0x14689ad2, 0x74748fac, 0x24070c6d, 0x2fcb2724, 0x02a60f1e, 0x64c5bf14, 0xc0e7eec2, 0x4f78ba94, 0xaf935d0f, 0x2a63c8a0, },
   {0x0abe60eb, 0x055c370e, 0xfd868643, 0x66920e3b, 0xd9962fdb, 0x28c4e884, 0xba20181b, 0x4629f729, 0xa46227a3, 0xdda4b31c, 0x08c33170, 0x62f46c46, },
   {0x1121ef3c, 0x45f6a69d, 0x2a3c1cba, 0x72ea5844, 0x84695953, 0xe1a52fd5, 0x89988e3f, 0xece45b7e, 0x484181d2, 0x28aa62b7, 0xc3a32e99, 0x00089d83, },
   {0x0da61ced, 0x409c3679, 0x00f8fb8e, 0xb016e3e9, 0xa5140207, 0x40632931, 0x402056cb, 0x59776176, 0x7db259e1, 0xc6b0bb9f, 0x2a5531fd, 0x6dbab352, },
   {0x0cd75f96, 0x1c2b72eb, 0x62acb27c, 0x09d28634, 0xb8481e5f, 0x027d277c, 0xafe79085, 0x83df6f70, 0x532cae34, 0x4af7258b, 0x392c3e43, 0x7ed6072c, },
   {0x13bd39aa, 0x6793ce05, 0x3d477233, 0x509ed8a1, 0x1e199508, 0x7c208b20, 0x5526a862, 0x628971c9, 0x99724985, 0x2c563e51, 0x0f250552, 0x3e61ef2d, },
   {0x17a813e9, 0x098a4769, 0xf49fe1de, 0x8006e78f, 0x94b29ccc, 0x0fa095bb, 0xa02623dd, 0x4ab89532, 0x456aa5e6, 0xe1a388b7, 0xbac39dd8, 0xe8a9e4b6, },
   {0x054a3df3, 0x22d8349e, 0x4ec3f5e1, 0x951e98a3, 0x3c78bfa7, 0x8d56cc97, 0xc9385a2e, 0x7e967cb7, 0xa703280d, 0x4a943306, 0x7a68998b, 0xfe9f6677, },
   {0x0152d2d8, 0x39f27a8a, 0x45aa7128, 0x53dcddcb, 0x7539ca80, 0x5ea54a3b, 0x55713a16, 0x6d4c4a94, 0x445268fc, 0xa2c119a9, 0xf947746c, 0x5c2d70b9, },
   {0x19f36bb6, 0x082c9be4, 0xfec35c68, 0x0fa7c513, 0x06ba4449, 0x67d63da6, 0x7183010e, 0x739cdbfb, 0xe5afbf3a, 0x02e670fa, 0xecfa6e66, 0x6a44fd28, },
};

uint32_t start[12] = {
   0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
};

uint32_t start2[12] = {
   0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3,
};

#define TOTALITEMS (384/32)

#ifdef PSEUDOPUSHER
static volatile uint32_t* pusher = (uint32_t*)0x10050000;

void init_pusher(uint32_t addr) {
    // NOTHING
}

void write_pusher(uint32_t addr, uint32_t *data) {
    //                         WR         | addr
    pusher[(0x200 >> 2) + 0] = 0x00000000 | (addr & 0x00FFFFFF);

    for(int i = 0; i < TOTALITEMS; i++) {
        pusher[(0x200 >> 2) + 1 + i] = data[i];
    }

    pusher[0] = 0x1;
    while(pusher[0]);
}

void read_pusher(uint32_t addr, uint32_t *data) {
    //                         RD         | addr
    pusher[(0x200 >> 2) + 0] = 0x01000000 | (addr & 0x00FFFFFF);

    for(int i = 0; i < TOTALITEMS; i++) {
        pusher[(0x200 >> 2) + 1 + i] = 0;
    }

    pusher[0] = 0x1;
    while(pusher[0]);

    for(int i = 0; i < TOTALITEMS; i++) {
        data[i] = pusher[(0x400 >> 2) + 1 + i];
    }

#define SERIALPUSHER_REG_FCTRL           0x60
#define SERIALPUSHER_REG_FFMT            0x64
}

#else // Full pusher

#include "serial_pusher.h"

void init_pusher(uint32_t addr) {
    // NOTHING
    pusher[(SERIALPUSHER_REG_SCKDIV >> 2)] = 5;
    pusher[(SERIALPUSHER_REG_SCKMODE >> 2)] = SERIALPUSHER_SCK_POL;
    pusher[(SERIALPUSHER_REG_CSMODE >> 2)] = 0; // 0 Auto, 2 Hold, 3 Off
}

void pusher_flush_rxfifo(uint32_t addr) {
    volatile int a;
    // The sending is now automatic, but we need to flush the incomming data
    for(int i = TOTALITEMS; i >= 0; i--) {
        // Wait until non-empty
        while(pusher[(SERIALPUSHER_REG_RXSTAT >> 2)] & (1 << i));
        a = pusher[(SERIALPUSHER_REG_RXFIFO >> 2) + i]; // Ignore the read
    }
}

void pusher_dummy(uint32_t addr) {
    //kputs("DEBUG: Entering pusher_dummy\r\n");
    pusher[(SERIALPUSHER_REG_CSMODE >> 2)] = 3; // Do not csx down

    for(int i = 0; i < (TOTALITEMS+1); i++) {
        pusher[(SERIALPUSHER_REG_TXFIFO >> 2) + TOTALITEMS - i] = 0;
    }

    pusher_flush_rxfifo(addr);
    pusher[(SERIALPUSHER_REG_CSMODE >> 2)] = 0; // Normal csx
    //kputs("DEBUG: Exiting pusher_dummy\r\n");
}

void write_pusher(uint32_t addr, uint32_t *data) {
    pusher_dummy(addr);
    //                                                    WR         | addr
    pusher[(SERIALPUSHER_REG_TXFIFO >> 2) + TOTALITEMS] = 0x00000000 | (addr & 0x00FFFFFF);

    for(int i = 0; i < TOTALITEMS; i++) {
        pusher[(SERIALPUSHER_REG_TXFIFO >> 2) + TOTALITEMS - 1 - i] = data[i];
    }

    pusher_flush_rxfifo(addr);
    pusher_dummy(addr);
}

void read_pusher(uint32_t addr, uint32_t *data) {
    pusher_dummy(addr);
    //                                                    RD         | addr
    pusher[(SERIALPUSHER_REG_TXFIFO >> 2) + TOTALITEMS] = 0x01000000 | (addr & 0x00FFFFFF);

    for(int i = 0; i < TOTALITEMS; i++) {
        pusher[(SERIALPUSHER_REG_TXFIFO >> 2) + TOTALITEMS - 1 - i] = 0;
    }

    // The receiving happens automatic, but need to monitor the fifo
    while(pusher[(SERIALPUSHER_REG_RXSTAT >> 2)] & (1 << TOTALITEMS));
    data[0] = pusher[(SERIALPUSHER_REG_RXFIFO >> 2) + TOTALITEMS]; // Ignore the read
    for(int i = 0; i < TOTALITEMS; i++) {
        while(pusher[(SERIALPUSHER_REG_RXSTAT >> 2)] & (1 << (TOTALITEMS-1-i)));
        data[i] = pusher[(SERIALPUSHER_REG_RXFIFO >> 2) + TOTALITEMS-1-i];
    }
    pusher_dummy(addr);
}
#endif

void bls12_381_test() {
    int count;
    uint32_t buf[12];
    while(bls12381_ctrl && bls12381_imem && bls12381_omem) {
        kputs("BLS 12 384 (on-chip) test!\r\n");
        for(int j = 0; j < 0x17; j++) {
            for(int i = 0; i < TOTALITEMS; i++) {
                bls12381_imem[j*16 + i] = vec[j][i];
            }
        }
        kputs("Doing triggers\r\n");
        bls12381_ctrl[0x000 >> 2] = 0x1;
        count = 0;
        while(1) {
            if(!(bls12381_ctrl[0x004 >> 2] & 1)) break;
            kputs("WAITING\r\n");
            if(count++ > 10) return;
        }

        bls12381_ctrl[0x000 >> 2] = 0x3;
        count = 0;
        while(1) {
            if(!(bls12381_ctrl[0x004 >> 2] & 1)) break;
            kputs("WAITING2\r\n");
            if(count++ > 10) return;
        }

        for(int j = 0; j < 0xC; j++) {
            kprintf("Reading omem(%x)\r\n", j);
            
            for(int i = 0; i < TOTALITEMS; i++) {
                kprintf("%x", bls12381_omem[j*16 + i]);
            }
            kputs("\r\n");
        }

        kputs("Done\r\n");

        if(is_htif) break;

        // Wait for 1000 ms 
        clkutils_delay_ns(1000000000, 1000000000 / timescale_freq);
    }

    while(pusher) {
        kputs("BLS 12 384 (pusher) test!\r\n");
        for(uint32_t j = 0; j < 0x17; j++) {
            kprintf("Writing address: %x\r\n", j+0x100);
            write_pusher(j + 0x100, vec[j]);
        }

        kputs("Doing triggers\r\n");

        // Trigger
        write_pusher(0x0, start);

        // Wait for busy
        count = 0;
        while(1) {
            read_pusher(0x1, buf);

            if(!(buf[11] & 1)) break;
            kputs("WAITING\r\n");
            if(count++ > 10) return;
        }

        // Trigger
        write_pusher(0x0, start2);

        // Wait for busy
        count = 0;
        while(1) {
            read_pusher(0x1, buf);

            if(!(buf[11] & 1)) break;
            kputs("WAITING2\r\n");
            if(count++ > 10) return;
        }

        for(uint32_t j = 0; j < 0xC; j++) {
            kprintf("Reading address: %x\r\n", j+0x200);
            read_pusher(j+0x200, buf);

            for(int i = 0; i < TOTALITEMS; i++) {
                kprintf("%x", buf[i]);
            }
            kputs("\r\n");
        }

        kputs("Done\r\n");

        if(is_htif) break;

        // Wait for 1000 ms 
        clkutils_delay_ns(1000000000, 1000000000 / timescale_freq);
    }
}

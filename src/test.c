#include <main.h>
#include <kprintf/kprintf.h>
#include <clkutils/clkutils.h>
#include "bls12_381_test.h"
#include "dilithium_test.h"

// Run on simulation using: 
// make -C sims/vcs run-binary-debug CONFIG=TomohiroConfig BINARY=~/Documents/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1"
// make -C sims/verilator run-binary-debug CONFIG=TomohiroConfig BINARY=~/Documents/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1" USE_FST=1
// make -C sims/verilator run-binary-debug CONFIG=CryptoMCUConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1" USE_FST=1
// make -C sims/verilator run-binary-debug CONFIG=CryptoMCUConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1 -I$(pwd)/../tsmc7dig/projects/yuto_dilithium_202604/src" USE_FST=1
// make -C sims/verilator run-binary-debug CONFIG=TetheredChipCryptoConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1 -I$(pwd)/../tsmc7dig/projects/yuto_dilithium_202604/src" USE_FST=1

// To implement in FPGA for measurements
// make -C fpga SUB_PROJECT=vcu108 CONFIG=RocketDuranVCU108Config bitstream

void test() {
    dilithium_test();
    bls12_381_test();
}
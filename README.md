# Tomohiro measurement system using RISC-V system

Just as stated, this code will allow to do measurements.
It works with simulations and real FPGA

# Build

To build the program, simply run (You need to have the RISC-V toolchain):

```bash
make bin
```

# Simulate

It depends what you actually want to simulate. Assuming this repository
is installed in `~/Documents/IKEDA_LAB/RISCVConsoleCode`, you can do the
following

## Simulate the comms from FPGA/CHIP (Tomohiro version)

Execute any of the following inside of `plankton-processor-chipyard`:

```bash
make -C sims/vcs run-binary-debug CONFIG=TomohiroConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1"
make -C sims/verilator run-binary-debug CONFIG=TomohiroConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1" USE_FST=1
make -C sims/vcs run-binary-debug CONFIG=TomohiroPreChipConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1"
```

## Simulate a direct integration of crypto-cores into the RISC-V system

Execute any of the following inside of `plankton-processor-chipyard`:

```bash
make -C sims/verilator run-binary-debug CONFIG=CryptoMCUConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1" USE_FST=1
make -C sims/verilator run-binary-debug CONFIG=CryptoMCUConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1 -I$(pwd)/../tsmc7dig/projects/yuto_dilithium_202604/src" USE_FST=1
```

## Simulate Bringup with Chip using the TLSerial (7nm setup)

Execute any of the following inside of `plankton-processor-chipyard`:

```bash
make -C sims/verilator run-binary-debug CONFIG=TetheredChipCryptoConfig BINARY=~/Documents/IKEDA_LAB/RISCVConsoleCode/build/out.elf LOADMEM=1 TIMEOUT_CYCLES=0 EXTRA_SIM_PREPROC_DEFINES="+define+UNIT_DELAY=1 -I$(pwd)/../tsmc7dig/projects/yuto_dilithium_202604/src" USE_FST=1
```

You can also run the simulation inside the `tsmc7dig` repository.

# Implementations

The implementations just allow to compile into hardware to the FPGAs

## Measurements for the 28nm chip (bls12_381 from Tomohiro)

Execute any of the following inside of `plankton-processor-chipyard`:

```bash
make -C fpga SUB_PROJECT=vcu108 CONFIG=RocketDuranVCU108Config bitstream
make -C fpga SUB_PROJECT=vcu108 CONFIG=RocketDuran40MHzVCU108Config bitstream
make -C fpga SUB_PROJECT=vcu108 CONFIG=RocketDuran50MHzVCU108Config bitstream
make -C fpga SUB_PROJECT=vcu108 CONFIG=RocketDuran100MHzVCU108Config bitstream
```

## Measurements for the 7nm chip

```bash
make -C fpga SUB_PROJECT=vcu108 CONFIG=VCU108CryptoBringupConfig bitstream
make -C fpga SUB_PROJECT=tr4 CONFIG=TR4CryptoBringupConfig bitstream
```

You can also run the fpga implementation inside the `tsmc7dig` 
repository.

# Programming FPGAs

To program FPGAs, it depends on the implementation.

## VCU108

The VCU108 uses the RAM-only image. You need to create an SDcard with
an specific partition layout. This is based on the
[vcu118](https://chipyard.readthedocs.io/en/latest/Prototyping/VCU118.html#setting-up-the-sdcard) setup. Execute the following:

NOTE: You can do it manually by doing `sudo gdisk /dev/sdc`.

```bash
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | sudo gdisk /dev/sdc
    o           # Create new GPT partition table
    Y           # Confirm the partition table creation
    x           # Expert mode
    l           # Sector alignment
    1           # Align to 1 sector
    m           # Return to normal mode
    n           # Create partition
    1           # Partition number 1
    34          # Start at this sector
    +1048576    # End at this sector -- 512MB
    af0a        # Apple APFS (To make it compatible with apple)
    n           # new partition
    2           # Partition number 2
                # Start at this sector -- default
                # End at this sector -- default
    af00        # Apple HFS/HFS+ (To make it compatible with apple)
    p           # print the in-memory partition table
    w           # write the partition table
    q           # and we're done
EOF
```

The VCU108 bootrom will find this partition (actually fixed at the 34th
sector) and copy the whole 512MB of it. To etch this into the SDcard 
that you just created, do the following:

```bash
sudo dd if=./build/out.bin of=/dev/sdc1
```

Connect the SDCard using a PMOD adapter. The VCU108 SDCard slot doesn't
work (is connected to another FPGA that is not the main FPGA).
[This chinese clone works OK](https://ja.aliexpress.com/item/1005006265716790.html). The SDCard is mapped to J52.

## TR4

The TR4 works with a QSPI attached into `GPIO-1` with the following
pinout:

```
(FPGA) - (PIN)              -> (Connect to)
DI  - JP10-P21 (GPIO1_18)
DO  - JP10-P22 (GPIO1_19)
CK  - JP10-P23 (GPIO1_20)
CE  - JP10-P24 (GPIO1_21)
VDD - JP10-P29 (+3.3V)
GND - JP10-P30 (GND)
```

You also need a JTAG connected. Use a FT232H. Any chinese one should work.
[Try this one](https://ja.aliexpress.com/item/32817551273.html) for example.

```
(FPGA) - (PIN)              -> (Connect to)
TDI    - JP10-P1 (GPIO1_0)  -> FT232H-AD1
TDO    - JP10-P2 (GPIO1_1)  -> FT232H-AD2
TCK    - JP10-P3 (GPIO1_2)  -> FT232H-AD3
TMS    - JP10-P4 (GPIO1_3)  -> FT232H-AD4
GND    - JP10-P12 (GND)     -> FT232H-GND
JRST   - JP10-P27 (GPIO_24) -> FT232H-AD6
```

Lastly, the UART. Connect it as the following:

```
(FPGA) - (PIN)                 (Connect to)
RXD    - JP10-P25 (GPIO_22) -> UART TX
TXD    - JP10-P26 (GPIO_23) -> UART RX
```

Once the setup is done. Try connecting to the JTAG using OpenOCD.

```bash
openocd -f ftdi.cfg
```

If everything connects correctly, run the following to etch the program:

```bash
riscv64-unknown-elf-gdb build/out.qspi.elf
# (inside gdb)
# target extended-remote :3333
# monitor reset halt
# load
# monitor reset halt
# monitor resume
# monitor shutdown
```

To continuously monitor the UART, do the following:

```bash
# In MAC
ls /dev/tty.usbserial-*
picocom -b 115200 /dev/tty.usbserial-1210
# In Linux
ls /dev/ttyUSB*
microcom -s 115200 -p /dev/ttyUSB0
```

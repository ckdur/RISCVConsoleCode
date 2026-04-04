#include "htif.h"
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <sys/signal.h>

#define SYS_write 64

extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;

static uintptr_t syscall(uintptr_t which, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
  volatile uint64_t magic_mem[8] __attribute__((aligned(64)));
  magic_mem[0] = which;
  magic_mem[1] = arg0;
  magic_mem[2] = arg1;
  magic_mem[3] = arg2;
  __sync_synchronize();

  tohost = (uintptr_t)magic_mem;
  while (fromhost == 0)
    ;
  fromhost = 0;

  __sync_synchronize();
  return magic_mem[0];
}

void __attribute__((noreturn)) htif_exit(uintptr_t code)
{
  tohost = (code << 1) | 1;
  while (1);
}

void exit(int code)
{
  htif_exit(code);
}

void abort()
{
  exit(128 + SIGABRT);
}

void htif_printstr(const char* s)
{
  syscall(SYS_write, 1, (uintptr_t)s, strlen(s));
}

int htif_putchar(int ch)
{
  static char buf[64] __attribute__((aligned(64)));
  static int buflen = 0;

  buf[buflen++] = ch;

  if (ch == '\n' || buflen == sizeof(buf))
  {
    syscall(SYS_write, 1, (uintptr_t)buf, buflen);
    buflen = 0;
  }

  return 0;
}

uintptr_t htif_handle_trap(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  htif_exit(1337);
}


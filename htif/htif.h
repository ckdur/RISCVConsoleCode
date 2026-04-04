#ifndef _HTIF_H
#define _HTIF_H

#include <stdint.h>

void __attribute__((noreturn)) htif_exit(uintptr_t code);
void htif_printstr(const char* s);
int htif_putchar(int ch);

#endif // _HTIF_H
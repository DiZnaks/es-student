#pragma once

#define VECTOR_TABLE 0x10000100
#define GPIO_IN_ADDR 0xd0000004

static void row(const char *name, uintptr_t start, uintptr_t end);

void mem_info(void);
void fw_info(void);
void boot_info(void);
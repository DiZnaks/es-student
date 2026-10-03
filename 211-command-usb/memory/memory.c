#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include "memory.h"
#include "device.h"
#include "command.h"

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

uint32_t data_variable = 100;
uint32_t bss_variable;

int main(void);

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

void fw_info(void){
    data_variable++;
    bss_variable++;

    uint32_t stack_variable = 1946;
    uint32_t *heap_variable = malloc(sizeof(uint32_t));
    
    if (heap_variable != NULL)
    {
           *heap_variable = 1951;
       }

    printf("\n%-10s %-10s %-10s\n", "object", "address", "value");

    uint16_t *main_code = (uint16_t *)((uintptr_t)main & ~1u);
    uint16_t *fw_info_code = (uint16_t *)((uintptr_t)fw_info & ~1u);

    printf("%-10s 0x%08X 0x%04X\n", "main", (unsigned)main, *main_code);
    printf("%-10s 0x%08X 0x%04X\n", "fw_info", (unsigned)fw_info, *fw_info_code);
    printf("%-10s 0x%08X\n", "commands", (unsigned)commands);

    for (uint i = 0; i < command_count; i++) {
        printf("  %-8s 0x%08X\n", 
               commands[i].name, 
               (unsigned)commands[i].handler);
    }

    printf("%-10s 0x%08X %s\n", "DEVICE_PROJECT", (unsigned)DEVICE_PROJECT, DEVICE_PROJECT);
    printf("%-10s 0x%08X %s\n", "DEVICE_BOARD", (unsigned)DEVICE_BOARD, DEVICE_BOARD);
    printf("%-10s 0x%08X %u\n", "data_variable", (unsigned)&data_variable, data_variable);
    printf("%-10s 0x%08X %u\n", "bss_variable", (unsigned)&bss_variable, bss_variable);
    printf("%-10s 0x%08X %u\n", "stack_variable", (unsigned)&stack_variable, stack_variable);
    printf("%-10s 0x%08X %u\n", "heap_variable", (unsigned)heap_variable, *heap_variable);

    free(heap_variable);
}

void mem_info(void)
{
    printf("%-10s %10s %10s %8s\n", "area", "start", "end", "size");

    row("flash", XIP_BASE, XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("sram", SRAM_BASE, SRAM_BASE + 264 * 1024);
    row("rom", ROM_BASE, ROM_BASE + 16 * 1024);

    row("image", (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
    row("free", (uintptr_t)&__flash_binary_end, XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
    row("text", (uintptr_t)&__boot2_end__, (uintptr_t)&__etext);

    row("data flash", (uintptr_t)&__etext, (uintptr_t)&__etext + ((uintptr_t)&__data_end__ - (uintptr_t)&__data_start__));
    row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);
    row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);
    row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);
    row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

    uint32_t boot2_size = (uintptr_t)&__boot2_end__ - (uintptr_t)&__boot2_start__;
    uint32_t text_size = (uintptr_t)&__etext - (uintptr_t)&__boot2_end__;
    uint32_t data_flash_size = (uintptr_t)&__data_end__ - (uintptr_t)&__data_start__;
    uint32_t bss_size = (uintptr_t)&__bss_end__ - (uintptr_t)&__bss_start__;
    uint32_t heap_size = (uintptr_t)&__HeapLimit - (uintptr_t)&__bss_end__;
    uint32_t stack_size = (uintptr_t)&__StackTop - (uintptr_t)&__StackBottom;

    uint32_t flash_image_total = boot2_size + text_size + data_flash_size;
    uint32_t flash_free_total = PICO_FLASH_SIZE_BYTES - flash_image_total;
    uint32_t ram_used_total = data_flash_size + bss_size;

    printf("\ntotal\n");
    printf("  flash image  %6u = boot2 %u + text %u + data %u\n",
           flash_image_total, boot2_size, text_size, data_flash_size);
    printf("  flash free   %6u of %u\n",
           flash_free_total, PICO_FLASH_SIZE_BYTES);
    printf("  ram used     %6u = data %u + bss %u\n",
           ram_used_total, data_flash_size, bss_size);
    printf("  ram free     %6u for heap and %u for stack\n",
           heap_size, stack_size);
}
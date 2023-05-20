#include <stdio.h>
#include <stdlib.h>
#include "XtensaLX.h"

#define ROM_SIZE 0x180

uint32_t readMemory(Xtensa_lx_CPU *CPU, uint32_t address, void *context)
{
   uint32_t value = 0;
   uint8_t *rom = (uint8_t *)context;
   if (address >= 0x180)
   {
      printf("\nTest Code:attempt to read from an unimplemented area %8X returning a 0.\n", address);
   }
   else
   {
      value = 0x00;
      for (int i = 0; i < 4; i++)
      {
         value = value << 8;
         value = value | rom[address + i];
      }
   }
   return value;
}

void writeMemory(Xtensa_lx_CPU *CPU, uint32_t address, uint32_t value, void *context)
{
   printf("\nTest Code:attempt to write to an unimplemented area in memory %8X with the value %8X value will not be written.\n", address, value);
}
void processInstruction(Xtensa_lx_CPU *CPU, uint8_t rom[ROM_SIZE])
{
   // process read and prompt CPU to execute next instruction
   // need to put together 32 bit instruction first and put things back on to the data line
   CPU->dataBus = 0; // clear the dataBus
   for (int i = 0; i < 4; i++)
   {
      CPU->dataBus = CPU->dataBus << 8;
      CPU->dataBus = CPU->dataBus | rom[CPU->addressLines + i];
   }
   // now that the databus is set the cpu is ready to execute the instruction
   xten_executeNext(CPU);
   xten_displayCPU(CPU);
}

int main(int argc, char *argv[])
{

   // set any chip development parameters

   // create a memory alternative
   uint8_t rom[ROM_SIZE] = {0};

   // create a new xtensa CPU
   Xtensa_lx_CPU *CPU = xten_createCPU(readMemory, writeMemory, (void *)rom);

   // attach any gpio style stuff

   // load program into memory at 0x00
   FILE *romFile = fopen("core_load_instruction_test.m", "rb");
   if (romFile == NULL)
   {
      printf("Issues with opening the rom file!");
   }
   else
   {
      fread(rom, 1, 0x180, romFile);
      fclose(romFile);

      // test prints
      xten_displayCPU(CPU);
      // execute loop
      for (int i = 0; i < 5; i++)
      {
         processInstruction(CPU, rom);
      }
   }
   return 0;
}
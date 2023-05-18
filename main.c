#include <stdio.h>
#include <stdlib.h>
#include "XtensaLX.h"

#define ROM_SIZE 0x180

void processInstruction(Xtensa_lx_CPU *CPU, uint8_t rom[ROM_SIZE])
{
   // First looks at write bit to determine if the CPU is intending to write
   if (CPU->write == XTEN_HIGH)
   {
      // process write
   }
   else
   {
      // process read and prompt CPU to execute next instruction
      // need to put together 32 bit instruction first and put things back on to the data line
      CPU->dataBus = 0; // clear the dataBus
      for (int i = 0; i < 4; i++)
      {
         CPU->dataBus = CPU->dataBus << 8;
         CPU->dataBus = CPU->dataBus | rom[CPU->addressLines];
      }
      // now that the databus is set the cpu is ready to execute the instruction
   }
}

int main(int argc, char *argv[])
{
   // create a new xtensa CPU
   Xtensa_lx_CPU CPU = xten_createCPU();

   // set any chip development parameters

   // create a memory alternative
   uint8_t rom[ROM_SIZE] = {0};

   // attach any gpio style stuff

   // load program into memory at 0x00
   FILE *romFile = fopen("machine_code.m", "rb");
   if (romFile == NULL)
   {
      printf("Issues with opening the rom file!");
   }
   else
   {
      fread(rom, 1, 0x180, romFile);
      fclose(romFile);

      // execute loop

      // test prints
      xten_displayCPU(&CPU);
   }
   return 0;
}
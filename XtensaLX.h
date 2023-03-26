#ifndef XTENSA_LX_H
#define XTENSA_LX_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>

/*CPU defines no magic numbers floating about*/
#define DEFAULT_REGISTER_FILE_SIZE 32
#define BOOLEAN_REGISTER_AMOUNT 16
#define FLOATING_POINT_REGISTER_AMOUNT 16
#define MAC16_REGISTER_AMOUNT 4

typedef struct Xtensa_lx_CPU{
    //variables for parts of the cpu will go here
    /*Starting with registers*/
    u_int32_t *registerFile;
    u_int32_t PC;
    u_int32_t *optionalFloatingPointRegisters;
    u_int32_t *optionalMAC16Registers;
    bool *bRegisters;
    //may want to add the optional windowless register files but need more details
    int windowOffset;
};


//This is where the functions will go static inline def with xten_ prefix
static inline Xtensa_lx_CPU xten_createCPU(int registerFileSize = DEFAULT_REGISTER_FILE_SIZE, bool floatingPointRegisters, bool mac16Registers){
    Xtensa_lx_CPU *resultingCPU;
    resultingCPU->registerFile = (u_int32_t*)malloc(registerFileSize*sizeof(u_int32_t));
    resultingCPU->bRegisters = (bool*)malloc(BOOLEAN_REGISTER_AMOUNT*sizeof(bool));
    if(floatingPointRegisters)
        resultingCPU->optionalFloatingPointRegisters = (u_int32_t*)malloc(FLOATING_POINT_REGISTER_AMOUNT*sizeof(u_int32_t));
    if(mac16Registers)
        resultingCPU->optionalMAC16Registers = (u_int32_t*)malloc(MAC16_REGISTER_AMOUNT*sizeof(u_int32_t));
    resultingCPU->windowOffset = 0;
}   

static inline void xten_freeCPU(Xtensa_lx_CPU* CPU){
    free(CPU->registerFile);
    free(CPU->bRegisters);
    if(CPU->optionalFloatingPointRegisters){
        free(CPU->optionalFloatingPointRegisters);
    }
    if(CPU->optionalMAC16Registers){
        free(CPU->optionalMAC16Registers);
    }
}





#ifdef __cplusplus
}
#endif

#endif
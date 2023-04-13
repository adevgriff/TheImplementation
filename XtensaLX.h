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
inline Xtensa_lx_CPU xten_createCPU(int registerFileSize = DEFAULT_REGISTER_FILE_SIZE, bool floatingPointRegisters, bool mac16Registers){
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

static inline void xten_coreLoadInstructions(u_int32_t opcode){
    //core load instructions are
    //L8UI      load zero-extended byte(8 bit unsigned load(8 bit offset))                             RRI8 
    //L16SI     load signed extended 16 bit quantity(16 bit signed load(8 bit shifted offset))         RRI8 
    //L16UI     load zero extended 16 bit quantity(16 bit unsigned load(8 bit shifted offset))         RRI8     
    //L32I      load 32 bit quantity(32 bit load(8 bit shifted offset))                                RRI8 
    //L32R      load literal at offset from PC(32 bit load pc relative(16 bit negative word offset))   RI16
}

static inline void xten_coreStoreInstructions(u_int32_t opcode){
    //core store instructions are
    //s8I       store byte                  RRI8
    //s16I      store 16 bit quantity       RRI8            
    //s32I      store 32 bit quantity       RRI8            
}

static inline void xten_coreMemoryOrderingInstructions(u_int32_t opcode){
    //core memory ordering instructions are
    //MEMW      wait for any possible memory ordering requirement       RRR
    //EXTW      wait for any possible external ordering requiremetn     RRR
}

static inline void xten_coreJumpCallInstructions(u_int32_t opcode){
    //core jump, call instructions are
    //CALL0     Call subroutine at PC plus offset place return address in A0                CALL
    //CALLX0    Call subroutine register specified location, place return address in A0     CALLX
    //RET       subroutine return through A0                                                CALLX         
    //J         jump to pc plus offset                                                      CALL 
    //JX        jump to register-specified location                                         CALLX             
}

static inline void xten_coreConditionalBranchInstructions(u_int32_t opcode){
    //core conditional branch instructions are
    //BALL      branch if all bits specified by a mask in one register are set in another register      RRI8
    //BANY      branch if any bits specified by a mask in one register are set in another register      RRI8
    //BBC       branch if the bit specified by antoher register is clear                                RRI8
    //BBS       branch if the bit specified by another register is set                                  RRI8
    //BBCI      branch if the bit specified by an immediate is clear                                    RRI8
    //BBSI      branch if the bit specified by an immediate is set                                      RRI8
    //BEQ       branch if a register equals another register                                            RRI8
    //BEQI      branch if a register equals an encoded constant                                         RRI8
    //BEQZ      branch if a register equals zero                                                        BRI12
    //BGE       branch if a register is greater than or equal to a register                             RRI8
    //BGEI      branch if a register is greater than or equal to an encoded constant                    RRI8
    //BGEU      branch if one register is greater than or equal to a register as unsigned               RRI8
    //BGEUI     branch if one register is greater than or equal to an encoded constatnt as unsigned     RRI8
    //BGEZ      branch if a register is greater than or equal to zero                                   BRI12
    //BLT       branch if one register is less than a register                                          RRI8 
    //BLTI      branch if one register is less than an encoded constant                                 RRI8         
    //BLTU      branch if one register is less than a register as unsigned                              RRI8             
    //BLTUI     branch if one register is less than an encoded constant as unsigned                     RRI8                     
    //BLTZ      branch if a register is less than zero                                                  BRI12
    //BNALL     branch if some bits specified by a mask in a register are clear in another register     RRI8
    //BNONE     branch if all bits spcified by a mask in a register are clear in another register       RRI8
    //BNE       branch if a register does not equal a register                                          RRI8
    //BNEI      branch if a register does not equal an encoded constant                                 RRI8
    //BNEZ      branch if a register does not equal zero                                                BRI12
}

static inline void xten_coreMoveInstructions(u_int32_t opcode){
    //core Move instructions are
    //MOVI          move a 12 bit immediate to a register                                           RRI8
    //MOVEQZ        move register if the contents of a register are equal to zero                   RRR
    //MOVEGEZ       move register if teh contents of a register are greater than or equal to zero   RRR
    //MOVLTZ        move register if the contents of a register are less than zero                  RRR
    //MOVNEZ        move register if the contents of a register are not zero                        RRR
}

static inline void xten_coreArithmeticInstructions(u_int32_t opcode){
    //core arithmetic instructions are
    /*********************************************************************/
    //ADDI      add a register and an 8-bit immediate                                       RRI8
    //ADDMI     add a register and a shifted 8 bit immediate                                RRI8
    //ADD       add two registers                                                           RRR
    //ADDX2     add two registers with one shifted left by one                              RRR
    //ADDX4     add two registers with one shifted left by two                              RRR
    //ADDX8     add two registers with one shifted left by three                            RRR
    //SUB       subtract two registers                                                      RRR        
    //SUBX2     subtract two registers with the un-negated one shifted left by one          RRR        
    //SUBX4     subtract two registers with the un-negated one shifted left by two          RRR        
    //SUBX8     subtract two registers with the un-negated one shifted left by three        RRR
    //NEG       negate a register                                                           RRR
    //ABS       absolute value                                                              RRR
}

static inline void xten_coreBitwiseLogicalInstructions(u_int32_t opcode){
    //core bitwise logical instructions are
    //AND       bitwise AND of two registers    RRR
    //OR        bitwise OR two registers        RRR
    //XOR       bitwise XOR two registers       RRR
}

static inline void xten_coreShiftInstructions(u_int32_t opcode){
    //core shift instructions are(issues in the ISA specifing right and left specific assumed them from the names)
    //EXTUI     extract field specified by immediates from a register                               RRR
    //SRLI      shift right logical by immediate                                                    RRR 
    //SRAI      shift right logical by immediate                                                    RRR 
    //SLLI      shift left logical by immediate                                                     RRR
    //SRC       shift right combined by SAR with two registers as input and one as output           RRR
    //SLL       shift left logical by SAR                                                           RRR
    //SRL       shift right logical by SAR                                                          RRR 
    //SRA       shift right logical by SAR                                                          RRR 
    //SSL       set SAR from register for left shift                                                RRR
    //SSR       set SAR from register for right shift                                               RRR 
    //SSAI      set SAR to immediate value                                                          RRR
    //SSA8B     use low 2 bits of address register to prepare SAR for SRC assuming big endian       RRR
    //SSA8L     use low 2 bits of address register to prepare SAR for SRC assuming little endian    RRR   
}

static inline void xten_coreProcessorControlInstructions(u_int32_t opcode){
    //core processor control instructions are
    //RSR       read a special register                                 RSR
    //WSR       write a special register                                RSR
    //XSR       read and write a special register in an exchange        RRR
    //RUR       read user special register                              ?
    //WUR       write user special register                             ? 
    //ISYNC     wait for instruction-Fetch-related changes to reslove   RRR
    //RSYNC     wait for dispatch related changes to resolve            RRR
    //ESYNC     wait for execution related changes to resolve           RRR
    //DSYNC     wait for data memory related changes to resolve         RRR
}

#ifdef __cplusplus
}
#endif

#endif
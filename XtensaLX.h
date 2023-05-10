#ifndef XTENSA_LX_H
#define XTENSA_LX_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>

/*CPU defines no magic numbers floating about*/
#define DEFAULT_REGISTER_FILE_SIZE 32
#define BOOLEAN_REGISTER_AMOUNT 16
#define FLOATING_POINT_REGISTER_AMOUNT 16
#define MAC16_REGISTER_AMOUNT 4

    typedef struct Xtensa_lx_CPU
    {
        // variables for parts of the cpu will go here
        /*Starting with registers*/
        uint32_t *registerFile;
        uint32_t PC;
        uint32_t *optionalFloatingPointRegisters;
        uint32_t *optionalMAC16Registers;
        bool *bRegisters;
        // may want to add the optional windowless register files but need more details
        int windowOffset;
    };

    // This is where the functions will go static inline def with xten_ prefix
    inline Xtensa_lx_CPU xten_createCPU(int registerFileSize = DEFAULT_REGISTER_FILE_SIZE, bool floatingPointRegisters, bool mac16Registers)
    {
        Xtensa_lx_CPU *resultingCPU;
        resultingCPU->registerFile = (uint32_t *)malloc(registerFileSize * sizeof(uint32_t));
        resultingCPU->bRegisters = (bool *)malloc(BOOLEAN_REGISTER_AMOUNT * sizeof(bool));
        if (floatingPointRegisters)
            resultingCPU->optionalFloatingPointRegisters = (uint32_t *)malloc(FLOATING_POINT_REGISTER_AMOUNT * sizeof(uint32_t));
        if (mac16Registers)
            resultingCPU->optionalMAC16Registers = (uint32_t *)malloc(MAC16_REGISTER_AMOUNT * sizeof(uint32_t));
        resultingCPU->windowOffset = 0;
    }

    static inline void xten_freeCPU(Xtensa_lx_CPU *CPU)
    {
        free(CPU->registerFile);
        free(CPU->bRegisters);
        if (CPU->optionalFloatingPointRegisters)
        {
            free(CPU->optionalFloatingPointRegisters);
        }
        if (CPU->optionalMAC16Registers)
        {
            free(CPU->optionalMAC16Registers);
        }
    }

    static inline void xten_coreLoadInstructions(uint32_t opcode)
    {
        // Two opcodes being used here RRI8 and RI16 here they are in big endian both of these are 24 bit instructions
        // RRI8 is [4 bit major opcode][4 bit t AR target or source,BR target or Source, 4bit sub opcode][s 4 bit, AR source, BR source, AR target][r AR target, BR target, 4 bit immediate, 4-bit sub-opcode][imm8 8 bit immediate]
        // RI16 is [4 bit major opcode][t 4 bit][imm16 16 bit immediate]

        // core load instructions are
        // L8UI      load zero-extended byte(8 bit unsigned load(8 bit offset))                             RRI8
        // major opcode 0010     4-bit sub opcode in r 0000
        // adds contents of register represented by s to 8 bit immediate
        // data at memory address represented by this addition is then stored
        // in register represented by t zero extended

        // L16SI     load signed extended 16 bit quantity(16 bit signed load(8 bit shifted offset))         RRI8
        // major opcode 0010     4-bit sub opcode stored in r 1001
        // 8 bit constant added to register represented by s shifted left by 1
        // 16 bits two bytes read from register at the memory location this makes the value
        // sign extended and then stored in register represented by t
        // without unaligned exception option least significant address bit is ignored
        // essentially subtract one from odd addresses before accessing

        // L16UI     load zero extended 16 bit quantity(16 bit unsigned load(8 bit shifted offset))         RRI8
        // major opcode 0010     4-bit sub opcode stored in r 0001
        // adds as and 8 bit immediate shifted left by 1
        // reads in data like in L16SI except the data is zero extended instead of sign extened

        // L32I      load 32 bit quantity(32 bit load(8 bit shifted offset))                                RRI8
        // major opcode 0010     4-bit sub opcode stored in r 0010
        // adds register as and 8 bit zero extended constant shifted left by 2
        // reads address this represents 4 bytes of it written to register at
        // this instruction ignores least significant two bits of the address calculated
        // without unaligned exception option

        // L32R      load literal at offset from PC(32 bit load pc relative(16 bit negative word offset))   RI16
        // major opcode 0001     no sub opcode for this one
        // PC relative 32 bit load
        // address formed by adding 16 bit one extended constant value shifted left by two
        // to teh address of teh L32R plus three with the two least significant bits cleared
        // specifies 32-bit aligned addresses from -262141 to -4 bytes from the address of teh L32R instruction
        // 32 bits are read from the address and written to at
        // one of few memory instrucitons that can access instruction RAM/ROM
    }

    static inline void xten_coreStoreInstructions(uint32_t opcode)
    {
        // RRI8 is [4 bit major opcode][4 bit t AR target or source,BR target or Source, 4bit sub opcode][s 4 bit, AR source, BR source, AR target][r AR target, BR target, 4 bit immediate, 4-bit sub-opcode][imm8 8 bit immediate]

        // core store instructions are
        // s8I       store byte                  RRI8
        // major opcode 0010     sub opcode at r 0100
        // adds as and zero exteneded immediate 1 byte is written from
        // the least significant 8 bits of register at to memory at the address
        // caclulated

        // s16I      store 16 bit quantity       RRI8
        // major opcode 0010     sub opcode at r 0101
        // forms address adding as and 8 bit immediate zero exteneded shifted left by one
        // 16 least significant bits from at are written to the address formed
        // least significant bit is ignored in the calculated address without the unaligned exception option

        // s32I      store 32 bit quantity       RRI8
        // major opcode 0010     sub opcode at r 0110
        // forms address by adding as with 8 bit zero exteneded constant shifted left by two
        // writes at to the memory address formed
        // least significant 2 bits of the address are ignored without unaligned exception option
        // can access instruction RAM
    }

    static inline void xten_coreMemoryOrderingInstructions(uint32_t opcode)
    {
        // RRR is [op0][t][s][r][op1][op2]

        // core memory ordering instructions are
        // MEMW      wait for any possible memory ordering requirement       RRR
        // major opcode 0000     subopcodes specified by op1 0000 op2 0000 rst in that order 0010 0000 1100
        // in this implementation is a no-op used to seperate load and store calls needing more time

        // EXTW      wait for any possible external ordering requiremetn     RRR
        // no paramiters whole thing is opcode 0000 0000 0010 0000 1101 0000
        // ensures changes from all previous instructions before will perform
        // any load, store, acquire, release, prefetch, or cash instructions
        // and everything that will has affected output pins
    }

    static inline void xten_coreJumpCallInstructions(uint32_t opcode)
    {
        // core jump, call instructions are
        // CALL0     Call subroutine at PC plus offset place return address in A0                CALL
        // subroutines without using register windows. The return address is palced in
        // a0 and the processor then branches to the target address
        // the return address is the address of the CALL0 instruction plus three
        // target instruction address must be 32 bit aligned allowing CALL0 to have a larger effective range
        // least sig two bits set to zero plus the sign-extened 18-bit offset shifted by two, plus four
        //
        // CALLX0    Call subroutine register specified location, place return address in A0     CALLX
        // calls subroutines without using register windows the return address is placed in a0 and teh processor
        // then branches to the target address,
        //
        // RET       subroutine return through A0    CALLX
        // 0000 0000 0000 0000 10 00 0000
        // This returns from routine called by either CALL0 or CALLX0 equivalent to the instruction JX A0
        // serparate instruction because some implementations may realize performace advantages from it being seperate
        //
        // J         jump to pc plus offset
        // Unconditional Jump 18 bit offset followed by opcode 00 0110
        // Performs an unconditional branch to the target address signed 18-bit PC-relative offset is used to specify the target address
        // address of the J instruction plus the sign-extended 18-bit offset range is -131068 to 131075
        // nextPC = PC + (offset 17 14 || offset) + 4
        //                                                  CALL
        // JX        jump to register-specified location  CALLX
        // unconditional jump based on register specified by as
        // perfoms an unconditional jump to the address in register as
        //
    }

    static inline void xten_coreConditionalBranchInstructions(uint32_t opcode)
    {
        // RRI8 is [4 bit major opcode][4 bit t AR target or source,BR target or Source, 4bit sub opcode][s 4 bit, AR source, BR source, AR target][r AR target, BR target, 4 bit immediate, 4-bit sub-opcode][imm8 8 bit immediate]

        // core conditional branch instructions are
        // BALL      branch if all bits specified by a mask in one register are set in another register      RRI8
        // branches if all bits specified by the mask in address register at are set in address register as
        // test performed by taking bitwise logical and of at and the complement of as and seeing if result is zero
        // target address is address of the BALL instruction plus the sign-extended 8-bit imm8 plus four if any
        // masked bits are clear execution continus with next sequential instruction
        // if((not AR[s]) and AR[t]) = 0^32 then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BANY      branch if any bits specified by a mask in one register are set in another register      RRI8
        // branches if any of the bits specified by mask in address register at are set in address register as
        // tested by bitwise logical and of as and at and testing if result is non-zero
        // target address given by address of BANY plus the sign-extended 8-bit imm8 field of the instruction plus
        // four if all masked bits are clear execution continues with the next sequential instruction
        // if(AR[s] and AR[t]) != 0^32 then nextPC = PC+(imm8 7 24||imm8) + 4
        //
        // BBC       branch if the bit specified by antoher register is clear                                RRI8
        // branches if teh bit specified by the low five bits of address register at is clear in addreses register as
        // for little-endian bit 0 is least significant bit and bi endian bit 0 is most significant bit
        // target instruction address of the branch is given bay address of BBC instruction plus the sign-extended 8-bit imm8
        // field plus four if specified bit is set execution continues with the next sequential instruction
        // b = AR[t]4..0 xor msbFirst^5
        // if AR[s] b = 0 then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BBS       branch if the bit specified by another register is set                                  RRI8
        // brances if the bit specified by the low five bits of address register at is set in address reigister as
        // target address is the address of the BBS instruction plus the sign-extended 8-bit imm8 field of the instruction
        // plus four. if the specified bit is clear, execution continues with the next sequential instruction
        // b = AR[t] 4..0 xor msbFirst^5
        // if AR[s]b != 0 then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BBCI      branch if the bit specified by an immediate is clear                                    RRI8
        // branches if specified by constant encoded in bbi field is clear in address register as
        // bbi field is split with bits 3..0 in bits 7..4 of the instruction word, and bit 4 in bit 12 of the instruction
        // word. target address given by address of the BBCI instruction plus sign-extended 8-bit imm8 field of the instruction
        // plus 4. If the specified bit is set, execution continues with the next sequential instruction.
        // b = bbi xor msbFirst^5
        // if AR[s] b = 0 then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BBSI      branch if the bit specified by an immediate is set                                      RRI8
        // branches if the bit specified by the constant encoded in bbi field is set in address register as
        // bbi field is split with bits 3..0 in bits 7..4 of the instruction word and bit 4 in bit 12 of the instruction word
        // the target instruction address of the branch is iven by teh address of the BBsinstruction plus the sign
        // extended 8-bit imm8 field of the instruction plus four. if the specified bit is clear execution continues with the
        // next sequential instruction.
        // b = bbi xor msbFirest^5
        // if AR[s]b != 0 then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BEQ       branch if a register equals another register                                            RRI8
        // branches if address registers as and at are equal
        // target instruction is address of the BEQ instruction plus the sign-extended 8-bit imm8 field of the instruction
        // plus 4. if registers are not equal execution continues with the next sequential instruction
        // if AR[s] = AR[t] then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BEQI      branch if a register equals an encoded constant                                         RRI8
        //  branches if as and constant encoded in the r field are equal
        // constant values encoded in the r field are not simply 0..15. constant values encoded by r, see table 3-17 on page 41
        // target instruction of the branch is given by address of BEQI instruction plus the sing-extended 8-bit imm8 field plus 4.
        // if register is not equal to the constant execution continues with the next sequential instruction.
        // if AR[s] = B4CONST(r) then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BEQZ      branch if a register equals zero                                                        BRI12
        // branches if as is equal to zero provides 12 bits of target range instead of the 8 in most conditional
        // branches target instruction address of the branch is given by the address of the BEQZ instruction plus
        // the sign-exteneded 12 bit imm12 field of the instruction plus 4.
        // if register as is not zero execution conintues with the next sequential instruction
        // if AR[s] = 0^32 then nextPC = PC + (imm12 11 20||imm12) + 4
        //
        // BGE       branch if a register is greater than or equal to a register                             RRI8
        // branches if address as is two's complement greater then or equal ot address at register at
        // target is BGE instruction address pluse sign-extended imm8 plus four.
        // if as is less than at execution continues with next sequential instruction
        // if AR[s] >= AR[t] then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BGEI      branch if a register is greater than or equal to an encoded constant                    RRI8
        // branches if as is twos complement greater than or equal to the constant encoded in the r field (table3-17) on page 41
        // target is address of BGEI instruction plus the sign-extended imm8 plus four if address register as is less
        // than the constant execution continues with the next sequential instruction
        // if AR[s] >= B4CONST(r) then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BGEU      branch if one register is greater than or equal to a register as unsigned               RRI8
        //  branches if as is unsigned greater then or equal to address register at
        // target address is BGEU address plus sign-extended imm8 plus 4 if as is less than at execution continues with next
        // sequential instruction.
        // if (0||AR[s]) >= (0||AR[t]) then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BGEUI     branch if one register is greater than or equal to an encoded constatnt as unsigned     RRI8
        // branches if address register as is unsigned greater than or equal to the constant encoded in r(see table 3-18 on pg 42)
        // target address is address of BGEUI plus sign-extended imm8 plus 4 if as less then constant execution continues
        // with next sequential instruction
        // if(-||AR[s]) >= (0||B4CONSTU(r)) then nextPC - PC + (imm8 7 24||imm8) + 4
        //
        // BGEZ      branch if a register is greater than or equal to zero                                   BRI12
        // branches if as is greather than or equal to zero (most significant bit is clear). 12 bit target range instead of 8 in most branches
        // target address is BGEZ address plus sign-extended imm12 plus 4 if register as is less than zero execution continues
        // with next sequential instruction
        // if AR[s]31 = 0 then nextPC = PC + (imm12 11 20||imm12) + 4
        //
        // BLT       branch if one register is less than a register                                          RRI8
        // branches if address register as is two's complement less than address register at
        // target is address of BLT plus sign-extended imm8 plus 4 if as greater than or equal to at execution continues
        // with next sequential instruction
        // if AR[s] < AR[t] then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BLTI      branch if one register is less than an encoded constant                                 RRI8
        // branches if as is two's complement less than the constant encoded in r(see table 3-17 on page 41)
        // target address is BLTI address plus sign-extended imm8 plus 4
        // if as is greater than or equal to the constant execution continues with the next sequential instruction
        // if AR[s] < B4CONST(r) then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BLTU      branch if one register is less than a register as unsigned                              RRI8
        // branches if as is unsigned less thna at
        // target is BLTU addres plus sign-extended imm8 plus four if as is greater than or equal to at execution continues
        // with the next sequential instruction
        // if (0||AR[s]) < (0||AR[t]) then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BLTUI     branch if one register is less than an encoded constant as unsigned                     RRI8
        // branches if as is unsigned less than the constant encoded in r field(see 3-18 on page 42)
        // target is BLTUI address pluse sign-extended imm8 plus 4 if as is greater than or equal to the constant
        // execution continues with the next sequential instruction
        // if(0||AR[s]) < (0||B4CONSTU(r)) then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BLTZ      branch if a register is less than zero                                                  BRI12
        // branches if as is less than zero(most significant bit is set) 12 bit target range
        // target is BLTZ address plus sign-extended imm12 plus 4 if as is greater than or equal to zero execution
        // continues with the next sequential instruction.
        // if AR[s]31 != 0 then nextPC = PC + (imm12 11 20||imm12) + 4
        //
        // BNALL     branch if some bits specified by a mask in a register are clear in another register     RRI8
        // branches if any of the bits specified by the mask in at are clear in as(if they are not all set). test
        // is performed by taking the bitwise logical and of at with the complement of as and testing if result is non-zero
        // target is BNALL address plus sign-extended 8-bit imm8 plus 4 if all masked bits are set execution continues
        // with the next sequential instruction
        // if((not AR[s]) and AR[t]) != 0 ^32 then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BNONE     branch if all bits spcified by a mask in a register are clear in another register       RRI8
        // branches if all of the bits specified by the mask in at are clear in as(if non of them are set) test is performed
        // by taking bitwise logical and of as with at and testing if result is zero
        // target is BNONE address plus the sign-extended imm8 plus 4 if any of the masked bits are set execution continues
        // with the next sequential instruction
        // if (AR[s] and AR[t]) = 0^32 then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BNE       branch if a register does not equal a register                                          RRI8
        // branches if as and at are not equal
        // target address is BNE address plus sign-extended imm8 plus 4 if the registers are equal execution continues with the next
        // sequential instruction
        // if AR[s] != AR[t] then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BNEI      branch if a register does not equal an encoded constant                                 RRI8
        // branches if as and constant encoded in r field(see table 3-17 on page 41) are not equal. target address is
        // BNEI address plus sign-extended imm8 plus 4. if register is equal to the constant, execution continues with the next sequential instruciton
        // if AR[s] != B4CONST(r) then nextPC = PC + (imm8 7 24||imm8) + 4
        //
        // BNEZ      branch if a register does not equal zero                                            BRI12
        // branches if as is not equal to zero 12 bit target range
        // target is BNEZ instruction plus sign-extended imm12 plus 4 if register as equals zero execution continues
        // with the next sequential instruction
        // if AR[s] != 0 ^32 then nextPC = PC + (imm12 11 20||imm12) + 4
        //
    }

    static inline void xten_coreMoveInstructions(uint32_t opcode)
    {
        // core Move instructions are
        // MOVI          move a 12 bit immediate to a register                                           RRI8
        // sets address reigster at to a constant in the range -2048..2047 encoded in the instruction word
        // constant is stored in two non-contiguous fields of the instruction word. processor decodes the constant specification
        // by concatenating the two fields and sign-extending the 12-bit value.
        // AR[t] = imm12 11 20 || imm12
        //
        // MOVEQZ        move register if the contents of a register are equal to zero                   RRR
        // conditianal move if equal to zero. if the contents of at are zero then the processor sets address register ar to the contents
        // of address register as. otherwise MOVEQZ performs no operation and leaves ar unchanged
        // if AR[t] = 0^32 then AR[r] = AR[s]
        // MOVEGEZ       move register if teh contents of a register are greater than or equal to zero   RRR
        // conditional move if greater than or equal to zero. if contents of at are greater than or equal to zero(most significant bit is clear)
        // then the processor sets ar to contents of register as. otherwise MOVGEZ performs no operation and leaves address register
        // ar unchanged
        // if AR[t] 31 = 0 then AR[r] = AR[s]
        //
        // MOVLTZ        move register if the contents of a register are less than zero                  RRR
        // if the contents of at are less than zero(most isgnificant bit is set), then processor sets
        // ar to the contents of as MOVLTZ performs no operation and leaves address register ar unchanged
        // if AR[t] 31 != 0 then AR[r] = AR[s]
        //
        // MOVNEZ        move register if the contents of a register are not zero                        RRR
        // if contents of at are non-zero processor sets ar to contents of as. otherwise MOVNEZ performs no operation and leaves
        // ar unchanged
        // if AR[t] != 0^32 then AR[r] = AR[s]
        //
    }

    static inline void xten_coreArithmeticInstructions(uint32_t opcode)
    {
        // core arithmetic instructions are
        /*********************************************************************/
        // ADDI      add a register and an 8-bit immediate                                       RRI8
        // calculates the two's complement 32-bit sum of as and constant in imm8 field assuming sign extended because it is useing twos complement
        // the low 32 bits of the sum are written to address register at arithmetic overflow is not detected
        // imm8 ranges from -128 to 127 and is sign-extended imm8
        // 24 bit instruction
        // AR[t] = AR[s] + (imm8 7 24||imm8)
        //
        // ADDMI     add a register and a shifted 8 bit immediate                                RRI8
        // extends the range of constant additoin often used in conjunction with load and store instructions to extend teh range of the base, pluse
        // offset the calculation.
        // calculates the two's complement 32-bit sum of as and operan imm8. low 32 bits of the sum are written to at
        // no arithmetic overflow is detected
        // the operand encoded in the instruction can have values that are multiples of 256 ranging from -32768 to 32512
        // that is decoded from a sign-extending imm8 and shifting the result left by eight bits
        // AR[t] = AR[s] + (imm8 7 16||imm8||0^8)
        //
        // ADD       add two registers                                                           RRR
        // calculates the two's complement 32-bit sum of as and at the low 32 bits are written to ar
        // arithmetic overflow is not detected
        // 24 bit instruction
        // AR[r] = AR[s] + AR[t]
        //
        // ADDX2     add two registers with one shifted left by one                              RRR
        // calculates the two's complement 32-bit sum of as shifted left by one bit and at the low 32bits of the sum are written to ar
        // arithmetic overflow is not detected
        // frequently used for address calculation and as part of sequences to bultiply by small constants
        // AR[r] = (AR[s]30..0||0) + AR[t]
        //
        // ADDX4     add two registers with one shifted left by two                              RRR
        // Calculates the two's complement 32-bit sum of as shifted left byyy two bits and address register at.
        // low 32 bits of the sum are written to ar no arithmetic overflow detected. frequently used for address calculation and as part
        // of a sequences to mulitply by small constants
        // AR[r] = (AR[s] 29..0||0^2) + AR[t]
        //
        // ADDX8     add two registers with one shifted left by three                            RRR
        // calculates teh two's complement 32 bit sum of as shifted left by 3 bits and address register at
        // low 32 bit of the sum are written to ar no arithmetic overflow detected
        // frequently used for address calculation and part of multiplcation sequence for small constants
        // AR[r] = (AR[s] 28..0||0^3) + AR[t]
        //
        // SUB       subtract two registers                                                      RRR
        // calculates the two's complement 32 bit difference of as and at the low 32 bits of difference are written to address register ar
        // no arithmetic overflow detected
        // AR[t] = AR[s] - AR[t]
        //
        // SUBX2     subtract two registers with the un-negated one shifted left by one          RRR
        // calculates the two's complement 32-bit difference of as shifted left byy 1 bit and at. low 32 bits written to ar
        // no arithmetic overflow detected
        // frequently used as part of sequences to multiply byy small constants
        // AR[r] = (AR[s] 30..0||0) - AR[t]
        //
        // SUBX4     subtract two registers with the un-negated one shifted left by two          RRR
        // calculates the two's complement 32-bit difference of as shifted left by two bits and at. low 32 bits of the difference are written to ar
        // no arithmetic overflow detected. frequently used for sequences to multiply by small constants
        // AR[r] = (AR[s] 29..0||0^2) - AR[t]
        //
        // SUBX8     subtract two registers with the un-negated one shifted left by three        RRR
        // calculates the two's complement 32-bit difference of as shifted left by 3 bits and at
        // low 32 bits are written to ar no arithmetic overflow detected
        // AR[r] = (AR[s] 28..0||0^3) - AR[t]
        //
        // NEG       negate a register                                                           RRR
        // calculates the two's complement negation of the contents of at and writes it to ar no arithmetic overflow detected.
        // AR[r] = 0 - AR[t]
        //
        // ABS       absolute value                                                              RRR
        // calculates the absolute value of contents of at and writes it to ar no arithmetic overflow detected
        // AR[r] = if AR[t] 31 then - AR[t] else AR[t]
        //
    }

    static inline void xten_coreBitwiseLogicalInstructions(uint32_t opcode)
    {
        // core bitwise logical instructions are
        // AND       bitwise AND of two registers    RRR
        // calculates bitwise logical and of as and at writing result to ar
        // AR[r] = AR[s] and AR[t]
        //
        // OR        bitwise OR two registers        RRR
        // calculates  the bitwise logical or of as and at writing result to ar
        // AR[r] = AR[s] or AR[t]
        //
        // XOR       bitwise XOR two registers       RRR
        // calculates the bitwise logical exclusive or of as and at writing result to ar
        // AR[r] = AR[s] xor AR[t]
        //
    }

    static inline void xten_coreShiftInstructions(uint32_t opcode)
    {
        // core shift instructions are(issues in the ISA specifing right and left specific assumed them from the names)
        // EXTUI     extract field specified by immediates from a register                               RRR
        // SRLI      shift right logical by immediate                                                    RRR
        // SRAI      shift right logical by immediate                                                    RRR
        // SLLI      shift left logical by immediate                                                     RRR
        // SRC       shift right combined by SAR with two registers as input and one as output           RRR
        // SLL       shift left logical by SAR                                                           RRR
        // SRL       shift right logical by SAR                                                          RRR
        // SRA       shift right logical by SAR                                                          RRR
        // SSL       set SAR from register for left shift                                                RRR
        // SSR       set SAR from register for right shift                                               RRR
        // SSAI      set SAR to immediate value                                                          RRR
        // SSA8B     use low 2 bits of address register to prepare SAR for SRC assuming big endian       RRR
        // SSA8L     use low 2 bits of address register to prepare SAR for SRC assuming little endian    RRR
    }

    static inline void xten_coreProcessorControlInstructions(uint32_t opcode)
    {
        // core processor control instructions are
        // RSR       read a special register                                 RSR
        // WSR       write a special register                                RSR
        // XSR       read and write a special register in an exchange        RRR
        // RUR       read user special register                              ?
        // WUR       write user special register                             ?
        // ISYNC     wait for instruction-Fetch-related changes to reslove   RRR
        // RSYNC     wait for dispatch related changes to resolve            RRR
        // ESYNC     wait for execution related changes to resolve           RRR
        // DSYNC     wait for data memory related changes to resolve         RRR
    }

#ifdef __cplusplus
}
#endif

#endif
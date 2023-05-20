#ifndef XTENSA_LX_H
#define XTENSA_LX_H
#define XTEN_DEBUGGING
#define XTEN_DEBUGGING_DETAILED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/*CPU defines no magic numbers floating about*/
#define DEFAULT_REGISTER_FILE_SIZE 32
#define REGISTER_WINDOW_SIZE 16
#define BOOLEAN_REGISTER_AMOUNT 16
#define FLOATING_POINT_REGISTER_AMOUNT 16
#define MAC16_REGISTER_AMOUNT 4
#define XTEN_MSB_ON 1
#define XTEN_MSB_OFF 0
#define XTEN_HIGH 1
#define XTEN_LOW 0

    typedef struct Xtensa_lx_CPU Xtensa_lx_CPU;

    static inline void xten_decodeQRST(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_decodeCALLN(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_decodeRST0(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_decodeRST1(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_decodeRST3(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_decodeSI(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_decodeLSAI(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_decodeOp0(Xtensa_lx_CPU *CPU, uint32_t opcode);

    static inline void xten_coreShiftInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_coreArithmeticInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_coreJumpCallInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_coreConditionalBranchInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_coreBitwiseLogicalInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_coreMoveInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_coreLoadInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_coreStoreInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_coreProcessorControlInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode);
    static inline void xten_coreMemoryOrderingInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode);

    void xten_helper_printBinary(uint32_t value);
    void xten_helper_printRegisters(uint32_t *reg_file, uint32_t offset);
    uint32_t xten_helper_signExtend32Bits(uint32_t value, int bits);

    /**
     * @brief This is a function pointer type for memory access the user must implement.
     * It represents a callback for reading from memory using the passed in CPU and address.
     *
     * @param CPU A pointer to the current CPU context.
     * @param address The address in memory to read from.
     * @return The value read from memory at the given address.
     */
    typedef uint32_t (*MemoryReadCallback)(Xtensa_lx_CPU *CPU, uint32_t address, void *context);

    /**
     * @brief This is a function pointer type for memory access the user must implement.
     * It represents a callback for writing to memory using the passed in CPU, address and value.
     *
     * @param CPU A pointer to the current CPU context.
     * @param address The address in memory to write to.
     * @param value The value to write to memory at the given address.
     */
    typedef void (*MemoryWriteCallback)(Xtensa_lx_CPU *CPU, uint32_t address, uint32_t value, int numBytes, void *context);

    /**
     * @brief struct representing an Xtensa CPU
     *
     * This struct stores register and option values for a simulated Xtensa CPU of an archtecture that can be
     * specified by the user through direct changes to the structure or provided functions.
     */
    typedef struct Xtensa_lx_CPU
    {
        // variables for parts of the cpu will go here
        /*Starting with registers*/
        uint32_t *registerFile; // only part of this register file will be used because in the core architecture register windowing is not enabled
        uint32_t PC;
        // uint32_t *optionalFloatingPointRegisters; not used in the core architecture
        // uint32_t *optionalMAC16Registers; not used in the core architecture
        // uint8_t *bRegisters; not used in the core architecture
        //  may want to add the optional windowless register files but need more details
        int windowOffset;       // this will remain zero in the core architecture because there is no register windowing
        uint8_t msbFirstOption; // this is set when the CPU is in big-endian mode
        uint8_t configurable;   // this is set to false when it is no longer defined to change certian CPU options options decided at a chip designer level
        // IO
        uint32_t addressLines;           // each bit is a pin representing the address the CPU is currently going to read from memory
        uint32_t dataBus;                // each bit is a input pin that will the value at an address to read from or data that is being written to other parts of the CPU
        uint8_t chipEnable;              // if chip not enabled a clock pulse makes no changes to the internal state of the CPU. ACTIVE HIGH
        uint8_t write;                   // this is set if on this clock pulse the CPU will be attempting a write on this clock pulse then data bus is set to value to be written and address is set to where
                                         // the data should be written. ACTIVE HIGH
        MemoryReadCallback readMemory;   // this handles memory reads function implemented by the user before CPU creation
        MemoryWriteCallback writeMemory; // this handles memory writes function implemented by the user before CPU creation

        uint32_t sar; // shift amount register only special register for the core profile

        void *callbackContext; // this allows the user to pass in any data they need to the callback implementations

    } Xtensa_lx_CPU;

    /**
     * @brief Executes next instruction set at the datapins of the CPU
     *
     * This function fetchs the instruction from the databus and sends it down the pipeline this may perform multiple clock cycles
     * at once. Due to this at times the CPU with be set to write and passed back to user in this senario the user can process the write
     * accordingly and call this instruction again to continue processing beyond the write stage setting CPU to get the next instruction in the core architecture.
     *
     * @param *CPU Xtensa_lx_CPU pointer takes the address of the Xtensa CPU to execute next instruction on
     */
    void xten_executeNext(Xtensa_lx_CPU *CPU)
    {
        // because core architecture is all that is implemented at the moment all opcodes are 24 bits
        uint32_t opcode = CPU->dataBus >> 8;
        xten_decodeOp0(CPU, opcode); // this will set things to decode and execute opcode
        // may result in an execute next with a flag set to do a process other then decoding and executing an instruction
        // this is to allow the user to process certain stages in the pipeline for the instruction that may require such processing
        // like writing to or reading data from memory.
        // increment the CPU->PC and address appropriately
        CPU->PC += 3;
        CPU->addressLines = CPU->PC;
    }

    /**
     * @brief Displays the CPU state in a legible way
     *
     * This function writes to the console in a clean way the state of all the registers and pins on the processor
     * allowing for the writing of processor before and after an instruction executes to debug proper execution of instructions.
     *
     * @param *CPU Xtensa_lx_CPU pointer to the CPU to display on console
     */
    void xten_displayCPU(Xtensa_lx_CPU *CPU)
    {
        printf("\tAddress Pins: ");
        xten_helper_printBinary(CPU->addressLines);
        printf("\n\tData Bus Pins: ");
        xten_helper_printBinary(CPU->dataBus);
        printf("\n\tProgram Counter: %d\n", CPU->PC);
        // print register file with the register window displayed in a clear way
        xten_helper_printRegisters(CPU->registerFile, CPU->windowOffset);
        printf((CPU->chipEnable == XTEN_HIGH) ? "\tChip Enabled\n" : "\tChip Disabled\n");
        printf((CPU->write == XTEN_HIGH) ? "\tChip Write Set\n" : "\tChip Read Set\n");
    }

    /**
     * @brief Sets the msbFirst option
     *
     * This function takes a cpu and sets its msbFirst option based on the flag being passed in once set for a particular cpu
     * this option should not be changed during execution as it is a parameter set by the chip designer and not the individl programmer.
     *
     * @param *CPU Xtensa_lx_CPU pointer takes the address of the Xtensa CPU to alter
     * @param flag uint8_t flag for msbFirst should use XTEN_MSB_ON or XTEN_MSB_OFF
     */
    void xten_ops_setMSBFirst(Xtensa_lx_CPU *CPU, uint8_t flag)
    {
        if (CPU->configurable)
        {
            CPU->msbFirstOption = (flag <= 0) ? XTEN_MSB_OFF : XTEN_MSB_ON;
        }
    }

    /**
     * @brief Reads write signal
     *
     * This function is used to check if the write signal is high or not returns a boolean true if writing this can be used for example
     * by the memory to check to see if the values in the data bus need to be written to the values at the address bus.
     *
     * @param *CPU Xtensa_lx_CPU pointer takes the address of the Xtensa CPU check the write pin of
     * @return boolean true when the CPU is writing on this clock pulse
     */
    bool xten_checkWrite(Xtensa_lx_CPU *CPU)
    {
        return (CPU->write == XTEN_HIGH);
    }

    /**
     * @brief Used to enable or disable this processor
     *
     * This function is used to enable and disable the processor passed in when recieving also weather the chip enable pin should be set to XTEN_HIGH
     * or XTEN_LOW the chip enable pin in this itteration of the simulator is active high
     *
     * @param *CPU Xtensa_lx_CPU pointer takes the address of the Xtensa CPU being altered
     * @param uint8_t value that should be set to either XTEN_HIGH or XTEN_LOW
     */
    void xten_setChipEnableState(Xtensa_lx_CPU *CPU, uint8_t chipEnableState)
    {
        CPU->chipEnable = (chipEnableState <= 0) ? XTEN_LOW : XTEN_HIGH;
    }

    /**
     * @brief reads specified pin of address line
     *
     * This function returns the state of a single specified pin of the address line of the passed in CPU
     *
     * @param *CPU Xtensa_lx_CPU pointer takes the address of the Xtensa CPU being altered
     * @param uint8_t will be %32 representing the pin to recieve the value of
     * @return uint8_t XTEN_HIGH or XTEN_LOW of the specified pin
     */
    uint8_t xten_readSpecifiedAddressPin(Xtensa_lx_CPU *CPU, uint8_t pin)
    {
        uint32_t mask = 1 << (pin % 32);
        return (CPU->addressLines & mask) ? XTEN_HIGH : XTEN_LOW;
    }

    /**
     * @brief reads specified pin of the data bus
     *
     * This function returns the state of a single specified pin of the data bus of the passed in CPU
     *
     * @param *CPU Xtensa_lx_CPU pointer takes the address of the Xtensa CPU being altered
     * @param uint8_t will be %32 representing the pin to recieve the value of
     * @return uint8_t XTEN_HIGH or XTEN_LOW of the specified pin
     */
    uint8_t xten_readSpecifiedDataPin(Xtensa_lx_CPU *CPU, uint8_t pin)
    {
        uint32_t mask = 1 << (pin % 32);
        return (CPU->dataBus & mask) ? XTEN_HIGH : XTEN_LOW;
    }

    /**
     * @brief writes specified pin of the data bus to specified value
     *
     * This function writes the pin specified of the data bus in the passed in CPU to be either XTEN_HIGH or XTEN_LOW depending on user input
     *
     * @param *CPU Xtensa_lx_CPU pointer takes the address of the Xtensa CPU being altered
     * @param uint8_t will be %32 representing the pin of the databus being written
     * @param uint8_t value the pin will be set to either XTEN_HIGH or XTEN_LOW
     */
    void xten_writeSpecifiedDataPin(Xtensa_lx_CPU *CPU, uint8_t pin, uint8_t value)
    {
        uint32_t mask = 1 << (pin % 32);
        if (value == XTEN_HIGH)
        {
            CPU->dataBus = CPU->dataBus | mask;
        }
        else
        {
            CPU->dataBus = CPU->dataBus & ~mask;
        }
    }

    // Clock pulse function goes here

    /**
     * @brief Creates a new Xtensa_lx_CPU object
     *
     * This function creates a new Xtensa_lx_CPU object with default values that can be changed directly by a confident user
     * or with functions designated to change CPU options prefixed with xten_ops_
     *
     * @return Xtensa_lx_CPU with default options
     */
    Xtensa_lx_CPU *xten_createCPU(MemoryReadCallback readMemory, MemoryWriteCallback writeMemory, void *callbackContext)
    {
        Xtensa_lx_CPU *resultingCPU;
        resultingCPU = (Xtensa_lx_CPU *)malloc(sizeof(Xtensa_lx_CPU));
        resultingCPU->registerFile = (uint32_t *)malloc(DEFAULT_REGISTER_FILE_SIZE * sizeof(uint32_t));
        // resultingCPU->bRegisters = (bool *)malloc(BOOLEAN_REGISTER_AMOUNT * sizeof(bool));
        resultingCPU->windowOffset = 0;                // no offset for initial window wont move on core architecture so only 16 registers
        resultingCPU->PC = 0;                          // start at instruction at address zero
        resultingCPU->msbFirstOption = XTEN_MSB_ON;    // default is big-endian
        resultingCPU->configurable = true;             // new CPU is still configureable
        resultingCPU->chipEnable = XTEN_HIGH;          // chip enabled by default
        resultingCPU->write = XTEN_LOW;                // chip not writing the first clock cycle
        resultingCPU->addressLines = resultingCPU->PC; // starting at CPU->PC address
        resultingCPU->dataBus = 0;                     // this is simply to initialize it to something

        resultingCPU->readMemory = readMemory;           // user defined function that handles the memory access portion of the pipeline the way the user defines
        resultingCPU->writeMemory = writeMemory;         // user defined function that handles the writeback portion of the pipeline the way the user defines
        resultingCPU->callbackContext = callbackContext; // callback contexts data that the user would like to use in the callback functions

        resultingCPU->sar = 0;

        // TODO ensure that the funcitons for memory read and write are implemented if not the CPU should be NULL for easy handeling of errors for the user
        if (readMemory == NULL || writeMemory == NULL || callbackContext == NULL)
        {
            resultingCPU = NULL;
        }

        return resultingCPU;
    }

    static inline void xten_freeCPU(Xtensa_lx_CPU *CPU)
    {
        if (CPU != NULL)
        {
            if (CPU->registerFile != NULL)
            {
                free(CPU->registerFile);
            }
            free(CPU);
        }
    }

    /****************************************This section is for decoding**************************************************************/

    // array for easy decoding of the r field special values
    uint32_t xten_table317[16] = {0xFFFFFFFF, 0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007, 0x00000008, 0x0000000A, 0x0000000C, 0x00000010, 0x00000020, 0x00000040, 0x00000080, 0x00000100};

    /**
     * @brief This decodes the passed in opcode fetched for the passed in CPU
     *
     * This function takes an Xtensa_lx_CPU and the opcode fetched for that CPU decodes and sends the opcode to be executed further
     * down the pipeline or to further decoding steps.
     *
     * @param CPU address of the Xtensa CPU
     * @param opcode uint32_t of the opcode the CPU should execute
     */
    static inline void xten_decodeOp0(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        // extract op0 keep in mind all core opcodes are actually 24 bits in size
        // op0 in all core opcodes when big endian is set
        uint32_t op0 = (opcode >> (CPU->msbFirstOption ? 20 : 0)) & 0x0F;
#ifdef XTEN_DEBUGGING
        printf("PC %X opcode %X:\n", CPU->PC, opcode);
#endif
#ifdef XTEN_DEBUGGING_DETAILED
        printf("At CPU->PC %X opcode %X op0 was found to be %X:\n", CPU->PC, opcode, op0);
#endif
        switch (op0 >> 2)
        {
        case 0x0:
            switch (op0)
            {
            case 0x0:
                // entering table decoding QRST193
                xten_decodeQRST(CPU, opcode);
                break;
            case 0x1:
                // goes to the L32R instruction
                xten_coreLoadInstructions(CPU, opcode);
                break;
            case 0x2:
                // entering table decoding LSAI216
                xten_decodeLSAI(CPU, opcode);
                break;
            case 0x3:
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                break;
            default:
                printf("\nsomething went wrong the switch could not find the op0 after finding it started with the prefix zero opcode is %8x \n", op0);
                break;
            }
            break;
        case 0x1:
            switch (op0)
            {
            case 0x4:
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                break;
            case 0x5:
                // entering table decoding CALLN 7-232
                xten_decodeCALLN(CPU, opcode);
                break;
            case 0x6:
                // entering table decoding SI 7-233
                xten_decodeSI(CPU, opcode);
                break;
            case 0x7:
                // entering table decoding B 7-238 all instructions decode with r in execution of branch instructions function
                xten_coreConditionalBranchInstructions(CPU, opcode);
                break;
            }
            break;
        case 0x2:
#ifdef XTEN_DEBUGGING
            printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            break;
        case 0x3:
#ifdef XTEN_DEBUGGING
            printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            break;
        default:
#ifdef XTEN_DEBUGGING
            printf("\tImpossible opcode encountered something went wrong with decoding logic.\n");
#endif
            break;
        }
    }

    /**
     * @brief This decodes the passed in opcode fetched for the passed in CPU that have already been designated part of QRST table.
     *
     * This function takes an Xtensa_lx_CPU and the opcode fetched for that CPU decodes and sends the opcode to be executed further
     * down the pipeline or to further decoding steps.
     *
     * @param CPU address of the Xtensa CPU
     * @param opcode uint32_t of the opcode the CPU should execute
     */
    static inline void xten_decodeQRST(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        uint32_t op1 = (opcode >> (CPU->msbFirstOption ? 4 : 16)) & 0x0F;
#ifdef XTEN_DEBUGGING
        printf("PC %X opcode %X:\n", CPU->PC, opcode);
#endif
#ifdef XTEN_DEBUGGING_DETAILED
        printf("\tAt CPU->PC %X opcode %X op1 was found to be %X:\n", CPU->PC, opcode, op1);
#endif
        switch (op1 >> 2)
        {
        case 0x0:
            switch (op1)
            {
            case 0x0:
                // RST0 table 7-194
                xten_decodeRST0(CPU, opcode);
                break;
            case 0x1:
                xten_decodeRST1(CPU, opcode);
                break;
            case 0x2:
                // RST2 table 7-209 all unimplimented
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                break;
            case 0x3:
                // RST3 table 7-210
                xten_decodeRST3(CPU, opcode);
                break;
            }
            break;
        case 0x1:
            switch (op1)
            {
            case 0x4:
                // EXTUI instruction
                xten_coreShiftInstructions(CPU, opcode); // TODO consider eleminating the repeat below this.
                break;
            case 0x5:
                // EXTUI instruction
                xten_coreShiftInstructions(CPU, opcode);
                break;
            case 0x6:
                // CUST0 table 7.3.2 reserved for designer designed opcodes
#ifdef XTEN_DEBUGGING
                printf("\tThis hits the designer designed opcode table.\n");
#endif
                break;
            case 0x7:
                // CUST1 table 7.3.2 reserved for designer designed opcodes
#ifdef XTEN_DEBUGGING
                printf("\tThis hits the designer designed opcode table.\n");
#endif
                break;
            }
            break;
        case 0x2:
#ifdef XTEN_DEBUGGING
            printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            break;
        case 0x3:
#ifdef XTEN_DEBUGGING
            printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            break;
        default:
#ifdef XTEN_DEBUGGING
            printf("\tImpossible opcode encountered something went wrong with decoding logic.\n");
#endif
            break;
        }
    }

    /**
     * @brief This decodes the passed in opcode fetched for the passed in CPU that have already been designated part of CALLN table.
     *
     * This function takes an Xtensa_lx_CPU and the opcode fetched for that CPU decodes and sends the opcode to be executed further
     * down the pipeline or to further decoding steps.
     *
     * @param CPU address of the Xtensa CPU
     * @param opcode uint32_t of the opcode the CPU should execute
     */
    static inline void xten_decodeCALLN(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        uint32_t n = (opcode >> (CPU->msbFirstOption ? 18 : 4)) & 0x03;
#ifdef XTEN_DEBUGGING
        printf("PC %X opcode %X:\n", CPU->PC, opcode);
#endif
#ifdef XTEN_DEBUGGING_DETAILED
        printf("\tAt CPU->PC %X opcode %X n was found to be %X:\n", CPU->PC, opcode, n);
#endif
        if (n == 0x0)
        {
            // call zero instruction
            xten_coreJumpCallInstructions(CPU, opcode);
        }
        else
        {
#ifdef XTEN_DEBUGGING
            printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
        }
    }

    /**
     * @brief This decodes the passed in opcode fetched for the passed in CPU that have already been designated part of SI table.
     *
     * This function takes an Xtensa_lx_CPU and the opcode fetched for that CPU decodes and sends the opcode to be executed further
     * down the pipeline or to further decoding steps.
     *
     * @param CPU address of the Xtensa CPU
     * @param opcode uint32_t of the opcode the CPU should execute
     */
    static inline void xten_decodeSI(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        uint32_t n = (opcode >> (CPU->msbFirstOption ? 18 : 4)) & 0x03;
#ifdef XTEN_DEBUGGING
        printf("PC %X opcode %X:\n", CPU->PC, opcode);
#endif
#ifdef XTEN_DEBUGGING_DETAILED
        printf("\tAt CPU->PC %X opcode %X n was found to be %X:\n", CPU->PC, opcode, n);
#endif
        if (n == 0x0)
        {
            // jump instruction
            xten_coreJumpCallInstructions(CPU, opcode);
        }
        else
        {
#ifdef XTEN_DEBUGGING
            printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
        }
    }

    /**
     * @brief This decodes the passed in opcode fetched for the passed in CPU that have already been designated part of LSAI table.
     *
     * This function takes an Xtensa_lx_CPU and the opcode fetched for that CPU decodes and sends the opcode to be executed further
     * down the pipeline or to further decoding steps.
     *
     * @param CPU address of the Xtensa CPU
     * @param opcode uint32_t of the opcode the CPU should execute
     */
    static inline void xten_decodeLSAI(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        uint32_t r = (opcode >> (CPU->msbFirstOption ? 8 : 12)) & 0x0F;
#ifdef XTEN_DEBUGGING
        printf("PC %X opcode %X:\n", CPU->PC, opcode);
#endif
#ifdef XTEN_DEBUGGING_DETAILED
        printf("\tAt CPU->PC %X opcode %X r was found to be %X:\n", CPU->PC, opcode, r);
#endif
        switch (r >> 2)
        {
        case 0x0:
            if (r == 0x3)
            {
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            }
            else
            {
                xten_coreLoadInstructions(CPU, opcode);
            }
            break;
        case 0x1:
            if (r == 0x7)
            {
                // CACHE sub c table 7-217 decoding cache opcodes that do not exist in core architecture
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            }
            else
            {
                xten_coreStoreInstructions(CPU, opcode);
            }
            break;
        case 0x2:
            switch (r)
            {
            case 0x8:
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                break;
            case 0x9:
                // L16SI instruction
                xten_coreLoadInstructions(CPU, opcode);
                break;
            case 0xa:
                // MOVI instruction
                xten_coreMoveInstructions(CPU, opcode);
                break;
            case 0xb:
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                break;
            }
            break;
        case 0x3:
            if (r == 0xc)
            {
                xten_coreArithmeticInstructions(CPU, opcode);
            }
            else
            {
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            }
            break;
        default:
#ifdef XTEN_DEBUGGING
            printf("\tImpossible opcode encountered something went wrong with decoding logic.\n");
#endif
            break;
        }
    }

    /**
     * @brief This decodes the passed in opcode fetched for the passed in CPU that have already been designated part of RST0 table.
     *
     * This function takes an Xtensa_lx_CPU and the opcode fetched for that CPU decodes and sends the opcode to be executed further
     * down the pipeline or to further decoding steps.
     *
     * @param CPU address of the Xtensa CPU
     * @param opcode uint32_t of the opcode the CPU should execute
     */
    static inline void xten_decodeRST0(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        uint32_t op2 = (opcode >> (CPU->msbFirstOption ? 0 : 20)) & 0x0F;
#ifdef XTEN_DEBUGGING
        printf("PC %X opcode %X:\n", CPU->PC, opcode);
#endif
#ifdef XTEN_DEBUGGING_DETAILED
        printf("\tAt CPU->PC %X opcode %X op2 was found to be %X:\n", CPU->PC, opcode, op2);
#endif
        switch (op2 >> 2)
        {
        case 0x0:
            if (op2 == 0x0)
            {
                // ST0 table
                // if r is 0x0000 then SNM0 table 196 otherwise all reserved or unimplemented then uses M some of these are implemented so it is important TODO
                if (((opcode >> (CPU->msbFirstOption ? 8 : 12)) & 0x0F) == 0x0)
                {
                    uint32_t m = (opcode >> (CPU->msbFirstOption ? 16 : 6)) & 0x03;
                    switch (m)
                    {
                    case 0x0:
                        // ILL unimplemented
                        break;
                    case 0x1:
                        // unimplemented
                        break;
                    case 0x2:
                        // JR table 197 reserved or unimplemented based on n or goes to following function set
                        xten_coreJumpCallInstructions(CPU, opcode);
                        break;
                    case 0x3:
                        // CALLX table 198 reserved or unimplemented based on n or goes to following function set
                        xten_coreJumpCallInstructions(CPU, opcode);
                        break;
                    }
                }
                else if (((opcode >> (CPU->msbFirstOption ? 8 : 12)) & 0x0F) == 0x2) // if R is two seems to go to the SYNC table confirmed missed first pass because it is surrounded by unimplemented opcodes
                {
                    // this is a hot fix for the lack of encoutering the SYNC table that bases the instructions it accesses off of the t value discovered missing when implementing EXTW and MEMW
                    uint32_t t = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F;
                    switch (t)
                    {
                    case 0xC:
                        xten_coreMemoryOrderingInstructions(CPU, opcode);
                        break;
                    case 0xD:
                        xten_coreMemoryOrderingInstructions(CPU, opcode);
                        break;
                    default:
#ifdef XTEN_DEBUGGING
                        printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                        break;
                    }
                }
                else
                {
#ifdef XTEN_DEBUGGING
                    printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                }
            }
            else
            {
                // bitwise logic instructions
                xten_coreBitwiseLogicalInstructions(CPU, opcode);
            }
            break;
        case 0x1:
            switch (op2)
            {
            case 0x4:
                // ST1 table 202 some of these are used fairly complex needs own function
            case 0x5:
                // TLB table 203no further tables none of the instructions implemented
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                break;
            case 0x6:
                // RT0 table 204 used if s = 0x0000 NEG 0x0001 ABS otherwise reserved
                uint32_t s = ((opcode >> (CPU->msbFirstOption ? 12 : 8)) & 0x0F);
                if (s == 0x0 || s == 0x1)
                {
                    xten_coreArithmeticInstructions(CPU, opcode);
                }
                else
                {
#ifdef XTEN_DEBUGGING
                    printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                }
                break;
            case 0x7:
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                break;
            }
            break;
        case 0x2:
            // all arithmetic functions
            xten_coreArithmeticInstructions(CPU, opcode);
            break;
        case 0x3:
            // all arithmetic functions
            xten_coreArithmeticInstructions(CPU, opcode);
            break;
        default:
#ifdef XTEN_DEBUGGING
            printf("\tImpossible opcode encountered something went wrong with decoding logic.\n");
#endif
            break;
        }
    }

    /**
     * @brief This decodes the passed in opcode fetched for the passed in CPU that have already been designated part of RST1 table.
     *
     * This function takes an Xtensa_lx_CPU and the opcode fetched for that CPU decodes and sends the opcode to be executed further
     * down the pipeline or to further decoding steps.
     *
     * @param CPU address of the Xtensa CPU
     * @param opcode uint32_t of the opcode the CPU should execute
     */
    static inline void xten_decodeRST1(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        uint32_t op2 = (opcode >> (CPU->msbFirstOption ? 0 : 20)) & 0x0F;
#ifdef XTEN_DEBUGGING
        printf("PC %X opcode %X:\n", CPU->PC, opcode);
#endif
#ifdef XTEN_DEBUGGING_DETAILED
        printf("\tAt CPU->PC %X opcode %X op2 was found to be %X:\n", CPU->PC, opcode, op2);
#endif
        switch (op2 >> 2)
        {
        case 0x0:
            // all shift instructions
            xten_coreShiftInstructions(CPU, opcode);
            break;
        case 0x1:
            switch (op2)
            {
            case 0x4:
                xten_coreShiftInstructions(CPU, opcode);
                break;
            case 0x5:
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                break;
            case 0x6:
                // XSR instruction
                xten_coreProcessorControlInstructions(CPU, opcode);
                break;
            case 0x7:
                // ACCER table 206 a bit odd uses op2 again to differentiate in a conflicting manner neither instruction in core architecture
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
                break;
            }
            break;
        case 0x2:
            // all shift instructions
            xten_coreShiftInstructions(CPU, opcode);
            break;
        case 0x3:
            // MUL instructions not core instructions
            // IMP table 207 on 1111 all instructions on this table unimplemented RFDX 208 table on r = 0x1110
            // neither instruction on RFDX table implemented
#ifdef XTEN_DEBUGGING
            printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            break;
        default:
#ifdef XTEN_DEBUGGING
            printf("\tImpossible opcode encountered something went wrong with decoding logic.\n");
#endif
            break;
        }
    }

    /**
     * @brief This decodes the passed in opcode fetched for the passed in CPU that have already been designated part of RST3 table.
     *
     * This function takes an Xtensa_lx_CPU and the opcode fetched for that CPU decodes and sends the opcode to be executed further
     * down the pipeline or to further decoding steps.
     *
     * @param CPU address of the Xtensa CPU
     * @param opcode uint32_t of the opcode the CPU should execute
     */
    static inline void xten_decodeRST3(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        uint32_t op2 = (opcode >> (CPU->msbFirstOption ? 0 : 20)) & 0x0F;
#ifdef XTEN_DEBUGGING
        printf("PC %X opcode %X:\n", CPU->PC, opcode);
#endif
#ifdef XTEN_DEBUGGING_DETAILED
        printf("\tAt CPU->PC %X opcode %X op2 was found to be %X:\n", CPU->PC, opcode, op2);
#endif
        switch (op2 >> 2)
        {
        case 0x0:
            // less than 2 implemented instructions RSR on 0 and WSR on 1
            if (op2 < 0x2)
            {
                xten_coreProcessorControlInstructions(CPU, opcode);
            }
            else
            {
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            }
            break;
        case 0x1:
            // all unimplemented
#ifdef XTEN_DEBUGGING
            printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            break;
        case 0x2:
            // all implemented move instructions
            xten_coreMoveInstructions(CPU, opcode);
            break;
        case 0x3:
            // greater than 13 implemented instructions RUR then WUR
            if (op2 > 0xd)
            {
                xten_coreProcessorControlInstructions(CPU, opcode);
            }
            else
            {
#ifdef XTEN_DEBUGGING
                printf("\tThis is an unimplemented or reserved opcode.\n");
#endif
            }
            break;
        default:
#ifdef XTEN_DEBUGGING
            printf("\tImpossible opcode encountered something went wrong with decoding logic.\n");
#endif
            break;
        }
    }

    /*******************************************End of decoding section***********************************************************************/

    static inline void xten_coreLoadInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {

        // Two opcodes being used here RRI8 and RI16 here they are in big endian both of these are 24 bit instructions
        // RRI8 is [4 bit major opcode][4 bit t AR target or source,BR target or Source, 4bit sub opcode][s 4 bit, AR source, BR source, AR target][r AR target, BR target, 4 bit immediate, 4-bit sub-opcode][imm8 8 bit immediate]
        // RI16 is [4 bit major opcode][t 4 bit][imm16 16 bit immediate]

        uint32_t address = 0x0; // size_t instead of uint32_t because during part of the operation can be negative we do not want it to end up negative though need to take a closer look at this TODO
        uint32_t value = 0x0;   // should not end up changing at if it is not changed which should not be the case unless an unimplemented instruction is encountered.

        // for each instruction they need to calculate the address and the value to be assigned to the register the data each time is written to register t
        // t is in the same place for all so it can be extracted first
        uint32_t t = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F; // this is the index of the register to assign data to regFile[t+windowOffset] = value being loaded
        // going to be if op0 is 0001 then set up RI16 style otherwise setup RRI8 style
        uint32_t op0 = (opcode >> (CPU->msbFirstOption ? 20 : 0)) & 0x0F;
        if (op0 == 0x1)
        {
            // L32R      load literal at offset from CPU->PC(32 bit load CPU->PC relative(16 bit negative word offset))   RI16
            // major opcode 0001     no sub opcode for this one
            // CPU->PC relative 32 bit load
            // address formed by adding 16 bit one extended constant value shifted left by two
            // to teh address of teh L32R plus three with the two least significant bits cleared
            // specifies 32-bit aligned addresses from -262141 to -4 bytes from the address of teh L32R instruction
            // 32 bits are read from the address and written to at
            // one of few memory instrucitons that can access instruction RAM/ROM
            printf("\n\tThe instruction is L32R\n");
            // Now we calculate the address for the L32R instruction

            uint16_t constValue = (opcode >> (CPU->msbFirstOption ? 0 : 8)) & 0xFFFF;
            int32_t oneExtendedConst = (int16_t)constValue;
            uint32_t address = (CPU->PC + 3 + (oneExtendedConst << 2)) & 0xFFFFFFFC;

            // we recieve the value for the instruction to load and manipulate as nessecerry
            value = CPU->readMemory(CPU, address, CPU->callbackContext);
        }
        else
        {
            // these instructions require s which defines what register is being added to create virtual address
            uint32_t s = (opcode >> (CPU->msbFirstOption ? 12 : 8)) & 0x0F;
            // instructions also require the 8 bit immidiate that is added to s to form the virtual address
            uint32_t imm8 = (opcode >> (CPU->msbFirstOption ? 0 : 16)) & 0xFF; // this is the zero extended version
            // the instruction is found by looking at r
            uint32_t r = (opcode >> (CPU->msbFirstOption ? 8 : 12)) & 0x0F;
            switch (r)
            {
            case 0x0:
                // core load instructions are
                // L8UI      load zero-extended byte(8 bit unsigned load(8 bit offset))                             RRI8
                // major opcode 0010     4-bit sub opcode in r 0000
                // adds contents of register represented by s to 8 bit immediate
                // data at memory address represented by this addition is then stored
                // in register represented by t zero extended
                printf("\n\tThe instruction is L8UI\n");
                address = CPU->registerFile[CPU->windowOffset + s] + imm8;
                value = CPU->readMemory(CPU, address, CPU->callbackContext) >> 24; // only 8 bits are read zero extended
                break;
            case 0x9:
                // L16SI     load signed extended 16 bit quantity(16 bit signed load(8 bit shifted offset))         RRI8
                // major opcode 0010     4-bit sub opcode stored in r 1001
                // 8 bit constant added to register represented by s shifted left by 1
                // 16 bits two bytes read from register at the memory location this makes the value
                // sign extended and then stored in register represented by t
                // without unaligned exception option least significant address bit is ignored
                // essentially subtract one from odd addresses before accessing
                printf("\n\tThe instruction is L16SI\n");
                address = CPU->registerFile[CPU->windowOffset + s] + (imm8 << 1);
                value = CPU->readMemory(CPU, address, CPU->callbackContext) >> 16; // only 16 bits are read sign extended
                value = xten_helper_signExtend32Bits(value, 16);
                break;
            case 0x1:
                // L16UI     load zero extended 16 bit quantity(16 bit unsigned load(8 bit shifted offset))         RRI8
                // major opcode 0010     4-bit sub opcode stored in r 0001
                // adds as and 8 bit immediate shifted left by 1
                // reads in data like in L16SI except the data is zero extended instead of sign extened
                printf("\n\tThe instruction is L16UI\n");
                address = CPU->registerFile[CPU->windowOffset + s] + (imm8 << 1);
                value = CPU->readMemory(CPU, address, CPU->callbackContext) >> 16; // only 16 bits are read sign extended
                break;
            case 0x2:
                // L32I      load 32 bit quantity(32 bit load(8 bit shifted offset))                                RRI8
                // major opcode 0010     4-bit sub opcode stored in r 0010
                // adds register as and 8 bit zero extended constant shifted left by 2
                // reads address this represents 4 bytes of it written to register at
                // this instruction ignores least significant two bits of the address calculated
                // without unaligned exception option
                printf("\n\tThe instruction is L32I\n");
                address = CPU->registerFile[CPU->windowOffset + s] + (imm8 << 2);
                value = CPU->readMemory(CPU, address, CPU->callbackContext);
                break;
            default:
                // if we end up here something is wrong in the machine code being executed
                printf("\n\tThe instruction is not implemented or something went wrong! This behaviour is undefined.\n");
                break;
            }
        }
        printf("\naddress is %8X and value to be assigned from that address is %8X\n", address, value);
        CPU->registerFile[CPU->windowOffset + t] = value;
    }

    static inline void xten_coreStoreInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        // RRI8 is [4 bit major opcode][4 bit t AR target or source,BR target or Source, 4bit sub opcode][s 4 bit, AR source, BR source, AR target][r AR target, BR target, 4 bit immediate, 4-bit sub-opcode][imm8 8 bit immediate]
        // firstly we extract the parts of the opcode
        uint32_t s = (opcode >> (CPU->msbFirstOption ? 12 : 8)) & 0x0F;
        uint32_t t = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F;
        uint32_t imm8 = (opcode >> (CPU->msbFirstOption ? 0 : 16)) & 0xFF; // zero extended
        uint32_t r = (opcode >> (CPU->msbFirstOption ? 8 : 12)) & 0x0F;
        uint32_t value = 0;

        switch (r)
        {
        case 0x4:
            // core store instructions are
            // s8I       store byte                  RRI8
            // major opcode 0010     sub opcode at r 0100
            // adds as and zero exteneded immediate 1 byte is written from
            // the least significant 8 bits of register at to memory at the address
            // caclulated
            printf("\n\tThe instruction is S8I\n");
            value = CPU->registerFile[CPU->windowOffset + t] & 0xFF;
            CPU->writeMemory(CPU, imm8 + CPU->registerFile[CPU->windowOffset + s], value, 1, CPU->callbackContext);
            break;
        case 0x5:
            // s16I      store 16 bit quantity       RRI8
            // major opcode 0010     sub opcode at r 0101
            // forms address adding as and 8 bit immediate zero exteneded shifted left by one
            // 16 least significant bits from at are written to the address formed
            // least significant bit is ignored in the calculated address without the unaligned exception option
            printf("\n\tThe instruction is S16I\n");
            value = CPU->registerFile[CPU->windowOffset + t] & 0xFFFF;
            CPU->writeMemory(CPU, (imm8 << 1) + CPU->registerFile[CPU->windowOffset + s], value, 2, CPU->callbackContext);
            break;
        case 0x6:
            // s32I      store 32 bit quantity       RRI8
            // major opcode 0010     sub opcode at r 0110
            // forms address by adding as with 8 bit zero exteneded constant shifted left by two
            // writes at to the memory address formed
            // least significant 2 bits of the address are ignored without unaligned exception option
            // can access instruction RAM
            printf("\n\tThe instruction is S32I\n");
            value = CPU->registerFile[CPU->windowOffset + t] & 0xFFFFFFFF;
            CPU->writeMemory(CPU, (imm8 << 2) + CPU->registerFile[CPU->windowOffset + s], value, 4, CPU->callbackContext);
            break;
        default:
            printf("\nSomething went wrong proceeded to xten_coreStoreInstructions without a valid opcode this error could have come from the code being run\n");
            break;
        }
    }

    static inline void xten_coreMemoryOrderingInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        // RRR is [op0][t][s][r][op1][op2]
        uint32_t t = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F;

        switch (t)
        {
        case 0xC:
            // core memory ordering instructions are
            // MEMW      wait for any possible memory ordering requirement       RRR
            // major opcode 0000     subopcodes specified by op1 0000 op2 0000 rst in that order 0010 0000 1100
            // in this implementation is a no-op used to seperate load and store calls needing more time
            printf("\n\tThe instruction is MEMW\n");
        case 0xD:
            // EXTW      wait for any possible external ordering requiremetn     RRR
            // no paramiters whole thing is opcode 0000 0000 0010 0000 1101 0000
            // ensures changes from all previous instructions before will perform
            // any load, store, acquire, release, prefetch, or cash instructions
            // and everything that will has affected output pins
            printf("\n\tThe instruction is EXTW\n");
        default:
            printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
            break;
        }
    }

    static inline void xten_coreJumpCallInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode) // TODO testing for jump and call instructions
    {
        // get parts
        uint32_t op0 = (opcode >> (CPU->msbFirstOption ? 20 : 0)) & 0xF;
        uint32_t n = (opcode >> (CPU->msbFirstOption ? 18 : 2)) & 0x3;
        uint32_t m = (opcode >> (CPU->msbFirstOption ? 16 : 6)) & 0x3;
        uint32_t offset = 0;

        // sort through the instructions
        if (op0 == 0x5)
        {
            // CALL0     Call subroutine at CPU->PC plus offset place return address in A0                CALL
            // subroutines without using register windows. The return address is palced in
            // a0 and the processor then branches to the target address
            // the return address is the address of the CALL0 instruction plus three
            // target instruction address must be 32 bit aligned allowing CALL0 to have a larger effective range
            // least sig two bits set to zero plus the sign-extened 18-bit offset shifted by two, plus four
            printf("\n\tThe instruction is CALL0\n");

            // place return address into a0
            CPU->registerFile[CPU->windowOffset] = CPU->PC + 3; // plus three to make sure it advances to next instruction on return

            // find target instruction address
            uint32_t address = CPU->PC & 0xFFFFFFFC;
            offset = (opcode >> (CPU->msbFirstOption ? 0 : 6)) & 0x3FFFF;
            if (offset & (1 << 17))
            {
                offset |= 0xFFFC0000;
            }
            offset = offset << 2;
            // noticed here that the program counter is set up in my implementation to point to the currently executing instruction this is an incorrect implementation
            // but not a lot of time to fix it if and when it is fixed the offset will need to have 3 added to it to point to the next instruction
            CPU->PC = address + offset - 3; // CPU->PC will increment by 3 at the end of the instruction
            CPU->addressLines = CPU->PC;
        }
        else if (op0 == 0x6)
        {
            // J         jump to CPU->PC plus offset                                                 CALL
            // Unconditional Jump 18 bit offset followed by opcode 00 0110
            // Performs an unconditional branch to the target address signed 18-bit CPU->PC-relative offset is used to specify the target address
            // address of the J instruction plus the sign-extended 18-bit offset range is -131068 to 131075
            // nextPC = CPU->PC + (offset 17 14 || offset) + 4
            printf("\n\tThe instruction is J\n");
            offset = (opcode >> (CPU->msbFirstOption ? 0 : 6)) & 0x3FFFF;
            if (offset & (1 << 17))
            {
                offset |= 0xFFFC0000;
            }
            // Plus 4 issue continued look at CALL0 instruction for details
            CPU->PC = CPU->PC + offset - 3; // CPU->PC will increment by 3 at the end of the instruction
            CPU->addressLines = CPU->PC;
        }
        else
        {
            // further testing nessesary
            if (n == 0x2)
            {
                // this is the JX instruction
                // JX        jump to register-specified location  CALLX
                // unconditional jump based on register specified by as
                // perfoms an unconditional jump to the address in register as
                printf("\n\tThe instruction is JX\n");
                uint32_t s = (opcode >> (CPU->msbFirstOption ? 12 : 8)) & 0xF;
                CPU->PC = CPU->registerFile[CPU->windowOffset + s] - 3; // CPU->PC will increment by 3 at the end of the instruction
                CPU->addressLines = CPU->PC;
            }
            else
            {
                if (m == 0x3)
                {
                    // CALLX0    Call subroutine register specified location, place return address in A0     CALLX
                    // calls subroutines without using register windows the return address is placed in a0 and teh processor
                    // then branches to the target address,
                    printf("\n\tThe instruction is CALLX0\n");
                    CPU->registerFile[CPU->windowOffset] = CPU->PC + 3; // plus three to make sure it advances to next instruction on return
                    uint32_t s = (opcode >> (CPU->msbFirstOption ? 12 : 8)) & 0xF;
                    CPU->PC = CPU->registerFile[CPU->windowOffset + s] - 3; // CPU->PC will increment by 3 at the end of the instruction
                    CPU->addressLines = CPU->PC;
                }
                else if (m == 0x2)
                {
                    // RET       subroutine return through A0    CALLX
                    // 0000 0000 0000 0000 10 00 0000
                    // This returns from routine called by either CALL0 or CALLX0 equivalent to the instruction JX A0
                    // serparate instruction because some implementations may realize performace advantages from it being seperate
                    printf("\n\tThe instruction is RET\n");
                    CPU->PC = CPU->registerFile[CPU->windowOffset] - 3; // CPU->PC will increment by 3 at the end of the instruction
                    CPU->addressLines = CPU->PC;
                }
            }
        }
    }

    static inline void xten_coreConditionalBranchInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode) // TODO create code to test a subset at least of these branch instructions
    {
        // RRI8 is [4 bit major opcode][4 bit t AR target or source,BR target or Source, 4bit sub opcode][s 4 bit, AR source, BR source, AR target][r AR target, BR target, 4 bit immediate, 4-bit sub-opcode][imm8 8 bit immediate]
        // most of these functions as they branch to other parts of the program are going to have plus 4 errors stemming from the lack of correct CPU->PC assignment
        uint32_t op0 = (opcode >> (CPU->msbFirstOption ? 20 : 0)) & 0x0F;
        uint32_t s = (opcode >> (CPU->msbFirstOption ? 12 : 8)) & 0x0F;
        uint32_t t = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F;
        uint32_t r = (opcode >> (CPU->msbFirstOption ? 8 : 12)) & 0x0F;
        uint32_t imm12 = (opcode >> (CPU->msbFirstOption ? 0 : 12)) & 0x0FFF;
        uint32_t imm8 = (opcode >> (CPU->msbFirstOption ? 0 : 6)) & 0xFF;

        int8_t offset = 0;
        uint32_t target = 0;

        bool willBranch8 = false;
        bool willBranch12 = false;

        if (op0 == 0x7)
        {
            switch (r)
            {
            case 0x0:
                // BNONE     branch if all bits spcified by a mask in a register are clear in another register       RRI8
                // branches if all of the bits specified by the mask in at are clear in as(if non of them are set) test is performed
                // by taking bitwise logical and of as with at and testing if result is zero
                // target is BNONE address plus the sign-extended imm8 plus 4 if any of the masked bits are set execution continues
                // with the next sequential instruction
                // if (AR[s] and AR[t]) = 0^32 then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BNONE\n");
                if ((CPU->registerFile[CPU->windowOffset + s] & CPU->registerFile[CPU->windowOffset + t]) == 0x0)
                {
                    willBranch8 = true;
                }
                break;
            case 0x1:
                // BEQ       branch if a register equals another register                                            RRI8
                // branches if address registers as and at are equal
                // target instruction is address of the BEQ instruction plus the sign-extended 8-bit imm8 field of the instruction
                // plus 4. if registers are not equal execution continues with the next sequential instruction
                // if AR[s] = AR[t] then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BEQ\n");
                if (CPU->registerFile[CPU->windowOffset + s] == CPU->registerFile[CPU->windowOffset + t])
                {
                    willBranch8 = true;
                }
                break;
            case 0x2:
            {
                // BLT       branch if one register is less than a register                                          RRI8
                // branches if address register as is two's complement less than address register at
                // target is address of BLT plus sign-extended imm8 plus 4 if as greater than or equal to at execution continues
                // with next sequential instruction
                // if AR[s] < AR[t] then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BLT\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];

                if (as < at)
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0x3:
            {
                // BLTU      branch if one register is less than a register as unsigned                              RRI8
                // branches if as is unsigned less thna at
                // target is BLTU addres plus sign-extended imm8 plus four if as is greater than or equal to at execution continues
                // with the next sequential instruction
                // if (0||AR[s]) < (0||AR[t]) then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BLTU\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                uint32_t at = CPU->registerFile[CPU->windowOffset + t];

                if (as < at)
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0x4:
            {
                // BALL      branch if all bits specified by a mask in one register are set in another register      RRI8
                // branches if all bits specified by the mask in address register at are set in address register as
                // test performed by taking bitwise logical and of at and the complement of as and seeing if result is zero
                // target address is address of the BALL instruction plus the sign-extended 8-bit imm8 plus four if any
                // masked bits are clear execution continus with next sequential instruction
                // if((not AR[s]) and AR[t]) = 0^32 then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BALL\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                uint32_t at = CPU->registerFile[CPU->windowOffset + t];

                if ((at & ~as) == 0)
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0x5:
            {
                // BBC       branch if the bit specified by antoher register is clear                                RRI8
                // branches if teh bit specified by the low five bits of address register at is clear in addreses register as
                // for little-endian bit 0 is least significant bit and bi endian bit 0 is most significant bit
                // target instruction address of the branch is given bay address of BBC instruction plus the sign-extended 8-bit imm8
                // field plus four if specified bit is set execution continues with the next sequential instruction
                // b = AR[t]4..0 xor msbFirst^5
                // if AR[s] b = 0 then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BBC\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                uint32_t at = CPU->registerFile[CPU->windowOffset + t];
                uint32_t bit = at & 0x1F;

                if (CPU->msbFirstOption)
                {
                    // For big endian, bit 0 is the most significant bit.
                    bit = 31 - bit;
                }
                if ((as & (1 << bit)) == 0)
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0x6:
                // break omitted because implementation same as the instruction below
            case 0x7:
            {
                // BBCI      branch if the bit specified by an immediate is clear                                    RRI8
                // branches if specified by constant encoded in bbi field is clear in address register as
                // bbi field is split with bits 3..0 in bits 7..4 of the instruction word, and bit 4 in bit 12 of the instruction
                // word. target address given by address of the BBCI instruction plus sign-extended 8-bit imm8 field of the instruction
                // plus 4. If the specified bit is set, execution continues with the next sequential instruction.
                // b = bbi xor msbFirst^5
                // if AR[s] b = 0 then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BBCI\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                uint32_t bbi4_0 = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F; // bbi bits 3..0
                uint32_t bbi5 = (opcode >> (CPU->msbFirstOption ? 12 : 12)) & 0x01;  // bbi bit 4
                uint32_t bbi = (bbi5 << 4) | bbi4_0;                                 // Combine to get the full bbi value
                if (CPU->msbFirstOption)
                {
                    // For big endian, bit 0 is the most significant bit.
                    bbi = 31 - bbi;
                }
                if ((as & (1 << bbi)) == 0)
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0x8:
            {
                // BANY      branch if any bits specified by a mask in one register are set in another register      RRI8
                // branches if any of the bits specified by mask in address register at are set in address register as
                // tested by bitwise logical and of as and at and testing if result is non-zero
                // target address given by address of BANY plus the sign-extended 8-bit imm8 field of the instruction plus
                // four if all masked bits are clear execution continues with the next sequential instruction
                // if(AR[s] and AR[t]) != 0^32 then nextPC = CPU->PC+(imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BANY\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                uint32_t at = CPU->registerFile[CPU->windowOffset + t];
                if ((as & at) != 0)
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0x9:
            {
                // BNE       branch if a register does not equal a register                                          RRI8
                // branches if as and at are not equal
                // target address is BNE address plus sign-extended imm8 plus 4 if the registers are equal execution continues with the next
                // sequential instruction
                // if AR[s] != AR[t] then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BNE\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                uint32_t at = CPU->registerFile[CPU->windowOffset + t];
                if (as != at)
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0xA:
            {
                // BGE       branch if a register is greater than or equal to a register                             RRI8
                // branches if address as is two's complement greater then or equal ot address at register at
                // target is BGE instruction address pluse sign-extended imm8 plus four.
                // if as is less than at execution continues with next sequential instruction
                // if AR[s] >= AR[t] then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BGE\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                if (as >= at)
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0xB:
            {
                // BGEU      branch if one register is greater than or equal to a register as unsigned               RRI8
                //  branches if as is unsigned greater then or equal to address register at
                // target address is BGEU address plus sign-extended imm8 plus 4 if as is less than at execution continues with next
                // sequential instruction.
                // if (0||AR[s]) >= (0||AR[t]) then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BGEU\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                uint32_t at = CPU->registerFile[CPU->windowOffset + t];
                if (as >= at)
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0xC:
            {
                // BNALL     branch if some bits specified by a mask in a register are clear in another register     RRI8
                // branches if any of the bits specified by the mask in at are clear in as(if they are not all set). test
                // is performed by taking the bitwise logical and of at with the complement of as and testing if result is non-zero
                // target is BNALL address plus sign-extended 8-bit imm8 plus 4 if all masked bits are set execution continues
                // with the next sequential instruction
                // if((not AR[s]) and AR[t]) != 0 ^32 then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BNALL\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                uint32_t at = CPU->registerFile[CPU->windowOffset + t];
                if ((at & ~as) != 0)
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0xD:
            {
                // BBS       branch if the bit specified by another register is set                                  RRI8
                // brances if the bit specified by the low five bits of address register at is set in address reigister as
                // target address is the address of the BBS instruction plus the sign-extended 8-bit imm8 field of the instruction
                // plus four. if the specified bit is clear, execution continues with the next sequential instruction
                // b = AR[t] 4..0 xor msbFirst^5
                // if AR[s]b != 0 then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BBS\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                uint32_t at = CPU->registerFile[CPU->windowOffset + t];
                uint32_t bit = at & 0x1F;
                if (as & (1 << bit)) // If the bit at 'bitIndex' in 'as' is set
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0xE:
                // break omitted because implementation same as the instruction below
            case 0xF:
            {
                // BBSI      branch if the bit specified by an immediate is set                                      RRI8
                // branches if the bit specified by the constant encoded in bbi field is set in address register as
                // bbi field is split with bits 3..0 in bits 7..4 of the instruction word and bit 4 in bit 12 of the instruction word
                // the target instruction address of the branch is iven by teh address of the BBsinstruction plus the sign
                // extended 8-bit imm8 field of the instruction plus four. if the specified bit is clear execution continues with the
                // next sequential instruction.
                // b = bbi xor msbFirst^5
                // if AR[s]b != 0 then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BBSI\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                uint32_t bbi4_0 = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F; // bbi bits 3..0
                uint32_t bbi5 = (opcode >> (CPU->msbFirstOption ? 12 : 12)) & 0x01;  // bbi bit 4
                uint32_t bbi = (bbi5 << 4) | bbi4_0;
                if (CPU->msbFirstOption)
                {
                    // For big endian, bit 0 is the most significant bit.
                    bbi = 31 - bbi;
                }
                if ((as & (1 << bbi)) != 0)
                {
                    willBranch8 = true;
                }
            }
            break;
            }
        }
        else if (op0 == 0x6)
        {
            switch (t)
            {
            case 0x1:
            {
                // BEQZ      branch if a register equals zero                                                        BRI12
                // branches if as is equal to zero provides 12 bits of target range instead of the 8 in most conditional
                // branches target instruction address of the branch is given by the address of the BEQZ instruction plus
                // the sign-exteneded 12 bit imm12 field of the instruction plus 4.
                // if register as is not zero execution conintues with the next sequential instruction
                // if AR[s] = 0^32 then nextPC = CPU->PC + (imm12 11 20||imm12) + 4
                printf("\n\tThe instruction is BEQZ\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                if (as == 0)
                {
                    willBranch12 = true;
                }
            }
            break;
            case 0x2:
            {
                // BEQI      branch if a register equals an encoded constant                                         RRI8
                //  branches if as and constant encoded in the r field are equal
                // constant values encoded in the r field are not simply 0..15. constant values encoded by r, see table 3-17 on page 41
                // target instruction of the branch is given by address of BEQI instruction plus the sing-extended 8-bit imm8 field plus 4.
                // if register is not equal to the constant execution continues with the next sequential instruction.
                // if AR[s] = B4CONST(r) then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BEQI\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                if (as == xten_table317[r])
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0x5:
            {
                // BNEZ      branch if a register does not equal zero                                            BRI12
                // branches if as is not equal to zero 12 bit target range
                // target is BNEZ instruction plus sign-extended imm12 plus 4 if register as equals zero execution continues
                // with the next sequential instruction
                // if AR[s] != 0 ^32 then nextPC = CPU->PC + (imm12 11 20||imm12) + 4
                printf("\n\tThe instruction is BNEZ\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                if (as != 0)
                {
                    willBranch12 = true;
                }
            }
            break;
            case 0x6:
            {
                // BNEI      branch if a register does not equal an encoded constant                                 RRI8
                // branches if as and constant encoded in r field(see table 3-17 on page 41) are not equal. target address is
                // BNEI address plus sign-extended imm8 plus 4. if register is equal to the constant, execution continues with the next sequential instruciton
                // if AR[s] != B4CONST(r) then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BNEI\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                if (as != xten_table317[r])
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0x9:
            {
                // BLTZ      branch if a register is less than zero                                                  BRI12
                // branches if as is less than zero(most significant bit is set) 12 bit target range
                // target is BLTZ address plus sign-extended imm12 plus 4 if as is greater than or equal to zero execution
                // continues with the next sequential instruction.
                // if AR[s]31 != 0 then nextPC = CPU->PC + (imm12 11 20||imm12) + 4
                printf("\n\tThe instruction is BLTZ\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                if ((as & 0x80000000) != 0)
                {
                    willBranch12 = true;
                }
            }
            break;
            case 0xA:
            {
                // BLTI      branch if one register is less than an encoded constant                                 RRI8
                // branches if as is two's complement less than the constant encoded in r(see table 3-17 on page 41)
                // target address is BLTI address plus sign-extended imm8 plus 4
                // if as is greater than or equal to the constant execution continues with the next sequential instruction
                // if AR[s] < B4CONST(r) then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BLTI\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                if ((int32_t)as < (int32_t)xten_table317[r])
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0xB:
            {
                // BLTUI     branch if one register is less than an encoded constant as unsigned                     RRI8
                // branches if as is unsigned less than the constant encoded in r field(see 3-18 on page 42)
                // target is BLTUI address pluse sign-extended imm8 plus 4 if as is greater than or equal to the constant
                // execution continues with the next sequential instruction
                // if(0||AR[s]) < (0||B4CONSTU(r)) then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BLTUI\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                if (as < xten_table317[r])
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0xD:
            {
                // BGEZ      branch if a register is greater than or equal to zero                                   BRI12
                // branches if as is greather than or equal to zero (most significant bit is clear). 12 bit target range instead of 8 in most branches
                // target address is BGEZ address plus sign-extended imm12 plus 4 if register as is less than zero execution continues
                // with next sequential instruction
                // if AR[s]31 = 0 then nextPC = CPU->PC + (imm12 11 20||imm12) + 4
                printf("\n\tThe instruction is BGEZ\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                if (as >= 0)
                {
                    willBranch12 = true;
                }
            }
            case 0xE:
            {
                // BGEI      branch if a register is greater than or equal to an encoded constant                    RRI8
                // branches if as is twos complement greater than or equal to the constant encoded in the r field (table3-17) on page 41
                // target is address of BGEI instruction plus the sign-extended imm8 plus four if address register as is less
                // than the constant execution continues with the next sequential instruction
                // if AR[s] >= B4CONST(r) then nextPC = CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BGEI\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                if ((int32_t)as >= (int32_t)xten_table317[r])
                {
                    willBranch8 = true;
                }
            }
            break;
            case 0xF:
            {
                // BGEUI     branch if one register is greater than or equal to an encoded constatnt as unsigned     RRI8
                // branches if address register as is unsigned greater than or equal to the constant encoded in r(see table 3-18 on pg 42)
                // target address is address of BGEUI plus sign-extended imm8 plus 4 if as less then constant execution continues
                // with next sequential instruction
                // if(-||AR[s]) >= (0||B4CONSTU(r)) then nextPC - CPU->PC + (imm8 7 24||imm8) + 4
                printf("\n\tThe instruction is BGEUI\n");
                uint32_t as = CPU->registerFile[CPU->windowOffset + s];
                if (as >= xten_table317[r])
                {
                    willBranch8 = true;
                }
            }
            break;
            default:
                printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
                break;
            }
        }
        else
        {
            printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
        }

        if (willBranch8)
        {
            // branch to target
            offset = (int8_t)imm8;
            target = CPU->PC + ((int32_t)offset << 2);
            CPU->PC = target - 3; // the minus three is required because the CPU->PC will increment at the end of this instruction
            CPU->addressLines = CPU->PC;
        }
        else if (willBranch12)
        {
            // branch to target
            int32_t offset = imm12;
            if (offset & 0x800)
            {                         // If the sign bit is set
                offset |= 0xFFFFF000; // Sign extend
            }
            CPU->PC = CPU->PC + offset - 3; // the minus three is required because the CPU->PC will increment at the end of this instruction
            CPU->addressLines = CPU->PC;
        }
    }

    static inline void xten_coreMoveInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        uint32_t op0 = (opcode >> (CPU->msbFirstOption ? 20 : 0)) & 0x0F;
        uint32_t s = (opcode >> (CPU->msbFirstOption ? 12 : 8)) & 0x0F;
        uint32_t t = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F;
        uint32_t r = (opcode >> (CPU->msbFirstOption ? 8 : 12)) & 0x0F;
        uint32_t op2 = (opcode >> (CPU->msbFirstOption ? 0 : 20)) & 0x0F;

        if (op0 == 0x2)
        {
            // core Move instructions are
            // MOVI          move a 12 bit immediate to a register                                           RRI8
            // sets address reigster at to a constant in the range -2048..2047 encoded in the instruction word
            // constant is stored in two non-contiguous fields of the instruction word. processor decodes the constant specification
            // by concatenating the two fields and sign-extending the 12-bit value.
            // AR[t] = imm12 11 20 || imm12
            //
            printf("\n\tThe instruction is MOVI\n");
            uint32_t imm12 = ((s << 8) | (opcode >> (CPU->msbFirstOption ? 0 : 16))) & 0xFF;
            CPU->registerFile[CPU->windowOffset + t] = ((int32_t)imm12 << 20) >> 20;
        }
        else if (op0 == 0x0)
        {
            switch (op2)
            {
            case 0x8:
                // MOVEQZ        move register if the contents of a register are equal to zero                   RRR
                // conditianal move if equal to zero. if the contents of at are zero then the processor sets address register ar to the contents
                // of address register as. otherwise MOVEQZ performs no operation and leaves ar unchanged
                // if AR[t] = 0^32 then AR[r] = AR[s]
                printf("\n\tThe instruction is MOVEQZ\n");
                if (CPU->registerFile[CPU->windowOffset + t] == 0)
                {
                    CPU->registerFile[CPU->windowOffset + r] = CPU->registerFile[CPU->windowOffset + s];
                }
                break;
            case 0xB:
                // MOVGEZ       move register if teh contents of a register are greater than or equal to zero   RRR
                // conditional move if greater than or equal to zero. if contents of at are greater than or equal to zero(most significant bit is clear)
                // then the processor sets ar to contents of register as. otherwise MOVGEZ performs no operation and leaves address register
                // ar unchanged
                // if AR[t] 31 = 0 then AR[r] = AR[s]
                printf("\n\tThe instruction is MOVGEZ\n");
                if ((CPU->registerFile[CPU->windowOffset + t] & 0x80000000) == 0)
                {
                    CPU->registerFile[CPU->windowOffset + r] = CPU->registerFile[CPU->windowOffset + s];
                }
                break;
            case 0xA:
                // MOVLTZ        move register if the contents of a register are less than zero                  RRR
                // if the contents of at are less than zero(most isgnificant bit is set), then processor sets
                // ar to the contents of as MOVLTZ performs no operation and leaves address register ar unchanged
                // if AR[t] 31 != 0 then AR[r] = AR[s]
                printf("\n\tThe instruction is MOVLTZ\n");
                if ((CPU->registerFile[CPU->windowOffset + t] & 0x80000000) != 0)
                {
                    CPU->registerFile[CPU->windowOffset + r] = CPU->registerFile[CPU->windowOffset + s];
                }
                break;
            case 0x9:
                // MOVNEZ        move register if the contents of a register are not zero                        RRR
                // if contents of at are non-zero processor sets ar to contents of as. otherwise MOVNEZ performs no operation and leaves
                // ar unchanged
                // if AR[t] != 0^32 then AR[r] = AR[s]
                printf("\n\tThe instruction is MOVNEZ\n");
                if (CPU->registerFile[CPU->windowOffset + t] != 0)
                {
                    CPU->registerFile[CPU->windowOffset + r] = CPU->registerFile[CPU->windowOffset + s];
                }
                break;
            default:
                printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
                break;
            }
        }
        else
        {
            printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
        }
    }

    static inline void xten_coreArithmeticInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode) // TODO write tests
    {
        uint32_t op0 = (opcode >> (CPU->msbFirstOption ? 20 : 0)) & 0x0F;
        uint32_t s = (opcode >> (CPU->msbFirstOption ? 12 : 8)) & 0x0F;
        uint32_t t = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F;
        uint32_t r = (opcode >> (CPU->msbFirstOption ? 8 : 12)) & 0x0F;
        uint32_t op2 = (opcode >> (CPU->msbFirstOption ? 0 : 20)) & 0x0F;

        if (op0 == 0x2)
        {
            if (r == 0xC)
            {
                // ADDI      add a register and an 8-bit immediate                                       RRI8
                // calculates the two's complement 32-bit sum of as and constant in imm8 field assuming sign extended because it is useing twos complement
                // the low 32 bits of the sum are written to address register at arithmetic overflow is not detected
                // imm8 ranges from -128 to 127 and is sign-extended imm8
                // 24 bit instruction
                // AR[t] = AR[s] + (imm8 7 24||imm8)
                printf("\n\tThe instruction is ADDI\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                int8_t imm8 = (opcode >> (int8_t)((CPU->msbFirstOption ? 0 : 20)) & 0x0F);
                CPU->registerFile[CPU->windowOffset + t] = as + imm8;
            }
            else if (r == 0XD)
            {
                // ADDMI     add a register and a shifted 8 bit immediate                                RRI8
                // extends the range of constant additoin often used in conjunction with load and store instructions to extend teh range of the base, pluse
                // offset the calculation.
                // calculates the two's complement 32-bit sum of as and operan imm8. low 32 bits of the sum are written to at
                // no arithmetic overflow is detected
                // the operand encoded in the instruction can have values that are multiples of 256 ranging from -32768 to 32512
                // that is decoded from a sign-extending imm8 and shifting the result left by eight bits
                // AR[t] = AR[s] + (imm8 7 16||imm8||0^8)
                printf("\n\tThe instruction is ADDMI\n");
            }
            else
            {
                printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
            }
        }
        else if (op0 == 0x0)
        {
            switch (op2)
            {
            case 0x8:
            {
                // ADD       add two registers                                                           RRR
                // calculates the two's complement 32-bit sum of as and at the low 32 bits are written to ar
                // arithmetic overflow is not detected
                // 24 bit instruction
                // AR[r] = AR[s] + AR[t]
                printf("\n\tThe instruction is ADD\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                CPU->registerFile[CPU->windowOffset + r] = as + at;
            }
            break;
            case 0x9:
            {
                // ADDX2     add two registers with one shifted left by one                              RRR
                // calculates the two's complement 32-bit sum of as shifted left by one bit and at the low 32bits of the sum are written to ar
                // arithmetic overflow is not detected
                // frequently used for address calculation and as part of sequences to bultiply by small constants
                // AR[r] = (AR[s]30..0||0) + AR[t]
                printf("\n\tThe instruction is ADDX2\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                as = as << 1;
                int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                CPU->registerFile[CPU->windowOffset + r] = as + at;
            }
            break;
            case 0xA:
            {
                // ADDX4     add two registers with one shifted left by two                              RRR
                // Calculates the two's complement 32-bit sum of as shifted left by two bits and address register at.
                // low 32 bits of the sum are written to ar no arithmetic overflow detected. frequently used for address calculation and as part
                // of a sequences to mulitply by small constants
                // AR[r] = (AR[s] 29..0||0^2) + AR[t]
                printf("\n\tThe instruction is ADDX4\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                as = as << 2;
                int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                CPU->registerFile[CPU->windowOffset + r] = as + at;
            }
            break;
            case 0xB:
            {
                // ADDX8     add two registers with one shifted left by three                            RRR
                // calculates teh two's complement 32 bit sum of as shifted left by 3 bits and address register at
                // low 32 bit of the sum are written to ar no arithmetic overflow detected
                // frequently used for address calculation and part of multiplcation sequence for small constants
                // AR[r] = (AR[s] 28..0||0^3) + AR[t]
                printf("\n\tThe instruction is ADDX8\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                as = as << 3;
                int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                CPU->registerFile[CPU->windowOffset + r] = as + at;
            }
            break;
            case 0xC:
            {
                // SUB       subtract two registers                                                      RRR
                // calculates the two's complement 32 bit difference of as and at the low 32 bits of difference are written to address register ar
                // no arithmetic overflow detected
                // AR[r] = AR[s] - AR[t]
                printf("\n\tThe instruction is SUB\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                CPU->registerFile[CPU->windowOffset + r] = as - at;
            }
            break;
            case 0xD:
            {
                // SUBX2     subtract two registers with the un-negated one shifted left by one          RRR
                // calculates the two's complement 32-bit difference of as shifted left byy 1 bit and at. low 32 bits written to ar
                // no arithmetic overflow detected
                // frequently used as part of sequences to multiply byy small constants
                // AR[r] = (AR[s] 30..0||0) - AR[t]
                printf("\n\tThe instruction is SUBX2\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                as = as << 1;
                int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                CPU->registerFile[CPU->windowOffset + r] = as - at;
            }
            break;
            case 0xE:
            {
                // SUBX4     subtract two registers with the un-negated one shifted left by two          RRR
                // calculates the two's complement 32-bit difference of as shifted left by two bits and at. low 32 bits of the difference are written to ar
                // no arithmetic overflow detected. frequently used for sequences to multiply by small constants
                // AR[r] = (AR[s] 29..0||0^2) - AR[t]
                printf("\n\tThe instruction is SUBX4\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                as = as << 2;
                int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                CPU->registerFile[CPU->windowOffset + r] = as - at;
            }
            break;
            case 0xF:
            {
                // SUBX8     subtract two registers with the un-negated one shifted left by three        RRR
                // calculates the two's complement 32-bit difference of as shifted left by 3 bits and at
                // low 32 bits are written to ar no arithmetic overflow detected
                // AR[r] = (AR[s] 28..0||0^3) - AR[t]
                printf("\n\tThe instruction is SUBX8\n");
                int32_t as = (int32_t)CPU->registerFile[CPU->windowOffset + s];
                as = as << 3;
                int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                CPU->registerFile[CPU->windowOffset + r] = as - at;
            }
            break;
            case 0x6:
            {
                if (s == 0x0)
                {
                    // NEG       negate a register                                                           RRR
                    // calculates the two's complement negation of the contents of at and writes it to ar no arithmetic overflow detected.
                    // AR[r] = 0 - AR[t]
                    printf("\n\tThe instruction is NEG\n");
                    int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                    CPU->registerFile[CPU->windowOffset + r] = -at;
                }
                else if (s == 0x1)
                {
                    // ABS       absolute value                                                              RRR
                    // calculates the absolute value of contents of at and writes it to ar no arithmetic overflow detected
                    // AR[r] = if AR[t] 31 then - AR[t] else AR[t]
                    printf("\n\tThe instruction is ABS\n");
                    int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                    CPU->registerFile[CPU->windowOffset + r] = abs(at);
                }
                else
                {
                    printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
                }
            }
            break;
            default:
                printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
                break;
            }
        }
        else
        {
            printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
        }
    }

    static inline void xten_coreBitwiseLogicalInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode) // TODO write tests
    {
        uint32_t s = (opcode >> (CPU->msbFirstOption ? 12 : 8)) & 0x0F;
        uint32_t t = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F;
        uint32_t r = (opcode >> (CPU->msbFirstOption ? 8 : 12)) & 0x0F;
        uint32_t op2 = (opcode >> (CPU->msbFirstOption ? 0 : 20)) & 0x0F;

        switch (op2)
        {
        case 0x1:
            // AND       bitwise AND of two registers    RRR
            // calculates bitwise logical and of as and at writing result to ar
            // AR[r] = AR[s] and AR[t]
            printf("\n\tThe instruction is AND\n");
            CPU->registerFile[CPU->windowOffset + r] = CPU->registerFile[CPU->windowOffset + s] & CPU->registerFile[CPU->windowOffset + t];
            break;
        case 0x2:
            // OR        bitwise OR two registers        RRR
            // calculates  the bitwise logical or of as and at writing result to ar
            // AR[r] = AR[s] or AR[t]
            printf("\n\tThe instruction is OR\n");
            CPU->registerFile[CPU->windowOffset + r] = CPU->registerFile[CPU->windowOffset + s] | CPU->registerFile[CPU->windowOffset + t];
            break;
        case 0x3:
            // XOR       bitwise XOR two registers       RRR
            // calculates the bitwise logical exclusive or of as and at writing result to ar
            // AR[r] = AR[s] xor AR[t]
            printf("\n\tThe instruction is XOR\n");
            CPU->registerFile[CPU->windowOffset + r] = CPU->registerFile[CPU->windowOffset + s] ^ CPU->registerFile[CPU->windowOffset + t];
            break;
        default:
            printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
            break;
        }
    }

    static inline void xten_coreShiftInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode) // TODO test and figure out how to single out EXTUI instruction
    {

        uint32_t s = (opcode >> (CPU->msbFirstOption ? 12 : 8)) & 0x0F;
        uint32_t t = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F;
        uint32_t r = (opcode >> (CPU->msbFirstOption ? 8 : 12)) & 0x0F;
        uint32_t op2 = (opcode >> (CPU->msbFirstOption ? 0 : 20)) & 0x0F;
        uint32_t op1 = (opcode >> (CPU->msbFirstOption ? 4 : 16)) & 0x0F;

        if (op2 == 0x4)
        {
            switch (r)
            {
            case 0x0:
                // SSR       set SAR from register for right shift                                               RRR
                // sets the shift amount register for a right shift the least significant 5 bits of as are written to SAR
                // the most significan tbit of SAR is cleared.
                // sa = AR[s]4..0
                // SAR = 0||sa
                printf("\n\tThe instruction is SSR\n");
                CPU->sar = 0;
                CPU->sar = CPU->sar | (CPU->registerFile[CPU->windowOffset + s] & 0x1F);
                break;
            case 0x1:
                // SSL       set SAR from register for left shift                                                RRR
                // sets the SAR for a left shift like SLL left shift amount is the 5 least significant bits of address register as.
                // 32 minus this amount is written to SAR. using 32 minus the left shift amount causes a right funnel shift and swapped high and low
                // input operands perform a left shift
                // sa=AR[s]4..0
                // SAR = 32 - (0||sa)
                printf("\n\tThe instruction is SSL\n");
                CPU->sar = 0;
                CPU->sar = CPU->sar | (32 - (CPU->registerFile[CPU->windowOffset + s] & 0x1F));
                break;
            case 0x2:
                // SSA8L     use low 2 bits of address register to prepare SAR for SRC assuming little endian    RRR (The L is for little endian not left)
                // sets the SAR for a right shift by multiples of eight(for example little-endian byte alignment). right shift amount is
                // the two least significant bits of as multiplied by eight and written to SAR. this is similar to SSR except the shift amount is multiplied by eight.
                // typically used to set up for an SRC isstruction to shift bytes. may be used with little-endian byte ordering to extract unaligned 332-bit values from non
                // aligned byte address(should not happen in this core architecture)
                // SAR = 0||AR[s]1..0||0^3
                printf("\n\tThe instruction is SSA8L\n");
                CPU->sar = (CPU->registerFile[CPU->windowOffset + s] & 0x3) << 3;
                break;
            case 0x3:
                // SSA8B     use low 2 bits of address register to prepare SAR for SRC assuming big endian       RRR (The B is for Big endian)
                // sets SAR for a left shift by multiples of eight(like for big-endian byyte alignment) left shift amount is the two
                // least significant bits of as multiplied by eight. 32 minus this amount is written to SAR. using 32 minus the left shift amount
                // causes a funnel right shift and swapped high and low input operands to perform a left shift. SSA8B is similar to SSL, except the
                // shift amount is multiplied by eight.
                // typically used to set up SRC instruction to shift bytes may be used with big-endian byte ordering to extract 32-bit balue from a non-alligned byte
                // address(should have no unaligned byte addresses in this core architecture)
                // SAR = 32 - (0||AR[s]1..0||0^3)
                printf("\n\tThe instruction is SSA8B\n");
                CPU->sar = 32 - ((CPU->registerFile[CPU->windowOffset + s] & 0x3) << 3);
                break;
            case 0x4:
                // SRAI      shift right logical by immediate                                                    RRR
                // arithmetically shifts the contents of at right inserting the sign of at on the left by a constant amount encoded in the instruction word in range 0..31
                // sa field is split with 3 bits 3..0 in bits 11..8 and bit for in bit 20 in the instruction word. sa being shift amount.
                // AR[r] = ((AR[t]31)^32||AR[t]) 31+sa..sa
                printf("\n\tThe instruction is SRAI\n");
                {
                    int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                    uint32_t sa = (op2 % 2) << 4 | s;
                    CPU->registerFile[CPU->windowOffset + r] = at >> sa;
                }
                break;
            default:
                if (op1 == 0x1)
                {
                    // SRLI      shift right logical by immediate                                                    RRR
                    // shifts contents of at right inserting zeros on the left by a constant amound encoded in the instruction word in the range 0..15
                    // no SRLI for shifts >= 16. EXTUI replaces these shifts
                    // AR[r] = (0^32||AR[t])31+sa..sa
                    printf("\n\tThe instruction is SRLI\n");
                    {
                        if (s >= 16)
                        {
                            // EXTUI     extract field specified by immediates from a register                               RRR
                            // performs unsigned bit field extraction from a 32 bit register value shifts the contents of address register at right by sa
                            // which is split stored in bits 16 11..8 of the instruction word. shift amount is then anded with a mask of maskimm least-significant
                            // 1 bits and result is written to ar. the number of mask bits can be values  1..16 stored in the op2 field as maskimm-1
                            // bits extracted are sa+op2..sa
                            // this operation is undefined for sa+op2 > 31
                            // mask = 0^21-op2||1^op2+1
                            // AR[r] = (0^32||AR[t]) 31+sa..sa and mask
                        }
                        else
                        {
                            int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                            CPU->registerFile[CPU->windowOffset + r] = at >> s;
                        }
                    }
                }
                printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
                break;
            }
        }
        else
        {
            switch (op2)
            {
            case 0x8:
                // SRC       shift right combined by SAR with two registers as input and one as output           RRR
                // performs a right shift of the concatenation of as and at by SAR shift amount register least significant 32 bits
                // of the shift result are written to address register ar. shifts with wider input than output are called funnel shift. SRC directly peforms right funnel shifts.
                // left funnel shifts are done by swapping the high and low operands to SRC and setting SAR to 32 minus the shift amount. SSL and SSA8B instructions
                // directly implement such SAR settings. SRC is undefined if SAR > 32.
                // sa = SAR 5..0
                // AR[r] = (AR[s]||AR[t]) 31+sa..sa
                printf("\n\tThe instruction is SRC\n");
                {
                    uint64_t concat = ((uint64_t)CPU->registerFile[CPU->windowOffset + s] << 32) | CPU->registerFile[CPU->windowOffset + t];
                    CPU->registerFile[CPU->windowOffset + r] = concat >> CPU->sar;
                }
                break;
            case 0xA:
                // SLL       shift left logical by SAR                                                           RRR
                // shifts the contents of as left by the number of bit positions specified(as 32 minus number of bit positions) in the SAR
                // writing the result to ar. loads SAR wqith 32-shift transforming SLL to be implemented in the SRC funnel shifter using the SLL data as the most
                // significant 32 bits and zero as teh least significant 32 bits.
                // undefined if SAR > 32.
                // sa = SAR5..0
                // AR[r] = (AR[s]||0^32)31+sa..sa
                printf("\n\tThe instruction is SLL\n");
                {
                    CPU->registerFile[CPU->windowOffset + r] = CPU->registerFile[CPU->windowOffset + s] << (32 - CPU->sar);
                }
                break;
            case 0x9:
                // SRL       shift right logical by SAR                                                          RRR
                // shifts contents of at right inserting zeros on the left by the number of bits specified by the SAR and writes results to ar
                // SSR or SSA8B are used to load SAE with the shift amount from an address register.
                // undefined fi SAR > 32
                // sa = SAR5..0
                // AR[r] = (0^32||AR[t])31+sa..sa
                printf("\n\tThe instruction is SRL\n");
                {
                    CPU->registerFile[CPU->windowOffset + r] = CPU->registerFile[CPU->windowOffset + t] >> CPU->sar;
                }
                break;
            case 0xB:
                // SRA       shift right logical by SAR                                                          RRR
                // arighmetically shifts the contents of at right inserting the sign of at on the left by the number of positions specified by SAR
                // and writes results to ar. typically SSR or SSA8B instructions are used to load SAR with the shift amount from an address reigister.
                // result is undefined if SAR>32
                // sa = SAR 5..0
                printf("\n\tThe instruction is SRA\n");
                {
                    int32_t at = (int32_t)CPU->registerFile[CPU->windowOffset + t];
                    CPU->registerFile[CPU->windowOffset + r] = at >> CPU->sar;
                }
                break;
            default:
                if (((opcode >> 5) & 0x7) == 0x2) // TODO fix this instruction trouble figuring out the SA register
                {
                    // EXTUI     extract field specified by immediates from a register                               RRR
                    // performs unsigned bit field extraction from a 32 bit register value shifts the contents of address register at right by sa
                    // which is split stored in bits 16 11..8 of the instruction word. shift amount is then anded with a mask of maskimm least-significant
                    // 1 bits and result is written to ar. the number of mask bits can be values  1..16 stored in the op2 field as maskimm-1
                    // bits extracted are sa+op2..sa
                    // this operation is undefined for sa+op2 > 31
                    // mask = 0^21-op2||1^op2+1
                    // AR[r] = (0^32||AR[t]) 31+sa..sa and mask
                    printf("\n\tThe instruction is EXTUI\n");
                    {
                        uint32_t sa = (op1 % 2) << 4 | s;
                        uint32_t at = CPU->registerFile[CPU->windowOffset + t] >> sa;
                        uint32_t maskimm = op2 + 1;
                        uint32_t mask = ((1U << maskimm) - 1);
                        CPU->registerFile[CPU->windowOffset + r] = at & mask;
                    }
                }
                if (((opcode >> 1) & 0xF) == 0x9)
                {
                    // SSAI      set SAR to immediate value                                                          RRR
                    // Sets the SAR to a constant the shift amount sa field is split with bits 3..0 in bits 11..8 of the instruction word
                    // and bit 4 in bit 4 of the instruction word. primarily useful to set the shift amount for SRC.
                    // SAR = 0||sa
                    printf("\n\tThe instruction is SSAI\n");
                    CPU->sar = (opcode & 0x1F) | s;
                }
                else if (((opcode >> 1) & 0xF) == 0x8)
                {
                    // SLLI      shift left logical by immediate
                    // shifts the contents of as left by constant amount in range 1..31 encoded in the instruction in sa shift amount
                    // sa is split with bits 3..0 in bits 7..4 of the instruction word and bit 4 in bit 20 of the instruction word
                    // sa encoded as 32-shift when the sa field is 0 the result of this instruction is undefined.
                    // asselmbler encodes this instruction as or when the sa is zero
                    // AR[r] = (AR[s]||0^32) 31+sa..sa
                    printf("\n\tThe instruction is SLLI\n");
                    uint32_t sa = (opcode >> 4) & 0xF;
                    sa = sa + ((opcode >> 16) & 0x10);
                    CPU->registerFile[CPU->windowOffset + r] = CPU->registerFile[CPU->windowOffset + s] << sa;
                }
                printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
                break;
            }
        }
    }

    static inline void xten_coreProcessorControlInstructions(Xtensa_lx_CPU *CPU, uint32_t opcode)
    {
        uint32_t t = (opcode >> (CPU->msbFirstOption ? 16 : 4)) & 0x0F;
        uint32_t op2 = (opcode >> (CPU->msbFirstOption ? 0 : 20)) & 0x0F;
        uint32_t sr = (opcode >> (CPU->msbFirstOption ? 8 : 8)) & 0xFF;

        switch (op2)
        {
        case 0x0:
            // RSR       read a special register                                 RSR
            // reads special register descibed in section 3.8.10 on page 45. contents of the special register designated by
            // the 8-bit sr field of the instruction word are written to register at.
            // WSR.* followed by an RSR.* should be separated with ESYNC to guarantee the value written is read back.
            // on some implementations of the processor latency of RSR.* is greater than one cycle so it is advantagous to sechedule
            // other instructions before instructions that use RSR.* results
            // if RSR.* with special numbers >= 64 they are privileged RSR.* for unconfigured registers generally raise an illegal instruction exception
            // sr = if msbFirst then s||r else r||s
            // if sr >= 64 and CRING != 0 then exception (privilegedInstructionCause) if expetion option
            // else tables in section 5.3 on page 208
            printf("\n\tThe instruction is RSR\n");
            if (sr == 0x03)
            { // this is the only special register in the core archetecture accessible this way
                CPU->registerFile[CPU->windowOffset + t] = CPU->sar;
            }
            break;
        case 0x1:
            // WSR       write a special register                                RSR
            // writes the special registers that are described in section 3.8.10 on page 45 section 5.3 page 208
            // has detailed info about this instruction for each register
            // contents of at are written to special register designated by sr field of the instruction word.
            // the point at which WSR to certain registers affects subsequent instructions is not always defined (SAR and ACC are exceptions)
            // in these cases page 208 will clarify things. usually using one of the delay instructions to fix things.
            // sr = if msbFirst then s||r else r||s
            // if sr >= 64 and CRING != 0 then exception(privilegedInstructionCause)
            // else see 208
            printf("\n\tThe instruction is WSR\n");
            if (sr == 0x03)
            { // this is the only special register in the core archetecture accessible this way
                CPU->sar = CPU->registerFile[CPU->windowOffset + t] & 0x1F;
            }
            break;
        case 0x6:
            // XSR       read and write a special register in an exchange        RRR
            /*XSR.* simultaneously reads and writes the special registers that are described in
            Section 3.8.10 “Processor Control Instructions” on page 45. See Section 5.3 on
            page 208 for more detailed information on the operation of this instruction for each
            Special Register.
            The contents of address register at and the Special Register designated by the immediate in the 8-bit sr field of the instruction word are both read. The read address register
            value is then written to the Special Register, and the read Special Register value is written to at. The name of the Special Register is used in place of the ‘*’ in the assembler
            syntax above and the translation is made to the 8-bit sr field by the assembler.
            XSR is an assembler macro for XSR.*, which provides compatibility with the older versions of the instruction containing either the name or the number of the Special Register.
            The point at which XSR.* to certain registers affects subsequent instructions is not always defined (SAR and ACC are exceptions). In these cases, the Special Register Tables
            in Section 5.3 on page 208 explain how to ensure the effects are seen by a particular
            point in the instruction stream (typically involving the use of one of the ISYNC, RSYNC,
            ESYNC, or DSYNC instructions). An XSR.* followed by an RSR.* to the same register
            should be separated with an ESYNC to guarantee the value written is read back. An
            XSR.PS followed by RSIL also requires an ESYNC. In general, the restrictions on XSR.*
            include the union of the restrictions of the corresponding RSR.* and WSR.*.
            Xtensa Instruction Set Architecture (ISA) Reference Manual 567
            XSR.* with Special Register numbers ≥ 64 is privileged. An XSR.* for an unconfigured
            register generally will raise an illegal instruction exception.*/
            // sr = msbFirst then s||r else r||s
            // if sr >= 64 and CRING != 0 then exception(privilegedInstructionCause)
            // else
            //   t0 = AR[t]
            //   t1 = see RSR frame of tables on 208
            printf("\n\tThe instruction is XSR\n");
            if (sr == 0x03)
            { // this is the only special register in the core archetecture accessible this way
                uint32_t at = CPU->registerFile[CPU->windowOffset + t];
                CPU->registerFile[CPU->windowOffset + t] = CPU->sar;
                CPU->sar = at & 0x1F;
            }
            break;
            // USER defined registers have not been implemented at this point in time
        case 0xE:
            //  RUR       read user special register                              ?
            //  reads TIE state that has been grouped into 32-bit quantities by the TIE user_register statement.
            // register number placed in st field of encoded instruction contents of the TIE user_register designated by
            // the 8 bit number 16*s+t are written to address register ar s and t correspond to respective fields of instruction word
            // AR[r] = user_register[st]
            printf("\n\tThe instruction is RUR\n");
            break;
        case 0x3:
            //  WUR       write user special register                             ?
            // writes TIE state that has been grouped into 32-bit quantities by the TIE user_register statement. register
            // number placed in the st field of the encoded instruction. contents of at are written to the TIE user_register designated
            // by the sr field of the instruction word.
            // user_register[sr] = AR[t]
            printf("\n\tThe instruction is WUR\n");
            break;
        default:
            switch (opcode)
            {
                // these last 4 function as nops for now
            case 0x000200:
                //  ISYNC     wait for instruction-Fetch-related changes to reslove   RRR
                /*ISYNC waits for all previously fetched load, store, cache, TLB, WSR.*, and XSR.*
                instructions that affect instruction fetch to be performed before fetching the next instruction. RSYNC, ESYNC, and DSYNC are performed as part of this instruction.
                The proper sequence for writing instructions and then executing them is:
                 write instructions
                 use DHWB to force the data out of the data cache (this step may be skipped if writethrough, bypass, or no allocate stores were used)
                 use ISYNC to wait for the writes to be visible to instruction cache misses
                 use multiple IHI instructions to invalidate the instruction cache for any lines that
                were modified (this step is not appropriate if the affected instructions are in InstRAM
                or cannot be cached)
                 use ISYNC to ensure that fetch pipeline will see the new instructions
                This instruction also waits for all previously executed WSR.* and XSR.* instructions that
                affect instruction fetch or register access processor state, including:
                 WSR.LCOUNT, WSR.LBEG, WSR.LEND
                 WSR.IBREAKENABLE, WSR.IBREAKA[i]
                 WSR.CCOMPAREn
                See the Special Register Tables in Section 5.3 on page 208 and Section 5.7 on
                page 240, for a complete description of the ISYNC instruction’s uses.*/
                // isync()
                printf("\n\tThe instruction is ISYNC\n");
                break;
            case 0x010200:
                //  RSYNC     wait for dispatch related changes to resolve            RRR
                // waits for all perviously fetched WSR instructions to be perfoemd before interpreting the register fields fo the next instruction
                // this operation is also performed as part of ISYNC. ESYNC and DSYNC are peroformed as part of this instruction
                // used after specific WSR calls before using resluts
                // execution of this instruction is specific to the execution pipeline
                printf("\n\tThe instruction is RSYNC\n");
                break;
            case 0x020200:
                //  ESYNC     wait for execution related changes to resolve           RRR
                // waits for all perviously fetched WSR and XSR instructions to be performed before next instruction uses any register values
                // performed as part of ISYNC and RSYNC. DSYNC is performedc as part of this instruction.
                // used after WSR.EPC* instructions specfic to the pipeline.
                printf("\n\tThe instruction is ESYNC\n");
                break;
            case 0x030200:
                //  DSYNC     wait for data memory related changes to resolve         RRR
                // waits for all previously fetched WSR.*, XSR.*, WDTLB, and IDTLB instructions to be performed before interpreting the virtual address
                // of next load or store instruction this is performed as part of ISYNC RSYNC and ESYNC
                // used for WSR.DBREAKC* and WSR.DBREAKA* instructions
                // pipeline specific
                printf("\n\tThe instruction is DSYNC\n");
                break;
            default:
                printf("\nSomething went wrong proceeded to xten_coreMemoryOrderingInstructions without a valid opcode this error could have come from the code being run\n");
                break;
            }
        }
    }

    /**
     * @brief prints the binary representation of a uint32_t
     *
     * This function takes a uint32_t and prints the binary representation with no whitespace.
     *
     * @param uint32_t value to print
     */
    void xten_helper_printBinary(uint32_t value)
    {
        for (int i = 31; i >= 0; i--)
        {
            // Right shift num i times, then bitwise AND with 1 to get the ith bit
            uint32_t bit = (value >> i) & 1;
            printf("%u", bit);
        }
    }

    /**
     * @brief prints the entire register file to console
     *
     * This function takes the register file to print and the offset of the first element of the register file
     * to be included in the register window and uses defines for the size of the register file and the offset
     * to display in a way that makes it clear what elements of the register file are what items in the register window.
     *
     * @param uint32_t* the register file
     * @param uint32_t offset of the first register in the window
     */
    void xten_helper_printRegisters(uint32_t *reg_file, uint32_t offset)
    {
        for (uint32_t i = 0; i < DEFAULT_REGISTER_FILE_SIZE; i++)
        {
            // Check if the current register is in the window
            if (i >= offset && i < offset + REGISTER_WINDOW_SIZE)
            {
                printf("\t* ");
            }
            else
            {
                printf("\t  ");
            }

            // Print register number and value
            printf("Register %2u: 0x%08x\n", i, reg_file[i]);
        }
    }

    /**
     * @brief returns the sign extended version of a uint32_t that represents a smaller value
     *
     * This function takes takes the smaller in size then uint23_t value and calculates the 32 bit version of that value sign extended this may seem confusing
     * since only uint32_t is being used this is because different instructions may use different methods to represent signed values and we want to have control over how it works
     *
     * @param uint32_t smaller value to sign extend.
     * @param int number of bits in the smaller in initial value
     * @return uint32_t the sign extended version of that value.
     */
    uint32_t xten_helper_signExtend32Bits(uint32_t value, int bits)
    {
        uint32_t mask = 1U << (bits - 1);                        // sets a 1 in the location of the sign bit
        uint32_t signExtendedValue = value & ((1U << bits) - 1); // makes sure the value is zero extended if it is not already
        return (signExtendedValue ^ mask) - mask;
    }

#ifdef __cplusplus
}
#endif

#endif
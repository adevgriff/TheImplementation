// This file is all the definitions of special registers
// Best to only figure out how to implement these when needed since most
//   are for extended parts of this architecture

/*Special registers are used in RSR, WSR, and XSR instructions
    there are a couple user registers that are used in different functions there numbers may
    repeat they are used in RUR and WUR instructions*/

#define ACCHI_NUM 17 // Accumulator high bits
#define ACCLO_NUM 16 // Accumulator low
#define AR_NUM -1    // N/A General-purpose register file
#define BR_NUM 4     // Boolean registers

#define CCOMPARE0_NUM 240
#define CCOMPARE1_NUM 241
#define CCOMPARE2_NUM 242
#define CCOMPARE3_NUM 243 // cycle number to generate interrupt

#define CCOUNT_NUM 234   // Cycle count
#define CPENABLE_NUM 224 // coprocessor enable bits

#define DBREAKA0_NUM 144
#define DBREAKA1_NUM 145
#define DBREAKA2_NUM 146
#define DBREAKA3_NUM 147
#define DBREAKA4_NUM 148
#define DBREAKA5_NUM 149
#define DBREAKA6_NUM 150
#define DBREAKA7_NUM 151
#define DBREAKA8_NUM 152
#define DBREAKA9_NUM 153
#define DBREAKA10_NUM 154
#define DBREAKA11_NUM 155
#define DBREAKA12_NUM 156
#define DBREAKA13_NUM 157
#define DBREAKA14_NUM 158
#define DBREAKA15_NUM 159 // data break address

#define DBREAKC0_NUM 160
#define DBREAKC1_NUM 161
#define DBREAKC2_NUM 162
#define DBREAKC3_NUM 163
#define DBREAKC4_NUM 164
#define DBREAKC5_NUM 165
#define DBREAKC6_NUM 166
#define DBREAKC7_NUM 167
#define DBREAKC8_NUM 168
#define DBREAKC9_NUM 169
#define DBREAKC10_NUM 170
#define DBREAKC11_NUM 171
#define DBREAKC12_NUM 172
#define DBREAKC13_NUM 173
#define DBREAKC14_NUM 174
#define DBREAKC15_NUM 175 // data break control

#define DEBUGCAUSE_NUM 133 // cause of last debug exception
#define DDR_NUM 104        // debug data register
#define DEPC_NUM 192       // Double exception PC

#define EPC1_NUM 177 // level 1 exception PC
#define EPC2_NUM 178
#define EPC3_NUM 179
#define EPC4_NUM 180
#define EPC5_NUM 181
#define EPC6_NUM 182
#define EPC7_NUM 183
#define EPC8_NUM 184
#define EPC9_NUM 185
#define EPC10_NUM 186
#define EPC11_NUM 187
#define EPC12_NUM 188
#define EPC13_NUM 189
#define EPC14_NUM 190
#define EPC15_NUM 191 // high level exception PC

#define EPS2_NUM 194
#define EPS3_NUM 195
#define EPS4_NUM 196
#define EPS5_NUM 197
#define EPS6_NUM 198
#define EPS7_NUM 199
#define EPS8_NUM 200
#define EPS9_NUM 201
#define EPS10_NUM 202
#define EPS11_NUM 203
#define EPS12_NUM 204
#define EPS13_NUM 205
#define EPS14_NUM 206
#define EPS15_NUM 207 // high level exception PS

#define EXCCAUSE_NUM 232 // casue of last exception/level 1 interrupt
#define EXCSAVE1_NUM 209 // level 1 exception save location
#define EXCSAVE2_NUM 210
#define EXCSAVE3_NUM 211
#define EXCSAVE4_NUM 212
#define EXCSAVE5_NUM 213
#define EXCSAVE6_NUM 214
#define EXCSAVE7_NUM 215
#define EXCSAVE8_NUM 216
#define EXCSAVE9_NUM 217
#define EXCSAVE10_NUM 218
#define EXCSAVE11_NUM 219
#define EXCSAVE12_NUM 220
#define EXCSAVE13_NUM 221
#define EXCSAVE14_NUM 222
#define EXCSAVE15_NUM 223
#define EXCSAVE16_NUM 224 // High level exception save location

#define EXCVADDR_NUM 238 // Exception virtual address
#define FCR_NUM 232      // Floating point control register
#define FR_NUM -1        // Floating-point registers
#define FSR_NUM 233      // Floating point status register

#define IBREAKA0_NUM 128
#define IBREAKA1_NUM 129
#define IBREAKA2_NUM 130
#define IBREAKA3_NUM 131
#define IBREAKA4_NUM 132
#define IBREAKA5_NUM 133
#define IBREAKA6_NUM 134
#define IBREAKA7_NUM 135
#define IBREAKA8_NUM 136
#define IBREAKA9_NUM 137
#define IBREAKA10_NUM 138
#define IBREAKA11_NUM 139
#define IBREAKA12_NUM 140
#define IBREAKA13_NUM 141
#define IBREAKA14_NUM 142
#define IBREAKA15_NUM 143 // instruction break address

#define IBREAKENABLE_NUM 96 // Instruction break enable bits
#define ICOUNT_NUM 236      // Instruction count
#define ICOUTNLEVEL_NUM 237 // instruction count level
#define INTENABLE_NUM 228   // Interrupt enable bits
#define INTCLEAR_NUM 227    // clear request bits in Interrupt
#define INTERRUPT_NUM 226   // read interrupt request bits

#define INTSET_NUM 226 // set request bits in interrupt
#define LBEG_NUM 0     // loop begin address
#define LCOUNT_NUM 2   // loop count
#define LEND_NUM 1     // loop end
#define LITBASE_NUM 5  // literal base

#define MISC0_NUM 244 - 247
#define MISC1_NUM 245
#define MISC2_NUM 246
#define MISC3_NUM 247 // MISC register 0-3

#define M0_NUM 32
#define M1_NUM 33
#define M2_NUM 34
#define M3_NUM 35 // MAC16 data registers

#define PC_NUM -1          // Program counter
#define PRID_NUM 235       // Processor ID
#define PS_NUM 230         // Processor State
#define SAR_NUM 3          // Shift amount register
#define SAV_NUM 11         // Shift amount valid
#define SCOMPARE1_NUM 12   // Expected data value for S32C1I
#define WindowBase_NUM 72  // base of current AR window
#define WindowStart_NUM 73 // Call window start bits
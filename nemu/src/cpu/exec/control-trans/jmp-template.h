#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute() {
    DATA_TYPE_S displacement = op_src->val;
    if (op_src->type == OP_TYPE_REG || op_src->type == OP_TYPE_MEM) 
    {
        cpu.eip = displacement - concat(decode_rm_, SUFFIX)(cpu.eip+1) - 1;
        print_asm_no_template1();
    } else {
        print_asm("jmp %x", cpu.eip + 1 + DATA_BYTE + displacement);
        cpu.eip += displacement;
    }
}
make_instr_helper(i)
make_instr_helper(rm)

#if DATA_BYTE == 4
make_helper(ljmp) {
    uint32_t op1 = instr_fetch(eip + 1, 4) - 7;
    uint16_t op2 = instr_fetch(eip + 5, 2);
    cpu.eip = op1;
    cpu.cs.val = op2;
    loadSregCache(R_CS);

    print_asm("ljmp %x, 0x%x", op2, op1+7);
    return 7;
}
#endif

#include "cpu/exec/template-end.h"
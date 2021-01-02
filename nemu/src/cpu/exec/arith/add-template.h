#include "cpu/exec/template-start.h"
#define instr add

static void do_execute() {
    DATA_TYPE result = op_dest->val + op_src->val;
    OPERAND_W(op_dest, result);
    concat(update_, SUFFIX)(result);
    int len = (DATA_BYTE << 3) - 1;
    cpu.CF = (result < op_dest->val);
    cpu.OF = ((op_dest->val >> len) == (op_src->val >> len) && (op_dest->val >> len) != cpu .SF);
    print_asm_no_template2();
}

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"
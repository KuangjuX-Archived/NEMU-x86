#include "cpu/exec/template-start.h"

#define instr lods

make_helper(concat(lods_, SUFFIX)) {
	// REG(R_EAX) = swaddr_read(reg_l(R_ESI), DATA_BYTE);
	// if (cpu.DF == 0) reg_l (R_ESI) += DATA_BYTE;
	// else reg_l (R_ESI) -= DATA_BYTE;
	
	// print_asm("loads%s", str(SUFFIX));
	// return 1;

	REG(R_EAX) = MEM_R(reg_l(R_ESI));
    if (cpu.DF == 0) reg_l(R_ESI) += DATA_BYTE;
    else reg_l(R_ESI) -= DATA_BYTE;
	print_asm_template2();
    return 1;
}

#include "cpu/exec/template-end.h"


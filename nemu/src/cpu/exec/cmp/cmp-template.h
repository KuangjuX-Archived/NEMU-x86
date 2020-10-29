#include "cpu/exec/template-start.h"

#define instr cmp

void do_execute() {
	DATA_TYPE ans = op_dest->val - op_src->val;
	concat(updateCPU_, SUFFIX) (ans);
	cpu.ZF = !ans;
	cpu.CF = op_dest->val < op_src->val;
	cpu.SF = ans >> ((DATA_BYTE<<3)-1);
	int tmp1 = (op_dest -> val) >> ((DATA_BYTE<<3)-1);
	int tmp2 = (op_src -> val) >> ((DATA_BYTE<<3)-1);
	cpu.OF = (tmp1!=tmp2 && tmp2 == cpu.SF);
	ans ^= ans>>4;
	ans ^= ans>>2;
	ans ^= ans>>1;
	ans &= 1;
	cpu.PF = !ans;
	print_asm_template2();
}

make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)
make_instr_helper(i2a)

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

#include "cpu/exec/template-end.h"
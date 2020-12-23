#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute () {
	DATA_TYPE_S imm = op_src -> val;
    if (op_src -> type == OP_TYPE_IMM){
        cpu.eip += imm;
    }else {
        int len = concat(decode_rm_, SUFFIX)(cpu.eip + 1);
        cpu.eip = imm - len - 1;
    }
    print_asm_template1();
}

make_instr_helper(i)
make_instr_helper(rm)

#if DATA_BYTE == 4

extern SegmentDescriptor *seg_desc;
SegmentDescriptor new_seg_desc;

make_helper(ljmp){
    seg_desc = &new_seg_desc;

    uint32_t op1 = instr_fetch(cpu.eip + 1,4);
    uint16_t op2 = instr_fetch(cpu.eip + 1 + 4,2);
    
    cpu.eip = op1 - 7;
    cpu.CS.selector = op2;

    //current_sreg = R_CS;

    // printf("%x\n",cpu.eip);
    // printf("%x\n",instr_fetch(cpu.eip,1));
    uint16_t idx = cpu.CS.selector >> 3;//index of sreg

	Assert((idx << 3) <= cpu.GDTR.limit,"Segement Selector Is Out Of The Limit!");
    
	uint32_t chart_addr = cpu.GDTR.base + (idx << 3);//chart addr
    
	seg_desc -> part1 = lnaddr_read(chart_addr, 4);
	seg_desc -> part2 = lnaddr_read(chart_addr + 4, 4);

    // printf("%x     %x\n",seg_desc -> part1,seg_desc -> part2);

	Assert(seg_desc -> p == 1, "Segement Not Exist!");//p bit, whether seg_desc exists

    uint32_t bases = 0;
	
    //printf("%x %x %x\n",seg_desc -> base1,seg_desc -> base2,seg_desc -> base3);

	bases |= ((uint32_t)seg_desc -> base1);
	bases |= ((uint32_t)seg_desc -> base2)<< 16;
	bases |= ((uint32_t)seg_desc -> base3)<< 24;
	//printf("%x\n",instr_fetch(cpu.eip,1));
    cpu.CS.base = bases;

    //printf("%x\n",instr_fetch(cpu.eip,1));

	uint32_t limits = 0;
	limits |= ((uint32_t)seg_desc -> limit1);
	limits |= ((uint32_t)seg_desc -> limit2) << 16;
	limits |= ((uint32_t)0xfff) << 24;
    cpu.CS.limit = limits;


	if (seg_desc -> g == 1) cpu.CS.limit <<= 12;//G = 0, unit = 1B;G = 1, unit = 4KB
    print_asm("ljump %x %x",op2,op1);
    return 1 + 6;    
}
#endif
#include "cpu/exec/template-end.h"
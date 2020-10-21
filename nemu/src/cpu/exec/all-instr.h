#include "prefix/prefix.h"

#include "data-mov/mov.h"
#include "data-mov/xchg.h"
#include "data-mov/movs.h"
#include "data-mov/movsb.h"
#include "data-mov/movsw.h"
#include "data-mov/movzb.h"
#include "data-mov/movzw.h"
#include "data-mov/cmovs.h"

#include "arith/dec.h"
#include "arith/inc.h"
#include "arith/neg.h"
#include "arith/imul.h"
#include "arith/mul.h"
#include "arith/idiv.h"
#include "arith/div.h"

#include "logic/and.h"
#include "logic/or.h"
#include "logic/not.h"
#include "logic/xor.h"
#include "logic/sar.h"
#include "logic/shl.h"
#include "logic/shr.h"
#include "logic/shrd.h"

#include "string/rep.h"

#include "misc/misc.h"

#include "special/special.h"

#include "call/call.h"

#include "jump/jmp.h"
#include "jump/ja.h"
#include "jump/jbe.h"
#include "jump/je.h"
#include "jump/jne.h"
#include "jump/js.h"
#include "jump/jns.h"
#include "jump/jl.h"
#include "jump/jge.h"
#include "jump/jle.h"
#include "jump/jg.h"

#include "push/push.h"
#include "pop/pop.h"
#include "ret/ret.h"

#include "add/add.h"
#include "add/adc.h"

#include "sub/sbb.h"
#include "sub/sub.h"

#include "cmp/cmp.h"

#include "test/test.h"

// #include "lea/lea.h"

#include "leave/leave.h"

#include "lods/lods.h"

#include "stos/stos.h"

#include "scas/scas.h"

#include "set/setne.h"

#include "nop/nop.h"

#include "clt/clt.h"


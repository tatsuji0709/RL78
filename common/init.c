//=====================================================================//
/*!	@file
	@brief	R78 ハードウェアー初期化 @n
			Copyright 2016 Kunihito Hiramatsu
	@author	平松邦仁 (hira@rvf-rc45.net)
*/
//=====================================================================//
#include <common/vect.h>

extern void rl78_run_preinit_array(void);
extern void rl78_run_init_array(void);
extern void rl78_run_fini_array(void);

void init()
{
	rl78_run_preinit_array();
	rl78_run_init_array();
	rl78_run_fini_array();

	ei();
}
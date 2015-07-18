#include <demosystem.h>

#include <discoball_part.h>
#include <tunnel_part.h>
#include <pqtorus_part.h>
#include <greets_part.h>
#include <end_part.h>

////BOUB
//#include <nds.h>
//#include <maxmod9.h>
//#include "soundbank_bin.h"

#define NB_PARTS 5

int main(void)
{   
	T_Part parts[NB_PARTS];
	
	CreateDiscoballPart(	&(parts[0]) );
	CreateTunnelPart(		&(parts[1]) );
	CreatePQTorusPart(		&(parts[2]) );
	CreateGreetsPart(		&(parts[3]) );
	CreateEndPart(			&(parts[4]) );
	
	DemoInit(parts, NB_PARTS);
	DemoPlay();

//	//BOUB
//	mmInitDefaultMem( (mm_addr)soundbank_bin );
//	mmSelectMode(MM_MODE_C);
//	mmLoad(0);
//	mmStart(0, MM_PLAY_LOOP);

    return 0;
}


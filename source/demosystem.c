#include <nds.h>
#include <demosystem.h>

#include <maxmod9.h>
#include "soundbank_bin.h"


static T_Part* DemoParts;
static int NbParts;
static int CurrentPart;


//timer counter (in milliseconds)
int32 TimerMillis=0;

/* Interruption timer en millisecondes */
void _TimerMillisInterrupt() 
{
    TimerMillis++;
}


/* Initialisation ************************************************************* 
 * --------------
 * Paramètres :
 * - parts : les parts qui devront être jouées
 * - nb_parts : le nombre de parts
 ******************************************************************************/
void DemoInit(T_Part* parts, int nb_parts)
{
	TimerMillis = 0;

	DemoParts = parts;
	NbParts = nb_parts;
	CurrentPart = 0;

	mmInitDefaultMem( (mm_addr)soundbank_bin );
	mmSelectMode(MM_MODE_C);
	mmLoad(0);

	// Initialisation du timer
	// Millisecond Timer refresh IRQ
    TIMER1_CR = 0;
    TIMER1_DATA = TIMER_FREQ(0x409);
    TIMER1_CR = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;

	irqEnable(IRQ_TIMER1);
    irqSet(IRQ_TIMER1, _TimerMillisInterrupt);
    
    glInit();
}

void DemoCreatePart(T_Part* part, PartInitFunc init, PartExecFunc exec, PartDeinitFunc deinit, SynchroHandler sync, int duration)
{
	if(part==NULL) return;
	
	part->init = init;
	part->exec = exec;
	part->deinit = deinit;
	part->sync = sync;
	
	part->duration = duration;
}

/* Avance à la part suivante **************************************************
 * -------------------------
 ******************************************************************************/
void DemoAdvancePart(void)
{
	if(CurrentPart < NbParts) {
		if(CurrentPart >= 0) {
			/* Deinit la part courante */		
			if(DemoParts[CurrentPart].deinit != NULL) {
				DemoParts[CurrentPart].deinit();
			}
			/* Unregister le handler de synchro */
	    	mmSetEventHandler(NULL);
		}
	
		CurrentPart++; // Avance
		
		if(CurrentPart < NbParts) {		
			/* Init la prochaine part */
			if(DemoParts[CurrentPart].init != NULL) {
				DemoParts[CurrentPart].init();
			}
			
			if(DemoParts[CurrentPart].sync != NULL) {
				/* Register le nouveau handler de synchro */
				mmSetEventHandler(DemoParts[CurrentPart].sync);
			}
			
			/* Initialise le temps de début de la part */
			DemoParts[CurrentPart].start_time = TimerMillis;
		}
	}
}

/* Exécute la demo ************************************************************
 * ---------------
 ******************************************************************************/
void DemoPlay(void)
{
	mmStart(0, MM_PLAY_LOOP);
	
	//CurrentPart = -1;
	//DemoAdvancePart();
	CurrentPart = 0;
	DemoParts[0].init();
	mmSetEventHandler(DemoParts[CurrentPart].sync);	
	DemoParts[0].start_time = TimerMillis;
	
	while(1) {
		if(CurrentPart < NbParts) {
			DemoParts[CurrentPart].exec(TimerMillis - DemoParts[CurrentPart].start_time);
			
			if(DemoParts[CurrentPart].duration != 0) {
				if((TimerMillis - DemoParts[CurrentPart].start_time) >= DemoParts[CurrentPart].duration) {
					//DemoAdvancePart();
					DemoParts[CurrentPart].deinit();
					mmSetEventHandler(NULL);
					CurrentPart++;
					if(CurrentPart == NbParts) {
						ledBlink(PM_LED_BLINK);
						return;
					}
					DemoParts[CurrentPart].init();
					mmSetEventHandler(DemoParts[CurrentPart].sync);
					DemoParts[CurrentPart].start_time = TimerMillis;
				}
			}
		}
		else {
			ledBlink(PM_LED_BLINK);
			return;
		}
	}	
}

int GetTimeMillis()
{
	return TimerMillis;
}

int GetTimeElapsed(int since)
{
	return (TimerMillis - since);
}

#ifndef DEMOSYSTEM_H
#define DEMOSYSTEM_H

#include <maxmod9.h>

#include <usings.h>

/******************************************************************************
 ******************************************************************************
                              SYNCHROS
  Effets lancés par les commandes de la musique, pour de parfaites synchros.
 ******************************************************************************
 ******************************************************************************/

/* mm_word SynchroHandler(mm_word msg, mm_word param) */
typedef mm_word (*SynchroHandler) (mm_word, mm_word);


 

/******************************************************************************
 ******************************************************************************
                              PARTS
  Ensemble d'effets exécutés ensemble pendant une certaine durée.
  Peuvent être accompagnées de synchros déclenchées par les commandes de la
  musique.
 ******************************************************************************
 ******************************************************************************/
/* Type pour la fonction d'initialisation d'une part (appelée une fois à l'initialisation 
   de la part) */
typedef void (*PartInitFunc) (void);

/* Type pour la fonction d'exécution d'une part */
typedef void (*PartExecFunc) (int);

/* Type pour la déinitialisation d'une part */
typedef void (*PartDeinitFunc) (void);

/* Une part */ 
typedef struct t_part {
	PartInitFunc  init ;
    PartExecFunc  exec ;
    PartDeinitFunc deinit;
    
    SynchroHandler sync;

	int start_time; // Début de la part, en ms - enregistré automatiquement au début de la part
    int duration ; // durée de la part, en ms
    
} T_Part ;

void DemoCreatePart(T_Part* part, PartInitFunc init, PartExecFunc exec, PartDeinitFunc deinit, SynchroHandler sync, int duration);
 

/******************************************************************************
 ******************************************************************************
                              MOTEUR
 ******************************************************************************
 ******************************************************************************/
                             
/* Initialisation ************************************************************* 
 * --------------
 * Paramètres :
 * - parts : les parts qui devront être jouées
 * - nb_parts : le nombre de parts
 ******************************************************************************/
void DemoInit(T_Part* parts, int nb_parts);

/* Avance à la part suivante **************************************************
 * -------------------------
 ******************************************************************************/
void DemoAdvancePart(void);

/* Exécute la demo ************************************************************
 * ---------------
 ******************************************************************************/
void DemoPlay(void);

int GetTimeMillis();
int GetTimeElapsed(int since);


#endif // DEMOSYSTEM_H

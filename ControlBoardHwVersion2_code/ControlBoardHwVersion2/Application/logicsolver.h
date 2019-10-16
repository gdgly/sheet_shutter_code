/*********************************************************************************
* FileName: logicsolver.h
* Description:
* This source file contains the prototype definition of all the services of ....
* Version: 0.1D
*
*
**********************************************************************************/

/****************************************************************************
 * Copyright 2014 Bunka Shutters.
 * This program is the property of the Bunka Shutters
 * and it shall not be reproduced, distributed or used
 * without permission of an authorized company official.
 * This is an unpublished work subject to Trade Secret
 * and Copyright protection.
*****************************************************************************/


/****************************************************************************
 *  Modification History
 *
 *  Revision		Date                  Name          			Comments
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
****************************************************************************/


/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables:
****************************************************************************/
//enum Logic_Solver_State
//{
//
//	Logic_Solver_Init = 0,
//	Logic_Solver_Power_ON_Init,
//	Logic_Solver_Drive_Instalation,
//	Logic_Solver_Drive_Run
//
//};
//
//extern enum Logic_Solver_State eLogic_Solver_State;

/****************************************************************************/


/******************************************************************************
 * logicSolverToTestCMDr
 *
 * Function Description:
 * This function is temporarily created to test CMDr. It clears the LS to CMDr
 * command being activated in main.c file for functional testing.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void logicSolverToTestCMDrCMDi(void);

/********************************************************************************/
/******************************************************************************
 * dataHandlerToTestCMDi
 *
 * Function Description:
 * This function is temporarily created to test CMDi. It checks if a command is
 * received from CMDi and responds accordingly.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void dataHandlerToTestCMDi(void);

/******************************************************************************
 * logicSolver
 *
 * Function Description:
 * This function handles the Core Logic of the Shutter
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void logicSolver(void);
void wirelesslogin_cyw(void);

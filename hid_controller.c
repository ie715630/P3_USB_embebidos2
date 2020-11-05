/*
 * hid_controller.c
 *
 *  Created on: Nov 5, 2020
 *      Author: sergio_mndz
 */

#include "hid_controller.h"

static int global_state = 0;

bool_t openPaint(uint8_t* buffer)
{
	static bool_t opened = FALSE;

	switch(global_state)
	{
	//Windows + R
	case STATE_0:
	{
		buffer[3] = KEY_LEFT_GUI;
		buffer[4] = KEY_R;
	}
	break;
	//Escribir MSPaint y presionar enter
	case STATE_1:
	{
		buffer[3] = KEY_M;
	}
	break;
	case STATE_2:
	{
		buffer[3] = KEY_S;
	}
	break;
	case STATE_3:
	{
		buffer[3] = KEY_P;
	}
	break;
	case STATE_4:
	{
		buffer[3] = KEY_A;
	}
	break;
	case STATE_5:
	{
		buffer[3] = KEY_I;
	}
	break;
	case STATE_6:
	{
		buffer[3] = KEY_N;
	}
	break;
	case STATE_7:
	{
		buffer[3] = KEY_T;
	}
	break;
	case STATE_8:
	{
		buffer[3] = KEY_ENTER;
		opened = TRUE;
	}
	break;
	default:
		break;
	}
	global_state = (STATE_8 == global_state) ? STATE_0 : (global_state + 1);

	return opened;
}

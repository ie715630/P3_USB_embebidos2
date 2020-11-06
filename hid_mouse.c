/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_hid.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "composite.h"

#include "hid_mouse.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SIZE_DRAW 50

typedef enum{
	DOWN,
	DOWN_1,
	DOWN_2,
	LEFT,
	RIGHT_1,
	RIGHT_2,
	COMPLETE
} number_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static usb_status_t USB_DeviceHidMouseAction(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_MouseBuffer[USB_HID_MOUSE_REPORT_LENGTH];
static usb_device_composite_struct_t *s_UsbDeviceComposite;
static usb_device_hid_mouse_struct_t s_UsbDeviceHidMouse;

/*******************************************************************************
 * Code
 ******************************************************************************/

uint8_t draw_number()
{
	uint8_t return_value = 0;
	static uint16_t counter = 0;
	static substate = 0;
	static uint16_t size = 0;

	s_UsbDeviceHidMouse.buffer[0] = 1U; //click

	switch(substate)
	{
		case 0:
			s_UsbDeviceHidMouse.buffer[1] = 0U; //click
			s_UsbDeviceHidMouse.buffer[2] = 0U; //x
			s_UsbDeviceHidMouse.buffer[3] = 1U; //y
			size = 250;
		break;
		case 1:
			s_UsbDeviceHidMouse.buffer[1] = 1U; //click
			size = 1;
			break;
		case 2:
			s_UsbDeviceHidMouse.buffer[2] = 1;
			s_UsbDeviceHidMouse.buffer[3] = 0U; //y
			size = 75;
			break;
		case 3:
			s_UsbDeviceHidMouse.buffer[2] = 0U; //x
			s_UsbDeviceHidMouse.buffer[3] = 1U; //y
			size = 400;
			break;
		case 4:
			s_UsbDeviceHidMouse.buffer[2] = (uint8_t)(0xFFU);
			s_UsbDeviceHidMouse.buffer[3] = 0U; //y
			size = 150;
			break;
		case 5:
			s_UsbDeviceHidMouse.buffer[2] = 1;
			s_UsbDeviceHidMouse.buffer[3] = 0U; //y
			size = 300;
			break;
		default:
			s_UsbDeviceHidMouse.buffer[1] = 0U; //click
			s_UsbDeviceHidMouse.buffer[2] = 0U; //x
			s_UsbDeviceHidMouse.buffer[3] = 0U; //y
			return_value = 1;
			counter = 0;
			break;
	}

	if(counter<size)
	{
		counter++;
	}
	else
	{
		counter = 0;
		substate++;
	}

	return return_value;
}

uint8_t move_mouse_left_win()
{
	static uint16_t counter = 0;
	uint8_t return_value = 0;

	counter++;

	s_UsbDeviceHidMouse.buffer[0] = 1U;

	if (counter < 2000)
	{
		s_UsbDeviceHidMouse.buffer[1] = 0U;
		s_UsbDeviceHidMouse.buffer[2] = (uint8_t)(0xFFU);
		s_UsbDeviceHidMouse.buffer[3] = 0U;
	}
	else if(counter < 2010)
	{
		s_UsbDeviceHidMouse.buffer[1] = 1U;
		s_UsbDeviceHidMouse.buffer[2] = 0U;
		s_UsbDeviceHidMouse.buffer[3] = 0U;
	}
	else
	{
		s_UsbDeviceHidMouse.buffer[1] = 0U;
		s_UsbDeviceHidMouse.buffer[2] = 0U;
		s_UsbDeviceHidMouse.buffer[3] = 0U;
		return_value = 1;
		counter = 0;
	}

	return return_value;
}


uint8_t move_mouse_right_win()
{
	static uint16_t counter = 0;
	uint8_t return_value = 0;

	counter++;

	s_UsbDeviceHidMouse.buffer[0] = 0x1U;

	if (counter < 2000)
	{
		s_UsbDeviceHidMouse.buffer[1] = 0U;
		s_UsbDeviceHidMouse.buffer[2] = 1U;
		s_UsbDeviceHidMouse.buffer[3] = 0U;
	}
	else if(counter < 2010)
	{
		s_UsbDeviceHidMouse.buffer[1] = 1U;
		s_UsbDeviceHidMouse.buffer[2] = 0U;
		s_UsbDeviceHidMouse.buffer[3] = 0U;
	}
	else
	{
		s_UsbDeviceHidMouse.buffer[1] = 0U;
		s_UsbDeviceHidMouse.buffer[2] = 0U;
		s_UsbDeviceHidMouse.buffer[3] = 0U;
		return_value = 1;
		counter = 0;
	}

	return return_value;
}

extern void state_machine();

/* Update mouse pointer location. Draw a rectangular rotation*/
static usb_status_t USB_DeviceHidMouseAction(void)
{

	state_machine();

    return USB_DeviceHidSend(s_UsbDeviceComposite->hidMouseHandle, USB_HID_MOUSE_ENDPOINT_IN,
                             s_UsbDeviceHidMouse.buffer, USB_HID_MOUSE_REPORT_LENGTH);
}

/* The device HID class callback */
usb_status_t USB_DeviceHidMouseCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;

    switch (event)
    {
        case kUSB_DeviceHidEventSendResponse:
            if (s_UsbDeviceComposite->attach)
            {
                return USB_DeviceHidMouseAction();
            }
            break;
        case kUSB_DeviceHidEventGetReport:
        case kUSB_DeviceHidEventSetReport:
        case kUSB_DeviceHidEventRequestReportBuffer:
            error = kStatus_USB_InvalidRequest;
            break;
        case kUSB_DeviceHidEventGetIdle:
        case kUSB_DeviceHidEventGetProtocol:
        case kUSB_DeviceHidEventSetIdle:
        case kUSB_DeviceHidEventSetProtocol:
            break;
        default:
            break;
    }

    return error;
}

/* The device callback */
usb_status_t USB_DeviceHidMouseSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        return USB_DeviceHidMouseAction(); /* run the cursor movement code */
    }
    return kStatus_USB_Error;
}

/* Set interface */
usb_status_t USB_DeviceHidMouseSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    if (USB_HID_KEYBOARD_INTERFACE_INDEX == interface)
    {
        return USB_DeviceHidMouseAction(); /* run the cursor movement code */
    }
    return kStatus_USB_Error;
}

/* Initialize the HID mouse */
usb_status_t USB_DeviceHidMouseInit(usb_device_composite_struct_t *deviceComposite)
{
    s_UsbDeviceComposite       = deviceComposite;
    s_UsbDeviceHidMouse.buffer = s_MouseBuffer;
    return kStatus_USB_Success;
}

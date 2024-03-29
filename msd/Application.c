// Chapter3Example2 - add Console Out to starter example
//
// john@usb-by-example.com
//

#include <stdbool.h>
#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);

CyU3PThread ApplicationThreadHandle;	// Handle to my Application Thread
uint16_t Delay;							// Determines LED blink rate


void GPIO_InterruptCallback(uint8_t gpioId)
{
	if (gpioId == Button) Delay = (Delay == 1000) ? 100 : 1000;
}

void ApplicationThread(uint32_t Value)
{
    CyU3PGpioClock_t GpioClock;
    CyU3PGpioSimpleConfig_t GpioConfig;
    uint32_t Counter = 0;
    CyU3PReturnStatus_t Status;

    // Spin up a Console Out function to display progress message
    Status = InitializeDebugConsole();
    CheckStatus("Start Debug Console", Status);

    // Since this application uses GPIO then I must start the GPIO clocks
    GpioClock.fastClkDiv = 2;
    GpioClock.slowClkDiv = 0;
    GpioClock.simpleDiv = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
    GpioClock.clkSrc = CY_U3P_SYS_CLK;
    GpioClock.halfDiv = 0;
    // Initialize the GPIO driver and register a Callback for interrupts
	Status = CyU3PGpioInit(&GpioClock, GPIO_InterruptCallback);
	CheckStatus("Initialize GPIO", Status);

    // Configure LED and Button GPIOs
	// LED is on UART_CTS which is currently been assigned to the UART driver, claim it back
    CyU3PDeviceGpioOverride(LED, true);
    CyU3PMemSet((uint8_t *)&GpioConfig, 0, sizeof(GpioConfig));
    GpioConfig.driveLowEn = true;
    GpioConfig.driveHighEn = true;
    CyU3PGpioSetSimpleConfig(LED, &GpioConfig);
    CheckStatus("Configure LED", Status);
    CyU3PMemSet((uint8_t *)&GpioConfig, 0, sizeof(GpioConfig));
	GpioConfig.inputEn = true;
	GpioConfig.intrMode = CY_U3P_GPIO_INTR_NEG_EDGE;
    CyU3PGpioSetSimpleConfig(Button, &GpioConfig);
    CheckStatus("Configure Button", Status);

    Delay = 1000;
    CyU3PDebugPrint(4, "\r\nMain loop now running\r\n");
    while (1)
    {
    	CyU3PThreadSleep(Delay);
    	CyU3PGpioSetValue(LED, (1 & Counter++));
    	CyU3PDebugPrint(4, "%d, ", Counter);
    }
}

// ApplicationDefine function called by RTOS to startup the application
void CyFxApplicationDefine(void)
{
    void *StackPtr = NULL;
    uint32_t Status;

    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACKSIZE);
    Status = CyU3PThreadCreate (&ApplicationThreadHandle, // Handle to my Application Thread
            "15:Chapter3_Example2",				// Thread ID and name
            ApplicationThread,     				// Thread entry function
            42,									// Parameter passed to Thread
            StackPtr,							// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACKSIZE,		// Allocated thread stack size
            APPLICATION_THREAD_PRIORITY,		// Thread priority
            APPLICATION_THREAD_PRIORITY,		// = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,				// Time slice not supported in FX3 implementation
            CYU3P_AUTO_START					// Start the thread immediately
            );

    if (Status != CY_U3P_SUCCESS) while(1);		// Get here on a failure, can't recover, just hang here
	// Once the programs get more complex we shall do something more elegant here
}


// Main sets up the CPU environment the starts the RTOS
int main (void)
{
    CyU3PIoMatrixConfig_t ioConfig;
    CyU3PReturnStatus_t Status;

 // Start with the default clock at 384 MHz
	Status = CyU3PDeviceInit(0);
	if (Status == CY_U3P_SUCCESS)
    {
		Status = CyU3PDeviceCacheControl(CyTrue, CyTrue, CyTrue);
		if (Status == CY_U3P_SUCCESS)
		{
			CyU3PMemSet((uint8_t *)&ioConfig, 0, sizeof(ioConfig));
			ioConfig.useUart   = true;							// We'll use this in the next example!
			ioConfig.lppMode   = CY_U3P_IO_MATRIX_LPP_UART_ONLY;
			ioConfig.gpioSimpleEn[0] = 0;						// No GPIOs 0 to 31 are needed
			ioConfig.gpioSimpleEn[1] = 1<<(Button-32);			// Button is on GPIO_45
			Status = CyU3PDeviceConfigureIOMatrix(&ioConfig);
			if (Status == CY_U3P_SUCCESS) CyU3PKernelEntry();	// Start RTOS, this does not return
		}
	}

    while (1);			// Get here on a failure, can't recover, just hang here
						// Once the programs get more complex we shall do something more elegant here
	return 0;			// Won't get here but compiler wants this!
}



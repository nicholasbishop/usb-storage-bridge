// Keyboard.c - demonstrate USB by enumerating as a keyboard
//
// john@usb-by-example.com
//
// This is an enhancement of the Keyboard in the book (which is now called
// Keyboard0) The program got into several hangs due to my use of DebugPrint in
// a Callback routines Yes, the book tells you not to do this.  So this example
// follow my advice! I now set flags in a DisplayEvent EventGroup and use
// BackgoundPrint to print these in main context
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern CyU3PReturnStatus_t InitializeUSB(void);
extern void CheckStatus(char *StringPtr, CyU3PReturnStatus_t Status);
extern void BackgroundPrint(uint32_t TimeToWait);

extern CyU3PDmaChannel glCPUtoUSB_Handle;
extern CyU3PEvent DisplayEvent; // Used to display events

CyU3PThread ApplicationThread;  // Handle to my Application Thread
CyBool_t glIsApplicationActive; // Set true once device is enumerated

CyU3PDmaChannel glUSBtoCPU_Handle; // Handle needed for Interrupt Out Endpoint
CyU3PDmaChannel glCPUtoUSB_Handle; // Handle needed for Interrupt In Endpoint

const char *BusSpeed[] = {"Not Connected", "Full ", "High ", "Super"};

void PacketReceived_Callback(CyU3PDmaChannel *Handle, CyU3PDmaCbType_t Type,
                             CyU3PDmaBuffer_t *DMAbuffer) {
  // I only get producer events at this callback
  CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;
  uint8_t Packet = *DMAbuffer->buffer;
  CyU3PDebugPrint(4, "\r\nPacket received = %x", Packet);
  Status = CyU3PDmaChannelSetXfer(Handle, 0);
  CheckStatus("Post another Producer buffer", Status);
}

void StartApplication(void)
// USB has been enumerated, time to start the application running
{
  CyU3PEpConfig_t epConfig;
  CyU3PDmaChannelConfig_t dmaConfig;
  CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

  // Display the enumerated device bus speed
  DebugPrint(4, "\r\nRunning at %sSpeed", BusSpeed[CyU3PUsbGetSpeed()]);

  // Configure and enable the Interrupt Endpoint
  CyU3PMemSet((uint8_t *)&epConfig, 0, sizeof(epConfig));
  epConfig.enable = CyTrue;
  epConfig.epType = CY_U3P_USB_EP_INTR;
  epConfig.burstLen = 1;
  // r	epConfig.streams = 0;
  epConfig.pcktSize = REPORT_SIZE;
  Status = CyU3PSetEpConfig(CY_FX_EP_CONSUMER, &epConfig);
  CheckStatus("Setup Interrupt In Endpoint", Status);

  // Create a manual DMA channel between CPU producer socket and USB
  CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
  dmaConfig.size = 16; // Minimum size, I only need REPORT_SIZE
  dmaConfig.count = 2; // KeyDown and KeyUp
  dmaConfig.prodSckId = CY_FX_CPU_PRODUCER_SOCKET;
  dmaConfig.consSckId = CY_FX_EP_CONSUMER_SOCKET;
  dmaConfig.dmaMode = CY_U3P_DMA_MODE_BYTE;
  // r dmaConfig.notification = 0;
  // r	dmaConfig.cb = NULL;
  // r	dmaConfig.prodHeader = 0;
  // r	dmaConfig.prodFooter = 0;
  // r	dmaConfig.consHeader = 0;
  // r	dmaConfig.prodAvailCount = 0;
  Status = CyU3PDmaChannelCreate(&glCPUtoUSB_Handle, CY_U3P_DMA_TYPE_MANUAL_OUT,
                                 &dmaConfig);
  CheckStatus("CreateCPUtoUSBdmaChannel", Status);

  // Set DMA Channel transfer size = infinite
  Status = CyU3PDmaChannelSetXfer(&glCPUtoUSB_Handle, 0);
  CheckStatus("CPUtoUSBdmaChannelSetXfer", Status);

  glIsApplicationActive = CyTrue; // Now ready to run!
}

void StopApplication(void)
// USB connection has been lost, time to stop the application running
{
  CyU3PEpConfig_t epConfig;
  CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

  glIsApplicationActive = CyFalse;

  // Close down and disable the endpoint then close the DMA channel
  CyU3PUsbFlushEp(CY_FX_EP_CONSUMER);
  CyU3PMemSet((uint8_t *)&epConfig, 0, sizeof(epConfig));
  // r	epConfig.enable = CyFalse;
  Status = CyU3PSetEpConfig(CY_FX_EP_CONSUMER, &epConfig);
  CheckStatus("Disable Producer Endpoint", Status);
  Status = CyU3PDmaChannelDestroy(&glCPUtoUSB_Handle);
  CheckStatus("Close USBtoCPU DMA Channel", Status);
}

const uint8_t Ascii2Usage[] = {
    // Create a lookup table that uses the ASCII character as an index and
    // produces a Modifier/Usage Code pair
    0, 0x2C, 2, 0x1E, 2, 0x34, 2, 0x20,
    2, 0x21, 2, 0x22, 2, 0x24, 0, 0x34, // 20..27    !"#$%&'
    2, 0x26, 2, 0x27, 2, 0x23, 2, 0x2E,
    0, 0x36, 0, 0x2D, 0, 0x37, 0, 0x38, // 28..2F   ()*+,-./
    0, 0x27, 0, 0x1E, 0, 0x1F, 0, 0x20,
    0, 0x21, 0, 0x22, 0, 0x23, 0, 0x24, // 28..2F   01234567
    0, 0x25, 0, 0x26, 2, 0x33, 0, 0x33,
    2, 0x36, 0, 0x2E, 2, 0x37, 2, 0x38, // 28..2F   89:;<=>?
    2, 0x1F, 2, 0x04, 2, 0x05, 2, 0x06,
    2, 0x07, 2, 0x08, 2, 0x09, 0, 0x0A, // 00..07 ^ @ABCDEFG
    2, 0x0B, 2, 0x0C, 2, 0x0D, 2, 0x0E,
    2, 0x0F, 2, 0x10, 2, 0x11, 2, 0x12, // 08..1F ^ HIJKLMNO
    2, 0x13, 2, 0x14, 2, 0x15, 2, 0x16,
    2, 0x17, 2, 0x18, 2, 0x19, 2, 0x1A, // 10..17 ^ PQRSTUVW
    2, 0x1B, 2, 0x1C, 2, 0x1D, 0, 0x2F,
    0, 0x31, 0, 0x30, 2, 0x23, 2, 0x2D, // 18..1F ^ XYZ[\]^_
    0, 0x2C, 0, 0x04, 0, 0x05, 0, 0x06,
    0, 0x07, 0, 0x08, 0, 0x09, 0, 0x0A, // 00..07 ^ @abcdefg
    0, 0x0B, 0, 0x0C, 0, 0x0D, 0, 0x0E,
    0, 0x0F, 0, 0x10, 0, 0x11, 0, 0x12, // 08..1F ^ hijklmno
    0, 0x13, 0, 0x14, 0, 0x15, 0, 0x16,
    0, 0x17, 0, 0x18, 0, 0x19, 0, 0x1A, // 10..17 ^ pqrstuvw
    0, 0x1B, 0, 0x1C, 0, 0x1D, 2, 0x2F,
    2, 0x31, 2, 0x30, 2, 0x35, 0, 0x28 // 18..1F ^ xyz{|}~
};

void SendPullEvent(PullEvent* event)
    // In this example characters typed on the debug console are sent as key strokes
    // The format of a keystroke is defined in the report descriptor; it is 8 bytes
    // long = Modifier, Reserved, UsageCode[6] A keyboard will send two reports, one
    // for key press and one for key release A 'standard' keyboard can encode up to
    // 6 key usages in one report, this example only does 1 CheckStatus calls
    // commented out following debug, reinsert them if needed
{
  CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;
  CyU3PDmaBuffer_t ReportBuffer;
  // First need a buffer to build the report
  Status = CyU3PDmaChannelGetBuffer(&glCPUtoUSB_Handle, &ReportBuffer,
                                    CYU3P_WAIT_FOREVER);
  //		CheckStatus("GetReportBuffer4KeyPress", Status);
  // Most of this report will be 0's
  ReportBuffer.count = REPORT_SIZE;
  CyU3PMemSet(ReportBuffer.buffer, 0, REPORT_SIZE);
  memcpy(ReportBuffer.buffer, event, REPORT_SIZE);
  // Send the pull event to the host
  Status = CyU3PDmaChannelCommitBuffer(&glCPUtoUSB_Handle, REPORT_SIZE, 0);
  CheckStatus("Send pull event", Status);
}

CyU3PQueue PullEventQueue;
PullEvent PullEventQueueStorage[4];

void GPIO_InterruptCallback(uint8_t gpioId) {
  static PullEvent event = { 0, 64 };

  if (gpioId != Button) {
     return;
  }

  CyU3PQueueSend(&PullEventQueue, &event, CYU3P_NO_WAIT);

  event.offset += event.length;
}

void InitGpio() {
  CyU3PGpioClock_t GpioClock;
  CyU3PGpioSimpleConfig_t GpioConfig;

  GpioClock.fastClkDiv = 2;
  GpioClock.slowClkDiv = 0;
  GpioClock.simpleDiv = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
  GpioClock.clkSrc = CY_U3P_SYS_CLK;
  GpioClock.halfDiv = 0;
  // Initialize the GPIO driver and register a Callback for interrupts
  CyU3PGpioInit(&GpioClock, GPIO_InterruptCallback);

  // Configure Button GPIOs
  CyU3PMemSet((uint8_t *)&GpioConfig, 0, sizeof(GpioConfig));
  GpioConfig.inputEn = CyTrue;
  GpioConfig.intrMode = CY_U3P_GPIO_INTR_NEG_EDGE;
  CyU3PGpioSetSimpleConfig(Button, &GpioConfig);
}

void ApplicationThread_Entry(uint32_t Value) {
  CyU3PReturnStatus_t Status = CY_U3P_SUCCESS;

  Status = InitializeDebugConsole();
  CheckStatus("Debug Console Initialized", Status);

  InitGpio();

  DebugPrint(4, "After InitGpio\n");

  Status = CyU3PQueueCreate(&PullEventQueue, 4, &PullEventQueueStorage,
                            sizeof(PullEventQueueStorage));
  DebugPrint(4, "Math: %d\n", sizeof(PullEvent));
  DebugPrint(4, "CyU3PQueueCreate -> %d\n", Status);
  CheckStatus("Create Queue", Status);

  // Create an Event Group that Callbacks can use to signal events to a
  // Background DebugPrint
  Status = CyU3PEventCreate(&DisplayEvent);
  CheckStatus("Create Event", Status);

  Status = InitializeUSB();
  CheckStatus("USB Initialized", Status);
  DebugPrint(4, "After InitializeUSB\n");

  // Wait for the USB connection to be up
  while (!glIsApplicationActive)
    BackgroundPrint(10);

  DebugPrint(4, "App is up, status=%d\n", Status);

  if (Status == CY_U3P_SUCCESS) {
    DebugPrint(4, "\r\nApplication started with %d\r\n", Value);
    // Now run forever
    while (1) {
      PullEvent event;
      DebugPrint(4, "waiting for button press...\n");
      CyU3PQueueReceive(&PullEventQueue, &event, CYU3P_WAIT_FOREVER);

      DebugPrint(4, "button pressed:\n");
      DebugPrint(4, "  offset=%d\n", event.offset);
      DebugPrint(4, "  length=%d\n", event.length);
      BackgroundPrint(1);
      //SendKeystroke(msg);

      SendPullEvent(&event);
    }
  }
  DebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n",
             Status);
  while (1)
    ; // Hang here
}
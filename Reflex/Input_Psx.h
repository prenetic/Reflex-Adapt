/*******************************************************************************
 * Reflex Adapt USB
 * PSX input module
 * 
 * Uses PsxNewLib
 * https://github.com/SukkoPera/PsxNewLib
 * 
 * Uses a modified version of Joystick Library
 * https://github.com/MHeironimus/ArduinoJoystickLibrary
 * 
*/

#include "src/DigitalIO/DigitalIO.h"
#include "src/PsxNewLib/PsxControllerHwSpi.h"

#include "src/ArduinoJoystickLibrary/Joy1.h"
#include "src/ArduinoJoystickLibrary/Guncon1.h"
#include "src/ArduinoJoystickLibrary/MouseRelative1.h"
#include "src/ArduinoJoystickLibrary/Jogcon1.h"

//Guncon config
//0=Mouse, 1=Joy, 2=Joy OffScreenEdge (MiSTer)
//#define GUNCON_FORCE_MODE 2

//NeGcon config
//0=Default, 1=MiSTer Wheel format with paddle
//#define NEGCON_FORCE_MODE 1
//If you dont want to force a mode but instead change the default:
//Don't enable the force mode and edit the isNeGconMiSTer variable below as you desire.

//PSX pins - Port 1
const byte PIN_PS1_ATT = 21;

//PSX pins - Port 2
const byte PIN_PS2_ATT = 5;

PsxController* psx;//variable to hold current reading port

PsxController* psxlist[] = {
  new PsxControllerHwSpi<PIN_PS1_ATT>(),
  new PsxControllerHwSpi<PIN_PS2_ATT>()
};

boolean isNeGcon = false;
boolean isJogcon = false;
boolean isGuncon = false;

boolean isNeGconMiSTer = false;

uint8_t specialDpadMask = 0x0;
const uint8_t SPECIALMASK_POPN = 0xE;

#ifdef ENABLE_REFLEX_PAD
  const Pad padPsx[] = {
    { PSB_SELECT,    2, 4*6, RECTANGLEBTN_ON, RECTANGLEBTN_OFF },
    { PSB_L3,        3, 4*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_R3,        3, 5*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_START,     2, 5*6, RECTANGLEBTN_ON, RECTANGLEBTN_OFF },
    { PSB_PAD_UP,    1, 1*6, UP_ON, UP_OFF },
    { PSB_PAD_RIGHT, 2, 2*6, RIGHT_ON, RIGHT_OFF },
    { PSB_PAD_DOWN,  3, 1*6, DOWN_ON, DOWN_OFF },
    { PSB_PAD_LEFT,  2, 0,   LEFT_ON, LEFT_OFF },
    { PSB_L2,        0, 2*6, SHOULDERBTN_ON, SHOULDERBTN_OFF },
    { PSB_R2,        0, 9*6, SHOULDERBTN_ON, SHOULDERBTN_OFF },
    { PSB_L1,        0, 0*6, SHOULDERBTN_ON, SHOULDERBTN_OFF },
    { PSB_R1,        0, 7*6, SHOULDERBTN_ON, SHOULDERBTN_OFF },
    { PSB_TRIANGLE,  1, 8*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_CIRCLE,    2, 9*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_CROSS,     3, 8*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_SQUARE,    2, 7*6, FACEBTN_ON, FACEBTN_OFF }
  };

  const Pad padPsxPopN[] = {
    { PSB_SELECT,    0, 3*6, RECTANGLEBTN_ON, RECTANGLEBTN_OFF },
    { PSB_START,     0, 5*6, RECTANGLEBTN_ON, RECTANGLEBTN_OFF },
    { PSB_CIRCLE,    1, 1*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_CROSS,     1, 3*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_SQUARE,    1, 5*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_PAD_UP,    1, 7*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_TRIANGLE,  2, 0*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_R1,        2, 2*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_L1,        2, 4*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_R2,        2, 6*6, FACEBTN_ON, FACEBTN_OFF },
    { PSB_L2,        2, 8*6, FACEBTN_ON, FACEBTN_OFF }
  };

  void loopPadDisplayCharsPsx(const uint8_t index, const PsxControllerProtocol padType, void* p, const bool force) {
    if(specialDpadMask == SPECIALMASK_POPN) {
      for(uint8_t i = 0; i < (sizeof(padPsxPopN) / sizeof(Pad)); ++i){
        const Pad pad = padPsxPopN[i];
        PrintPadChar(index, padDivision[index].firstCol, pad.col, pad.row, pad.padvalue, p && static_cast<PsxController*>(p)->buttonPressed(static_cast<PsxButton>(pad.padvalue)), pad.on, pad.off, force);
      }
    } else {
      for(uint8_t i = 0; i < (sizeof(padPsx) / sizeof(Pad)); ++i){
        if(padType != PSPROTO_DUALSHOCK && padType != PSPROTO_DUALSHOCK2 && (i == 1 || i == 2))
          continue;
        if(padType == PSPROTO_NEGCON && (i == 0 || i == 8 || i == 9))
          continue;
        const Pad pad = padPsx[i];
        PrintPadChar(index, padDivision[index].firstCol, pad.col, pad.row, pad.padvalue, p && static_cast<PsxController*>(p)->buttonPressed(static_cast<PsxButton>(pad.padvalue)), pad.on, pad.off, force);
      }      
    }

  }

  void ShowDefaultPadPsx(const uint8_t index, const PsxControllerProtocol padType) {
    //print default joystick state to oled screen
  
    //const uint8_t firstCol = padDivision[index].firstCol; //index == 0 ? 0 : 11*6;
    //const uint8_t lastCol = padDivision[index].lasCol;//index == 0 ? 10*6 : 127;

    display.clear(padDivision[index].firstCol, padDivision[index].lastCol, oledDisplayFirstRow + 1, 7);
    display.setCursor(padDivision[index].firstCol, 7);

    switch(padType) {
      case PSPROTO_DIGITAL:
      case PSPROTO_NEGCON:
        if(specialDpadMask == SPECIALMASK_POPN)
          display.print(F("POP N"));
        else
          display.print(isNeGcon ? F("NEGCON") : PSTR_TO_F(PSTR_DIGITAL));
        break;
      case PSPROTO_DUALSHOCK:
        display.print(F("DUALSHOCK"));
        break;
      //case PSPROTO_DUALSHOCK2:
      //  display.print(F("DUALSHOCK2"));
      //  break;
      case PSPROTO_FLIGHTSTICK:
        display.print(F("FLIGHTSTICK"));
        break;
      //case PSPROTO_NEGCON:
      //  display.print(F("NEGCON-"));
      //  display.print(isNeGcon ? F("ANALOG") : PSTR_TO_F(PSTR_DIGITAL));
      //  break;
      case PSPROTO_JOGCON:
        display.print(F("JOGCON"));
        break;
      default:
        display.print(PSTR_TO_F(PSTR_NONE));
        return;
    }
  
    if (index < 2) {
      loopPadDisplayCharsPsx(index, padType, NULL, true);
    }
  }
#endif

static uint8_t outputIndex = 0;

//boolean haveController = false;
bool haveController[] = {false,false};


boolean enableMouseMove = false; //used on guncon and jogcon modes

void handleDpad(const bool isjog = false) {
  uint8_t dpad = B0;
  bitWrite(dpad, 0, !psx->buttonPressed (PSB_PAD_UP));
  bitWrite(dpad, 1, !psx->buttonPressed (PSB_PAD_DOWN));
  bitWrite(dpad, 2, !psx->buttonPressed (PSB_PAD_LEFT));
  bitWrite(dpad, 3, !psx->buttonPressed (PSB_PAD_RIGHT));
  //usbStick[outputIndex]->setHatSwitch(0, hatTable[dpad]);

  if (specialDpadMask)
    dpad = (dpad | specialDpadMask) & 0xF;

  if (isjog)
    ((Jogcon1_*)usbStick[outputIndex])->setHatSwitch(hatTable[dpad]);
  else
    ((Joy1_*)usbStick[outputIndex])->setHatSwitch(hatTable[dpad]);
}


//optional includes
#ifdef GUNCON_SUPPORT
  #include "Psx_guncon.h"
#endif

#ifdef JOGCON_SUPPORT
  #include "Psx_jogcon.h"
#endif

#ifdef NEGCON_SUPPORT
  #include "Psx_negcon.h"
#endif

bool loopDualShock() {
  static byte lastLX[] = { ANALOG_IDLE_VALUE, ANALOG_IDLE_VALUE };
  static byte lastLY[] = { ANALOG_IDLE_VALUE, ANALOG_IDLE_VALUE };
  static byte lastRX[] = { ANALOG_IDLE_VALUE, ANALOG_IDLE_VALUE };
  static byte lastRY[] = { ANALOG_IDLE_VALUE, ANALOG_IDLE_VALUE };

  #ifdef ENABLE_REFLEX_PAD
    static PsxControllerProtocol lastPadType[] = { PSPROTO_UNKNOWN, PSPROTO_UNKNOWN };
    PsxControllerProtocol currentPadType[] = { PSPROTO_UNKNOWN, PSPROTO_UNKNOWN };
    const uint8_t inputPort = outputIndex; //todo fix
  #endif

  byte analogX = ANALOG_IDLE_VALUE;
  byte analogY = ANALOG_IDLE_VALUE;
  //word convertedX, convertedY;

  const bool digitalStateChanged = psx->buttonsChanged();//check if any digital value changed (dpad and buttons)
  bool stateChanged = digitalStateChanged;
  
  const PsxControllerProtocol proto = psx->getProtocol();

  #ifdef ENABLE_REFLEX_PAD
    if(proto != lastPadType[inputPort])
      ShowDefaultPadPsx(inputPort, proto);
    currentPadType[inputPort] = proto;
  #endif

  
  switch (proto) {
  case PSPROTO_DIGITAL:
    //if (!stateChanged)
    //  return false;
  case PSPROTO_NEGCON:
  case PSPROTO_DUALSHOCK:
  case PSPROTO_DUALSHOCK2:
  case PSPROTO_FLIGHTSTICK:
  {
    handleDpad();

    uint16_t buttonData = 0;

    //controller buttons
    bitWrite(buttonData, 0, psx->buttonPressed (PSB_SQUARE));
    bitWrite(buttonData, 1, psx->buttonPressed (PSB_CROSS));
    bitWrite(buttonData, 2, psx->buttonPressed (PSB_CIRCLE));
    bitWrite(buttonData, 3, psx->buttonPressed (PSB_TRIANGLE));
    bitWrite(buttonData, 4, psx->buttonPressed (PSB_L1));
    bitWrite(buttonData, 5, psx->buttonPressed (PSB_R1));
    bitWrite(buttonData, 6, psx->buttonPressed (PSB_L2));
    bitWrite(buttonData, 7, psx->buttonPressed (PSB_R2));
    bitWrite(buttonData, 8, psx->buttonPressed (PSB_SELECT));
    bitWrite(buttonData, 9, psx->buttonPressed (PSB_START));
    bitWrite(buttonData, 10, psx->buttonPressed (PSB_L3));
    bitWrite(buttonData, 11, psx->buttonPressed (PSB_R3));


//if(proto != PSPROTO_DIGITAL)

    //analog sticks
    if (psx->getLeftAnalog(analogX, analogY) && proto != PSPROTO_NEGCON) {
      //convertedX = convertRange(ANALOG_MIN_VALUE, ANALOG_MAX_VALUE, analogX);
      //convertedY = convertRange(ANALOG_MIN_VALUE, ANALOG_MAX_VALUE, analogY);
      //usbStick[outputIndex]->setXAxis(convertedX);
      //usbStick[outputIndex]->setYAxis(convertedY);
      ((Joy1_*)usbStick[outputIndex])->setAnalog0(analogX); //x
      ((Joy1_*)usbStick[outputIndex])->setAnalog1(analogY); //y
    } else {
      //usbStick[outputIndex]->setXAxis(16384);
      //usbStick[outputIndex]->setYAxis(16384);
      analogX = ANALOG_IDLE_VALUE;
      analogY = ANALOG_IDLE_VALUE;
      ((Joy1_*)usbStick[outputIndex])->setAnalog0(ANALOG_IDLE_VALUE);
      ((Joy1_*)usbStick[outputIndex])->setAnalog1(ANALOG_IDLE_VALUE);
    }
    
    if (lastLX[outputIndex] != analogX || lastLY[outputIndex] != analogY)
      stateChanged = true;
      
    lastLX[outputIndex] = analogX;
    lastLY[outputIndex] = analogY;

    if (psx->getRightAnalog(analogX, analogY) && proto != PSPROTO_NEGCON) {
      //convertedX = convertRange(ANALOG_MIN_VALUE, ANALOG_MAX_VALUE, analogX);
      //convertedY = convertRange(ANALOG_MIN_VALUE, ANALOG_MAX_VALUE, analogY);
      //usbStick[outputIndex]->setRxAxis(convertedX);
      //usbStick[outputIndex]->setRyAxis(convertedY);
      ((Joy1_*)usbStick[outputIndex])->setAnalog2(analogX); //z (right x)
      ((Joy1_*)usbStick[outputIndex])->setAnalog3(analogY); //rz (right y)
    } else {
      //usbStick[outputIndex]->setRxAxis(16384);
      //usbStick[outputIndex]->setRyAxis(16384);
      analogX = ANALOG_IDLE_VALUE;
      analogY = ANALOG_IDLE_VALUE;
      ((Joy1_*)usbStick[outputIndex])->setAnalog2(ANALOG_IDLE_VALUE);
      ((Joy1_*)usbStick[outputIndex])->setAnalog3(ANALOG_IDLE_VALUE);
    }
    
    if (lastRX[outputIndex] != analogX || lastRY[outputIndex] != analogY)
      stateChanged = true;

    lastRX[outputIndex] = analogX;
    lastRY[outputIndex] = analogY;

    ((Joy1_*)usbStick[outputIndex])->setButtons(buttonData);

    if(stateChanged) {
      usbStick[outputIndex]->sendState();

      #ifdef ENABLE_REFLEX_PAD
        if (inputPort < 2) {
          loopPadDisplayCharsPsx(inputPort, proto, psx, false);
        }
      #endif
    }

    break;
  }
  default:
    break;
  }


  #ifdef ENABLE_REFLEX_PAD
    /*for (uint8_t i = 0; i < 2; i++) {
      if(lastPadType[i] != currentPadType[i] && currentPadType[i] == PSPROTO_UNKNOWN) {
        ShowDefaultPadPsx(i, currentPadType[i]);
      }
      lastPadType[i] = currentPadType[i];
    }*/
    lastPadType[inputPort] = currentPadType[inputPort];
  #endif

  return digitalStateChanged;
}

void psxSetup() {

  //initialize the "attention" pins as OUTPUT HIGH.
  fastPinMode (PIN_PS1_ATT, OUTPUT);
  fastPinMode (PIN_PS2_ATT, OUTPUT);
  //fastPinMode (PIN_PS3_ATT, OUTPUT);
  //fastPinMode (PIN_PS4_ATT, OUTPUT);
  //fastPinMode (PIN_PS5_ATT, OUTPUT);
  //fastPinMode (PIN_PS6_ATT, OUTPUT);

  fastDigitalWrite (PIN_PS1_ATT, HIGH);
  fastDigitalWrite (PIN_PS2_ATT, HIGH);
  //fastDigitalWrite (PIN_PS3_ATT, HIGH);
  //fastDigitalWrite (PIN_PS4_ATT, HIGH);
  //fastDigitalWrite (PIN_PS5_ATT, HIGH);
  //fastDigitalWrite (PIN_PS6_ATT, HIGH);
  
  psx = psxlist[0];

  //if forcing specific mode
#ifdef ENABLE_REFLEX_PSX_JOG
  PsxControllerProtocol proto = (deviceMode == REFLEX_PSX_JOG ? PSPROTO_JOGCON : PSPROTO_UNKNOWN);
#else
  PsxControllerProtocol proto = PSPROTO_UNKNOWN;
#endif

  if (psx->begin ()) {
    //delay(150);//200
    //haveController = true;
    haveController[0] = true;
    //const PsxControllerProtocol proto = psx->getProtocol();


    //if not forced a mode, then read from currenct connected controller
    if(proto == PSPROTO_UNKNOWN)
      proto = psx->getProtocol();

    if (proto == PSPROTO_GUNCON) {
      isGuncon = true;
    } else if (proto == PSPROTO_NEGCON) {
      isNeGcon = true;

      //Configure NeGcon mode
      #if defined(NEGCON_FORCE_MODE) && NEGCON_FORCE_MODE >= 0 && NEGCON_FORCE_MODE < 2
        #if NEGCON_FORCE_MODE == 1
          isNeGconMiSTer = true;
        #endif
      #else //NEGCON_FORCE_MODE
        if (psx->buttonPressed(PSB_CIRCLE)) //NeGcon A / Volume B
          isNeGconMiSTer = !isNeGconMiSTer;
      #endif //NEGCON_FORCE_MODE
    } else { //jogcon can't be detected during boot as it needs to be in analog mode

#ifdef JOGCON_SUPPORT
    if(proto == PSPROTO_JOGCON) {
      isJogcon = true;
      if (psx->buttonPressed(PSB_L2))
        enableMouseMove = true;      
    } else if (proto == PSPROTO_DIGITAL) { //Try to detect by it's id
      if (psx->enterConfigMode ()) {
        if (psx->getControllerType () == PSCTRL_JOGCON) {
          isJogcon = true;
          if (psx->buttonPressed(PSB_L2))
            enableMouseMove = true;
        }
        psx->exitConfigMode ();
      }
    }
#endif

      
      if (psx->buttonPressed(PSB_SELECT)) { //dualshock used in guncon mode to help map axis on emulators.
        isGuncon = true;
      }
      /*else if (proto == PSPROTO_JOGCON || psx->buttonPressed(PSB_L1)) {
        isJogcon = true;
      } else if (psx->buttonPressed(PSB_L2)) {
        isJogcon = true;
        enableMouseMove = true;
      }*/
    }
  } else { //no controller connected
    if (proto == PSPROTO_JOGCON) //forced jogcon mode?
      isJogcon = true;
  }

  if (isNeGcon) {
    #ifdef NEGCON_SUPPORT
      negconSetup();
    #endif
  } else if (isJogcon) {
    #ifdef JOGCON_SUPPORT
      jogconSetup();
    #endif
  } else {
    if (isGuncon) {
      #ifdef GUNCON_SUPPORT
        gunconSetup();
      #endif
    } else { //dualshock [default]

      if (proto == PSPROTO_DIGITAL
      && psx->buttonPressed(PSB_PAD_DOWN)
      && psx->buttonPressed(PSB_PAD_LEFT)
      && psx->buttonPressed(PSB_PAD_RIGHT))
        specialDpadMask = SPECIALMASK_POPN;

      totalUsb = 2;//MAX_USB_STICKS;
      for (uint8_t i = 0; i < totalUsb; i++) {
        usbStick[i] = new Joy1_("ReflexPSDS1", JOYSTICK_DEFAULT_REPORT_ID + i, JOYSTICK_TYPE_GAMEPAD, totalUsb,
          true,//includeXAxis,
          true,//includeYAxis,
          true,//includeZAxis,
          true,//includeRzAxis,
          false,//includeThrottle,
          false,//includeBrake,
          false);//includeSteering
      }
    }

    //send reset state for all outputs
    for (uint8_t i = 0; i < totalUsb; i++) {
      usbStick[i]->resetState();
      usbStick[i]->sendState();
    }
  
  }

  sleepTime = 100;//isJogcon ? 100 : 1000;

  dstart(115200);
  debugln(F("Ready!"));
}

inline bool __attribute__((always_inline))
psxLoop() {
  static bool isReadSuccess[] = {false,false};
  static bool isEnabled[] = {false,false};
  bool stateChanged = false;

  outputIndex = 0;

  if(isJogcon) {
    #ifdef JOGCON_SUPPORT
      if (!haveController[0]) {
        init_jogcon();
      } else {
        if(!psx->read()){
          //haveController = false;
          haveController[0] = false;
        } else {
          stateChanged = handleJogconData();
        }
      }
    #endif
    return stateChanged;//haveController[0];
  }

  //if (millis() - last >= POLLING_INTERVAL) {
  //  last = millis();


    //nothing detected yet
    if (!isEnabled[0] && !isEnabled[1]) {
      for (uint8_t i = 0; i < 2; i++) {
        //isEnabled[i] = haveController[i] || psxlist[i]->begin();
        isEnabled[i] = haveController[i] || (haveController[i] = psxlist[i]->begin());
        //haveController[i] = haveController[i] || psxlist[i]->begin();
        //isEnabled[i] = haveController[i];
      }
      #if defined REFLEX_USE_OLED_DISPLAY && defined ENABLE_PSX_GENERAL_OLED
      //display.setCursor(0, 7);
      //display.clearToEOL();
      //display.print(F("Ports "));
      setOledDisplay(true);
      //clearOledLineAndPrint(0, 7 - oledDisplayFirstRow, F("Ports "));
      //clearOledLineAndPrint(0, 6 - oledDisplayFirstRow, F("Connect a single pad."));
      //clearOledLineAndPrint(0, 7 - oledDisplayFirstRow, F("Connect two pads and reset device."));

      //clearOledLineAndPrint(0, 5 - oledDisplayFirstRow, F("To begin, connect:"));
      //clearOledLineAndPrint(0, 6 - oledDisplayFirstRow, F("- A single pad."));
      //clearOledLineAndPrint(0, 7 - oledDisplayFirstRow, F("- Two pads, then reset device."));

      //clearOledLineAndPrint(6, 6 - oledDisplayFirstRow, F("CONNECT A CONTROLLER"));
      //clearOledLineAndPrint(4*6, 7 - oledDisplayFirstRow, F("TO INITIALIZE"));

      display.clear(0, 127, 7, 7);
      display.setRow(7);


      for (uint8_t i = 0; i < 2; i++) {
        //const uint8_t firstCol = i == 0 ? 0 : 12*6;
        display.setCol(padDivision[i].firstCol);
        if (!isEnabled[0] && !isEnabled[1])
          display.print(PSTR_TO_F(PSTR_NONE));  
        else if (!isEnabled[i])
          display.print(PSTR_TO_F(PSTR_NA));
      }
/*      
      if(!isEnabled[0] && !isEnabled[1]) {
        
        for (uint8_t i = 0; i < 2; i++) {
          const uint8_t firstCol = i == 0 ? 0 : 12*6;
          display.setCol(firstCol);
          display.print(PSTR_TO_F(PSTR_NONE));
        }
      }

      if(isEnabled[0] || isEnabled[1]) {
        for (uint8_t i = 0; i < 2; i++) {
          //display.setCol(36 + (i*12)); //each char is 6 cols
          const uint8_t firstCol = i == 0 ? 0 : 12*6;
          display.setCol(firstCol); //each char is 6 cols
          if(isEnabled[i])
            display.print(PSTR_TO_F(PSTR_NONE));
          else
            display.print(PSTR_TO_F(PSTR_NA));
        }
      }
      */

      
      //for (uint8_t i = 0; i < 2; i++) {
      //  if(isEnabled[i]){
      //    display.setCol(36 + (i*12)); //each char is 6 cols
      //    display.print(i+1);
      //  }
      //}
      #endif
    }


    //read all ports

    for (uint8_t i = 0; i < 2; i++) {
      psx = psxlist[i];
      isReadSuccess[i] = false;

      if (!isEnabled[i])
        continue;
      
      if (!haveController[i]) {
        if (psx->begin()) {
          haveController[i] = true;
          ShowDefaultPadPsx(i, psx->getProtocol());
        }
      } else {
        isReadSuccess[i] = psx->read();
        if (!isReadSuccess[i]){ //debug (F("Controller lost.")); debug (F(" last values: x = ")); debug (lastX); debug (F(", y = ")); debugln (lastY);
          haveController[i] = false;
          ShowDefaultPadPsx(i, PSPROTO_UNKNOWN);
        }
      }
      if(isGuncon)//only use first port for guncon
        break;
    }


    for (uint8_t i = 0; i < totalUsb; i++) {
      if (haveController[i] && isReadSuccess[i]) {
        psx = psxlist[i];
        if (isNeGcon) {
          #ifdef NEGCON_SUPPORT
            stateChanged |=loopNeGcon();
          #endif
        } else if (isGuncon) {
          #ifdef GUNCON_SUPPORT
            loopGuncon();
            stateChanged = true;
          #endif
        } else {
          stateChanged |= loopDualShock();
        }
      }
      outputIndex++;
    }

  //} end if (millis() - last >= POLLING_INTERVAL)
  return stateChanged;
}

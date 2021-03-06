#ifndef PVOUT_H
  #define PVOUT_H
  #include <pin.h>
  
  class PVOutGPIO
  {
    private:
    pin* valvePin;
    unsigned long vlvBits;
    byte pinCount;
    
    public:
    PVOutGPIO(byte count) {
      pinCount = count;
      valvePin = (pin *) malloc(pinCount * sizeof(pin));
    }

    ~PVOutGPIO() {
      free(valvePin);
    }
  
    void setup(byte pinIndex, byte digitalPin) {
      valvePin[pinIndex].setup(digitalPin, OUTPUT);
    }
  
    void init(void) { 
      set(0);
    }
    
    void set(unsigned long vlvBits) { 
      for (byte i = 0; i < pinCount; i++) {
        if (vlvBits & (1<<i)) valvePin[i].set(); else valvePin[i].clear();
      }
      this->vlvBits = vlvBits;
    }
    
    unsigned long get() { return vlvBits; }
  };
  
  class PVOutMUX
  {
    private:
    pin muxLatchPin, muxDataPin, muxClockPin, muxEnablePin;
    boolean muxEnableLogic;
    unsigned long vlvBits;
    
    public:
    PVOutMUX(byte latchPin, byte dataPin, byte clockPin, byte enablePin, boolean enableLogic) {
      muxLatchPin.setup(latchPin, OUTPUT);
      muxDataPin.setup(dataPin, OUTPUT);
      muxClockPin.setup(clockPin, OUTPUT);
      muxEnablePin.setup(enablePin, OUTPUT);
      muxEnableLogic = enableLogic;
    }
    
    void init(void) {
      if (muxEnableLogic) {
        //MUX in Reset State
        muxLatchPin.clear(); //Prepare to copy pin states
        muxEnablePin.clear(); //Force clear of pin registers
        muxLatchPin.set();
        delayMicroseconds(10);
        muxLatchPin.clear();
        muxEnablePin.set(); //Disable clear
      } else {
        set(0);
        muxEnablePin.clear();
      }
    }
    
    void set(unsigned long vlvBits) {
      //ground latchPin and hold low for as long as you are transmitting
      muxLatchPin.clear();
      //clear everything out just in case to prepare shift register for bit shifting
      muxDataPin.clear();
      muxClockPin.clear();
    
      //for each bit in the long myDataOut
      for (byte i = 0; i < 32; i++)  {
        muxClockPin.clear();
        //create bitmask to grab the bit associated with our counter i and set data pin accordingly (NOTE: 32 - i causes bits to be sent most significant to least significant)
        if ( vlvBits & ((unsigned long)1<<(31 - i)) ) muxDataPin.set(); else muxDataPin.clear();
        //register shifts bits on upstroke of clock pin  
        muxClockPin.set();
        //zero the data pin after shift to prevent bleed through
        muxDataPin.clear();
      }
    
      //stop shifting
      muxClockPin.clear();
      muxLatchPin.set();
      delayMicroseconds(10);
      muxLatchPin.clear();
      this->vlvBits = vlvBits;
    }
    
    unsigned long get() { return vlvBits; }
  };
  
  class PVOutMODBUS
  {
    private:
    unsigned long vlvBits;

    public:
    void init(void);
    void set(unsigned long) { this->vlvBits = vlvBits; }
    unsigned long get() { return vlvBits; }
  };
#endif //ifndef PVOUT_H

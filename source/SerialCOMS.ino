/*
  This is a slightly modified version of SerialCOMS.ino provided
  in the demo code.
  I'm not interested in sending control data. Just reading and 
  displaying values.
*/


uint8_t rx_len;
uint8_t * address;
uint8_t buffer[256]; //address for temporary storage and parsing buffer
uint8_t structSize;
uint8_t rx_array_inx;  //index for RX parsing buffer
uint8_t calc_CS;	   //calculated Chacksum

//This shit contains stuff borrowed from EasyTransfer lib
boolean KSPBoardReceiveData() {
/*
  NOTE : 
  Very important that the struct is packed because the PIC is 32bit
  and having the uint8_t right at the start of the vessel structSize
  causes 3 empty bytes to follow it (due to aligning to start of word)
*/
  if ((rx_len == 0)&&(Serial.available()>3)){
    while(Serial.read()!= 0xBE) {
      if (Serial.available() == 0){
        return false;  
      }
    }
    //display.print("^");
    if (Serial.read() == 0xEF){
      rx_len = Serial.read();   
      id = Serial.read(); 
      rx_array_inx = 1;

      switch(id) {
        case 0:{
          structSize = sizeof(HPacket);   
          address = (uint8_t*)&HPacket;
          //display.print("H");
          break;
        }
        case 1:{
          structSize = sizeof(VData);   
          address = (uint8_t*)&VData;     
          // display.print("V");
          break;
        }
      }
    }
    // Make sure we recieved the correct amount of data
    if(rx_len > structSize){
      rx_len = 0;
      return false;
    }   
  }
  if(rx_len != 0){
    while(Serial.available() && rx_array_inx <= rx_len){
      buffer[rx_array_inx++] = Serial.read();
    }
    buffer[0] = id;
    if(rx_len == (rx_array_inx-1)){
      //seem to have got whole message
      //last uint8_t is CS
      calc_CS = rx_len;
      for (int i = 0; i<rx_len; i++){
        calc_CS^=buffer[i];
      }
      if(calc_CS == buffer[rx_array_inx-1]){//CS good
        memcpy(address,buffer,structSize);
        rx_len = 0;
        rx_array_inx = 1;
        return true;
      }else{
        //failed checksum, need to clear this out anyway
        rx_len = 0;
        rx_array_inx = 1;
        return false;
      }
    }
  }
  return false;
}

void KSPBoardSendData(uint8_t * address, uint8_t len){
  uint8_t CS = len;
  Serial.write(0xBE);
  Serial.write(0xEF);
  Serial.write(len);
  
  for(int i = 0; i<len; i++){
    CS^=*(address+i);
    Serial.write(*(address+i));
  }
  
  Serial.write(CS);
}

// Handshake

void Handshake(){
  digitalWrite(GLED,HIGH); 

  HPacket.id = 0;
  HPacket.M1 = 3;
  HPacket.M2 = 1;
  HPacket.M3 = 4;

  KSPBoardSendData(details(HPacket));
}

void KSPinput() {
  now = millis();

  if (KSPBoardReceiveData()){
    deadtimeOld = now;
    switch(id) {
      case 0:{
        //Handshake packet
        Handshake();
        break;
      }
      case 1:{
        Indicators();
        break;
      }
    }

    //We got some data, turn the green led on
    digitalWrite(GLED,HIGH);
    Connected = true;
  }else{ 
    //if no message received for a while, go idle
    deadtime = now - deadtimeOld; 
    if (deadtime > IDLETIMER){
      deadtimeOld = now;
      Connected = false;
      LEDSAllOff();
    }    
  }
}


void KSPoutput() {
  now = millis();
  controlTime = now - controlTimeOld; 
  if (controlTime > CONTROLREFRESH){
    controlTimeOld = now;
    controls();
  }    
}

void controls() {
  // I'm not currently interested in sending control data
  if (Connected) {
  //if (1) {
    /*
    //--------- This is how you do main controls
    if (digitalRead(SASPIN)){  
      MainControls(SAS, HIGH);
    }else{
      MainControls(SAS, LOW);
    }
    if (digitalRead(RCSPIN)){
      MainControls(RCS, HIGH);
    }else{
      MainControls(RCS, LOW);
    }
    //--------- This is how you do control groups
    if (digitalRead(CG1PIN)){
      ControlGroups(1, HIGH);
    }else{
      ControlGroups(1, LOW);      
    }
    //This is an example of reading analog inputs to an axis, with deadband and limits
    CPacket.Throttle = constrain(map(analogRead(THROTTLEPIN),THROTTLEDB,1024-THROTTLEDB,0,1000),0, 1000);
  */
    KSPBoardSendData(details(CPacket));
  }
}

void controlsInit() {
  /*
  pinMode(SASPIN, INPUT_PULLUP);
  pinMode(RCSPIN, INPUT_PULLUP);
  pinMode(CG1PIN, INPUT_PULLUP);
  */
}

void MainControls(uint8_t n, boolean s) {
  if (s){
    CPacket.MainControls |= (1 << n);       // forces nth bit of x to be 1.  all other bits left alone.
  }else{
    CPacket.MainControls &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.
  }
}

void ControlGroups(uint8_t n, boolean s) {
  if (s){
    CPacket.ControlGroup |= (1 << n);       // forces nth bit of x to be 1.  all other bits left alone.
  }else{
    CPacket.ControlGroup &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.
  }
}


void Indicators() {
  caution = 0;
  warning = 0;

  caution += VData.G > GCAUTION;
  warning += VData.G > GWARN;
  caution += VData.LiquidFuelS/VData.LiquidFuelTotS*100 < FUELCAUTION;
  warning += VData.LiquidFuelS/VData.LiquidFuelTotS*100 < FUELWARN;

  if (caution != 0)
    digitalWrite(YLED,HIGH);
  else
    digitalWrite(YLED,LOW);

  if (warning != 0)
    digitalWrite(RLED,HIGH);
  else
    digitalWrite(RLED,LOW);
}

void initLEDS() {
  pinMode(GLED,OUTPUT);
  digitalWrite(GLED,HIGH);

  pinMode(YLED,OUTPUT);
  digitalWrite(YLED,HIGH);

  pinMode(RLED,OUTPUT);
  digitalWrite(RLED,HIGH);
}

void LEDSAllOff() {
  digitalWrite(GLED,LOW);
  digitalWrite(YLED,LOW);
  digitalWrite(RLED,LOW);
}


void InitTxPackets() {
  HPacket.id = 0; 
  CPacket.id = 101;
}


















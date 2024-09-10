#define startMarker 254
#define endMarker 255
#define specialByte 253
#define maxMessage 16

// the program could be rewritten to use local variables instead of some of these globals
//  however globals make the code simpler
//  and simplify memory management

byte bytesRecvd = 0;
byte dataSentNum = 0; // the transmitted value of the number of bytes in the package i.e. the 2nd byte received
byte dataRecvCount = 0;

byte dataRecvd[maxMessage];
byte dataSend[maxMessage];
byte tempBuffer[maxMessage];

byte dataSendCount = 0; // the number of 'real' bytes to be sent to the PC
byte dataTotalSend = 0; // the number of bytes to send to PC taking account of encoded bytes

boolean inProgress = false;
boolean startFound = false;
boolean allReceived = false;

// Motor Pin Reference Array
int motor_pins[] = {9, 10};

//================

void setup()
{
  pinMode(13, OUTPUT); // the onboard LED
  pinMode(motor_pins[0], OUTPUT);
  pinMode(motor_pins[1], OUTPUT);

  Serial.begin(57600);
  Serial.write("Ready\n");

  // initialize PWM channels to stall frequency
  setMotorSpeed(motor_pins[0], 187);
  setMotorSpeed(motor_pins[1], 187);
}

//================

void loop()
{

  getSerialData();

  processData();
}

//================

void getSerialData()
{

  // Receives data into tempBuffer[]
  //   saves the number of bytes that the PC said it sent - which will be in tempBuffer[1]
  //   uses decodeHighBytes() to copy data from tempBuffer to dataRecvd[]

  // the Arduino program will use the data it finds in dataRecvd[]

  if (Serial.available() > 0)
  {

    byte x = Serial.read();
    // Serial.print((String)"d " + x + "\n");

    if (x == startMarker)
    {
      bytesRecvd = 0;
      inProgress = true;
      // blinkLED(2);
      // debugToPC("start");
    }

    if (inProgress)
    {
      tempBuffer[bytesRecvd] = x;
      bytesRecvd++;
    }

    if (x == endMarker)
    {
      // debugToPC("end");
      inProgress = false;
      allReceived = true;

      // save the number of bytes that were sent
      dataSentNum = tempBuffer[1];

      decodeHighBytes();
    }
  }
}

//============================

void processData()
{

  // processes the data that is in dataRecvd[]

  if (allReceived)
  {

    // for demonstration just copy dataRecvd to dataSend
    dataSendCount = dataRecvCount;
    for (byte n = 0; n < dataRecvCount; n++)
    {
      dataSend[n] = dataRecvd[n];
    }

    // Input formatted as < motor1_speed, motor2_speed >
    setMotorSpeed(motor_pins[0], dataRecvd[0]);
    setMotorSpeed(motor_pins[1], dataRecvd[1]);

    dataToPC();

    allReceived = false;
  }
}

//============================

void decodeHighBytes()
{

  //  copies to dataRecvd[] only the data bytes i.e. excluding the marker bytes and the count byte
  //  and converts any bytes of 253 etc into the intended numbers
  //  Note that bytesRecvd is the total of all the bytes including the markers
  dataRecvCount = 0;
  for (byte n = 1; n < bytesRecvd - 1; n++)
  { // 2 skips the start marker and the count byte, -1 omits the end marker
    byte x = tempBuffer[n];
    if (x == specialByte)
    {
      // debugToPC("FoundSpecialByte");
      n++;
      x = x + tempBuffer[n];
    }
    dataRecvd[dataRecvCount] = x;
    dataRecvCount++;
  }
}

//====================

void dataToPC()
{

  // expects to find data in dataSend[]
  //   uses encodeHighBytes() to copy data to tempBuffer
  //   sends data to PC from tempBuffer
  encodeHighBytes();

  Serial.write(startMarker);
  // Serial.write(dataSendCount);
  Serial.write(tempBuffer, dataTotalSend);
  Serial.write(endMarker);
}

//============================

void encodeHighBytes()
{
  // Copies to temBuffer[] all of the data in dataSend[]
  //  and converts any bytes of 253 or more into a pair of bytes, 253 0, 253 1 or 253 2 as appropriate
  dataTotalSend = 0;
  for (byte n = 0; n < dataSendCount; n++)
  {
    if (dataSend[n] >= specialByte)
    {
      tempBuffer[dataTotalSend] = specialByte;
      dataTotalSend++;
      tempBuffer[dataTotalSend] = dataSend[n] - specialByte;
    }
    else
    {
      tempBuffer[dataTotalSend] = dataSend[n];
    }
    dataTotalSend++;
  }
}

//=========================

void debugToPC(char arr[])
{
  // byte nb = 0;
  Serial.write(startMarker);
  // Serial.write(nb);
  Serial.print(arr);
  Serial.write(endMarker);
}

//=========================

void debugToPC_2(int arr[], int N)
{
  // byte nb = 0;
  Serial.write(startMarker);
  // Serial.write(nb);
  for (int i = 0; i < N; i++)
  {
    Serial.write(arr[i]);
  }
  Serial.write(endMarker);
}

//=========================

void debugToPC(byte num)
{
  // byte nb = 0;
  Serial.write(startMarker);
  // Serial.write(nb);
  Serial.print(num);
  Serial.write(endMarker);
}

//=========================

void blinkLED(byte numBlinks)
{
  for (byte n = 0; n < numBlinks; n++)
  {
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);
  }
}

//=========================

void setMotorSpeed(int pin, int speed)
{
  // int arr[] = {motorID, speed};

  if (speed >= 0 && speed <= 255)
  {
    analogWrite(pin, speed);
    // debugToPC_2(arr, 2);
  }
}

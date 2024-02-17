//Distance Sensor
const int trigPin = 7;
const int echoPin = 10;
long duration;
int distance=27;//Our printer starts always from here

//Conversion factors(empirical)
float m = -0.043;
float q = 24.45;

//Electromagnet
const int magnetPin = 11;
int magnetDuty = 54;//it applies a DC voltage of 5 V

//DC motor1(printer)
const int in3Pin_horizontal = 9;//left movement
const int in4Pin_horizontal = 3;//right movement
const int duty_printer = 120;//experimental value

//DC motor2(pulley)
const int in1Pin_vertical = 6;//counter clockwise rotation(never used)
const int in2Pin_vertical = 5;//clockwise rotation
const int duty_up = 28;//experimental value
const int duty_up_object = 35;//experimental value
const int duty_down = 5;//experimental value
const int duty_pulley = 15;//experimental value
const int duty_pulley_object = 20;//experimental value

//SerialPort Connection
const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
boolean newData = true;

int dataNumber = 0;   

void setup() {
  // put your setup code here, to run once:
  pinMode(in1Pin_vertical, OUTPUT);
  pinMode(in2Pin_vertical, OUTPUT);
  pinMode(in3Pin_horizontal, OUTPUT);
  pinMode(in4Pin_horizontal, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(magnetPin, OUTPUT);

  Serial.begin(9600); // Starts the serial communication
}

void loop() {
  // main code here, to run repeatedly:
  //Read data
  recvWithEndMarker();
   if (newData == true)
  {
    int targetDist = atoi(receivedChars);
    double targetDistance = (double) targetDist;
    targetDistance = m*targetDistance+q;
    int converted_targetDistance = int(targetDistance);
    movePulley(duty_up);//pulls up the pulley
    analogWrite(in2Pin_vertical, duty_pulley);//it keeps high the pulley(a counter torque is applied)
    delay(500);
    MoveToLocation(converted_targetDistance);
    analogWrite(magnetPin, magnetDuty);//activate the electromagnet
    delay(500);
    movePulley(duty_down);//the pulley goes down and we apply a counter torque
    delay(5000);
    movePulley(duty_up_object);//pulls up the pulley with the object
    analogWrite(in2Pin_vertical, duty_pulley_object);//it keeps high the pulley with the object (a counter torque is applied)
    delay(500);
    MoveHome();
    delay(500);
    movePulley(duty_down);//the pulley goes down and we apply a counter torque
    analogWrite(magnetPin, 0);//deactivate the electromagnet
    newData=false;//end
  }
} 

void MoveToLocation(int dst)
{
  distance = getDistance();
  while (distance > 2 && distance > dst)
  {
  // Move left
    analogWrite(in3Pin_horizontal,duty_printer);
    analogWrite(in4Pin_horizontal,0);
    distance = getDistance();
  }
  //stop
 analogWrite(in3Pin_horizontal,0 );
 analogWrite(in4Pin_horizontal,0);

}

void MoveHome()
{
  distance = getDistance();
  while (distance < 27)
  {
  // Move right
    analogWrite(in3Pin_horizontal,0);
    analogWrite(in4Pin_horizontal,duty_printer);
    distance = getDistance();
  }
  //stop
   analogWrite(in3Pin_horizontal,0);
   analogWrite(in4Pin_horizontal,0);
}

void movePulley(int duty)
{
  analogWrite(in1Pin_vertical, 0);
  analogWrite(in2Pin_vertical, duty);
  delay(700);
  analogWrite(in1Pin_vertical, 0);
  analogWrite(in2Pin_vertical, 0);
}

int getDistance()
{
   //DISTANCE SENSOR: Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.println(distance);
  return distance;
}

void recvWithEndMarker() {
    static byte index = 0;
    char endMarker = 'a';
    char rc;
    
    if (Serial.available() > 0) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[index] = rc;
            index++;
            if (index >= numChars) {
                index = numChars - 1;
            }
        }
        else {
            receivedChars[index] = '\0'; // terminate the string
            index = 0;
            newData = true;
        }
    }
}


    <group>
        <group>
            <name>Turnout Servo PWM Calibration</name>
            <int size='2'>
                <name>Servo PWM Min</name>
                <description>PWM Value for Servo 0 Degree Position</description>
                <min>0</min><max>4095</max>
                <default>120</default>
            </int>
            <int size='2'>
                <name>Servo PWM Max</name>
                <description>PWM Value for Servo 180 Degree Position</description>
                <min>0</min><max>4095</max>
                <default>590</default>
            </int>
        </group>
        <group replication='8'>
            <name>Servos</name>
            <repname>Servo</repname>
            <string size='8'><name>Description</name></string>
            <group replication='3'>
                <repname>Position</repname>
                <eventid><name>EventID</name></eventid>
                <int size='1'>
                    <name>Servo Position in Degrees</name>
                    <min>0</min><max>180</max>
                </int>
            </group>
        </group>
    </group>
    <group replication='17'>
        <name>I/O</name>
        <repname>Pin 9</repname>
        <repname>Pin 10</repname>
        <repname>Pin 11</repname>
        <repname>Pin 12</repname>
        <repname>Pin 13</repname>
        <repname>Pin 14</repname>
        <repname>Pin 15</repname>
        <repname>Pin 16</repname>
        <repname>Pin 17</repname>
        <repname>Pin 18</repname>
        <repname>Pin 19</repname>
        <repname>Pin 20</repname>
        <repname>Pin 21</repname>
        <repname>Pin 22</repname>
        <repname>Pin 26</repname>
        <repname>Pin 27</repname>
        <repname>Pin 28</repname>
        <string size='8'><description>PDescription og this IO pin</description>/string>
        <group><name>Input/Producer</name>
          <int size=1>
            <name>Type:</name>
            <default>0</default> 
            <map>
              <relation><property>0</property><value>None</value></relation> 
              <relation><property>1</property><value>On/Off</value></relation> 
              <relation><property>2</property><value>Toggle</value></relation> 
              <relation><property>3</property><value>Frog0</value></relation> 
              <relation><property>4</property><value>Frog1</value></relation> 
              <relation><property>5</property><value>Frog2</value></relation> 
              <relation><property>6</property><value>Frog3</value></relation> 
              <relation><property>7</property><value>Frog4</value></relation> 
              <relation><property>8</property><value>Frog5</value></relation> 
              <relation><property>9</property><value>Frog6</value></relation> 
              <relation><property>10</property><value>Frog7</value></relation> 
            </map>
          </int>
          <eventid><name>Input high</name><description>Send this event when the input goes high</description> </eventid>
          <eventid><name>Input low</name><description>Send this event when the input goes low</description> </eventid>
        </group>
        <group><name>Output/Consumer</name>
          <int size=1>
            <name>Type:</name>
            <default>0</default> 
            <map>
              <relation><property>0</property><value>None</value></relation> 
              <relation><property>1</property><value>Non-inverted</value></relation> 
              <relation><property>2</property><value>Inverted</value></relation> 
            </map>
          </int>
          <int size=1>
            <name>On-time:</name>
            <description>For steady set to Forever, for pulse choose a pulse period
            <default>0</default> 
            <map>
              <relation><property>0</property><value>Steady</value></relation> 
              <relation><property>1</property><value>10 ms</value></relation> 
              <relation><property>2</property><value>20 ms</value></relation> 
              <relation><property>3</property><value>50 ms</value></relation> 
              <relation><property>4</property><value>100 ms</value></relation> 
              <relation><property>5</property><value>200 ms</value></relation> 
              <relation><property>6</property><value>500 ms</value></relation> 
              <relation><property>7</property><value>1 sec</value></relation> 
              <relation><property>8</property><value>2 sec</value></relation> 
              <relation><property>9</property><value>5 sec</value></relation> 
              <relation><property>10</property><value>10 sec</value></relation> 
              <relation><property>11</property><value>20 sec</value></relation> 
              <relation><property>12</property><value>50 sec</value></relation> 
            </map>
          </int>
          <int size=1>
            <name>Off-time:</name>
            <default>0</default> 
            <map>
              <relation><property>0</property><value>Steady</value></relation> 
              <relation><property>1</property><value>10 ms</value></relation> 
              <relation><property>2</property><value>20 ms</value></relation> 
              <relation><property>3</property><value>50 ms</value></relation> 
              <relation><property>4</property><value>100 ms</value></relation> 
              <relation><property>5</property><value>200 ms</value></relation> 
              <relation><property>6</property><value>500 ms</value></relation> 
              <relation><property>7</property><value>1 sec</value></relation> 
              <relation><property>8</property><value>2 sec</value></relation> 
              <relation><property>9</property><value>5 sec</value></relation> 
              <relation><property>10</property><value>10 sec</value></relation> 
              <relation><property>11</property><value>20 sec</value></relation> 
              <relation><property>12</property><value>50 sec</value></relation> 
            </map>
          </int>
          <eventid><name>Input high</name><description>Send this event when the input goes high</description> </eventid>
          <eventid><name>Input low</name><description>Send this event when the input goes low</description> </eventid>
        </group>
    </group>

#define NUM_SERVOS 8
#define NUM_IOPINS 17

          uint16_t ServoPwmMin;
          uint16_t ServoPwmMax;
          struct {
            char desc[8];        // description of this Servo Turnout Driver
            struct {
              EventID eid;       // consumer eventID
              uint8_t pos;       // position
            } pos[NUM_POS];
            uint8_t mspeed; // movement speed
            uint8_t pUp;    // profile 1->2,3 or 2->3
            uint8_t pDown;  // profile 3->2,1 or 2->1
            uint8_t rpin;   // relay pin for frog-polarity
          } servos[NUM_SERVOS];
          struct {
            char desc[8];       // description of this IO pin
            uint8_t iotype;     // type: none, non-invereted, inverted
            uint8_t onperiod, offperiod;
            EventID eOn;        // eventID to turn output on
            EventID eOff;       // eventID to turn output off
          } iopin[NUM_IOPINS];

const IOPins[] = { 9,10,11,12,13,14,15,16,17,18,19,20,22,26,27,28 };
const Period[] = { 0, 10,20,50,100,200,500,1000,2000,5000,10000,20000,50000,
                   60000, 120000, 240000, 600000, 1200000, 2400000 };

void serviceEvent(int index) {
  if(index<24) {
    ...
  } else {
    pindex = (index-24)/2;
    onoff = (index-24)%2;
    uint8_t pin = IOPins[ mem->iopin[pindex] ];
    state[pindex] = onoff; 
    if(onoff) stop[pindex] = ...onperiod;
  }
}
processProducers() {
  static int n = 0;
  if(state[n] && (millis()-...[n].onperiod)>stop[]) {
    state[n] = false;
    ...
  }
}

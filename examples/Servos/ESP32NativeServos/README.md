# ESP32NativeServos
This sketch implements servos on an ESP32 using the ESPServo library.  

It is very simple, but good easily be extended with the ServoEase library, say.

It CDI looks like the following in JMRI:

<img width="769" alt="image" src="https://github.com/user-attachments/assets/a3b039e2-af73-45dd-a14c-b1e8ed9097a7">

# Using with Wifi:

Open JMRI with a LCC Simulation connection.  Then open LCC/Start Hub and LCC/onfigure Nodes. 

In the sketch, change the following lines like so:

<img width="795" alt="image" src="https://github.com/user-attachments/assets/c785f117-4e34-4137-921f-af5a6cff6da7">

Then upload the sketch.  It should report:

<img width="257" alt="image" src="https://github.com/user-attachments/assets/23b54b4b-9600-4850-bdb1-a4a3a7d3ba72">

Go to your computers Wifi setting, or on your phone, and you should see: 

<img width="363" alt="image" src="https://github.com/user-attachments/assets/702d7da6-46ef-45f4-9332-f920c246b79e">

Click on WifiGCAP, and a window should open as below.  If it doesn't, try opening a browser windon 192.168.4.1

<img width="324" alt="image" src="https://github.com/user-attachments/assets/ff11271f-d037-4107-8a0f-e757fb62fbb6">

Click on "Configure Wifi" and fill in the approprite credentials, and click OK. 

The sketch should report: 

<img width="344" alt="image" src="https://github.com/user-attachments/assets/54400229-2611-4e68-9a27-2299dc7ffe6b">

If it doesn;t, try rebooting.  

Eventually the node should show up in the Network Tree Window:

<img width="426" alt="image" src="https://github.com/user-attachments/assets/db645157-41c5-4e64-bd27-0a2406020c83">

Open up the NOdes entry:

<img width="425" alt="image" src="https://github.com/user-attachments/assets/65462c2f-3e36-48c8-8160-56128a082c33">

and click on "Open COnfoguration Dialog", and a new window should open:

<img width="787" alt="image" src="https://github.com/user-attachments/assets/bd08ef1c-1753-4d13-9d6e-426cd721ed63">

Open the unnamed Segment entry, and you should get the view shown at the top on the page.  

You can try the sketch, if aa servo is attached to pin 25 by selecting Servo 1, and position 1, and then pushing 
the "More/Trigger" button.  This sends the associated eventid to the bus, and should move the servo.  

<img width="734" alt="image" src="https://github.com/user-attachments/assets/9d6ce571-68bd-4c8a-bd2f-407b3b93ff79">

Here is a servo connected to a M5 Atom, pin 25, 5V and ground: 


![AtomwithServo](https://github.com/user-attachments/assets/9bd35ec8-53d9-4f45-a1cd-918e63c82d86)












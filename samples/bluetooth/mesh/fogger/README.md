The purpose of this project is to replace the wired remote control that comes with an inexpensive fog machine with a custom one that is powered by a [Nordic Thingy:52](https://www.nordicsemi.com/Software-and-tools/Prototyping-platforms/Nordic-Thingy-52) so the machine can be controlled via [Bluetooth Mesh](https://www.bluetooth.com/learn-about-bluetooth/bluetooth-technology/mesh/).

![Fog machine with new remote]()

### About
Cheap fog machines usually have a wired remote control with a single button and an LED that indicates that the machine is ready to produce fog. Unfortunately, the simplicity of these machines makes hacking them more complicated. This project replaces the original remote control with a project box that mimics the original button and LED functionality. Inside the box a 5V wall wort is used to allow the Thingy:52 to both detect when the fog machine is ready and recharge the Thingy:52's battery. A relay replaces the original remote control's button to put the Thingy:52 into control without exposing it to AC.

The firmware exposes two mesh elements:
  1. Element 1
    1. Config Server
    1. Health Server
    1. Generic OnOff Server
  1. Element 2
    1. Generic OnOff Server

The Generic OnOff Server in element 1 works like the button on the remote control. Setting this element to 1 will produce fog until it is written back to 0 or the heater becomes active. It is automatically set back to 0 when the heater becomes active. If the fog machine is not able to produce fog then setting it to 1 has no effect.

The Generic OnOff Server in element 2 is set to 1 when the fog machine is capable of producing fog and 0 when the heater is heating. Writing to this element has no effect. A client can read this value to determine whether or not the fog machine will be able to immediately produce fog. Note that the heater can become active at any time.

The firmware for this project is based on the [Bluetooth Mesh Light sample](https://github.com/nrfconnect/sdk-nrf/tree/v1.3-branch/samples/bluetooth/mesh/light) in the [nRF Connect SDK](https://www.nordicsemi.com/Software-and-tools/Software/nRF-Connect-SDK) (NCS). A description of the hardware, including photos and schematics, is available [here](https://inductivekickback.blogspot.com/).

### Building
This project is built from the v1.3.0 tag of NCS. The recommended project location is "nrf/samples/bluetooth/mesh/fogger".
```
west build -b thingy52_nrf52832
```

### Provisioning and configuration
Provisioning assigns an address range to the device and adds it to the mesh network. Complete the following steps in the nRF Mesh app for [Android](https://play.google.com/store/apps/details?id=no.nordicsemi.android.nrfmeshprovisioner&hl=en) or [iOS](https://apps.apple.com/us/app/nrf-mesh/id1380726771):
* Tap `Add node` to start scanning for unprovisioned mesh devices.
* Select the `Mesh Light` device to connect to it.
* Tap `Identify` and then `Provision` to provision the device.
* When prompted select the OOB method and follow the instructions in the app.

Once the provisioning is complete the app returns to the Network screen.

Complete the following steps in the nRF Mesh app to configure models:
* On the Network screen, tap the `Mesh Light` node.
* Basic information about the mesh node and its configuration is displayed.
* In the Mesh node view, expand the first element.
* Tap `Generic OnOff Server` to see the model's configuration.
* Bind the model to application keys to make it open for communication:
  * Tap `BIND KEY` at the top of the screen.
  * Select `Application Key 1` from the list.
* In the Mesh node view, expand the second element.
* Tap `Generic OnOff Server` to see the model's configuration.
* Bind the model to application keys to make it open for communication:
  * Tap `BIND KEY` at the top of the screen.
  * Select `Application Key 1` from the list.  

# KitchenLightsController
IoT Particle Photon powered kitchen lights controller. Switch and dim LED and halogen lights.


Assembly
--------

Mount SMD components.

Solder on JP1, 12V DC power in.
For 2 & 3 min molex headers use sponge to keep connectors in place. Solder single pin, and then reflow that pushing the connector square to the board.
Solder JP35, JP47, JP34, JP48, JP49 - 3 pin molex connectors. Farnell: 9731156
Solder JP201, 202, 203, 301, 302, 303, 401, 402, 403. Optionally solder JP501 or 502. (or both). Fit only as many as you need. Farnell: 9731148

Solder JP43 (terminal block).


Fit C1 (330nF. Farnell: 1902922
Solder headers for Photon. eBay.
F1 Fuse folder. Farnell: 1453528
IC1 7805 or eBay switching version
C4 1000uF 16v. Farnell: 9693610
Connect JP37 if using onboard 5V supply for NeoPixels.
Fit X2 if using USB 5V switch.
Fuse 5A quick blow: Farnell: 1354578

Before powering up:
Check 12V rails for short circuit.
Check 5V rails for short.
Check 3v3 rails for short.

Power up using low current limit on bench PSU (ca. 50m-100A)
Check 5v rail and adjust reg as needed. Expect 3 LEDs to light (12v, 5V and ext. 5V). 14-20mA consumption.


V2.03 Changed:
Moved via under Photon to not mess with silk screen.
Moved OSH logo to near USB A Socket to give it more room and allow the 5V LED to be reverse mounted.
TODO:
Move 5V LED to be reverse mounted
Change 5V usb switched connector to be 0.1" molex not screw terminal.
Correct Channel 4 & Ext. 5V supply screw terminals to be the correct way around (double line on outside of the board).
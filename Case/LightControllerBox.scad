boxWidth = 114;
boxDepth = 100;
boxHeight = 40;
//boxHeight = 5;
standOffHeight = 8;
$fn = 20;

// Normal offset is center. Use this
// to push the box left/right/up/down
pcbXOffsetInBox = -0;
pcbYOffsetInBox = 2;

pcbWidth = 100;
pcbHeight = 90;
pcbMountOffset = 5; // m55 in from top/bottom for the PCB mounting hole

pcbOffsetX = ((boxWidth - pcbWidth) /2) + pcbXOffsetInBox; // 7.5mm
pcbOffsetY = ((boxDepth - pcbHeight) /2) + pcbYOffsetInBox;  // 15mm

switchDiameter = 17.5; 
switchX = (boxWidth/4) *3;
switchY = boxHeight - 15;

// Mounting. Tabs/Holes?
includeTabs = true;
includeHoles = false;
includeMountingPosts = false;

// Which part to build?
showBox = true;
showLid = false;
includeSwitchHole = false;

// Debugging
showPcb = false;
showSwitch = false;


// -----------------------------------------
// -----------------------------------------
module GenericBase(xDistance, yDistance, zHeight) {
	// roundedBox([xDistance, yDistance, zHeight], 2, true);

	// Create a rectangluar base to work from that
	// is xDistance by yDistance and zHeight height.

	// This is effectivly a cube with rounded corners

	// extend the base out by 3.5 from holes by using minkowski
	// which gives rounded corners to the board in the process
	// matching the Gadgeteer design
	
	$fn=30;
	radius = 5; //bezelSize;
	translate([radius,radius,0]) {
		minkowski()
		{
			// 3D Minkowski sum all dimensions will be the sum of the two object's dimensions
			cube([xDistance-(radius*2), yDistance-(radius*2), zHeight /2]);
			cylinder(r=radius,h=zHeight/2);
		}
	}
}

module OuterBox() {
    difference() {
        union() {
            GenericBase(boxWidth,boxDepth,boxHeight);
        }
        union() {
            translate([2,2,2]) {
                GenericBase(boxWidth - 4,boxDepth-4,boxHeight);
            }
            
            //translate([boxWidth /2,boxDepth/2,0]) {
            /*
            // Through hold
            translate([boxWidth /2,boxDepth-15,0]) {
                cylinder(d=15, h=3);
            }
            */
            
            // Drill out the mounting holes
            if (includeHoles) {
                MountingHoles();
            }
            
            if (includeSwitchHole) {
                SwitchHole();
            }
            
            cableExitHoles();
            
            usbConnectorHole();
            
            powerConnectorHole();
        }
    }
}

module cableExitHole(width) {
    #cube([width, 10,20]);
    translate([ (width/2), 10, 0]) {
        rotate([90,0,0]) {
            #cylinder(d=width, h=10);
        }
    }
}

module powerConnectorHole() {   
    
    translate([pcbOffsetX + 9.5, boxDepth-4,  standOffHeight+1]) {    
        #cube([16,7, 11]);
    }
}

// This may need supports.
module usbConnectorHole() {
    translate([boxWidth-4, pcbOffsetY + 36,  standOffHeight]) {    
        #cube([8,18, 8 + 3]);
    }
}

// Cut out holes in the wall for the light cables
// to exit.
module cableExitHoles() {
    
    translate([pcbOffsetX, boxDepth -4,  boxHeight-12]) {    
        // Channel 1
        translate([20.5, 0, 0]) {
            cableExitHole(5.2);
        }
        
        // Channel 2
        translate([37, 0, 0]) {
            cableExitHole(5.2);
        }
        
        // Channel 3
        translate([52.7, 0, 0]) {
            cableExitHole(5.2);
        }
        
        // Channel 4
        translate([65, 0, 0]) {
            cableExitHole(5.2);
        }
        
        // NeoPixels
        translate([76.7, 0, 0]) {
            cableExitHole(5.2);
        }
    }   
}

module PcbMounts() {
    
    // Mount holes @5,5 in corners of PCV        
    PcbMount(pcbOffsetX + pcbMountOffset, pcbOffsetY + pcbMountOffset);
    PcbMount(pcbOffsetX + pcbMountOffset, pcbOffsetY + pcbHeight - pcbMountOffset);
    PcbMount(pcbOffsetX + pcbWidth - pcbMountOffset, pcbOffsetY + pcbMountOffset);
    PcbMount(pcbOffsetX + pcbWidth - pcbMountOffset, pcbOffsetY + pcbHeight - pcbMountOffset);
}

module PcbMount(x,y) {
mountSize = 8;
mountHoleSize = 4.5;
    
    translate([x,y,0]) {
        // Outer pad + inner hole (Not all the way through!)
        difference() {
            union() {
                cylinder(d=mountSize, h=standOffHeight);
            }
            union() {
                translate([0,0,2]) {
                    #cylinder(d=mountHoleSize, h=standOffHeight-2);
                }
            }
        }
    }
}

module LidPosts() {
    LidPost(0,0);
    LidPost(boxWidth-(2*5),0);
    LidPost(0,boxDepth-(2*5));
    LidPost(boxWidth-(2*5),boxDepth-(2*5));
}

module LidPost(x,y) {
    mountSize = 7;
    mountHoleSize = 3; //M3 hole
    lidDepth = 0;

    translate([x + 5,y + 5,0]) {
        // Outer pad + inner hole (Not all the way through!)
        difference() {
            union() {
                cylinder(d=mountSize, h=boxHeight - lidDepth);
            }
            union() {
                // 10mm hole from top only
                translate([0,0,boxHeight - lidDepth - 10]) {
                    #cylinder(d=mountHoleSize, h=10);
                }
            }
        }
    }
}

module MountingHoles() {
    offsetFromCorner = 30;
    MountingHole(offsetFromCorner,offsetFromCorner);
    MountingHole(boxWidth-offsetFromCorner,offsetFromCorner);
    MountingHole(offsetFromCorner,boxDepth-offsetFromCorner);
    MountingHole(boxWidth-offsetFromCorner,boxDepth - offsetFromCorner);
}

module MountingHole(x,y) {
    translate([x,y,0]) {
        #cylinder(d=4, h=2);
    }
}

module MountingTabs() {
    
    tabOffset = 15;
    tabWidth = 12;
    tabLength = 8;
    
    MountingTab(0, tabOffset, tabWidth, tabLength);
    MountingTab(0, boxDepth - tabOffset - tabWidth, tabWidth, tabLength);
    
    translate([boxWidth,boxDepth,0]) {
        rotate([0,0,180]) {
            MountingTab(0,15, tabWidth, tabLength);
            MountingTab(0, boxDepth - tabOffset - tabWidth, tabWidth, tabLength);
        }
    }
}

module MountingTab(x,y, tabWidth, tabLength, tabThickness = 2) {
    
    translate([x - tabLength,y ,0]) {
        
        
        difference() {
            union() {
                cube([tabLength,tabWidth,tabThickness]);
                
                translate([0,(tabWidth/2) ,0]) {
                    cylinder(d=tabWidth, h=tabThickness);
                }
            }
            union() {
                // Hole for screw
                translate([0,(tabWidth/2) ,0]) {
                    cylinder(d=4, h=4);
                }
            }
        }
    }
}

module SwitchHole() {
        
    rotate([90,0,0]) {
        translate([switchX,switchY,-3]) {
            #cylinder(d=switchDiameter, h=5);
        }
    }
}

module ShowPcb(x,y) {
    
    color("purple") {
        translate([x,y,standOffHeight]) {
            
            difference() {
                union() {
                    GenericBase(pcbWidth,pcbHeight,1.6);
                    
                    // Add on the Photon
                    translate([7.5, 10.2, 0]) {
                        cube([37,21, 15.5]);
                        
                        // TODO: Add projection for plug.
                    }
                    
                    // Add on the USB connector.
                    translate([89.6, 37.6, 0]) {
                        cube([14,15, 9.5]);
                        
                        // Add projection for plug.
                        translate([16, 0, 0]) {
                            cube([14,15, 9.5]);
                        }
                    }
                    
                    // Add on the socket for the main DC input
                    translate([11.4, 82.7, 1.6]) {
                        cube([12,14.6, 8.5]);
                        
                        // TODO: Add projection for plug.
                        
                        translate([0, 16,1.6]) {
                            cube([12,25, 6]);
                        }
                    }
                    
                    // Add on the 7805
                    translate([95.5, 68, 0]) {
                        cube([8.5,10, 19]);
                    }
                    // ?? Heatsink for 7805?
                    
                    // Add sensor connectors
                    // Switch
                    translate([22, 0, 0]) {
                        cube([8, 6,18]);
                    }
                    
                    // PIR
                    translate([2, 32, 0]) {
                        cube([6, 8,18]);
                    }
                    
                    // Light
                    translate([2, 43, 0]) {
                        cube([6, 8,18]);
                    }
                    
                    // Temperature
                    translate([2, 56.5, 0]) {
                        cube([6, 8,18]);
                    }
                    
                    // Add light connectors
                    // Channel 1
                    translate([20.5, 59, 0]) {
                        cube([5.2, 19.8,18]);
                    }
                    
                    // Channel 2
                    translate([37, 59, 0]) {
                        cube([5.2, 19.8,18]);
                    }
                    
                    // Channel 3
                    translate([52.7, 59, 0]) {
                        cube([5.2, 19.8,18]);
                    }
                    
                    // Channel 4
                    translate([65, 71.5, 0]) {
                        cube([5.2, 6,18]);
                    }
                    
                    // NeoPixels
                    translate([76.7, 71.5, 0]) {
                        cube([7.6, 6, 18]);
                    }
                    
                    // NeoPixels capacitor
                    translate([80, 62.5, 0]) {
                        cylinder(d=10, h=19);
                    }
                }
                union() {
                    // Project the holes 8mm down to show the thread
                    // and 4mm.
                    translate([0,0,-8]) {
                        // Show holes for the PCB mounts
                        translate([5,5,0]) #cylinder(d=3, h=12);
                        translate([pcbWidth-5,5,0]) #cylinder(d=3, h=12);
                        translate([5,pcbHeight-5,0]) #cylinder(d=3, h=12);
                        translate([pcbWidth-5,pcbHeight-5,0]) #cylinder(d=3, h=12);
                    }
                }
            }
        }
    }
}

// Lid
module Lid() {
    // 2mm overlap.
    translate([-2,-2,0]) { 
        difference() {
            union() {
                GenericBase(boxWidth+4,boxDepth+4,8);
                
                //translate([2.1,2.1,-6]) {
                    //GenericBase(boxWidth - 4.2,boxDepth-4.2,8);
                //}
            }
            union() {
                //LidHoles();
                
                translate([1.5,1.5,0]) {
                    // 0.5mm gap all around.
                    #GenericBase(boxWidth + 1, ,boxDepth+1,6);
                }
            }
        }
    }
}

module LidHoles() {
    LidHole(0,0);
    LidHole(boxWidth-(2*5),0);
    LidHole(0,boxDepth-(2*5));
    LidHole(boxWidth-(2*5),boxDepth-(2*5));
}

module LidHole(x,y) {
    mountSize = 7;
    mountHoleSize = 3; //M3 hole
    lidDepth = 0;

    // 10mm cut 
    translate([x + 5,y + 5,-8]) {
        cylinder(d=mountSize+4, h=8);
    }
    
    translate([x + 5,y + 5,-10]) {
        cylinder(d=mountHoleSize+2, h=20);
    }
            
}

if (showBox) {
    OuterBox();
    PcbMounts();
    
    if (includeMountingPosts) {
        LidPosts();
    }
    
    if (includeTabs) {
        MountingTabs();
    }
}

if (showPcb) {
    // Debug help.
    ShowPcb(pcbOffsetX,pcbOffsetY);
}

if (showSwitch) {
    rotate([90,0,0]) {
        
        // Switch outer.
        translate([switchX,switchY,0]) {
            cylinder(d=18, h=3);
        }
        
        // Approx switch body length = 38mm;
        translate([switchX,switchY,-38]) {
            cylinder(d=16.5, h=38);
        }
    }
}

if (showLid) {
    translate([0,0,42]) {
        Lid();
    }
}
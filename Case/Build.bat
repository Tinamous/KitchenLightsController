@echo off

@echo Deleting old STL files.
del *.stl

@echo Building Case with switch
"C:\Program Files\OpenSCAD\openscad.com" -o LightControllerBox.stl -D showBox=true;showLid=false;showPcb=false;;showSwitch=false;includeTabs=true;includeSwitchHole=true LightControllerBox.scad

@echo Building Case without switch
"C:\Program Files\OpenSCAD\openscad.com" -o LightControllerBox-NoSwitch.stl -D showBox=true;showLid=false;showPcb=false;;showSwitch=false;includeTabs=true;includeSwitchHole=false LightControllerBox.scad

@echo Building Lid
"C:\Program Files\OpenSCAD\openscad.com" -o LightControllerBoxLid.stl -D showBox=false;showLid=true;showPcb=false;;showSwitch=false;includeTabs=true LightControllerBox.scad





@echo off

@echo Deleting old STL files.
del *.stl

@echo Building Case
"C:\Program Files\OpenSCAD\openscad.com" -o LightControllerBox.stl -D showBox=true;showLid=false;showPcb=false;;showSwitch=false;includeTabs=true LightControllerBox.scad

@echo Building Lid
"C:\Program Files\OpenSCAD\openscad.com" -o LightControllerBoxLid.stl -D showBox=false;showLid=true;showPcb=false;;showSwitch=false;includeTabs=true LightControllerBox.scad





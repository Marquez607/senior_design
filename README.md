#W Wheelson IoT Drone Platform
## MCU DETAILS 
### CC3235S <- base MCU 
* https://www.ti.com/product/CC3235S#design-development
### Launchpad :
* link: https://www.ti.com/tool/LAUNCHXL-CC3235S
### CC3235MODAS <- module that has antenna solution implemented
* link: https://www.ti.com/product/CC3235MODAS?keyMatch=&tisearch=search-everything&usecase=partmatches
### Launchpad :
* link: https://www.ti.com/tool/LAUNCHCC3235MOD

### NOTE: clean project before each build to make sure image file gets generated
* https://dev.ti.com/tirex/explore/node?node=AFiRjv8UYvIlGLFBWWEcQw__fc2e6sr__LATEST

### OUR BOARD
* may need to have a serial flash along side the MCU, steal the one from TI schematics
* Might only be necessary if we use the SysConfig ImageCreator 
* https://dev.ti.com/tirex/explore/node?node=AFiRjv8UYvIlGLFBWWEcQw__fc2e6sr__LATEST

<!-- ## Machine Learning 
Part of the project could be incorporating a camera to detect something. Right now it'd be 
cool if we could tell the drone to go to x,y gps location and look for trash. We can just 
get some trash dataset and train an support vector machine (or some other light weight ML model)

TinyML Guide (porting sklearn to c/++)
https://techcommunity.microsoft.com/t5/educator-developer-blog/using-tinyml-to-classify-audio/ba-p/2163436

Trash classification:
https://medium.com/geekculture/classifying-waste-images-with-machine-learning-14b249f11544#66db

Kaggle Waste Dataset:
https://www.kaggle.com/techsash/waste-classification-data -->

## WIFI CONNECTIVITY
For sanity's sake, the system will get deployed on using a LAN router we have control over
I'll be using a static IP address for the robot/drone that I'll configure using the router. 
Without a static IP, I'd probably have to seput some kind of website or blah blah blah 

## USER INTERFACE
We can quickly make a webapp using pywebio:
https://pywebio.readthedocs.io/en/latest/search.html?q=image&check_keywords=yes&area=default#

This can be hosted on the same "server" as the wifi drone controller which will just be a computer on our LAN network.
This way, it can be used on both PC and mobile without having to compile multiple applications and should be sufficient 
for the scope of this project. 

Jank way to implement video streaming in pywebio:
https://github.com/pywebio/PyWebIO/issues/107

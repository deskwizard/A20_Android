Commit of:
- New default wallpaper (orion nebula)
- Modification of Android Audio service for 30 steps of volume instead of 10

---------------------------------------------------------------------------------------------------

Initial commit of "full_base.mk" file:

Removed from base build:

   OpenWnn
   libWnnEngDic
   libWnnJpnDic
   libwnndict

---------------------------------------------------------------------------------------------------
Initial commit of:

1) opendingus "radio": Works with RDA5807 kernel driver for I2C radio control by command line
- Added Android.mk
- Direct tune is WORK IN PROGRESS
- ***  Warning : I2C bus is hardcoded in radio.c ***

2) i2c-tools 3.1.1: Created Android.mk file as required by howto *** Working ***

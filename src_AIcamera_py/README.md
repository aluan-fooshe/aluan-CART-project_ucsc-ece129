# /aluan-CART-project_ucsc-ece129/src_AIcamera_py
 
>⚠️ Note: This README belongs to the aluan-CART-project_ucsc-ece129
 repository.

----
----
## C.A.R.T. - Carry Assist Robotic Transport
### ECE 129A CAPSTONE Project Proposal (Fall 2025)

  Project 10  
  C.A.R.T. - Carry Assist Robotic Transport (Robotic_Cart_presented-draft.pdf)  
  Project Brief: [Carry_Assist_Robotic_Transport__ProjectBrief.pdf](docs/Carry_Assist_Robotic_Transport__ProjectBrief.pdf)  
  Presentation (presented in class): [Robotic_Cart_presented-draft.pdf](slides/Robotic_Cart_presented-draft.pdf)  
  Presentation (revised draft): [Carry_Assist_Robotic_Transport__Slideshow.pdf](slides/Carry_Assist_Robotic_Transport__Slideshow.pdf)


<br><br>
<b><u>Brief Summary of C.A.R.T</u></b>
<br>

```[Problem]```
Many individuals, including delivery workers, university students, and people with physical disabilities, face significant challenges in reaching various locations due to limited accessibility and the high cost of infrastructure.

----
----

## Important Notes & Updates

#### 1. The ".service" file

The "ai_camera_headless.service" file is required to be run in bash in order for the Raspberry Pi 5 to run the "ai_camera_headless.py" file in the background without the screen monitor setup or the CaptureCard + laptop setup.

```bash
$ sudo systemctl enable ai_camera_headless.service
```

In order to see the live output of the code, run this command in bash;

```bash
$ sudo journalctl -u ai_camera_headless.service -f
```

In order to stop the ai camera service, run this command in bash; 

```bash
$ sudo systemctl stop ai-camera_basicTest.service
```

The directory path of the original file on the Raspberry Pi 5 is "/etc/systemd/system/ai_camera_headless.service"


#### 2. Raspberry Pi 5 config setting modifications

These setting modifications are necessary since one of the UART pins on the Raspberry pi would be HIGH whenever the Raspberry Pi 5 would be rebooted.

```bash
$ sudo cat /boot/firmware/config.txt
# For more options and information see
# http://rptl.io/configtxt
# Some settings may impact device functionality. See link above for details

# Uncomment some or all of these to enable the optional hardware interfaces
#dtparam=i2c_arm=on
#dtparam=i2s=on
#dtparam=spi=on

# Enable audio (loads snd_bcm2835)
dtparam=audio=on

# Additional overlays and parameters are documented
# /boot/firmware/overlays/README

# Automatically load overlays for detected cameras
camera_auto_detect=1

# Automatically load overlays for detected DSI displays
display_auto_detect=1

# Automatically load initramfs files, if found
auto_initramfs=1

# Enable DRM VC4 V3D driver
dtoverlay=vc4-kms-v3d
max_framebuffers=2

# Don't have the firmware create an initial video= setting in cmdline.txt.
# Use the kernel's default instead.
disable_fw_kms_setup=1

# Run in 64-bit mode
arm_64bit=1

# Disable compensation for displays with overscan
disable_overscan=1

# Run as fast as firmware / board allows
arm_boost=1

[cm4]
# Enable host mode on the 2711 built-in XHCI USB controller.
# This line should be removed if the legacy DWC2 controller is required
# (e.g. for USB device mode) or if USB support is not required.
otg_mode=1

[cm5]
dtoverlay=dwc2,dr_mode=host

[all]
dtparam=uart0=off
enable_uart=0
dtoverlay=uart3-pi5
```
# Game-hacking

# Synopsis
The purpose of this repository is to keep track of my game hacking projects related to MMOs. A lot of this code is based on previous pixel searching bots I made for the same game, but I felt that they were too inefficient and limited. Thus, I created these memory and packet based bots that give me unlimited possibilities as far as automating the game. A lot of the game functions and base pointers were found using a combination of x86dbg and Cheat Engine debugger. The game client, which isn't posted here, also includes a patched in anticheat bypass that was very simple to make by simply following a few function calls in x86. This allows me to have complete control over the game without being kicked out or banned (unless I get caught by a GM that is).

# Motivation
Ever since I was young I have had a fascination with game hacking; being able to manipulate games to do whatever I want, figure out how they work to exploit them,
add new features or automate the boring parts is something that is exciting for me.

# Installation/Usage
This code isn't really intended to be used by anyone but me, so it is more of a demonstration of what I have accomplished. Because of this, a lot of this code may not be properly documented as it requires a detailed description of my reverse engineering efforts

# Description of files

## Keyboard_functions.h:
### Declares all of the functions that require interaction with the custom keyboard driver. Examples of functions included in this file include:
1. Sending a quick keystroke (key down, then key release)
2. Holding down input (Key down only, must be release manually)
3. Releasing input
4. Sending/Holding special input (such as shift, escape, control, page down, etc)
5. Sending a string to be typed
6. A movement function (Outdated, uses DirectInput to inject keys into game)

## Mouse_functions.h:
### Declares all of the functions that require interaction with the custom mouse driver. Examples of functions included in this file include:
1. Move cursor to an absolute location on the screen (for example, passing it X:400 and Y:500 will move the cursor to those exact coordinates on the screen)
2. Move cursor to a relative location on screen (for example, passing it X:100 and Y:50 will move the cursor 100 pixels to the right and 50 pixels down from its current      location
3. Send Left/Right click, as well as mousewheel up/down

## Dll.h:
### Declares and defines all general purpose functions used within the hack program. This is the main part of the project, and as such it has many functions that are specific to the game such as:
1. Packet encryption/decryption
2. Pointer initialization
3. Entity list enumeration
4. Bot functions (Trainers, autogatherers)

## Declarations.h
### Declares all general purpose variables used within the program. Includes:
1. Character values
2. Encryption table

## public_mouse/keyboard.h
### Used for declaring IOTCL commands for communication with respective drivers

## dllmain.cpp
### This is where the injection to the game occurs, as well as where the main thread runs

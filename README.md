# OpenGLProject

## Description

This project is a basic demonstration of OpenGL functionalities that I made as a part of my Computer Graphics exam.

It was written using C++ and the OpenGL 3.3 specification


## How to install

This project is built for Windows 32-bit on Visual Studio 2019, but works perfectly fine on 64-bit version of Windows too. 

Dependencies you need to install to run the project:
-Visual Studio 2019
-Assimp library for importing 3d models

To install assimp library you can go [here] (https://github.com/assimp/assimp/releases/tag/v3.3.1/) and download and install assimp-sdk-3.3.1-setup_vs2017.exe. After that go to (install-folder)/bin/x32 and copy assimp-vc140-mt.dll file to your system folder (usually Windows/System32 or Windows/SysWOW64). 

If you can't or don't want to go through this project dependancy installation process, there is a video demonstration of a project that I made using OBS studio.

## How to use

To run the project simply go to (repo-folder)/bin/Win32/Debug and run exe file or open and run whole solution from Visual Studio in base repo folder.

## Controls
| Key | Description |
| :---  | :--- |
| ```W``` , ```A```, ```S```, ```D``` | Moving the camera |
| ```F1``` | Open/Close GUI that let's you change scene parameters |
| ```Mouse left/right/up/down``` | Changing the camera angle |
| ```Mouse scroll``` | Zooming the camera |
| ```ESC``` | Exiting the game |
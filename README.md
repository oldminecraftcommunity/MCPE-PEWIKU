# Pewiku
**Whats it?**  
Implementation of a block-based game inspired by Minecraft.  
Right now this project corresponds to Minecraft Pocket Edition v0_6_2  

**Goals:**  
- Full compliance with MCPE 0.7 & 0.8  
- Saving MCPE experience from gameplay  

**Tasks is right now:**  
- Controls on PC
- Fix bugs  
- Add Unix building  
- Polish Android building  


# Assets
You should take the assets from the original MCPE or use your custom assets.  


# Building
You *can* to go to the `handheld`/`project` folder and select a platform, but it's better to build from the project root.

### Build all
useful for dev  
build linux and server: `make build_all`  
### Standalone server
build: `make build-server`  
build & run: `make run-server`  

### Windows
Building using Visual Studio 2022 is supported.  
Open the .sln, build, and run!  
### Linux (Debian)
Install all dependencies, see them: **`make deps`**  
build: `make build-client`  
build & run: `make run-client` (optional: add `DEBUG=1` for debug mode)  
### Android
android - new solution  
android_java - old for android 2.2  
*guide will appear later*  
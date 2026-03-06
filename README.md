# Pewiku
**Whats it?**  
Implementation of a block-based game inspired by Minecraft.  
Right now this project corresponds to Minecraft Pocket Edition v0.6.1  

**Goals:**  
- Full compliance with MCPE 0.7 & 0.8  
- Saving MCPE experience from gameplay  

**Tasks is right now:**  
- Fix bugs  
- Add Unix building  
- Polish Android building  


# Assets
You should take the assets from the original MCPE or use your custom assets.  


# Building
You need to go to the `handheld`/`project` folder and select a platform

### Build all
useful for dev  
build linux and server: `make build_all`  
### Standalone server
build: `make build_server`  
build & run: `make run_server`  

### Windows
Building using Visual Studio 2022 is supported.  
Open the .sln, build, and run!  
### Linux (Debian)
Install all dependencies, see them: **`make deps`**  
build: `make build_linux`  
build & run: `make run_linux`  
### Android
android - new solution  
android_java - old for android 2.2  
*guide will appear later*  
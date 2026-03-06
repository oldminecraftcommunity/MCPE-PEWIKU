cmake_minimum_required(VERSION 3.5)

find_package (Threads)

# set(CMAKE_C_FLAGS "-m32")
# set(CMAKE_CXX_FLAGS "-m32 -std=c++14")

set(HANDHELD_DIR "${PROJECT_SOURCE_DIR}/handheld")
set(PROJECT_DIR "${HANDHELD_DIR}/project")
set(SRC_DIR "${HANDHELD_DIR}/src")

include_directories("${PROJECT_DIR}/lib_projects/raknet/jni/RaknetSources")
add_subdirectory("${PROJECT_DIR}/lib_projects/raknet/jni" "${CMAKE_CURRENT_BINARY_DIR}/raknet")
add_definitions(-DSTANDALONE_SERVER)

set(CompileFiles
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/main_dedicated.cpp
    ${SRC_DIR}/platform/input/Controller.cpp
    ${SRC_DIR}/platform/input/Keyboard.cpp
    ${SRC_DIR}/platform/input/Mouse.cpp
    ${SRC_DIR}/platform/input/Multitouch.cpp
    ${SRC_DIR}/platform/time.cpp
    ${SRC_DIR}/platform/CThread.cpp
    ${SRC_DIR}/NinecraftApp.cpp
    ${SRC_DIR}/Performance.cpp
    ${SRC_DIR}/SharedConstants.cpp
    ${SRC_DIR}/client/IConfigListener.cpp
    ${SRC_DIR}/client/Minecraft.cpp
    ${SRC_DIR}/client/Options.cpp
    ${SRC_DIR}/client/OptionsFile.cpp
    ${SRC_DIR}/client/OptionStrings.cpp
    ${SRC_DIR}/client/gamemode/GameMode.cpp
    ${SRC_DIR}/client/gamemode/CreativeMode.cpp
    ${SRC_DIR}/client/gamemode/SurvivalMode.cpp
    ${SRC_DIR}/client/player/LocalPlayer.cpp
    ${SRC_DIR}/client/player/RemotePlayer.cpp
    ${SRC_DIR}/client/player/input/KeyboardInput.cpp
    ${SRC_DIR}/locale/I18n.cpp
    ${SRC_DIR}/nbt/Tag.cpp
    ${SRC_DIR}/network/command/CommandServer.cpp
    ${SRC_DIR}/network/ClientSideNetworkHandler.cpp
    ${SRC_DIR}/network/NetEventCallback.cpp
    ${SRC_DIR}/network/Packet.cpp
    ${SRC_DIR}/network/RakNetInstance.cpp
    ${SRC_DIR}/network/ServerSideNetworkHandler.cpp
    ${SRC_DIR}/server/ArgumentsSettings.cpp
    ${SRC_DIR}/server/ServerLevel.cpp
    ${SRC_DIR}/server/ServerPlayer.cpp
    ${SRC_DIR}/util/DataIO.cpp
    ${SRC_DIR}/util/Mth.cpp
    ${SRC_DIR}/util/StringUtils.cpp
    ${SRC_DIR}/util/PerfTimer.cpp
    ${SRC_DIR}/world/Direction.cpp
    ${SRC_DIR}/world/entity/AgableMob.cpp
    ${SRC_DIR}/world/entity/Entity.cpp
    ${SRC_DIR}/world/entity/EntityFactory.cpp
    ${SRC_DIR}/world/entity/FlyingMob.cpp
    ${SRC_DIR}/world/entity/HangingEntity.cpp
    ${SRC_DIR}/world/entity/Mob.cpp
    ${SRC_DIR}/world/entity/MobCategory.cpp
    ${SRC_DIR}/world/entity/Motive.cpp
    ${SRC_DIR}/world/entity/Painting.cpp
    ${SRC_DIR}/world/entity/PathfinderMob.cpp
    ${SRC_DIR}/world/entity/SynchedEntityData.cpp
    ${SRC_DIR}/world/entity/ai/control/MoveControl.cpp
    ${SRC_DIR}/world/entity/animal/Animal.cpp
    ${SRC_DIR}/world/entity/animal/Chicken.cpp
    ${SRC_DIR}/world/entity/animal/Cow.cpp
    ${SRC_DIR}/world/entity/animal/Pig.cpp
    ${SRC_DIR}/world/entity/animal/Sheep.cpp
    ${SRC_DIR}/world/entity/animal/WaterAnimal.cpp
    ${SRC_DIR}/world/entity/item/FallingTile.cpp
    ${SRC_DIR}/world/entity/item/ItemEntity.cpp
    ${SRC_DIR}/world/entity/item/PrimedTnt.cpp
    ${SRC_DIR}/world/entity/item/TripodCamera.cpp
    ${SRC_DIR}/world/entity/monster/Creeper.cpp
    ${SRC_DIR}/world/entity/monster/Monster.cpp
    ${SRC_DIR}/world/entity/monster/PigZombie.cpp
    ${SRC_DIR}/world/entity/monster/Skeleton.cpp
    ${SRC_DIR}/world/entity/monster/Spider.cpp
    ${SRC_DIR}/world/entity/monster/Zombie.cpp
    ${SRC_DIR}/world/entity/projectile/Arrow.cpp
    ${SRC_DIR}/world/entity/projectile/Throwable.cpp
    ${SRC_DIR}/world/entity/player/Inventory.cpp
    ${SRC_DIR}/world/entity/player/Player.cpp
    ${SRC_DIR}/world/food/SimpleFoodData.cpp
    ${SRC_DIR}/world/inventory/BaseContainerMenu.cpp
    ${SRC_DIR}/world/inventory/ContainerMenu.cpp
    ${SRC_DIR}/world/inventory/FillingContainer.cpp
    ${SRC_DIR}/world/inventory/FurnaceMenu.cpp
    ${SRC_DIR}/world/item/ArmorItem.cpp
    ${SRC_DIR}/world/item/BedItem.cpp
    ${SRC_DIR}/world/item/DyePowderItem.cpp
    ${SRC_DIR}/world/item/Item.cpp
    ${SRC_DIR}/world/item/ItemInstance.cpp
    ${SRC_DIR}/world/item/HangingEntityItem.cpp
    ${SRC_DIR}/world/item/HatchetItem.cpp
    ${SRC_DIR}/world/item/HoeItem.cpp
    ${SRC_DIR}/world/item/PickaxeItem.cpp
    ${SRC_DIR}/world/item/ShovelItem.cpp
    ${SRC_DIR}/world/item/crafting/ArmorRecipes.cpp
    ${SRC_DIR}/world/item/crafting/Recipe.cpp
    ${SRC_DIR}/world/item/crafting/Recipes.cpp
    ${SRC_DIR}/world/item/crafting/FurnaceRecipes.cpp
    ${SRC_DIR}/world/item/crafting/OreRecipes.cpp
    ${SRC_DIR}/world/item/crafting/StructureRecipes.cpp
    ${SRC_DIR}/world/item/crafting/ToolRecipes.cpp
    ${SRC_DIR}/world/item/crafting/WeaponRecipes.cpp
    ${SRC_DIR}/world/level/Explosion.cpp
    ${SRC_DIR}/world/level/Level.cpp
    ${SRC_DIR}/world/level/LightLayer.cpp
    ${SRC_DIR}/world/level/LightUpdate.cpp
    ${SRC_DIR}/world/level/MobSpawner.cpp
    ${SRC_DIR}/world/level/Region.cpp
    ${SRC_DIR}/world/level/TickNextTickData.cpp
    ${SRC_DIR}/world/level/biome/Biome.cpp
    ${SRC_DIR}/world/level/biome/BiomeSource.cpp
    ${SRC_DIR}/world/level/chunk/LevelChunk.cpp
    ${SRC_DIR}/world/level/dimension/Dimension.cpp
    ${SRC_DIR}/world/level/levelgen/CanyonFeature.cpp
    ${SRC_DIR}/world/level/levelgen/DungeonFeature.cpp
    ${SRC_DIR}/world/level/levelgen/LargeCaveFeature.cpp
    ${SRC_DIR}/world/level/levelgen/LargeFeature.cpp
    ${SRC_DIR}/world/level/levelgen/RandomLevelSource.cpp
    ${SRC_DIR}/world/level/levelgen/feature/Feature.cpp
    ${SRC_DIR}/world/level/levelgen/synth/ImprovedNoise.cpp
    ${SRC_DIR}/world/level/levelgen/synth/PerlinNoise.cpp
    ${SRC_DIR}/world/level/levelgen/synth/Synth.cpp
    ${SRC_DIR}/world/level/material/Material.cpp
    ${SRC_DIR}/world/level/pathfinder/Path.cpp
    ${SRC_DIR}/world/level/storage/ExternalFileLevelStorage.cpp
    ${SRC_DIR}/world/level/storage/ExternalFileLevelStorageSource.cpp
    ${SRC_DIR}/world/level/storage/FolderMethods.cpp
    ${SRC_DIR}/world/level/storage/LevelData.cpp
    ${SRC_DIR}/world/level/storage/LevelStorageSource.cpp
    ${SRC_DIR}/world/level/storage/RegionFile.cpp
    ${SRC_DIR}/world/level/tile/BedTile.cpp
    ${SRC_DIR}/world/level/tile/ChestTile.cpp
    ${SRC_DIR}/world/level/tile/CropTile.cpp
    ${SRC_DIR}/world/level/tile/DoorTile.cpp
    ${SRC_DIR}/world/level/tile/EntityTile.cpp
    ${SRC_DIR}/world/level/tile/FurnaceTile.cpp
    ${SRC_DIR}/world/level/tile/GrassTile.cpp
    ${SRC_DIR}/world/level/tile/HeavyTile.cpp
    ${SRC_DIR}/world/level/tile/LightGemTile.cpp
    ${SRC_DIR}/world/level/tile/MelonTile.cpp
    ${SRC_DIR}/world/level/tile/Mushroom.cpp
    ${SRC_DIR}/world/level/tile/NetherReactor.cpp
    ${SRC_DIR}/world/level/tile/NetherReactorPattern.cpp
    ${SRC_DIR}/world/level/tile/StairTile.cpp
    ${SRC_DIR}/world/level/tile/StemTile.cpp
    ${SRC_DIR}/world/level/tile/StoneSlabTile.cpp
    ${SRC_DIR}/world/level/tile/TallGrass.cpp
    ${SRC_DIR}/world/level/tile/Tile.cpp
    ${SRC_DIR}/world/level/tile/TrapDoorTile.cpp
    ${SRC_DIR}/world/level/tile/entity/ChestTileEntity.cpp
    ${SRC_DIR}/world/level/tile/entity/NetherReactorTileEntity.cpp
    ${SRC_DIR}/world/level/tile/entity/SignTileEntity.cpp
    ${SRC_DIR}/world/level/tile/entity/TileEntity.cpp
    ${SRC_DIR}/world/level/tile/entity/FurnaceTileEntity.cpp
    ${SRC_DIR}/world/phys/HitResult.cpp
)

message(${CMAKE_LIBRARY_ARCHITECTURE})
add_executable(pewiku_server ${CompileFiles})
target_link_libraries(pewiku_server raknet ${CMAKE_THREAD_LIBS_INIT})

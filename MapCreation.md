# Custom Map Creation Guide for Longvinter

## Table of Contents

- [Creating Your Map](#creating-your-map)
- [Designating Vendors / Travel Points](#designating-vendors--travel-points)
- [Compendium Areas](#compendium-areas)
- [Vendors](#vendors)
- [Quests](#quests)
- [Arenas](#arenas)
- [Bunkers and Caves](#bunkers-and-caves)
- [Plants, Fishing Spots, and Animals](#plants-fishing-spots-and-animals)
- [Music Playback](#music-playback)
- [Biomes](#biomes)
- [Cliffs & Roads](#cliffs--roads)
- [Example of Point Of Interest (POI)](#example-of-point-of-interest-poi)
- [Additional Notes](#additional-notes)


## Creating Your Map:

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/f2659cc6-a504-409d-9ac9-cc650c1d8768)

1. Open the Unreal Engine Editor. 
2. Navigate to Content-> Maps in the content browser. 
3. Duplicate the LongvinterIsland map and rename it. 
4. Begin creating your custom map.

## Designating Vendors / Travel Points:

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/ee199a81-dd45-49b2-9084-4c66d8aec0c0)

**Starting Points:** Within the map, you will find five boats grouped closely together. These denote the starting points for players when they join the game. Your map should include five distinct locations for players to start the game. These areas should provide a few fishing spots and items for players to gather and sell, enabling them to purchase their way to the Sgt. Lake base camp. The Sgt. Lake base camp can be modified to your liking, but it should always include a basic vendor.

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/27d5a1ba-68a4-4c80-b674-8e363a7e04bb)

To add more travel locations, duplicate an existing boat, move it to your chosen location, and rename it. Set the Starter Point Integer to 9999 for non-starting locations, designate the available services, and establish the travel price.

**IMPORTANT:** When duplicating the boat, ensure the Starter Point Integer is set to 9999. This denotes that it can be accessed from any location except the starting points.

## Compendium Areas:

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/bb1354ab-d4cb-4780-aa9b-2a47ce09e79a)

The example shows all the compendium vendors and their respective gates. You may choose to create a new island for each compendium or mix them together. Feel free to duplicate and use the access-limiting gates at multiple different locations.

## Vendors:

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/e51955d0-2bb7-434d-806a-253f451aa3ec)

Near the boats, you will find various vendors from the original map. Each location should include some vendors and turrets. Check inside the vendors for a vendor and quest catalog for examples of how to add new items for purchase and sale. These vendors can be freely duplicated and edited.

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/ed594ac8-433d-4648-b9c2-fe86c39c8f56)

Each vendor has a BuildingBlock component. If you want players to be able to build near the vendor, you can reduce the Sphere Radius of this component.

## Quests:

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/952596fa-fca6-47ff-91ed-f98685474069)

Quests include tasks and a reward. The Type is the quest's name, and you can add as many tasks as you like. However, you are limited to the existing names and tasks. You can customize the required number of completed tasks and the rewards. If you set a reward as Mk, add 1 item to the "RewardBasedOnType" array and set that as the amount of Mk to be awarded.

Quests and Tasks can be triggered with the actor A_QuestTrigger. You can place these on the map to initiate and trigger the quests listed.

## Arenas:

Your map should include at least two arenas for players to chase loot and battle each other. Remember to add an A_ArenaZone actor to your arena zone to enable PVP inside the arena for PVE servers.

## Bunkers and Caves:

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/85c8fe24-7ba3-4ed1-8468-a1b30f308fc7)

The bunkers and cave entrances can be placed anywhere on your map. However, there should only be one bunker per island. Unused bunkers can be hidden under the map below Sgt. Lake. The Repouuro cave is already on the map, and you are free to modify the area.

## Plants, Fishing Spots, and Animals:

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/9caea997-ffe6-49ff-9b52-48a40b66687e)

Above the compendium vendors are examples of all the plant, fishing spot, and animal spawns. These can be duplicated and scattered around the map to add points of interest. You can adjust the health and quantity of spawned NPCs and alter the fish spawn table to create your variations. Ensure all elements can be found.

## Music Playback:

Use A_PlayMusicVolume to play music when the player enters the volume. Here are the currently available songs:

- Wav_Music01
- Wav_Music03
- Wav_Music04
- Wav_Music07
- Theme_Sailing01
- Wav_MusicCave

## Biomes:

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/97502989-359e-4e62-88c7-f0f96b85068a)
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/3394c2fb-02e0-42d5-b97c-7f425003b9c4)

Ensure you have selected Landscape1 from the map.

Around the example actors, you will find examples of all the biomes: Summer, Spring, Winter, and Autumn. We recommend using the Foliage tool to place trees, bushes, rocks, and non-gameplay related static meshes, as they are instanced, allowing you to place many with minimal performance impact.

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/17eadc16-c7a8-4df4-884c-a993ba4635d4)

Press Shift + 1 to enter landscape editing, where you can Sculpt and Paint to create different islands. There are three different landscape heights as shown in the picture. Use the third tool to switch between the different heights:

1. Normal land height
2. Shallow water that doesn't drain more energy
3. Deep water that forces the player to swim

In the paint section, all available biome colors are available for you to paint onto the ground. We recommend keeping the tool strength at around a 0.5 value.

## Cliffs & Roads:

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/b1df86a7-9a6a-4f3f-bc67-b187b26f44dc)

In the summer biome example, you can observe cliffs and roads. Roads can guide players and help them find points of interest more easily. Place roads where you see fit and press the "CHECK ALL NEIGHBORS" button to round the ends. Note that there should be at least two blocks to create a rounded ending. 

For cliffs, refer to the examples in all biomes. All cliff controls are available in the details panel. You can construct up to three levels of cliffs.

## Example of Point Of Interest (POI):

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/95e0368b-f783-492e-869e-9a98d8c56442)

There are a few example POIs near the biomes. An essential element to replicate when creating more of these is the A_ObstructionVolume which conceals objects when the player enters the building. In the example, there are antennas and the hangar top. Some objects can be hidden and only revealed when the player enters the volume, with the objects to de-occlude. 

For POIs, the only style guide is that all entrances should face either 45 degrees to the left or right from straight down and never directly downwards.

## Additional Notes:

Utilize A_WaterBuildingAllowedArea for areas where players are allowed to build, usually in deep sea areas where players cannot block the boat paths.

Only official assets are permitted when building these maps.

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/48365153/203d394f-d40c-4e2a-a334-8f89ede7f685)

Refer to the prefab folders at Content->Prefabs for multiple elements that you can use when building the world. An efficient way to view all the different meshes is to add a filter to static meshes, as shown in the picture. This is the easiest method to find all possible elements to add to your map.

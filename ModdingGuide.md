# Longvinter Modding Guide

## Setup
1. Download [Unreal Engine 4.26](https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/) source build 
2. [Build](https://docs.unrealengine.com/4.26/en-US/ProductionPipelines/DevelopmentSetup/BuildingUnrealEngine/) the engine from the source
3. Add a new directory called `Projects/` to the `UE4Games.uprojectdirs` file located in the Unreal Engine root folder  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/59697938-5649-44e4-9f57-05fbdf65dab3)
4. Download the Longvinter modding project and place it inside `UE/Projects/` (Create the folder if it does not exist yet)
5. Run the `GenerateProjectFiles.bat` in your UE4 root directory
6. Open the `UE4.sln`
7. Verify that the build scripts are listed in the **Programs/Automation** directory  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/055aba11-be4a-4f50-b42c-79ebbcda0676)
8. Open **Properties** for `SimpleUGC.Automation` and choose the **Build** tab on the left. Set the **Output Path** to your source build's `Engine\Binaries\DotNET\AutomationScripts\` directory for both **Development and Debug Configurations**
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/37234462-af93-464b-b43f-1c86fbc05696)
10. **Build** the AutomationTool project
11. **Build** the Longvinter modding project in the Development configuration
12. **Build** and run the Longvinter modding project in the Development Editor configuration

**[More info about the SimpleUGC plugin and how to build/use it can be found here](https://github.com/EpicGames/UGCExample/blob/release/Documentation/QuickStart.md)**

## Making a Mod
1. Open the Longvinter modding project  
2. Go to `File` &rarr; `Create UGC`
3. Fill in the `Name`, `Author` and `Description` and press `Create Mod`
4. You should now have a new empty folder in the content browser

## Adding a new Item Data Holder
Right click in the empty mod folder and click `Blueprint Class`  

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/be1aa9cc-4722-4b76-89f4-1b8690bd6e62)

Select `A_ItemDataHolder` under the `All Classes` and add it to the mod

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/ed146279-3454-4bfc-8d18-045e561fa28f)

Now add a Replacement Actor Component to the new Item Data Holder so that it can replace the one already in the game

Open the Item Data Holder and press `Add Component` &rarr; `Replacement Actor`  

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/45fa471d-3b24-459c-b1bd-2664081ab514)

Click on the just added Replacement Actor and go to the details panel  
Under there you should see `Simple UGC` &rarr; `Actor Replacement`

Add `A_ItemDataHolder` to the Actor Classes to replace list  

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/4faf624e-7ba6-46d0-82e2-104153e97bda)

Lastly make sure that `Show Inherited Variables` is checked  

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/1c599ef7-ef5e-4474-a1a5-1baeb143aa86)

## Adding a new placeable

Start by right clicking on the mod folder again and adding a A_Placeable from the Blueprint Classes

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/aa98c667-2989-4666-9ece-4196adabddc6)

Open the new placeable and customize it to your liking  
You can for example change the model

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/809185e6-efc8-4b81-b1e4-6bc5bf922cba)  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/b58b6c49-364b-4c74-8784-d7977aef5a10)  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/51a93dba-a4ee-4c97-bb3a-d77f8f4781ac)

To change the default values select the root placeable

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/261319f1-9332-46f1-b2b7-78f0d24648ac)

If you want to edit the placement collision for example, you will need to check the `Make Custom Collision Check` box

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/7042cada-2df6-4853-b49f-878445450694)

After you are done with the placeable then it's time to add it to the Item Data Holder that we added earlier  

Go to the Item Data Handler and select the `AdditionalItems` variable

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/efd85c7c-3ffc-4c0d-b834-1f9504a3dcc2)

Now in the details panel you can add a new item under the Default Value section  

In this example I am changing the following:
1. `Name`
2. `Texture` which is the icon that shows in the inventory
3. `Category` which is placeable in this case
4. `SpawnedPlaceableReference` which is the actual placeable actor that we created

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/3b41cc63-d449-4d94-a12a-166f5c4592bc)

### Testing the mod

### Uploading to the Steam Workshop

...
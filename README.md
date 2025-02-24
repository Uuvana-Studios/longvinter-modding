# Longvinter Modding Guide
**For assistance and discussions, visit the 🔧-modding channel on the official **[Longvinter Discord](https://discord.com/invite/longvinter)**.**

### Table of contents
1. [Setup](#setup)
2. [Creating a mod](#creating-a-mod)
    1. [Item Data Holder](#adding-a-new-item-data-holder)
    2. [Placeable](#adding-a-new-placeable)
    3. [Weapon](#adding-a-new-weapon)
3. [Packaging](#packaging-the-mod)
4. [Testing](#testing-the-mod)
5. [Steam Workshop](#uploading-to-the-steam-workshop)

## Setup
1. Download [Unreal Engine 4.26](https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/) source build 
2. [Build](https://docs.unrealengine.com/4.26/en-US/ProductionPipelines/DevelopmentSetup/BuildingUnrealEngine/) the engine from the source
3. Add a new directory called `Projects/` to the `UE4Games.uprojectdirs` file located in the Unreal Engine root folder  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/59697938-5649-44e4-9f57-05fbdf65dab3)
4. Download this repository and place it inside `UE/Projects/` (Create the folder if it does not exist yet)
5. Run the `GenerateProjectFiles.bat` in your UE4 root directory
6. Open the `UE4.sln`
7. **Build** the **Longvinter** project in the **Development Editor** configuration  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/85a63e77-64a9-4106-bbb0-9575878be89a)  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/af57eadf-fece-4399-b38f-ff0177947ede)
9. Verify that the build scripts are listed in the **Programs/Automation** directory  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/055aba11-be4a-4f50-b42c-79ebbcda0676)
8. Open **Properties** for `SimpleUGC.Automation` and choose the **Build** tab on the left. Set the **Output Path** to your source build's `Engine\Binaries\DotNET\AutomationScripts\` directory for both **Development and Debug Configurations**
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/37234462-af93-464b-b43f-1c86fbc05696)
10. **Build** the **AutomationTool** project in the **Development** config  
12. Right click the **Longvinter** project and click **Set as Startup Project**  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/adcee4e6-4a2b-4239-910b-da58f38b564b)
14. Run the **Longvinter** project in **Development Editor** config  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/b68f6747-5d24-491f-bbc2-269da8dc1dce)

**[More info about the SimpleUGC plugin and how to build/use it can be found here](https://github.com/EpicGames/UGCExample/blob/release/Documentation/QuickStart.md)**

## Creating a Mod
1. Open the Longvinter modding project  
2. Go to `File` &rarr; `Create UGC`
3. Fill in the `Name`, `Author` and `Description` and press `Create Mod`
4. You should now have a new empty folder in the content browser

### Adding a new **Item Data Holder**
The **Item Data Holder** has a list of every single item in the game  
Making a custom Item Data Holder allows you to add new items and replace existing ones

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

### Adding a new **Placeable**

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

Go to the Item Data Holder and select the `AdditionalItems` variable

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/efd85c7c-3ffc-4c0d-b834-1f9504a3dcc2)

Now in the details panel you can add a new item under the Default Value section  

In this example I am changing the following:
1. `Name`
2. `Texture` which is the icon that shows in the inventory
3. `Category` which is placeable in this case
4. `SpawnedPlaceableReference` which is the actual placeable actor that we created

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/3b41cc63-d449-4d94-a12a-166f5c4592bc)

### Adding a new **Weapon**

Start by going to your Item Data Holder and clicking the AdditionalItems variable

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/efd85c7c-3ffc-4c0d-b834-1f9504a3dcc2)

Add a new element to the array and change the values to your liking

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/4507b5bb-7849-4237-9b70-454e4cc01d38)

To make a weapon you will need the following
1. `Wearable` set to **true**
2. `Weapon` set to **true**
3. `Slot` set to **2**
4. `Category` set to **Equipment**

Inside WeaponStats you will find all weapon related values like the damage and fire rate

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/733c2db2-99dd-4d15-b7de-af43f1d51afe)

## Packaging the mod

Before the mod can be loaded in the shipping verion of the game it needs to be packaged  
Test your mod before packaging by clicking the play in editor button

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/63d1689c-39bf-42e7-a4bb-82ad9181fb14)

To package your game just press `File` &rarr; `Package UGC`

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/fedb3a15-6af4-42e4-abe3-382044005345)

After the engine is done packaging you should get a packaged .zip file of the mod

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/d8fb3111-1c38-4e07-8954-023da58efef3)

## Testing the mod

Extract the .zip file to `\Longvinter\SteamMods\<mod_name>`  
Make sure that the `<mod_name>` folder is named the same as the **.uplugin** file you just extracted  
If you are missing the `SteamMods` folder then run the `MakeSymbolicLink.bat` in the game folder as adminstrator  

You should now have something like this  
`F:\SteamLibrary\steamapps\common\Longvinter\Longvinter\SteamMods\test`  
![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/062390e9-09db-432d-8c3b-c62a7887a238)

The last step is to run the `CopyMods.bat` in the game folder as adminstrator  
This should copy your files from the `SteamMods` folder to the `Mods` folder in a correct format

Now you can simply start your game from steam and go to the host server tab to test

## Uploading to the Steam Workshop

Uploading to the workshop is done through the workshop upload tool in game  
You can find the upload tool on the mods page in the settings

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/53ba7ce2-872e-4013-b21f-f9e246797868)

Now just fill in the information and copy paste the `Mod folder` path  
For the `Preview image` make sure that the image is less than **1MB** in size and in a **square format**  
The usual display size in the Steam Workshop is 200×200 pixels

![image](https://github.com/Uuvana-Studios/longvinter-modding/assets/73204452/a52ea15e-ba2f-468a-b79c-69fdcde99b90)

After clicking the upload button the Mod will start uploading  
If the mod upload was successful you should get **OK** and the window should close  
If you get **LimitExceeded** it means that your preview image is too large

**You can find the files to the unpackaged mod created in this guide on the [Steam Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=2976144882).**

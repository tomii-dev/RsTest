# RsTest
## Overview
RsTest hopes to provide an engine-agnostic solution to testing various parts of the RenderStream protocol provided by disguise (https://www.disguise.one/renderstream), and to serve as a rather extensive example of RenderStream API integration.

Built with OpenGL, RsTest allows you to create scenes, add/remove objects, make changes to lighting, set textures, and many more. All of the objects in your RsTest scene can be manipulated within the disguise software, through the use of remote parameters. 

## Setup
You **MUST** have the disguise software installed to use this application; and if you don't have it installed I'm not really sure what you're up to here but it's great to have ya.

If you head to the releases, you can grab an installer that will set up everything you need to get going. After that, just open up your d3 project, right click on the timeline and make a RenderStream layer as shown below.

![](https://i.imgur.com/qSguMLC.png)

Hit OK and make sure you set the layer type to RenderStream. Right click workload, and set the Asset to be rstest.

![](https://i.imgur.com/8l8JxYS.png)

Your layer can be mapped to a surface, or to a camera plate; the choice is yours. For more information, check out this link to learn more about the software: https://www.disguise.one/en/learn-support/ 

When mapped to a camera, RsTest will respond to the camera's movement and rotation and allow you to move around in the RsTest scene using the d3 camera. To map RsTest to a camera, first create a new channel mapping in your cluster workload.

![](https://i.imgur.com/hd5MfSv.png)

Now click the mapping it creates and it will bring up this window.

![](https://i.imgur.com/hWh1fG4.png)

Create a new mapping, call it whatever you want, I'm calling mine "camera thing" (descriptive I know), and make sure to set its type to CameraPlate. Then in the window that pops up add your camera to the Screens.

![](https://i.imgur.com/wau29BB.png)

With that all done, upon hitting Start on the workload RsTest will now map to the front of our camera, and if we have a look through the view of our camera, our beautiful RsTest scene appears.

![](https://i.imgur.com/rUztSrL.png)

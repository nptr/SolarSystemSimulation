# Solar System Simulation

"Solar System Simulation", despite the name, is not really a simulation, but only animated and serves me as computer graphics playground. It started as university project with fixed pipeline OpenGL which I evolved further in the course and later in my free time.<br>


![Some Snapshots](https://user-images.githubusercontent.com/5485569/96901273-57c1d180-1493-11eb-8456-9f4e84456c9b.png)

Now it features a variety of things:
+ Programmable Pipeline (w/ OpenGL 3.3)
+ Variance Shadow Mapping (VSM)
+ Point-, Spot- and Directional Lights (all with VSM)
+ Phong Reflection Model
+ Normal & Specular Mapping
+ Discrete Level-of-Detail
+ Multisample Anti-Aliasing
+ Glow Effects using Gaussian Blur
+ Correct Transparency
+ Ray-Sphere Intersection
+ Sky "Box" without geometry
+ Gamma Awareness

## Usage
| Action           | Keys        |
|------------------|-------------|
| Simulation Speed | 0-9         |
| Movement         | WASD        |
| Rotation         | RMB + Mouse |
| Object Selection | LMB         |
| Fullscreen       | F11         |

## Building
Use the Visual Studio 2013 C++ solution. No platform specific code is used (directly), so it might<br> run under Linux with only reasonable amount of work required. Dependencies (not included) are:
+ GLFW (Linking required) ([Github](https://github.com/glfw/glfw))([WWW](https://www.glfw.org/))
+ GLM (Header only) ([Github](https://github.com/g-truc/glm))([WWW](https://glm.g-truc.net/))

Other libraries I used (which are included):
+ ImGui ([Github](https://github.com/ocornut/imgui))
+ stb_image.h ([Github](https://github.com/nothings/stb))

## References
Because I had to learn from someone. Also, in case you feel I have forgotten a reference here, message me!

__VSM Basics:__<br>
Fabien Sanglard, _"Softshadow with GLUT,GLSL and VSM"_, 2009, https://fabiensanglard.net/shadowmappingVSM/

Andrew Lauritzen, _"Summed-Area Variance Shadow Maps"_, 2009, GPU Gems 3, https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps

__Model Loading and Normal Mapping:__<br>
Calvin1602 et. al., _"Free tutorials for modern Opengl (3.3 and later) in C/C++"_, http://www.opengl-tutorial.org/

__Star & Night Sky Rendering:__<br>
Krishty, _"Hochwertiges Rendern von Sternen"_, 2009, https://zfx.info/viewtopic.php?f=11&t=08


## Textures

__Mercury__<br>
https://www.solarsystemscope.com/textures/

__Venus__<br>
https://bjj.mmedia.is/data/venus/venus.html<br>
by Björn Jónsson

__Earth__<br>
May, Blue Marble Next Generation w/ Topography, from<br>
https://visibleearth.nasa.gov/images/74343/may-blue-marble-next-generation-w-topography<br><br>
"Africa – cloudy and hazy everywhere except Africa and Middle East" From "Natural Earth III"<br>
on Tom Patterson, www.shadedrelief.com<br>

__Mars__<br>
http://planetpixelemporium.com/mars.html<br>
by James Hastings-Trew

__Jupiter__<br>
http://bjj.mmedia.is/data/jupiter_css/index.html<br>
by Björn Jónsson

__Saturn__<br>
http://planetpixelemporium.com/saturn.html<br>
https://www.deviantart.com/alpha-element/art/Stock-Image-Saturn-Rings-393767006

__Uranus__<br>
http://planetpixelemporium.com/uranus.html<br>
by James Hastings-Trew

__Neptune__<br>
http://planetpixelemporium.com/neptune.html<br>
by James Hastings-Trew

__Moon__<br>
https://www.solarsystemscope.com/textures/

__Io__<br>
based on https://astrogeology.usgs.gov/maps/io-voyager-galileo-global-mosaics<br>
It differs in details however and I can't find the source anymore.

__Europa__<br>
http://stevealbers.net/albers/sos/sos.html#EUROPA<br>
by Steve Albers
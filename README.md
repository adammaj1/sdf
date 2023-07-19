



* [Disk - distance 2D in Shadertoy by IQ](https://www.shadertoy.com/view/3ltSW2) 
* [2D distance functions by Inigo Quilez]( https://iquilezles.org/articles/distfunctions2d/)
* [explanation : Shader Tutorial | Intro to Signed Distance Fields on YouTube by Suboptimal Engineer](https://www.youtube.com/watch?v=pEdlZ9W2Xs0) 
 





# Images

Test image = unit circle 
 
![](./png/circle_1000.png "description") 

8 bit color 

Continous gradient 

![](./png/circle_sdf_8bit_cont_1000.png "description") 

![](./png/circle_sdf_8bit_cont_with_circle_1000.png "description") 


Discrete gradient = Level Set Method ( LSM)

![](./png/circle_sdf_8bit_discrete_1000.png "description") 

![circle_sdf_8bit_discrete_LCM_1000.png](./png/circle_sdf_8bit_discrete_LCM_1000.png "description") 

![circle_sdf_8bit_discrete_LSCM_1000.png](./png/circle_sdf_8bit_discrete_LSCM_1000.png "description") 

![circle_sdf_8bit_discrete_LSCM_with_circle_1000.png](./png/circle_sdf_8bit_discrete_LSCM_with_circle_1000.png "description") 

24 bit color

![](./png/circle_sdf_24bit_1000.png "description") 


![](./png/circle_sdf_24bit_white_1000.png"description") 

# source code 
* [d.c](./src/d.c) - c code = one file program
* [d.sh](./src/d.sh)- bash file
* [Makefile](./src/Makefile) - Makefile


To run the program

  make


Dependencies
* Gnu Make
* Image Magic convert
* Bash
* gcc




# Git


Subdirectory


```
mkdir png
git add *.png
git mv  *.png ./png
git commit -m "move"
git push -u origin main
```

then link the images:

   ![](./png/n.png "description") 

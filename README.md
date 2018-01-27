# collide2d

Author: Sebastien Alaiwan

A tiny standalone 2D collision detection and response module,
and a SDL demo app to test it.

<p align="center"><img src="screenshot.gif" width="50%"></p>

To run the demo, simply launch:
```
./run.sh
```

You can also compile it yourself, using the following command:
```
g++ sdl_demo.cpp collision.cpp `sdl2-config --cflags --libs` -o collide2d
```


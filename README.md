# ball-and-stick-man

A homework for Computer Graphics class

![in-game footage of project](img/footage.gif)

## Features

-   Adjustable joints for arms, forearms and legs
-   Wave and walk in-place (with ease)
-   Walk around
-   Camera rotation and elevation (with auto-adjusting focus spot)

# Controls

## Movement

```
                        Forward
                           |
              Left arm     |     Right arm
                     |     |     |
    Left forearm     |     |     |     Right forearm     Camera up
               |     |     |     |     |                    |
            *-----*-----*-----*-----*-----*              *-----*
            |  Q  |  W  |  E  |  R  |  T  |              |  ▲  |
            *-----*-----*-----*-----*-----*        *-----*-----*-----*
                  |  S  |  D  |  F  |              |  ◀︎  |  ▼  |  ►  |
                  *-----*-----*-----*              *-----*-----*-----*
                     |     |     |                    |     |     |
                  Left     |      Right     Camera left     |     Camera right
                           |                                |
                       Backward                        Camera down
```

> Toggle the caps to reverse the effect.

## Animations

-   **Left-click mouse:** Switch between walking modes
-   **Right-click mouse:** Toggle waving

## Requirements

-   glut

### Instructions for Windows/Visual Studio

-   Download `glut` and place its files into those paths:

    | File       | Location                                                                              |
    | ---------- | ------------------------------------------------------------------------------------- |
    | glut32.h   | C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\VS\include |
    | glut32.lib | C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\VS\lib\x86 |
    | glut32.dll | C:\Windows\SysWOW64                                                                   |

-   Build the code from Visual Studio

### Instructions for Mac

-   Mac has glut already. Compile and run the code with command below:

    ```
    g++ -o ball-and-stick-man.o src/main.cpp -framework GLUT -framework OpenGL -std=c++11 -Wno-c++11-narrowing
    ./ball-and-stick-man.o
    ```

## License

GNU General Public License v3.0  
Read the [LICENSE](LICENSE) file for details

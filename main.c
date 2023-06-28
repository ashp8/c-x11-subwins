#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define NUM_SUB_WINDOWS 4

typedef struct{
   int type;
   Window window;
   int x;
   int y;
} CustomEvent;

unsigned long getRandomColor(Display *display);
Window createWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned long border, unsigned long background);

int main(int argc, char *argv[]){
    Display *display;
    Window mainWin;
    Window subWins[NUM_SUB_WINDOWS];
    XEvent event;
    XWindowAttributes mainWinAttrs;
    int offsetX[NUM_SUB_WINDOWS], offsetY[NUM_SUB_WINDOWS];
    int subWinX[NUM_SUB_WINDOWS], subWinY[NUM_SUB_WINDOWS];
    int dragInProgress[NUM_SUB_WINDOWS] = {0};

    display = XOpenDisplay(NULL);
    if(display == NULL){
        fprintf(stderr, "Cannot open display!\n");
        return 1;
    }

    srand(time(NULL));
    
    mainWin = createWindow(display, DefaultRootWindow(display), 100, 100, 400, 300, 1, WhitePixel(display, DefaultScreen(display)));

    int xStep = 50;
    int yStep = 50;
    int subWinWidth = 50;
    int subWinHeight = 50;

    for(int i = 0; i < NUM_SUB_WINDOWS; i++){
        unsigned long color = getRandomColor(display); 
        subWinX[i] = (i % 2) * xStep;
        subWinY[i] = (i / 2) * yStep;
        subWins[i] = createWindow(display, mainWin, subWinX[i], subWinY[i], subWinWidth, subWinHeight, 1, color);
    }

    while(1){
        XNextEvent(display, &event);

        switch (event.type) {
            case Expose:
                for(int i = 0; i < NUM_SUB_WINDOWS; i++){
                    XWindowAttributes subWinAttr;
                    XGetWindowAttributes(display, subWins[i], &subWinAttr);
                    XMoveResizeWindow(display, subWins[i], subWinX[i], subWinY[i],  subWinAttr.width, subWinAttr.height);
                }
                break;
            case ButtonPress:
                for(int i = 0; i < NUM_SUB_WINDOWS; i++){
                    if(event.xbutton.button == Button1 && event.xbutton.window == subWins[i]){
                        offsetX[i] = event.xbutton.x_root - subWinX[i];
                        offsetY[i] = event.xbutton.y_root - subWinY[i];
                        dragInProgress[i] = 1;

                        XRaiseWindow(display, subWins[i]);
                    }
                }
                break;
            case ButtonRelease:
                for(int i = 0; i < NUM_SUB_WINDOWS; i++){
                    if(event.xbutton.button == Button1 && dragInProgress[i]){
                        dragInProgress[i] = 0;
                        subWinX[i] = event.xbutton.x_root - offsetX[i];
                        subWinY[i] = event.xbutton.y_root - offsetY[i];
                    }
                }
                break;
            case MotionNotify:
                for(int i = 0; i < NUM_SUB_WINDOWS; i++){
                    if(dragInProgress[i]){
                        subWinX[i] = event.xmotion.x_root - offsetX[i];
                        subWinY[i] = event.xmotion.y_root - offsetY[i];
                        XMoveWindow(display, subWins[i], subWinX[i], subWinY[i]);
                    }
                }
                break;
            case ClientMessage:
                break;
        }

    }

    return 0;
}

unsigned long getRandomColor(Display *display){
    XColor color;
    color.red = rand() % 65536;
    color.green = rand() % 65536;
    color.blue = rand() % 65536;
    color.flags = DoRed | DoGreen | DoBlue;
    XAllocColor(display, DefaultColormap(display, DefaultScreen(display)),&color); 
    return color.pixel;
}

Window createWindow(Display *display, Window parent, int x, int y, unsigned int width,
        unsigned int height, unsigned long border, unsigned long background){
    Window window = XCreateSimpleWindow(display, parent, x, y, width, height, border,border, background);
    XSelectInput(display, window, ButtonPressMask | ButtonReleaseMask | Button1MotionMask | ExposureMask);
    XMapWindow(display, window);
    return window;
}


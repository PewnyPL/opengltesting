#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h> //Biblioteka pomocnicza
#include <iostream>
#include <fstream>

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

GLint szer = 640; //
GLint wyso = 480; // wysokoœc i szerokosc okna

GLvoid ReSizeGLScene(GLsizei width, GLsizei height) //prosta metoda do obslugi zmiany rozmiaru okna
{
    if (height==0) height=1; //aby zapobiec dzieleniu przez zero

    glViewport(0,0,width,height); //ustawienie rozmiaru obszaru OpenGL do rozmiaru okna

    glMatrixMode(GL_PROJECTION); //macierz projekcji
    glLoadIdentity(); //reset macierzy
    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f); //zmiana perspektywy i wspólczynnika rozmiaru okna

    glMatrixMode(GL_MODELVIEW); //macierz modelu
    glLoadIdentity();
}

int InitGL()
{
    glShadeModel(GL_SMOOTH); //wygladzanie swiatla
    glClearColor(0.0f,0.0f,0.0f,0.0f); //tlo
    glClearDepth(1.0f); //bufor glebokosci
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); //korekcja perspektywy
    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          szer,
                          wyso,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    GLfloat * xs; //punkty
    GLfloat * ys;
    GLfloat * zs;

    GLfloat * nxs; //wektory normalne
    GLfloat * nys;
    GLfloat * nzs;

    GLfloat * us; //mapowanie tekstur
    GLfloat * vs;
    int ile;

    std::ifstream plik("nautilus.3d");
    if(plik)
    {
        plik>>ile;
        xs = new GLfloat[ile];
        ys = new GLfloat[ile];
        zs = new GLfloat[ile];
        nxs = new GLfloat[ile];
        nys = new GLfloat[ile];
        nzs = new GLfloat[ile];
        us = new GLfloat[ile];
        vs = new GLfloat[ile];
        for(int i=0;i<ile;i++)
        {
            plik>>xs[i];
            xs[i]/=60.0f;
            plik>>ys[i];
            ys[i]/=60.0f;
            plik>>zs[i];
            zs[i]/=60.0f;
            plik>>nxs[i];
            plik>>nys[i];
            plik>>nzs[i];
            plik>>us[i];
            plik>>vs[i];
        }
        plik.close();
    }

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glLoadIdentity();

            glTranslatef(0.0f,-2.0f,-6.0f);

            glPushMatrix();
            glRotatef(theta, 0.0f, 1.0f, 0.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<ile; i++)
                {
                    glVertex3f(xs[i],ys[i],zs[i]);
                }
            glEnd();

            glPopMatrix();

            SwapBuffers(hDC);

            theta += 1.0f;
            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        case WM_SIZE:
        {
            ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));
            break;
        }

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
    ReSizeGLScene(szer,wyso);
    InitGL();
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}


#ifndef ARDUINO

#include <windows.h>
#include <stdlib.h>

#include "game.hpp"

static HWND hwnd;
static uint8_t* pixels;
static HDC hdc;
static HDC hdc_mem;
static PAINTSTRUCT paint{};
static int ww, wh;
static HBRUSH brush_black = NULL;

static DWORD const dwStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
static DWORD const dwExStyle = WS_EX_CLIENTEDGE;

static constexpr int ZOOM = 6;
static constexpr int FBW = 128;
static constexpr int FBH = 64;

uint8_t wait_btn()
{
    MSG msg{};
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if(msg.hwnd == hwnd && msg.message == WM_KEYDOWN)
        {
            switch(msg.wParam)
            {
            case VK_UP   : return BTN_UP;
            case VK_DOWN : return BTN_DOWN;
            case VK_LEFT : return BTN_LEFT;
            case VK_RIGHT: return BTN_RIGHT;
            case 'A'     : return BTN_A;
            case 'B'     : return BTN_B;
            default      : break;
            }
        }
    }
    exit(0);
}

static void refresh()
{
    InvalidateRect(hwnd, NULL, FALSE);
}

void paint_left()
{
    for(int i = 0; i < 512; ++i)
    {
        int x = i % 64;
        int y = (i / 64) * 8;
        uint8_t b = buf[i];
        for(int j = 0; j < 8; ++j, b >>= 1)
        {
            uint8_t color = (b & 1) ? 0xff : 0x00;
            uint8_t* p = &pixels[(y * 128 + x) * 4];
            *p++ = color;
            *p++ = color;
            *p++ = color;
        }
    }
    refresh();
    for(auto& b : buf) b = 0;
}

void paint_right()
{
    for(int i = 0; i < 512; ++i)
    {
        int x = i % 64;
        int y = (i / 64) * 8;
        uint8_t b = buf[i];
        for(int j = 0; j < 8; ++j, b >>= 1)
        {
            uint8_t color = (b & 1) ? 0xff : 0x00;
            uint8_t* p = &pixels[(y * 128 + x + 64) * 4];
            *p++ = color;
            *p++ = color;
            *p++ = color;
        }
    }
    refresh();
    for(auto& b : buf) b = 0;
}

static constexpr int RESIZE_SNAP_PIXELS = 32;
static void calculate_resize_snap(void)
{
    if(ww > RESIZE_SNAP_PIXELS && (ww + RESIZE_SNAP_PIXELS / 2) % FBW < RESIZE_SNAP_PIXELS)
    {
        ww += RESIZE_SNAP_PIXELS / 2;
        ww -= (ww % FBW);
    }
    if(wh > RESIZE_SNAP_PIXELS && (wh + RESIZE_SNAP_PIXELS / 2) % FBH < RESIZE_SNAP_PIXELS)
    {
        wh += RESIZE_SNAP_PIXELS / 2;
        wh -= (wh % FBH);
    }
}

static int imx, imy, imw, imh;
static int imdirty;

static void update_im(void)
{
    if(ww < FBW || wh < FBH)
    {
        int f = ww * FBH - wh * FBW;
        if(f > 0)
        {
            /* window is too wide: bars on the left and right */
            imw = wh * FBW / FBH;
            imh = wh;
        }
        else
        {
            /* window is too narrow: bars on the top and bottom */
            imh = ww * FBH / FBW;
            imw = ww;
        }
    }
    else
    {
        int n = tmin(ww / FBW, wh / FBH);
        imw = FBW * n;
        imh = FBH * n;
    }
    imx = (ww - imw) / 2;
    imy = (wh - imh) / 2;
    imdirty = 1;
}

static LRESULT CALLBACK window_proc(HWND w, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_SIZING:
    {
        RECT* r = (RECT*)lParam;
        RECT cr = { 0, 0, 0, 0 };
        int dw, dh;
        AdjustWindowRectEx(&cr, dwStyle, FALSE, dwExStyle);
        dw = cr.right - cr.left;
        dh = cr.bottom - cr.top;
        ww = r->right - r->left - dw;
        wh = r->bottom - r->top - dh;
        calculate_resize_snap();
        dw += ww;
        dh += wh;
        switch(wParam)
        {
        case WMSZ_BOTTOMLEFT:
            r->left = r->right - dw;
        case WMSZ_BOTTOM:
            r->bottom = r->top + dh;
            break;
        case WMSZ_TOPLEFT:
            r->top = r->bottom - dh;
        case WMSZ_LEFT:
            r->left = r->right - dw;
            break;
        case WMSZ_TOPRIGHT:
            r->right = r->left + dw;
        case WMSZ_TOP:
            r->top = r->bottom - dh;
            break;
        case WMSZ_BOTTOMRIGHT:
            r->bottom = r->top + dh;
        case WMSZ_RIGHT:
            r->right = r->left + dw;
            break;
        }
        break;
    }
    case WM_SIZE:
        ww = (int)LOWORD(lParam);
        wh = (int)HIWORD(lParam);
        update_im();
        refresh();
        break;
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
    case WM_PAINT:
        BeginPaint(w, &paint);
        if(ww == FBW && wh == FBH)
            BitBlt(hdc, 0, 0, FBW, FBH, hdc_mem, 0, 0, SRCCOPY);
        else
        {
            if(imdirty)
            {
                RECT r;
                r.top = 0;
                r.left = 0;
                r.bottom = wh;
                r.right = ww;
                FillRect(hdc, &r, brush_black);
                imdirty = 0;
            }
            StretchBlt(hdc, imx, imy, imw, imh, hdc_mem, 0, 0, FBW, FBH, SRCCOPY);
        }
        EndPaint(w, &paint);
    default:
        return DefWindowProcA(w, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
{
#if 0
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "ardurogue",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        ZOOM * 128,
        ZOOM * 64,
        0
    );
#endif

    WNDCLASS wc{};
    static char const* const CLASS_NAME = "ardurogue";
    wc.lpfnWndProc = window_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    ww = ZOOM * FBW;
    wh = ZOOM * FBH;
    update_im();

    RECT wr;
    wr.left = wr.top = 0;
    wr.right = ww;
    wr.bottom = wh;
    AdjustWindowRectEx(&wr, dwStyle, FALSE, dwExStyle);
    hwnd = CreateWindowEx(
        dwExStyle,
        CLASS_NAME,
        CLASS_NAME,
        dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        (int)(wr.right - wr.left),
        (int)(wr.bottom - wr.top),
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
        return 0;

    hdc = GetDC(hwnd);
    hdc_mem = CreateCompatibleDC(hdc);

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = FBW;
    bmi.bmiHeader.biHeight = -FBH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HBITMAP hbitmap = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
    if(hbitmap == 0)
        return 0;
    SelectObject(hdc_mem, hbitmap);

    brush_black = CreateSolidBrush(RGB(0, 0, 0));
    SetStretchBltMode(hdc, COLORONCOLOR);

    ShowWindow(hwnd, SW_NORMAL);

    game_setup();
    for(;;)
        game_loop();

    return 0;
}

#endif

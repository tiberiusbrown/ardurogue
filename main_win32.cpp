#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <Shlobj.h>
#include <strsafe.h>
#include <stdlib.h>

#include "game.hpp"

#ifndef NDEBUG
#include <stdio.h>
#include <time.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_BOX
#include "stb_image_write.h"
#include "stb_image_resize.h"
#endif

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

static constexpr int SZOOM = 2; // screenshot zoom

static char persistent_path[MAX_PATH] = {};
static uint8_t persistent_data[1024];
static bool path_defined = false;

uint8_t read_persistent(uint16_t addr)
{
    return persistent_data[addr % 1024];
}

void update_persistent(uint16_t addr, uint8_t data)
{
    persistent_data[addr % 1024] = data;
}

void flush_persistent()
{
    if(!path_defined) return;
    HANDLE f = CreateFileA(
        persistent_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if(f != INVALID_HANDLE_VALUE)
    {
        (void)WriteFile(f, persistent_data, 1024, NULL, NULL);
        CloseHandle(f);
    }
}

void seed()
{
    rand_seed = (uint16_t)__rdtsc();
    if(rand_seed == 0) rand_seed = 0xbabe;
}

static void screenshot()
{
#ifndef NDEBUG
    char fname[256];
    time_t rawtime;
    struct tm* ti;
    time(&rawtime);
    ti = localtime(&rawtime);
    snprintf(fname, sizeof(fname), "screenshot_%04d%02d%02d%02d%02d%02d.png",
        ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday,
        ti->tm_hour + 1, ti->tm_min, ti->tm_sec);
    uint8_t* resized = (uint8_t*)malloc(FBW * FBH * SZOOM * SZOOM * 4);
    stbir_resize_uint8(
        pixels, FBW, FBH, 0,
        resized, FBW * SZOOM, FBH * SZOOM, 0,
        4);
    stbi_write_png(fname, FBW * SZOOM, FBH * SZOOM, 4, resized, 0);
    free(resized);
#endif
}

void wait()
{
    constexpr UINT_PTR timer_id = 0x1001;
    MSG msg{};
    SetTimer(hwnd, timer_id, 100, (TIMERPROC)NULL);
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if(msg.hwnd == hwnd && msg.message == WM_TIMER && msg.wParam == timer_id)
            return;
    }
}

uint8_t wait_btn()
{
    MSG msg{};
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if(msg.hwnd == hwnd && msg.message == WM_KEYDOWN)
        {
            // note: is repeat if
            //       (HIWORD(msg.lParam) & KF_REPEAT) != 0
            switch(msg.wParam)
            {
            case VK_UP     : return BTN_UP;
            case VK_DOWN   : return BTN_DOWN;
            case VK_LEFT   : return BTN_LEFT;
            case VK_RIGHT  : return BTN_RIGHT;
            case 'A'       : return BTN_A;
            case 'B'       : return BTN_B;
            case VK_F2     : screenshot(); break;
            case VK_ESCAPE : ExitProcess(0);
            default        : break;
            }
        }
    }
    ExitProcess(0);
}

static void refresh()
{
    InvalidateRect(hwnd, NULL, FALSE);
}

void paint_offset(uint8_t offset, bool clear)
{
    for(int i = 0; i < 512; ++i)
    {
        int x = i % 64;
        int y = (i / 64) * 8;
        uint8_t b = buf[i];
        for(int j = 0; j < 8; ++j, b >>= 1)
        {
            uint8_t color = (b & 1) ? 0xff : 0x00;
            uint8_t* p = &pixels[((y + j) * 128 + x + offset) * 4];
            *p++ = color;
            *p++ = color;
            *p++ = color;
            *p++ = 0xff;
        }
    }
    refresh();
    if(clear)
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
#if 1
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
            /* fallthrough */
        case WMSZ_BOTTOM:
            r->bottom = r->top + dh;
            break;
        case WMSZ_TOPLEFT:
            r->top = r->bottom - dh;
            /* fallthrough */
        case WMSZ_LEFT:
            r->left = r->right - dw;
            break;
        case WMSZ_TOPRIGHT:
            r->right = r->left + dw;
            /* fallthrough */
        case WMSZ_TOP:
            r->top = r->bottom - dh;
            break;
        case WMSZ_BOTTOMRIGHT:
            r->bottom = r->top + dh;
            /* fallthrough */
        case WMSZ_RIGHT:
            r->right = r->left + dw;
            break;
        }
        break;
    }
#endif
    case WM_SIZE:
        ww = (int)LOWORD(lParam);
        wh = (int)HIWORD(lParam);
        update_im();
        refresh();
        break;
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
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

#ifdef NDEBUG
void main(void)
#else
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
#endif
{
#ifndef NDEBUG
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;
#else
    HINSTANCE hInstance = GetModuleHandleA(NULL);
#endif{

    WNDCLASS wc{};
    BITMAPINFO bmi{};
    RECT wr;
    HBITMAP hbitmap;
    static char const* const CLASS_NAME = "ArduRogue";

    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, persistent_path)))
    {
        StringCchCatA(persistent_path, sizeof(persistent_path), "\\ardurogue_save");
        HANDLE f = CreateFile(
            persistent_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL);
        if(f != INVALID_HANDLE_VALUE)
        {
            (void)ReadFile(f, persistent_data, 1024, NULL, NULL);
            CloseHandle(f);
        }
        path_defined = true;
    }

    wc.lpfnWndProc = window_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    RegisterClass(&wc);

    ww = ZOOM * FBW;
    wh = ZOOM * FBH;
    update_im();

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
        goto byebye;

    hdc = GetDC(hwnd);
    hdc_mem = CreateCompatibleDC(hdc);

    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = FBW;
    bmi.bmiHeader.biHeight = -FBH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hbitmap = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
    if(hbitmap == 0)
        goto byebye;
    SelectObject(hdc_mem, hbitmap);

    brush_black = CreateSolidBrush(RGB(0, 0, 0));
    SetStretchBltMode(hdc, COLORONCOLOR);

    ShowWindow(hwnd, SW_NORMAL);

    run();

byebye:
    (void)0;
#ifndef NDEBUG
    return 0;
#endif
}

#ifdef NDEBUG
#pragma function(memset)
void* __cdecl memset(void* dst, int c, size_t count)
{
    char* bytes = (char*)dst;
    while(count--) *bytes++ = (char)c;
    return dst;
}
#pragma function(memcpy)
void* __cdecl memcpy(void* dst, void const* src, size_t count)
{
    char* bytes = (char*)dst;
    char const* src_bytes = (char*)src;
    while(count--) *bytes++ = *src_bytes++;
    return dst;
}
#endif

#endif

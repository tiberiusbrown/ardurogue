#if defined(__EMSCRIPTEN__)

#include <emscripten.h>
#include <emscripten/html5.h>
#include "game.hpp"

#include <stdio.h>

struct rgba_t { uint8_t r, g, b, a; };
static rgba_t buffer[128 * 64];

static uint8_t last_pressed = 0;
static uint8_t persistent_data[1024];

static EM_BOOL keydown_callback(int type, EmscriptenKeyboardEvent const* e, void* user)
{
    if(type == EMSCRIPTEN_EVENT_KEYDOWN)
    {
        switch(e->keyCode)
        {
            case 38: last_pressed = BTN_UP; break;
            case 40: last_pressed = BTN_DOWN; break;
            case 37: last_pressed = BTN_LEFT; break;
            case 39: last_pressed = BTN_RIGHT; break;
            case 65: last_pressed = BTN_A; break;
            case 66: last_pressed = BTN_B; break;
            default: break;
        }
    }
    return EM_TRUE;
}

void wait()
{
    emscripten_sleep(100);
}

uint8_t wait_btn()
{
    for(;;)
    {
        if(last_pressed != 0)
        {
            uint8_t r = last_pressed;
            last_pressed = 0;
            return r;
        }
        emscripten_sleep(1);
    }
}

void seed()
{
    rand_seed = uint16_t(emscripten_random() * 65534.f + 1.5f);
}

static void update_canvas()
{
    EM_ASM_({
        let data = HEAPU8.subarray($0, $0 + $1 * $2 * 4);
        let canvas = Module['canvas'];
        canvas.width = innerWidth;
        canvas.height = innerHeight;
        let context = canvas.getContext('2d');
        context.imageSmoothingEnabled = false;
        
        let temp_canvas = document.createElement('canvas');
        let temp_context = temp_canvas.getContext('2d');
        temp_canvas.width = 128;
        temp_canvas.height = 64;
        let image_data = temp_context.createImageData($1, $2);
        image_data.data.set(data);
        temp_context.putImageData(image_data, 0, 0);
        
        let scale = Math.floor(Math.min(
            canvas.width / 128,
            canvas.height / 64));
        let w = 128 * scale;
        let h = 64 * scale;
        
        context.fillStyle = '#000';
        context.fillRect(0, 0, canvas.width, canvas.height);
        context.drawImage(
            temp_canvas,
            Math.floor((canvas.width - w) / 2),
            Math.floor((canvas.height - h) / 2),
            w, h);
    }, (uint32_t*)buffer, 128, 64);
}

static EM_BOOL resize_callback(int type, EmscriptenUiEvent const* e, void* user)
{
    if(type == EMSCRIPTEN_EVENT_RESIZE)
        update_canvas();
    return EM_TRUE;
}

void paint_offset(uint8_t offx, bool clear)
{    
    for(int y = 0, i = 0; y < 64; y+= 8)
    {
        for(int x = 0; x < 64; ++x, ++i)
        {
            uint8_t m = buf[i];
            for(int j = 0; j < 8; ++j, m >>= 1)
            {
                rgba_t* p = &buffer[(y + j) * 128 + x + offx];
                if(m & 1) *p = { 255, 255, 255, 255 };
                else      *p = {   0,   0,   0, 255 };
            }
        }
    }
    
    if(clear)
        for(auto& b : buf) b = 0;
    
    update_canvas();
}

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
    FILE* f = fopen("/offline/ardurogue_save", "wb");
    if(f)
    {
        fwrite(persistent_data, 1, sizeof(persistent_data), f);
        fclose(f);
        EM_ASM(
            FS.syncfs(function (err) {assert(!err);});
        );
    }
    else printf("ERROR 2\n");
}

static bool pers_done = false;

extern "C" void EMSCRIPTEN_KEEPALIVE initpers()
{
    FILE* f = fopen("/offline/ardurogue_save", "rb");
    if(f)
    {
        fread(persistent_data, 1, sizeof(persistent_data), f);
        fclose(f);
    }
    pers_done = true;
}

int main()
{
    emscripten_set_resize_callback(
        EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, resize_callback);
    emscripten_set_keydown_callback(
        EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, keydown_callback);
        
    EM_ASM(
        FS.mkdir('/offline');
        FS.mount(IDBFS, {}, '/offline');
        FS.syncfs(true, function (err) { ccall('initpers', 'v'); });
    );
    
    while(!pers_done)
        emscripten_sleep(1);
    
    run();

    return 0;
}

#endif

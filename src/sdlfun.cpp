

// SDL ��غ���

#include "jymain.h"
#include "PotDll.h"
#include "charset.h"
#include "sdlfun.h"
#include "mainmap.h"
#include "piccache.h"
#include <map>

HSTREAM currentMusic = 0;            //�����������ݣ�����ͬʱֻ����һ������һ������
#define WAVNUM 5
HSAMPLE WavChunk[WAVNUM];            //������Ч���ݣ�����ͬʱ���ż��������������
BASS_MIDI_FONT midfonts;
int currentWav = 0;                  //��ǰ���ŵ���Ч

#define RECTNUM  20
SDL_Rect ClipRect[RECTNUM];          // ��ǰ���õļ��þ���
int currentRect = 0;

//#define SURFACE_NUM  20
//SDL_Texture* tmp_Surface[SURFACE_NUM];   //JY_SaveSurʹ��

//����ESC��RETURN��SPACE����ʹ���ǰ��º����ظ���
int KeyFilter(void* data, SDL_Event* event)
{
    static int Esc_KeyPress = 0;
    static int Space_KeyPress = 0;
    static int Return_KeyPress = 0;
    int r = 1;
    switch (event->type)
    {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym)
        {
        case SDLK_ESCAPE:
            if (1 == Esc_KeyPress)
            {
                r = 0;
            }
            else
            {
                Esc_KeyPress = 1;
            }
            break;
        case SDLK_RETURN:
            if (1 == Return_KeyPress)
            {
                r = 0;
            }
            else
            {
                Return_KeyPress = 1;
            }
            break;
        case SDLK_SPACE:
            if (1 == Space_KeyPress)
            {
                r = 0;
            }
            else
            {
                Space_KeyPress = 1;
            }
            break;
        default:
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event->key.keysym.sym)
        {
        case SDLK_ESCAPE:
            Esc_KeyPress = 0;
            break;
        case SDLK_SPACE:
            Space_KeyPress = 0;
            break;
        case SDLK_RETURN:
            Return_KeyPress = 0;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return r;
}

// ��ʼ��SDL
int InitSDL(void)
{
    int r;
    int i;
    //char tmpstr[255];
    int so = 22050;
    r = SDL_Init(SDL_INIT_VIDEO);
    if (r < 0)
    {
        JY_Error(
            "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    //atexit(SDL_Quit);    ���������⣬���ε�
    //SDL_VideoDriverName(tmpstr, 255);
    //JY_Debug("InitSDL: Video Driver: %s\n", tmpstr);
    InitFont();  //��ʼ��
    r = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (r < 0)
    {
        g_EnableSound = 0;
        JY_Error("Init audio error!");
    }
    if (g_MP3 == 1)
    {
        so = 44100;
    }
    if (!BASS_Init(-1, so, 0, 0, NULL))
    {
        JY_Error("Can't initialize device");
        g_EnableSound = 0;
    }
    currentWav = 0;
    for (i = 0; i < WAVNUM; i++)
    {
        WavChunk[i] = 0;
    }
    SDL_SetEventFilter(KeyFilter, NULL);
    if (g_MP3 != 1)
    {
        midfonts.font = BASS_MIDI_FontInit(g_MidSF2, 0);
        if (!midfonts.font)
        {
            JY_Error("BASS_MIDI_FontInit error ! %d", BASS_ErrorGetCode());
        }
        midfonts.preset = -1; // use all presets
        midfonts.bank = 0; // use default bank(s)
        BASS_MIDI_StreamSetFonts(0, &midfonts, 1); // set default soundfont
    }
    return 0;
}

// �˳�SDL
int ExitSDL(void)
{
    ExitFont();
    StopMIDI();
    if (midfonts.font)
    {
        BASS_MIDI_FontFree(midfonts.font);
    }
    for (int i = 0; i < WAVNUM; i++)
    {
        if (WavChunk[i])
        {
            //Mix_FreeChunk(WavChunk[i]);
            BASS_SampleFree(WavChunk[i]);
            WavChunk[i] = 0;
        }
    }
    //Mix_CloseAudio();
    BASS_Free();
#ifdef WIN32
    if (g_Tinypot) { PotDestory(g_Tinypot); }
#endif
    JY_LoadPicture("", 0, 0);    // �ͷſ��ܼ��ص�ͼƬ����
    SDL_Quit();
    return 0;
}

// ת��ARGB����ǰ��Ļ��ɫ
Uint32 ConvertColor(Uint32 color)
{
    Uint8* p = (Uint8*)&color;
    return SDL_MapRGBA(g_Surface->format, *(p + 2), *(p + 1), *p, 255);
}


// ��ʼ����Ϸ����
int InitGame(void)
{
    int w = g_ScreenW;
    int h = g_ScreenH;
    if (g_Rotate)
    {
        swap(w, h);
    }
    //putenv ("SDL_VIDEO_WINDOW_POS");
    //putenv ("SDL_VIDEO_CENTERED=1");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, g_Softener);
    g_Window = SDL_CreateWindow((const char*)u8"����Ⱥ����", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_RESIZABLE);
    SDL_SetWindowIcon(g_Window, IMG_Load("ff.ico"));
    g_Renderer = SDL_CreateRenderer(g_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    g_Texture = CreateRenderedTexture(g_ScreenW, g_ScreenH);
    g_TextureShow = CreateRenderedTexture(g_ScreenW, g_ScreenH);
    g_TextureTmp = CreateRenderedTexture(g_ScreenW, g_ScreenH);

    g_Surface = SDL_CreateRGBSurface(0, 1, 1, 32, RMASK, GMASK, BMASK, AMASK);
    //SDL_WM_SetCaption("The Fall of Star",_("ff.ico"));         //������ʾ���ڵ�
    //SDL_WM_SetIcon(IMG_Load(_("ff.ico")), NULL);
    if (g_FullScreen == 1)
    {
        SDL_SetWindowFullscreen(g_Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else
    {
        SDL_SetWindowFullscreen(g_Window, 0);
    }
    if (g_Window == NULL || g_Renderer == NULL || g_Texture == NULL || g_TextureShow == NULL)
    {
        JY_Error("Cannot set video mode");
    }
    Init_Cache();
    JY_PicInit("");        // ��ʼ����ͼcache
    g_Particle.setRenderer(g_Renderer);
    g_Particle.setPosition(w / 2, 0);
    g_Particle.getDefaultTexture();
    return 0;
}

// �ͷ���Ϸ��Դ
int ExitGame(void)
{
    SDL_DestroyTexture(g_Texture);
    SDL_DestroyTexture(g_TextureShow);
    SDL_DestroyRenderer(g_Renderer);
    SDL_DestroyWindow(g_Window);
    JY_PicInit("");
    JY_LoadPicture("", 0, 0);
    JY_UnloadMMap();     //�ͷ�����ͼ�ڴ�
    JY_UnloadSMap();     //�ͷų�����ͼ�ڴ�
    JY_UnloadWarMap();   //�ͷ�ս����ͼ�ڴ�
    return 0;
}

int RenderToTexture(SDL_Texture* src, SDL_Rect* src_rect, SDL_Texture* dst, SDL_Rect* dst_rect, double angle, SDL_Point*center, SDL_RendererFlip filp)
{
	SDL_SetRenderTarget(g_Renderer, dst);
	return SDL_RenderCopyEx(g_Renderer, src, src_rect, dst_rect, angle, center, filp);
}

SDL_Texture* CreateRenderedTexture(SDL_Texture* ref)
{
    int w, h;
    SDL_QueryTexture(ref, NULL, NULL, &w, &h);
    return CreateRenderedTexture(w, h);
}

SDL_Texture* CreateRenderedTexture(int w, int h)
{
    return SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
}

//����ͼ���ļ���������ʽҲ���Լ���
//x,y =-1 ����ص���Ļ����
//    ���δ���أ�����أ�Ȼ��blit��������أ�ֱ��blit
//  str �ļ��������Ϊ�գ����ͷű���
int JY_LoadPicture(const char* str, int x, int y)
{
    static char filename[255] = "\0";
    static SDL_Texture* tex = NULL;
    static SDL_Rect r;
    SDL_Surface* tmppic;
    SDL_Surface* pic = NULL;
    if (strlen(str) == 0)          // Ϊ�����ͷű���
    {
        if (pic)
        {
            SDL_FreeSurface(pic);
            pic = NULL;
        }
        return 0;
    }
    if (strcmp(str, filename) != 0)   // ����ǰ�ļ�����ͬ�����ͷ�ԭ�����棬�����±���
    {
        if (tex)
        {
            SDL_DestroyTexture(tex);
            tex = NULL;
        }
        tmppic = IMG_Load(str);
        if (tmppic)
        {
            pic = SDL_ConvertSurfaceFormat(tmppic, g_Surface->format->format, 0);   // ��Ϊ��ǰ��������ظ�ʽ
            if ((x == -1) && (y == -1))
            {
                x = (g_ScreenW - pic->w) / 2;
                y = (g_ScreenH - pic->h) / 2;
            }
            r.x = x;
            r.y = y;
            r.w = pic->w;
            r.h = pic->h;
            tex = SDL_CreateTextureFromSurface(g_Renderer, pic);
            SDL_FreeSurface(pic);
            SDL_FreeSurface(tmppic);
            strcpy(filename, str);
        }
    }
    if (tex)
    {
        RenderToTexture(tex, NULL, g_Texture, &r, NULL, NULL, SDL_FLIP_NONE);
    }
    else
    {
        JY_Error("JY_LoadPicture: Load picture file %s failed! %s", str, SDL_GetError());
    }
    return 0;
}



//��ʾ����
//flag = 0 ��ʾȫ������  =1 ����JY_SetClip���õľ�����ʾ�����û�о��Σ�����ʾ
int JY_ShowSurface(int flag)
{
    SDL_SetRenderTarget(g_Renderer, g_TextureShow);
    if (flag == 1)
    {
        if (currentRect > 0)
        {
            for (int i = 0; i < currentRect; i++)
            {
                SDL_Rect* r = ClipRect + i;
                SDL_RenderCopy(g_Renderer, g_Texture, r, r);
            }
        }
    }
    else
    {
        SDL_RenderCopy(g_Renderer, g_Texture, NULL, NULL);
    }
    SDL_SetRenderTarget(g_Renderer, NULL);
    //SDL_Rect r;
    //SDL_RenderGetClipRect(g_Renderer, &r);
    SDL_RenderSetClipRect(g_Renderer, NULL);
    if (g_Rotate == 0)
    {
        SDL_RenderCopy(g_Renderer, g_TextureShow, NULL, NULL);
    }
    else
    {
        SDL_RenderCopyEx(g_Renderer, g_TextureShow, NULL, NULL, 90, NULL, SDL_FLIP_NONE);
    }
    SDL_RenderPresent(g_Renderer);
    //SDL_RenderSetClipRect(g_Renderer, &r);
    return 0;
}

//��ʱx����
int JY_Delay(int x)
{
    SDL_Delay(x);
    g_DelayTimes++;
    return 0;
}


// ������ʾͼ��
// delaytime ÿ�ν�����ʱ������
// Flag=0 �Ӱ�������1����������
int JY_ShowSlow(int delaytime, int Flag)
{
    int i;
    int step;
    int t1, t2;
    int alpha;
    SDL_Texture* lps1;  // ������ʱ����
    lps1 = SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, g_ScreenW, g_ScreenH);
    if (lps1 == NULL)
    {
        JY_Error("JY_ShowSlow: Create surface failed!");
        return 1;
    }
    SDL_SetRenderTarget(g_Renderer, lps1);
    SDL_RenderCopy(g_Renderer, g_Texture, NULL, NULL);
    //SDL_BlitSurface(g_Surface, NULL, lps1, NULL);    //��ǰ���渴�Ƶ���ʱ����
    for (i = 0; i <= 32; i++)
    {
        if (Flag == 0)
        {
            step = 32 - i;
        }
        else
        {
            step = i;
        }
        t1 = (int)JY_GetTime();
        //SDL_SetRenderDrawColor(g_Renderer, 0, 0, 0, 0);
        //SDL_RenderFillRect(g_Renderer, NULL);          //��ǰ������
        alpha = step << 3;
        if (alpha > 255)
        {
            alpha = 255;
        }
        //SDL_SetTextureAlphaMod(lps1, (Uint8)alpha);  //����alpha
        //SDL_RenderCopy(g_Renderer, lps1, NULL, NULL);
        //SDL_BlitSurface(lps1, NULL, g_Surface, NULL);
        SDL_SetRenderTarget(g_Renderer, g_Texture);
        SDL_RenderCopy(g_Renderer, lps1, NULL, NULL);
        SDL_SetRenderDrawColor(g_Renderer, 0, 0, 0, alpha);
        SDL_SetRenderDrawBlendMode(g_Renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(g_Renderer, NULL);
        JY_ShowSurface(0);
        t2 = (int)JY_GetTime();
        if (delaytime > t2 - t1)
        {
            JY_Delay(delaytime - (t2 - t1));
        }
        //JY_GetKey();
    }
    SDL_DestroyTexture(lps1);       //�ͷű���
    return 0;
}


#ifdef HIGH_PRECISION_CLOCK

__int64 GetCycleCount()
{
    __asm _emit 0x0F
    __asm _emit 0x31
}

#endif

//�õ���ǰʱ�䣬��λ����
double JY_GetTime()
{
#ifdef HIGH_PRECISION_CLOCK
    return (double)(GetCycleCount()) / (1000 * CPU_FREQUENCY);
#else
    return (double)SDL_GetTicks();
#endif
}

//��������
int JY_PlayMIDI(const char* filename)
{
    static char currentfile[255] = "\0";
    if (g_EnableSound == 0)
    {
        JY_Error("disable sound!");
        return 1;
    }
    if (strlen(filename) == 0)    //�ļ���Ϊ�գ�ֹͣ����
    {
        StopMIDI();
        strcpy(currentfile, filename);
        return 0;
    }
    if (strcmp(currentfile, filename) == 0) //�뵱ǰ�����ļ���ͬ��ֱ�ӷ���
    {
        return 0;
    }
    StopMIDI();
    //currentMusic = BASS_MIDI_StreamCreateFile(0, filename, 0, 0, 0, 0);
    currentMusic = BASS_StreamCreateFile(0, filename, 0, 0, 0);
    if (!currentMusic)
    {
        JY_Error("Open music file %s failed! %d", filename, BASS_ErrorGetCode());
        return 1;
    }
    if (g_MP3 == 1)
    {
        BASS_MIDI_StreamSetFonts(currentMusic, &midfonts, 1);
    } // set for current stream too
    BASS_ChannelSetAttribute(currentMusic, BASS_ATTRIB_VOL, (float)(g_MusicVolume / 100.0));
    BASS_ChannelFlags(currentMusic, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
    BASS_ChannelPlay(currentMusic, FALSE);
    strcpy(currentfile, filename);
    return 0;
}

//ֹͣ��Ч
int StopMIDI()
{
    if (currentMusic)
    {
        BASS_ChannelStop(currentMusic);
        BASS_StreamFree(currentMusic);
        currentMusic = 0;
    }
    return 0;
}

//������Ч
int JY_PlayWAV(const char* filename)
{
    HCHANNEL ch;
    if (g_EnableSound == 0)
    {
        return 1;
    }
    if (WavChunk[currentWav])            //�ͷŵ�ǰ��Ч
    {
        //Mix_FreeChunk(WavChunk[currentWav]);
        BASS_SampleStop(WavChunk[currentWav]);
        BASS_SampleFree(WavChunk[currentWav]);
        WavChunk[currentWav] = 0;
    }
    //WavChunk[currentWav]= Mix_LoadWAV(filename);  //���ص���ǰ��Ч
    WavChunk[currentWav] = BASS_SampleLoad(0, filename, 0, 0, 1, 0);
    if (WavChunk[currentWav])
    {
        //Mix_VolumeChunk(WavChunk[currentWav],g_SoundVolume);
        //Mix_PlayChannel(-1, WavChunk[currentWav], 0);  //������Ч
        ch = BASS_SampleGetChannel(WavChunk[currentWav], 0);
        BASS_ChannelSetAttribute(ch, BASS_ATTRIB_VOL, (float)(g_SoundVolume / 100.0));
        BASS_ChannelFlags(ch, 0, BASS_SAMPLE_LOOP);
        BASS_ChannelPlay(ch, 0);
        currentWav++;
        if (currentWav >= WAVNUM)
        {
            currentWav = 0;
        }
    }
    else
    {
        JY_Error("Open wav file %s failed!", filename);
    }
    return 0;
}

// �õ�ǰ�水�µ��ַ�
int JY_GetKey(int* key, int* type, int* mx, int* my)
{
    SDL_Event event;
    int win_w, win_h, r;
    *key = -1;
    *type = -1;
    *mx = -1;
    *my = -1;
    while (SDL_PollEvent(&event))
        //if (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYDOWN:
            *key = event.key.keysym.sym;
            if (*key == SDLK_SPACE)
            {
                *key = SDLK_RETURN;
            }
            *type = 1;
            break;
        case SDL_KEYUP:
            //*key = event.key.keysym.sym;
            //if (*key == SDLK_SPACE)
            //{
            //    *key = SDLK_RETURN;
            //}
            break;
        case SDL_MOUSEMOTION:           //����ƶ�
            SDL_GetWindowSize(g_Window, &win_w, &win_h);
            *mx = event.motion.x * g_ScreenW / win_w;
            *my = event.motion.y * g_ScreenH / win_h;
            if (g_Rotate) { swap(*mx, *my); }
            *type = 2;
            break;
        case SDL_MOUSEBUTTONDOWN:       //�����
            SDL_GetWindowSize(g_Window, &win_w, &win_h);
            *mx = event.motion.x * g_ScreenW / win_w;
            *my = event.motion.y * g_ScreenH / win_h;
            if (g_Rotate) { swap(*mx, *my); }
            if (event.button.button == SDL_BUTTON_LEFT)         //���
            {
                *type = 3;
            }
            else if (event.button.button == SDL_BUTTON_RIGHT)   //�Ҽ�
            {
                *type = 4;
            }
            else if (event.button.button == SDL_BUTTON_MIDDLE)      //�м�
            {
                *type = 5;
            }
            break;
        case SDL_MOUSEWHEEL:
            //�޾Ʋ��������������
            if (event.wheel.y == 1)
            {
                *type = 6;
            }
            else if (event.wheel.y == -1)
            {
                *type = 7;
            }
            break;
        case SDL_QUIT:
        {
            static int quit = 0;
            if (quit == 0)
            {
                quit = 1;
                lua_getglobal(pL_main, "Menu_Exit");
                lua_call(pL_main, 0, 1);
                r = (int)lua_tointeger(pL_main, -1);
                lua_pop(pL_main, 1);
                //if (MessageBox(NULL, "��ȷ��Ҫ�ر���Ϸ��?", "ϵͳ��ʾ", MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
                if (r == 1)
                {
                    ExitGame();       //�ͷ���Ϸ����
                    ExitSDL();        //�˳�SDL
                    exit(1);
                }
                quit = 0;
            }
        }
        break;
        default:
            break;
        }
    }

    return *key;
}

int JY_GetKeyState(int key)
{
    return SDL_GetKeyboardState(NULL)[SDL_GetScancodeFromKey(key)];
}

//���òü�
int JY_SetClip(int x1, int y1, int x2, int y2)
{
    SDL_Rect rect;
    SDL_SetRenderTarget(g_Renderer, g_Texture);
    if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0)
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = g_ScreenW;
        rect.h = g_ScreenH;
        SDL_RenderSetClipRect(g_Renderer, &rect);
        //SDL_SetClipRect(g_Surface, NULL);
        currentRect = 0;
    }
    else
    {
        SDL_Rect rect;
        rect.x = (Sint16)x1;
        rect.y = (Sint16)y1;
        rect.w = (Uint16)(x2 - x1);
        rect.h = (Uint16)(y2 - y1);
        ClipRect[currentRect] = rect;
        SDL_RenderSetClipRect(g_Renderer, &rect);
        currentRect = currentRect + 1;
        if (currentRect >= RECTNUM)
        {
            currentRect = 0;
        }
    }
    return 0;
}


// ���ƾ��ο�
// (x1,y1)--(x2,y2) ������ϽǺ����½�����
// color ��ɫ
int JY_DrawRect(int x1, int y1, int x2, int y2, int color)
{
    Uint32 c;
    c = ConvertColor(color);
    HLine32(x1, x2, y1, c);
    HLine32(x1, x2, y2, c);
    VLine32(y1, y2, x1, c);
    VLine32(y1, y2, x2, c);
    return 0;
}


//��ˮƽ��
void HLine32(int x1, int x2, int y, int color)
{
    Uint8 r = (Uint8)((color & RMASK) >> 16);
    Uint8 g = (Uint8)((color & GMASK) >> 8);
    Uint8 b = (Uint8)((color & BMASK));
    Uint8 a = 255;
    SDL_SetRenderTarget(g_Renderer, g_Texture);
    SDL_SetRenderDrawColor(g_Renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(g_Renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderDrawLine(g_Renderer, x1, y, x2, y);
}

//�洹ֱ��
void VLine32(int x1, int x2, int y, int color)
{
    Uint8 r = (Uint8)((color & RMASK) >> 16);
    Uint8 g = (Uint8)((color & GMASK) >> 8);
    Uint8 b = (Uint8)((color & BMASK));
    Uint8 a = 255;
    SDL_SetRenderTarget(g_Renderer, g_Texture);
    SDL_SetRenderDrawColor(g_Renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(g_Renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderDrawLine(g_Renderer, y, x1, y, x2);
}



// ͼ�����
// ���x1,y1,x2,y2��Ϊ0���������������
// color, ���ɫ����RGB��ʾ���Ӹߵ����ֽ�Ϊ0RGB
int JY_FillColor(int x1, int y1, int x2, int y2, int color)
{
    Uint8 r = (Uint8)((color & RMASK) >> 16);
    Uint8 g = (Uint8)((color & GMASK) >> 8);
    Uint8 b = (Uint8)((color & BMASK));
    Uint8 a = (Uint8)((color & AMASK) >> 24);
    //int c = ConvertColor(color);
    SDL_SetRenderTarget(g_Renderer, g_Texture);
    SDL_SetRenderDrawColor(g_Renderer, r, g, b, 255);
    SDL_SetRenderDrawBlendMode(g_Renderer, SDL_BLENDMODE_BLEND);
    if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0)
    {
        SDL_RenderFillRect(g_Renderer, NULL);
        //SDL_FillRect(g_Surface, NULL, c);
    }
    else
    {
        SDL_Rect rect;
        rect.x = (Sint16)x1;
        rect.y = (Sint16)y1;
        rect.w = (Uint16)(x2 - x1);
        rect.h = (Uint16)(y2 - y1);
        SDL_RenderFillRect(g_Renderer, &rect);
        //SDL_FillRect(g_Surface, &rect, c);
    }
    return 0;
}

// �����䰵
// ��Դ����(x1,y1,x2,y2)�����ڵ����е����Ƚ���
// bright ���ȵȼ� 0-256
int JY_Background(int x1, int y1, int x2, int y2, int Bright, int color)
{
    SDL_Rect r1;
    if (x2 <= x1 || y2 <= y1)
    {
        return 0;
    }
    Bright = 256 - Bright;
    if (Bright > 255)
    {
        Bright = 255;
    }
    r1.x = (Sint16)x1;
    r1.y = (Sint16)y1;
    r1.w = (Uint16)(x2 - x1);
    r1.h = (Uint16)(y2 - y1);
    Uint8 r = (Uint8)((color & RMASK) >> 16);
    Uint8 g = (Uint8)((color & GMASK) >> 8);
    Uint8 b = (Uint8)((color & BMASK));
    Uint8 a = 255;
    SDL_SetRenderTarget(g_Renderer, g_Texture);
    SDL_SetRenderDrawColor(g_Renderer, r, g, b, Bright);
    SDL_SetRenderDrawBlendMode(g_Renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(g_Renderer, &r1);
    return 1;
}

//����mpeg
// esckey ֹͣ���ŵİ���
int JY_PlayMPEG(char* filename, int esckey)
{
#ifdef WIN32
    if (g_Tinypot == NULL)
    {
        g_Tinypot = PotCreateFromWindow(g_Window);
    }
    StopMIDI();
    int r = PotInputVideo(g_Tinypot, filename);
    if (r == 1)
    {
        SDL_Event e;
        e.type = SDL_QUIT;
        SDL_PushEvent(&e);
    }
#endif
    //g_Tinypot = NULL;
    return 0;
}

//ȡs��ֵ
int JY_SetSound(int id, int flag)
{
	if (flag == 1)
	{
		g_SoundVolume = id;                  // �������� 0 �ر� 1 ��
	}
	else if (flag == 2)
	{
		g_MusicVolume = id;
		BASS_ChannelSetAttribute(currentMusic, BASS_ATTRIB_VOL, (float)(g_MusicVolume / 100.0));
	}
	return 0;
}

// ȫ���л�
int JY_FullScreen()
{
    //SDL_Surface* tmpsurface;
    ////const SDL_VideoInfo *info;
    //Uint32 flag = g_Surface->flags;
    //tmpsurface = SDL_CreateRGBSurface(SDL_SWSURFACE, g_Surface->w, g_Surface->h, g_Surface->format->BitsPerPixel,
    //    g_Surface->format->Rmask, g_Surface->format->Gmask, g_Surface->format->Bmask, g_Surface->format->Amask);
    //SDL_BlitSurface(g_Surface, NULL, tmpsurface, NULL);
    g_FullScreen = 1 - g_FullScreen;
    if (g_FullScreen == 1)
    {
        SDL_SetWindowFullscreen(g_Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else
    {
        SDL_SetWindowFullscreen(g_Window, 0);
    }
    //if (flag & SDL_FULLSCREEN)    //ȫ�������ô���
    //{ g_Surface = SDL_SetVideoMode(g_Surface->w, g_Surface->h, 0, SDL_SWSURFACE); }
    //else
    //{ g_Surface = SDL_SetVideoMode(g_Surface->w, g_Surface->h, g_ScreenBpp, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN); }
    //SDL_BlitSurface(tmpsurface, NULL, g_Surface, NULL);
    JY_ShowSurface(0);
    //SDL_FreeSurface(tmpsurface);
    //info = SDL_GetVideoInfo();
    //JY_Debug("hw_available=%d,wm_available=%d", info->hw_available, info->wm_available);
    //JY_Debug("blit_hw=%d,blit_hw_CC=%d,blit_hw_A=%d", info->blit_hw, info->blit_hw_CC, info->blit_hw_A);
    //JY_Debug("blit_sw=%d,blit_sw_CC=%d,blit_sw_A=%d", info->blit_hw, info->blit_hw_CC, info->blit_hw_A);
    //JY_Debug("blit_fill=%d,videomem=%d", info->blit_fill, info->video_mem);
    //JY_Debug("Color depth=%d", info->vfmt->BitsPerPixel);
    return 0;
}



#define SURFACE_NUM  20
SDL_Texture* tmp_Surface[SURFACE_NUM];   //JY_SaveSurʹ��
//������Ļ����ʱ����
//������Ļ����ʱ����
int JY_SaveSur(int x, int y, int w, int h)
{
	int id = -1;
	int i;
	SDL_Rect r1;
	for (i = 0; i < SURFACE_NUM; i++)
	{
		if (tmp_Surface[i] == NULL)
		{
			id = i;
			break;
		}
	}
	if (id < 0) { return -1; }
	if (w + x > g_ScreenW) { w = g_ScreenW - x; }
	if (h + y > g_ScreenH) { h = g_ScreenH - h; }
	if (w <= 0 || h <= 0) { return -1; }
	r1.x = x;
	r1.y = y;
	r1.w = w;
	r1.h = h;
	if (tmp_Surface[id] != NULL)
	{
		SDL_DestroyTexture(tmp_Surface[id]);
	}
	tmp_Surface[id] = SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
	SDL_SetRenderTarget(g_Renderer, tmp_Surface[id]);
	SDL_RenderCopy(g_Renderer, g_Texture, &r1, NULL);
	//tmp_Surface[id] = SDL_CreateRGBSurface(SDL_SWSURFACE, r1.w, r1.h, g_Surface->format->BitsPerPixel
	//    , g_Surface->format->Rmask, g_Surface->format->Gmask, g_Surface->format->Bmask, g_Surface->format->Amask);
	//SDL_BlitSurface(g_Surface, &r1, tmp_Surface[id], NULL);
	return id;
}

//������ʱ���浽��Ļ
int JY_LoadSur(int id, int x, int y)
{
    SDL_Rect r1;
    if (id < 0 || id > SURFACE_NUM - 1 || tmp_Surface[id] == NULL)
    {
        return 1;
    }
    r1.x = (Sint16)x;
    r1.y = (Sint16)y;
    if (tmp_Surface[id] == NULL)
    {
        return 1;
    }
    SDL_QueryTexture(tmp_Surface[id], NULL, NULL, &r1.w, &r1.h);
    SDL_SetRenderTarget(g_Renderer, g_Texture);
    SDL_RenderCopy(g_Renderer, tmp_Surface[id], NULL, &r1);
    //SDL_BlitSurface(tmp_Surface[id], NULL, g_Surface, &r1);
    return 0;
}

//�ͷ�
int JY_FreeSur(int id)
{
    if (id < 0 || id > SURFACE_NUM - 1 || tmp_Surface[id] == NULL)
    {
        return 1;
    }
    if (tmp_Surface[id] != NULL)
    {
        SDL_DestroyTexture(tmp_Surface[id]);
        tmp_Surface[id] = NULL;
    }
    return 0;
}

//������ת90��
SDL_Rect RotateRect(const SDL_Rect* rect)
{
    SDL_Rect r;
    r.x = (Sint16)(g_ScreenH - rect->y - rect->h);
    r.y = rect->x;
    r.w = rect->h;
    r.h = rect->w;
    return r;
}

//������ת90��
SDL_Rect RotateReverseRect(const SDL_Rect* rect)
{
    SDL_Rect r;
    r.x = rect->y;
    r.y = (Sint16)(g_ScreenH - rect->x - rect->w);
    r.w = rect->h;
    r.h = rect->w;
    return r;
}


#pragma once

#include "SDL2/SDL.h"
#include <stdio.h>
#include <vector>
#include "ZipFile.h"
//PicCache.c

#define TEXTURE_NUM 10

// ����ʹ�õ�����
struct CacheNode                         //��ͼcache����ڵ�
{
    CacheNode()
    {
        for (int i = 0; i < TEXTURE_NUM; i++)
        {
            tt[i] = NULL;
        }
    }
    ~CacheNode()
    {
        if (s) { SDL_FreeSurface(s); }
        //if (t) { SDL_DestroyTexture(t); }
        for (int i = 0; i < TEXTURE_NUM; i++)
        {
            if (tt[i]) { SDL_DestroyTexture(tt[i]); }
        }
    }
    void toTexture();

    SDL_Surface* s = NULL;                      //����ͼ��Ӧ�ı���
    SDL_Texture* t = NULL;                      //����ͼ��Ӧ������
    int t_count = 1;
    SDL_Texture* tt[TEXTURE_NUM];
    int w = 0;                                  //��ͼ���
    int h = 0;                                  //��ͼ�߶�
    int xoff = 0;                               //��ͼƫ��
    int yoff = 0;
    int id;                                     //��ͼ���
    int fileid;                                 //��ͼ�ļ����
};

struct PicFileCache                             //��ͼ�ļ�����ڵ�
{
    int num = 0;                                //�ļ���ͼ����
    int* idx = NULL;                            //idx������
    int filelength = 0;                         //grp�ļ�����
    FILE* fp = NULL;                            //grp�ļ����
    unsigned char* grp = NULL;                  //grp������
    int width;                                  //ָ�����
    int height;                                 //ָ���߶�
    int percent;                                //ָ������
    std::vector<CacheNode*> pcache;             //�ļ������е���ͼ��Ӧ��cache�ڵ�ָ�룬Ϊ�����ʾû�С�
    char path[512];                             //Ŀ¼
    char suffix[12];                            //��׺��
    int type = 0;                               //0-idx/grp, 1-png path with index.ka
    std::vector<short> offset;                  //ƫ��ֵ
    ZipFile zip_file;
};
#define PIC_FILE_NUM 10000                        //�������ͼ�ļ�(idx/grp)����

int Init_Cache();
int JY_PicInit(const char* PalletteFilename);
int JY_PicLoadFile(const char* idxfilename, const char* grpfilename, int id, int width, int height);
int JY_LoadPic(int fileid, int picid, int x, int y, int flag, int value,int color = 0, int width = -1, int height = -1, double angle = NULL, SDL_RendererFlip reversal = SDL_FLIP_NONE, int percent = 100);
int LoadPic(int fileid, int picid, struct CacheNode* cache);
int JY_LoadPNGPath(const char* path, int fileid, int num, int percent, const char* suffix);
int JY_LoadPNG(int fileid, int picid, int x, int y, int flag, int value, int percent);
int JY_GetPNGXY(int fileid, int picid, int* w, int* h, int* xoff, int* yoff);
int JY_GetPicXY(int fileid, int picid, int* w, int* h, int* xoff, int* yoff);
SDL_Texture* CreateTextureFromRLE(unsigned char* data, int w, int h, int datalong);
int LoadPalette(const char* filename);
int RenderTexture(SDL_Texture* lps, int x, int y, int flag, int value, int color, int width = -1, int height = -1, double angle = NULL , SDL_RendererFlip reversal = SDL_FLIP_NONE, int percent = 100);


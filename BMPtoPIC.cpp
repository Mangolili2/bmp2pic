#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <ctime>
#include <climits>
#include <iomanip>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <vector>
#include <windows.h>
#pragma warning 
using namespace std;

unsigned char* pBmp; //读入图像数据的指针
int biBitCount = 24; //传递比特数     //默认按原图为24

struct pic_infor    //pic文件图像文件头
{
    bool isColor;             //1表示为彩色图像，0表示为灰度图像
    bool M;                   //保留
    short int Notes_size;     //注释区字节数
    short int Rows;           //行       
    short int Columns;        //列       
    short int Rows_start;     //行起点 
    short int Columns_start;  //列起点
    short int T;              //保留
    char some_information[48]; //其他参数

};

struct pic_Text_Notes       //注释区
{
    char* Text_Notes;
};

struct pic_RGB              //色盘区
{
    struct RGB  //红、绿、蓝
    {
        uint8_t rgbRed;     //指定红色强度 
        uint8_t rgbGreen;   //指定绿色强度 
        uint8_t rgbBlue;    //指定蓝色强度
    }ColorTable[256];
};

struct pic_data             //图像数据
{
    unsigned char* data_pic;
};

struct pic     //pit图像文件格式
{
    pic_infor infor;     //头文件
    pic_Text_Notes Note; //注释
    pic_RGB RGB;         //色盘区 
    pic_data data;       //图像数据
};

pair<bool, pic> read_bmp(char* path)   //读取bmp图像文件数据
{
    pic v;   //存放想要的信息

    FILE* fp = fopen(path, "rb");   //二进制只读的方式打开bmp文件
    if (fp == 0)  return { 0,v };

    fseek(fp, sizeof(BITMAPFILEHEADER), 0);//跳过位图文件头，因为没有想要的信息

    //读取位图信息头
    BITMAPINFOHEADER head;  
    fread(&head, sizeof(BITMAPINFOHEADER), 1, fp);

    //读取调色板数据
    RGBQUAD* pColorTable = new RGBQUAD[256];  
    fread(pColorTable, sizeof(RGBQUAD), 256, fp);

    //存放RGB
    bool fl = true; //判断是否是灰度图
    for (int i = 0; i < 256; i++)
    {
        if (pColorTable[i].rgbBlue != pColorTable[i].rgbGreen || pColorTable[i].rgbGreen != pColorTable[i].rgbRed)
            fl = false;
        v.RGB.ColorTable[i].rgbRed = pColorTable[i].rgbRed;
        v.RGB.ColorTable[i].rgbGreen = pColorTable[i].rgbGreen;
        v.RGB.ColorTable[i].rgbBlue = pColorTable[i].rgbBlue;
    }
    if (fl)  v.infor.isColor = 0;
    else    v.infor.isColor = 1;

    v.infor.Rows = head.biHeight;
    v.infor.Columns = head.biWidth;

    biBitCount = head.biBitCount;

    v.infor.Notes_size = 0;
    v.Note.Text_Notes = 0;

    int line_byte = (head.biWidth * head.biBitCount / 8 + 3) / 4 * 4;
    
    //图像数据读入
    v.data.data_pic = new unsigned char[line_byte * head.biHeight];
    fread(v.data.data_pic, 1, line_byte * head.biHeight, fp);

    fclose(fp);
    return { 1,v };
}
void save_bmp(char* path, pic v)   //保存bmp文件
{
    FILE* fp = fopen(path, "wb");
    if (fp == 0) { cout << "error in path (save)" << endl; return; }

    int line_byte = (v.infor.Columns * biBitCount / 8 + 3) / 4 * 4;
    // cout << line_byte << endl;
    int colorTablesize = 0;

    if (biBitCount == 8)
        colorTablesize = 1024;

   //文件头
    BITMAPFILEHEADER fileHead;   //文件头
    fileHead.bfType = 0x4D42;

    fileHead.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + line_byte * v.infor.Rows;
    fileHead.bfReserved1 = 0;
    fileHead.bfReserved2 = 0;
    fileHead.bfOffBits = 56;
    fwrite(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);

    //消息头
    BITMAPINFOHEADER head;
    head.biBitCount = biBitCount;
    head.biClrImportant = 0;
    head.biClrUsed = 0;
    head.biCompression = 0;
    head.biHeight = v.infor.Rows;
    head.biPlanes = 1;
    head.biSize = 40;
    head.biSizeImage = line_byte * v.infor.Rows;
    head.biWidth = v.infor.Columns;
    head.biXPelsPerMeter = 0;
    head.biYPelsPerMeter = 0;
    fwrite(&head, sizeof(BITMAPINFOHEADER), 1, fp);

    //调色板
    RGBQUAD now_col[256];
    for (int i = 0; i < 256; i++)
    {
        now_col[i].rgbReserved = 0;
        now_col[i].rgbRed = v.RGB.ColorTable[i].rgbRed;
        now_col[i].rgbBlue = v.RGB.ColorTable[i].rgbBlue;
        now_col[i].rgbGreen = v.RGB.ColorTable[i].rgbGreen;
    }
    fwrite(&now_col, sizeof(RGBQUAD), 256, fp);

    // }
   
    //图像数据
    fwrite(pBmp, v.infor.Rows * line_byte, 1, fp);
    fclose(fp);
    return;
}
pair<bool, pic> read_pic(char* path)  //读取pic文件
{
    pic v;  //存放信息
    FILE* fp = fopen(path, "rb");  //二进制只读方式打开pic文件
    if (fp == 0)  return { 0,v };

    fread(&v.infor, sizeof(pic_infor), 1, fp);  //文件头读入（注释除外）
    fread(&v.Note, v.infor.Notes_size * sizeof(char), 1, fp);//注释读入
    fread(&v.RGB, sizeof(pic_RGB), 1, fp);//色盘区读入

    int line_byte = (v.infor.Columns * biBitCount / 8 + 3) / 4 * 4;
    //图像数据读入
    pBmp = new unsigned char[line_byte * v.infor.Rows];
    fread(pBmp, 1, line_byte * v.infor.Rows, fp);
    fclose(fp);
    return { 1,v };
}
bool save_pic(char* path, pic t)    //保存pic文件
{
    FILE* fp = fopen(path, "wb");   //用二进制写的方式打开pic文件
    if (fp == 0) { cout << "error in path" << endl; return 0; }

    //文件头读入(除注释区)
    fwrite(&t.infor, 64 * sizeof(unsigned char), 1, fp);

    //注释区读入
    fwrite(&t.infor + 64, t.infor.Notes_size * sizeof(char), 1, fp);

    //色盘区读入
    fwrite(&t.RGB, sizeof(pic_RGB), 1, fp);

    //图像数据读入
    int line_byte = (t.infor.Columns * biBitCount / 8 + 3) / 4 * 4;
    fwrite(t.data.data_pic, t.infor.Rows * line_byte * sizeof(unsigned char), 1, fp);

    fclose(fp);
    return 1;
}

void bmp_to_pic()
{
    char readPath[] = "bmp文件路径";
    auto t = read_bmp(readPath);
    if (t.first == 0) { cout << "error in path" << endl; return; }
    char writePath[] = "生成的pic文件存放路径";
    save_pic(writePath, t.second);
    return;
}

void pic_to_bmp()
{
    char readPath[] = "pic文件路径";
    auto t = read_pic(readPath);
    if (!t.first) { cout << "error in path" << endl; return; }
    char writePath[] = "pic转换为bmp文件的存放路径";
    save_bmp(writePath, t.second);
    return;
}

int main()
{
    bmp_to_pic();
    pic_to_bmp();
    return 0;
}
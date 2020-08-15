
#include <iostream>
#include <sstream>
#include <windows.h>
#include <curl/curl.h>  
#include <openssl/md5.h>
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<string>
#include <fstream>
#include <atlbase.h>
#include "ocr/ocr.h"
#include"SaveBitmapToFile.h"
#include <direct.h>
#pragma warning(disable:4996)  //使允许使用sprintf()等函数
using namespace std;
//全局变量
HINSTANCE hInst;//HINSTANCE 是Windows里的一种数据类型，其实就是一个无符号的长整形，是32位的，是用于标示（记录）一个程序的实例。它与HMODULE是一样的（通用的，这两种类型最终就是无符号长整形）。HINSTANCE， 分开看就是 H + INSTANCE, 其中H代表 HANDLE（再程序中翻译为“句柄”的意思），INSTANCE 中文就是"实例"的意思。想得到一个实例， 可通过全局API函数 GetModuleHandle 得到，参数传入模块的名字（exe或者DLL的名字），返回的类型是HMODULE，其实就是HINSTANCE类型。 或者在入口函数（如WinMain）的参数可以得到（是操作系统传进入的），你可以在入口函数里把这个变量的值保存起来以方便后面使用。
static TCHAR szClassName[] = TEXT(" Easy-Tran");  //窗口类名
string szInText;
string szOutText;
int mode;               //翻译模式 1-智能模式 2-英译中 3-

//函数声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); //窗口过程
void RegistWindowClass(HINSTANCE hInstance);          //注册窗口类
HWND InitWindow(HINSTANCE hInstance);                 //初始化窗口
HWND InitInBox(HWND hWnd, HINSTANCE hInstance);        //初始化编辑框（文本输入框）
HWND InitOutBox(HWND hWnd, HINSTANCE hInstance);      //初始化文本框（输出翻译结果）
HWND InitBotton(HWND hWnd, HINSTANCE hInstance);      //初始化按钮  （翻译按钮）
HWND OcrBotton(HWND hWnd, HINSTANCE hInstance);      //初始化按钮  （截屏按钮）
HWND InitModeGroup(HWND hWnd, HINSTANCE hInstance);    //初始化分组框（模式选项）
HWND InitCheckBox(HWND hPreWnd, HINSTANCE hInstance);  //初始化复选框 (智能模式)
HWND InitMode1(HWND hPreWnd, HINSTANCE hInstance);    //初始化单选框1 (英译中)
HWND InitMode2(HWND hPreWnd, HINSTANCE hInstance);    //初始化单选框2（中译英）

HFONT InitFont();                                 //初始化字体
void SetFont(HWND target, HFONT hFont);            //为窗口控件设置字体

//char* buffer =_getcwd(NULL, 0);

//string base = buffer;
//string basepic = base + "/test.bmp";

string string_To_UTF8(const string& str);  //来源： zhangxueyang1的博客
string UTF8_To_string(const string& str);
string wchartTostring(wchar_t* pWCStrKey);
int Translate();
void readFileJson();
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PSTR szCmdLine,
    int iCmdShow
)
{
    
    
    HWND     hwnd;  //窗口句柄
    MSG      msg;   //消息

    hInst = hInstance;

    RegistWindowClass(hInstance);
    hwnd = InitWindow(hInstance);

    ShowWindow(hwnd, iCmdShow);//显示窗口
    UpdateWindow(hwnd);//更新（绘制）窗口

  //消息循环
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);  //翻译消息
        DispatchMessage(&msg);  //分派消息
    }

    return msg.wParam;
}

//窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    
    PAINTSTRUCT ps;
    HDC hdc;
    HDC hdcStatic;

    HDC         hdc2;
  
    HBITMAP        hBitmap;
    HDC            hdcMem;
   
    //const  char * filePath=basepic.c_str();
    const  char* filePath = "test.bmp";
    int wmID;//被触发的按钮id
    int wmEvent; //被触发的按钮事件


    static HFONT hFont;
    static HWND hOutBox;
    static HWND hInBox;
    static HWND hButton;
    static HWND hocr;
    static HWND hModeGroup;
    static HWND hMode1;
    static HWND hMode2;
    static HWND hCheckBox;
    static POINT ptstart;
    static POINT ptend;
    static bool ocrjudge = false;

    static HBRUSH hBrushWhite;
    //获取翻译模式 (智能/中英/英中)
    if (SendMessage(hCheckBox, BM_GETCHECK, 0, 0) == BST_CHECKED)
        mode = 1;
    else if (SendMessage(hMode1, BM_GETCHECK, 0, 0) == BST_CHECKED)
        mode = 2;
    else if (SendMessage(hMode2, BM_GETCHECK, 0, 0) == BST_CHECKED)
        mode = 3;
    else
        mode = 0;
    //处理消息
    switch (message) {
    case  WM_CREATE:
        hBrushWhite = CreateSolidBrush(RGB(0xff, 0xff, 0xff)); //白色画刷

        //初始化窗口组件
        hFont = InitFont();
        hOutBox = InitOutBox(hWnd, hInst);
        hButton = InitBotton(hWnd, hInst);
        hInBox = InitInBox(hWnd, hInst);
        hocr = OcrBotton(hWnd, hInst);
        hModeGroup = InitModeGroup(hWnd, hInst);
        hCheckBox = InitCheckBox(hModeGroup, hInst);
        hMode1 = InitMode1(hModeGroup, hInst);
        hMode2 = InitMode2(hModeGroup, hInst);
        

        //字体优化
        SetFont(hOutBox, hFont);
        SetFont(hInBox, hFont);
        SetFont(hButton, hFont);
        SetFont(hocr, hFont);
        SetFont(hModeGroup, hFont);
        SetFont(hCheckBox, hFont);
        SetFont(hMode1, hFont);
        SetFont(hMode2, hFont);

        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
         //TODO: // 在此添加任意绘图代码...
        EndPaint(hWnd, &ps);
        break;
    case WM_CTLCOLORSTATIC:
        return (INT_PTR)hBrushWhite;  //返回画刷句柄，给静态文本框着色
    case WM_COMMAND:
       // 处理按钮事件
        wmID = LOWORD(wParam); //被触发的按钮id
        wmEvent = HIWORD(wParam); //被触发的按钮事件

        switch (wmID)
        {
        case 3:  // 翻译按钮
        {TCHAR buff[1000];
        GetWindowText(hInBox, buff, 1000);
        szInText.clear();
        szInText.append(wchartTostring(buff));        
        szInText = string_To_UTF8(szInText);
        Translate();            
        SetWindowText(hOutBox, CA2W(szOutText.c_str()));
        break;
        }
        case 8://OCR
        {
            ocrjudge = true;
            SetCapture(hWnd);
        }
        //default:
            //不处理的消息要交给 DefWindowProc 处理。
            //return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_LBUTTONDOWN:
        if (ocrjudge)
        {
            SetWindowText(hOutBox, CA2W("正在截图。。。"));
            //ptstart.x=LOWORD(lParam);
            //ptstart.y=HIWORD(lParam);
            
            
            GetCursorPos(&ptstart);
            FILE* myfp1;
           /* myfp1 = fopen("C:/Users/hehe/Desktop/ocr.txt", "w+");
            fprintf(myfp1, "%d,%d,", ptstart.x, ptstart.y);
            fclose(myfp1);*/
            
        }
        break;
    case WM_LBUTTONUP:
        if (ocrjudge)
        {
            
            SetWindowText(hOutBox, CA2W("正在截图。。。"));
            ocrjudge = false;
            
            
            GetCursorPos(&ptend);
            /*FILE* myfp2;
            myfp2 = fopen("C:/Users/hehe/Desktop/pos.txt", "w+");
            fprintf(myfp2, "%d",a);
            fclose(myfp2)*/;
            /*if (wParam & MK_LBUTTON)
            {
                ptend.x = LOWORD(lParam);
                ptend.y = HIWORD(lParam);
            }*/
            
            /*FILE* myfp1;
            myfp1 = fopen("C:/Users/hehe/Desktop/ocr.txt", "a");
            fprintf(myfp1, "%d,%d", ptend.x, ptend.y);
            fclose(myfp1);*/
            
            ReleaseCapture();
            hdc2= GetDC(NULL);

            
            hBitmap = CreateCompatibleBitmap(hdc2, ptend.x - ptstart.x, ptend.y - ptstart.y);
            hdcMem = CreateCompatibleDC(hdc2);

            SelectObject(hdcMem, hBitmap);
            
            BitBlt(hdcMem, 0, 0, ptend.x-ptstart.x, ptend.y-ptstart.y, hdc2, ptstart.x, ptstart.y, SRCCOPY);
            SaveBitmapToFile(CA2W(filePath), hBitmap);

            ReleaseDC(hWnd, hdcMem);
            ReleaseDC(hWnd, hdc2);
            string ocrapp_id = "21761657";
            string ocrapi_key = "DCkeU1USRXEwDKlbxsLRAVkm";
            string ocrsecret_key = "jyP1KTHqSSEMz6iBOm2GUdLT2ZTHQ2qN";
            aip::Ocr client(ocrapp_id, ocrapi_key, ocrsecret_key);
            Json::Value result;

            std::string image;
            aip::get_file_content(filePath, &image);



            // 如果有可选参数
            std::map<std::string, std::string> options;

            options["language_type"] = "CHN_ENG";
            options["detect_direction"] = "true";
            options["detect_language"] = "true";
            options["probability"] = "true";

            // 带参数调用通用文字识别, 图片参数为本地图片
            result = client.general_basic(image, options);
            int total = result["words_result_num"].asInt();
            string ocrtext = "";
            for (int num = 1; num <= total; num++)
            {
                ocrtext += result["words_result"][num - 1]["words"].asString();
            }

            string ocrText = UTF8_To_string(ocrtext);
            SetWindowText(hInBox, CA2W(ocrText.c_str()));
           
        }
        break;
    

    case WM_DESTROY:
        DeleteObject(hFont);//删除创建的字体
        PostQuitMessage(0);
        break;
    
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void RegistWindowClass(HINSTANCE hInstance)
{

    WNDCLASS wndclass;  //窗口类

    //为窗口类的各个字段赋值
    wndclass.style = CS_HREDRAW | CS_VREDRAW;  //窗口风格
    wndclass.lpfnWndProc = WndProc;  //窗口过程
    wndclass.cbClsExtra = 0;  //暂时不需要理解
    wndclass.cbWndExtra = 0;  //暂时不需要理解
    wndclass.hInstance = hInstance;  //当前窗口句柄
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);  //窗口图标
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);  //鼠标样式
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);  //窗口背景画刷（白色）
    wndclass.lpszMenuName = NULL;  //窗口菜单
    wndclass.lpszClassName = szClassName;  //窗口类名
    //注册窗口
    RegisterClass(&wndclass);
}

HWND InitWindow(HINSTANCE hInstance)
{
    return CreateWindow(
        szClassName,  //窗口名字
        TEXT("Easy-Tran"),  //窗口标题（出现在标题栏）
        WS_OVERLAPPEDWINDOW,  //窗口风格
        //CW_USEDEFAULT,  //初始化时x轴的位置
        //CW_USEDEFAULT,  //初始化时y轴的位置
        0,0,
        600,  //窗口宽度
        600,  //窗口高度
        NULL,  //父窗口句柄
        NULL,  //窗口菜单句柄
        hInstance,  //当前窗口的句柄
        NULL  //不使用该值
    );
}


HWND InitInBox(HWND hWnd, HINSTANCE hInstance)
{
    return CreateWindow(
        TEXT("edit"),
        TEXT("InitInBox"),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_LOWERCASE | ES_AUTOVSCROLL,
        10, 40, 460, 200,
        hWnd, (HMENU)1, hInst, NULL
    );
}

HWND InitOutBox(HWND hWnd, HINSTANCE hInstance)
{
    return CreateWindow(
        TEXT("edit"),     //框的类名
        TEXT("InitOutBox"),  //控件的文本
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_LOWERCASE | ES_AUTOVSCROLL | ES_READONLY,
        10 /*X坐标*/, 350 /*Y坐标*/, 460 /*宽度*/, 200 /*高度*/,
        hWnd,  //父窗口句柄
        (HMENU)2,  //为控件指定一个唯一标识符
        hInstance,  //当前程序实例句柄
        NULL
    );
}

HWND InitBotton(HWND hWnd, HINSTANCE hInstance)
{
    return CreateWindow(
        TEXT("button"), //按钮控件的类名
        TEXT("翻译"),
        WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT/*扁平样式*/,
        280 /*X坐标*/, 270 /*Y坐标*/, 80 /*宽度*/, 50/*高度*/,
        hWnd, (HMENU)3 /*控件唯一标识符*/, hInst, NULL
    );
}
HWND OcrBotton(HWND hWnd, HINSTANCE hInstance)
{
    return CreateWindow(
        TEXT("button"), //ocr
        TEXT("截屏"),
        WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT/*扁平样式*/,
        390 /*X坐标*/, 270 /*Y坐标*/, 80 /*宽度*/, 50/*高度*/,
        hWnd, (HMENU)8 /*控件唯一标识符*/, hInst, NULL
    );
}

HWND InitModeGroup(HWND hWnd, HINSTANCE hInstance)
{
    return CreateWindow(
        TEXT("button"), TEXT("MODE"),
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        10, 240, 240, 105,
        hWnd, (HMENU)4, hInst, NULL
    );
}

HWND InitCheckBox(HWND hPreWnd, HINSTANCE hInstance)
{
    return CreateWindow(
        TEXT("button"),
        TEXT("智能模式"),
        WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON/*复选框*/,
        5, 25, 100, 40,
        hPreWnd, (HMENU)5, hInst, NULL
    );
}

HWND InitMode1(HWND hPreWnd, HINSTANCE hInstance)
{
    return  CreateWindow(
        TEXT("button"), TEXT("英文 → 中文"),
        WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON,
        110, 15, 120, 40,
        hPreWnd, (HMENU)6, hInst, NULL
    );
}

HWND InitMode2(HWND hPreWnd, HINSTANCE hInstance)
{
    return CreateWindow(
        TEXT("button"), TEXT("中文 → 英文"),
        WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON,
        110, 55, 120, 40,
        hPreWnd, (HMENU)7, hInst, NULL
    );
}

HFONT InitFont()
{
    return CreateFont(
        -16/*高度*/, -8/*宽度*/, 0/*不用管*/, 0/*不用管*/, 400 /*一般这个值设为400*/,
        FALSE/*不带斜体*/, FALSE/*不带下划线*/, FALSE/*不带删除线*/,
        DEFAULT_CHARSET,  //这里我们使用默认字符集，还有其他以 _CHARSET 结尾的常量可用
        OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,  //这行参数不用管
        DEFAULT_QUALITY,  //默认输出质量
        FF_DONTCARE,  //不指定字体族
        TEXT("微软雅黑")  //字体名
    );
}

void SetFont(HWND target, HFONT hFont)
{
    SendMessage(
        target,  //欲设置字体的控件句柄
        WM_SETFONT,  //消息名（消息类型）
        (WPARAM)hFont,  //字体句柄
        NULL  //传空值即可
    );
}

int Translate()
{
        char* from;
        char* to ;

        //模式选择
        switch (mode)
        {
        case 1:
        {
            from = "auto";
            to = "zh";
            break;
        }
        case 2:
        {
            from = "en";
            to = "zh";
            break;
        }
        case 3:
        {
            from = "zh";
            to = "en";
            break;
        }
        default:
            return 0;
        }
        CURL* curl;
        CURLcode res;//CURLcode是一种枚举类型
        curl = curl_easy_init();
        FILE* fp1;
        fp1 = fopen("myfile1.json", "w");
        FILE* fp2;
        fp2 = fopen("myfile2.json", "w");
        
        if (curl) {
            char myurl[1000] = "http://api.fanyi.baidu.com/api/trans/vip/translate?";
            char* appid = "20200712000517884";    //replace myAppid with your own appid
            //const char* q = "我是好孩子?";          //replace apple with your own text to be translate, ensure that the input text is encoded with UTF-8!
            
            
            const char*q = szInText.c_str(); //szInText为UTF-8的string
            char *output = curl_easy_escape(curl, q, strlen(q)); //url_encode
            std::cout <<"output=" <<output;
            
            char salt[60];
            int a = 100;
            sprintf(salt, "%d", a);
            char* secret_key = "3KZqBCvBWwbvLeYZ776F";   //replace mySecretKey with your own mySecretKey
            char sign[120] = "";
            strcat(sign, appid);
            strcat(sign, q);
            strcat(sign, salt);
            strcat(sign, secret_key);
            printf("%s\n", sign);
            unsigned char md[16];
            int i;
            char tmp[3] = { '\0' }, buf[33] = { '\0' };
            MD5(( const unsigned char*)sign, strlen(sign), md);
            for (i = 0; i < 16; i++) {
                sprintf(tmp, "%2.2x", md[i]);
                strcat(buf, tmp);
            }
            printf("%s\n", buf);
            strcat(myurl, "appid=");
            strcat(myurl, appid);
            strcat(myurl, "&q=");
            strcat(myurl, output);//url-encode
            strcat(myurl, "&from=");
            strcat(myurl, from);
            strcat(myurl, "&to=");
            strcat(myurl, to);
            strcat(myurl, "&salt=");
            strcat(myurl, salt);
            strcat(myurl, "&sign=");
            strcat(myurl, buf);
            printf("%s\n", myurl);
           // 设置访问的地址
            curl_easy_setopt(curl, CURLOPT_URL, myurl);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp1); //将返回的html内容输出到fp指向的文件
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, fp2);
    
           res = curl_easy_perform(curl); //此句接受返回的翻译
            
            /* Check for errors */
            if (res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
    
            /* always cleanup */
            curl_easy_cleanup(curl);
            fclose(fp1);
            fclose(fp2);
            readFileJson();
            
  
    }
    /***********************
    * 返回值说明：
    * 52000	成功
    * 52001	请求超时
    * 52002	系统错误
    * 52003	未授权用户
    * 54000	必填参数为空
    * 54001	签名错误
    * 54003	访问频率受限
    * 54004	账户余额不足
    * 54005	长query请求频繁
    * 58000	客户端IP非法
    * 58001	译文语言方向不支持
    ***********************/
    return 0;
}
void readFileJson()
{
    Json::Reader reader;
    Json::Value root;
    //从文件中读取，保证当前文件有demo.json文件  
    ifstream in("myfile1.json", std::ios::binary);
    if (reader.parse(in, root))
    {
        //读取子节点信息  
        string dst = root["trans_result"][0]["dst"].asString();//是UTF-8 string
        /* FILE* fp = fopen("C:/Users/hehe/Desktop/test.txt", "w");
        fprintf(fp, dst.c_str());
        fclose(fp);*/
        szOutText = UTF8_To_string(dst);
       /* FILE* fp = fopen("C:/Users/hehe/Desktop/test.txt", "w");
        fprintf(fp, szOutText.c_str());
        fclose(fp);*/
    }
    in.close();
}


string string_To_UTF8(const string& str)
{
    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

    wchar_t* pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴 
    ZeroMemory(pwBuf, nwLen * 2 + 2);

    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char* pBuf = new char[nLen + 1];
    ZeroMemory(pBuf, nLen + 1);

    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
    std::string retStr(pBuf);
    delete[]pwBuf;
    delete[]pBuf;
    pwBuf = NULL;
    pBuf = NULL;
    return retStr;
}
string  wchartTostring(wchar_t* pWCStrKey)
{
    //第一次调用确认转换后单字节字符串的长度，用于开辟空间
    int pSize = WideCharToMultiByte(CP_OEMCP, 0, pWCStrKey, wcslen(pWCStrKey), NULL, 0, NULL, NULL);
    char* pCStrKey = new char[pSize + 1];
    //第二次调用将双字节字符串转换成单字节字符串
    WideCharToMultiByte(CP_OEMCP, 0, pWCStrKey, wcslen(pWCStrKey), pCStrKey, pSize, NULL, NULL);
    pCStrKey[pSize] = '\0';
   // 如果想要转换成string，直接赋值即可
    string pKey = pCStrKey;
    return pKey;
}

string UTF8_To_string(const string& str)
{
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* pwBuf = new wchar_t[nwLen + 1];    //一定要加1，不然会出现尾巴 
    memset(pwBuf, 0, nwLen * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
    char* pBuf = new char[nLen + 1];
    memset(pBuf, 0, nLen + 1);
    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
    std::string strRet = pBuf;
    delete[]pBuf;
    delete[]pwBuf;
    pBuf = NULL;
    pwBuf = NULL;
    return strRet;
}




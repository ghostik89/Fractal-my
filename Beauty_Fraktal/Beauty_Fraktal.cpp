// Beauty_Fraktal.cpp: Определяет точку входа для приложения.
//

#include "stdafx.h"
#include "Beauty_Fraktal.h"
#include "ppl.h"
#include <functional>

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//my items
const size_t MaxIterations(8000); //Maximum iterations before infinity
size_t IteratePoint(double zReal, double zImaginary, double cReal,
	double cImaginary);
//choose color for pixel
COLORREF Color(int n);
//Draw picture
void DrawSet(HWND hwnd);
//parallel draw fractal
void DrawSetParallel(HWND hWnd);

COLORREF Color(int n) {
	if (n == MaxIterations) return RGB(0,0,0);

	const int nColors = 16;

	switch (n % nColors) {
	case 0: return RGB(100,100,100);
	case 1: return RGB(100, 0, 0);
	case 2: return RGB(200, 0, 0);
	case 3: return RGB(100, 100, 0);
	case 4: return RGB(200, 100, 0);
	case 5: return RGB(100, 100, 100);
	case 6: return RGB(0, 200, 0);
	case 7: return RGB(0, 100, 100);
	case 8: return RGB(0, 200, 100);
	case 9: return RGB(0, 100, 200);
	case 10: return RGB(0, 200, 200);
	case 11: return RGB(0, 0, 200);
	case 12: return RGB(100, 0, 100);
	case 13: return RGB(200, 0, 100);
	case 14: return RGB(100, 0, 200);
	case 15: return RGB(200, 0, 200);
	default: return RGB(200,200,200);
	};
}

void DrawSet(HWND hWnd) {
	//Get size of client area
	RECT rect;
	GetClientRect(hWnd, &rect);
	int imageHeignt(rect.bottom);
	int imageWidth(rect.right);

	//create line of pixels
	HDC hdc(GetDC(hWnd));				//get cotext 
	HDC memDc = CreateCompatibleDC(hdc);//for draw

	HBITMAP bmp = CreateCompatibleBitmap(hdc, imageWidth, 1);
	HGDIOBJ oldBmp = SelectObject(memDc, bmp);//choose image DC

	//client area
	const double realMin(-2.1);//max double value
	double imaginaryMin(-1.3); //
	double imaginaryMax(+1.3);

	double realMax(realMin + (imaginaryMax - imaginaryMin)* imageWidth /
		(imageHeignt - 1));

	double realScale((realMax - realMin) / (imageWidth));
	double imaginaryScale((imaginaryMax - imaginaryMin)/(imageHeignt - 1));
	double cReal(0.0), cImaginary(0.0); // save in c
	double zReal(0.0), zImaginary(0.0); // save in z
	//
	for (int y = 0; y < imageHeignt; y++) {
		zImaginary = cImaginary = imaginaryMax - y * imaginaryScale;
		for (int x = 0; x < imageWidth; x++) {
			zReal = cReal = realMin + x * realScale;

			//set color for n
			SetPixel(memDc,x,0,Color(IteratePoint(zReal,zImaginary,cReal,cImaginary)));
		}
		//share string pixels in window
		BitBlt(hdc, 0, y, imageWidth, 1, memDc, 0, 0, SRCCOPY);
	}
	SelectObject(memDc, oldBmp);
	DeleteObject(bmp);
	DeleteDC(memDc);
	ReleaseDC(hWnd, hdc);
}

void DrawSetParallel(HWND hWnd) {
	HDC hdc(GetDC(hWnd));//Получить контекст устройства

	//Получить размерность клиентской области, составляющие размер изобрважения
	RECT rect; //размеры окна
	GetClientRect(hWnd, &rect);
	int imageheight(rect.bottom);
	int imageWidth(rect.right);

	//Создать один ряд пикселей изображения
	HDC memDC1 = CreateCompatibleDC(hdc);//Получить контекст устройства
										 //для рисования пикселей
	HDC memDC2 = CreateCompatibleDC(hdc);
	HDC memDC3 = CreateCompatibleDC(hdc);
	HDC memDC4 = CreateCompatibleDC(hdc);

	HBITMAP bmp1 = CreateCompatibleBitmap(hdc, imageWidth, 1);
	HBITMAP bmp2 = CreateCompatibleBitmap(hdc, imageWidth, 1);
	HBITMAP bmp3 = CreateCompatibleBitmap(hdc, imageWidth, 1);
	HBITMAP bmp4 = CreateCompatibleBitmap(hdc, imageWidth, 1);
	//Выбрать изображение в DC1
	HGDIOBJ oldBmp1 = SelectObject(memDC1, bmp1);
	//Выбрать изображение в DC2
	HGDIOBJ oldBmp2 = SelectObject(memDC2, bmp2);
	//Выбрать изображение в DC3
	HGDIOBJ oldBmp3 = SelectObject(memDC3, bmp3);
	//Выбрать изображение в DC4
	HGDIOBJ oldBm42 = SelectObject(memDC4, bmp4);

	//Оси клиентской области
	const double realMin(-2.1); //Минимальное вещественное значение
	double imaginaryMin(-1.3); //Минимальное мнимое значение
	double imaginaryMax(1.3); //Максимальное мнимое значения


	//установить максимум вещественного значения, чтобы оси имели
	//одинаковый масштаб
	double realMax(realMin + (imaginaryMax - imaginaryMin) * imageWidth / imageheight);

	//получить коэфиценты масштабирования для координат
	//пикслей
	double realScale((realMax - realMin)/(imageWidth - 1));
	double imaginaryScale((imaginaryMax - imaginaryMin) / (imageheight - 1));

	// лямбда выражение для создания строки выражения 
	std::function<void(HDC&, int)> rowCalc = [&](HDC& memDC, int yLocal) {
		double zReal(0.0), cReal(0.0);
		double zImaginary(imaginaryMax - yLocal * imaginaryScale);
		double cimaginary(zImaginary);
		//перебрать пиксели в строке
		for (int x = 0; x < imageWidth; ++x) {
			zReal = cReal = realMin + x * realScale;

			//установить цвет пикселя на основании n
			SetPixel(memDC, x, 0, Color(IteratePoint(zReal, zImaginary, cReal, cimaginary)));
		}
	};

	//Перебрать все строки изображения
	for (int y = 3;y < imageheight; y += 3) {
		//Concurrency::parallel_invoke([&] {rowCalc(memDC1, y - 3)}, );
	}
}


size_t IteratePoint(double zReal, double zImaginary, double cReal, double cImaginary) {
	double zReal2(0.0), zImaginary2(0.0);
	size_t n(0);

	for (; n < MaxIterations; ++n) {
		zReal2 = zReal * zReal;
		zImaginary2 = zImaginary * zImaginary;
		if (zReal2 + zImaginary2 > 4) // if distance more, than 2
			break;//

		//calc next value
		zImaginary = 2 * zReal*zImaginary + cImaginary;
		zReal = zReal2 - zImaginary2 + cReal;
	}
	return n;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BEAUTYFRAKTAL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BEAUTYFRAKTAL));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  НАЗНАЧЕНИЕ: регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BEAUTYFRAKTAL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_BEAUTYFRAKTAL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   НАЗНАЧЕНИЕ: сохраняет обработку экземпляра и создает главное окно.
//
//   КОММЕНТАРИИ:
//
//        В данной функции дескриптор экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится на экран главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      100, 100, 900, 900, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  НАЗНАЧЕНИЕ:  обрабатывает сообщения в главном окне.
//
//  WM_COMMAND — обработать меню приложения
//  WM_PAINT — отрисовать главное окно
//  WM_DESTROY — отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
			DrawSet(hWnd);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

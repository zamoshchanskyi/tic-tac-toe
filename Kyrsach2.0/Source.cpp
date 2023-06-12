#include <windows.h>
#include <GdiPlus.h>
#include <fstream>
#include <string>
#include "resource.h"

//Глобальні змінні
HINSTANCE hInst;
LPCTSTR szWindowClass = L"QWERTY";
LPCTSTR szTitle = L"Хрестики-нулики";

using namespace Gdiplus;
#pragma comment (lib, "Gdiplus.lib")
using namespace std;

int fiveWin = 5; // для пяти подряд крестиков или ноликов
int draw = 0; //рисование полей
int vertical = 4;
int height = 0; //высота
int width = 0;	//ширина
int rowsX = 30; //ряды
int colsY = 18; //столбцы
const int X = 18;
const int Y = 30;
int place[540][6];
int map[X][Y]; // массив для заполнения нулей
int flagStep = 0;
int winner = 0; // флаг определяющий победителя
static int step = 0; // определяет крестик или нолик
HBITMAP hBitmap_menu;
static BITMAP BM;
string Name, Name2;
bool player = 0;
int game_stage = 0;
int FlagInfo = 0;

//Попередній опис функцій
ATOM MyRegisterClass(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawRect(HDC, HWND);
void DrawSymbols(HDC, HWND);
INT_PTR CALLBACK Dialog1(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Dialog2(HWND, UINT, WPARAM, LPARAM);

struct Buttons
{
	HWND hButton;
	int id;
};
Buttons MyButtons[8];

//Основна функція
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG msg;
	ULONG_PTR gdiplusToken;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	GdiplusShutdown(gdiplusToken);
	return msg.wParam;
}

//Реєстрація класу вікна
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcx;
	wcx.cbSize = sizeof(WNDCLASSEX);

	wcx.style = CS_HREDRAW | CS_VREDRAW; //стиль вікна
	wcx.lpfnWndProc = (WNDPROC)WndProc; //віконна процедура
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance; //дескриптор програми
	wcx.hIcon = LoadIcon(NULL, IDI_QUESTION); //визначення іконки
	wcx.hCursor = LoadCursor(NULL, IDC_SIZE); //визначення курсору
	wcx.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH); //установка фону
	wcx.lpszMenuName = NULL; //визначення меню
	wcx.lpszClassName = szWindowClass; //ім'я класу
	wcx.hIconSm = NULL;

	return RegisterClassEx(&wcx); //реєстраці класу вікна
}

//Створює вікно програми
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hInst = hInstance;
	hWnd = CreateWindow(szWindowClass, //ім'я класу вікна
		szTitle, //назва програми
		WS_OVERLAPPEDWINDOW, //стиль вікна
		0, //положення по X
		0, //положення по Y
		1280, //розмір по X
		750, //розмір по Y
		NULL, //дескриптор батьківскього вікна
		NULL, // дескриптор меню вікна
		hInstance, //дескриптор програми
		NULL); //параметри створення

	if (!hWnd)
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);  //показати вікно
	UpdateWindow(hWnd); //оновити вікно

	return TRUE;
}

//Заполнения поля нулями
void Map_Null()
{
	for (int i = 0; i < X; i++)
	{
		for (int j = 0; j < Y; j++)
		{
			map[i][j] = false;
		}
	}
}

/* Проверяем диагонали */
bool checkDiagonal(int offsetX, int offsetY, int choice)
{
	bool toright; // сверху слева вниз направо
	bool toleft; // сверху справа вниз налево.
	toright = true;
	toleft = true;
	int counter_right = 0, counter_left = 0; // счетчики диагоналей
	int j = offsetX;
	for (int i = offsetY; i < fiveWin + offsetY; i++, j++)
	{
		// суть 1 & 1 = 1 и 1 & 0 = 0 
		toright = toright && (map[j][i] == choice);
		toleft = toleft && (map[fiveWin + offsetY - i - 1 + offsetX][i] == choice);
		if (toleft)
			counter_left++;
		if (toright)
			counter_right++;
	}

	if (counter_left == 5 || counter_right == 5)
		return true;
	return false;
}

/* Проверяем горизонтальные и вертикальные линии */
bool checkLanes(int offsetX, int offsetY, int choice)
{
	bool cols;// вертикальная линия
	bool rows;// горизантальная линия
	int counter_row = 0, counter_col = 0; // счетчик
	for (int row = offsetX; row < fiveWin + offsetX; row++)
	{
		cols = true;
		rows = true;
		for (int col = offsetY; col < fiveWin + offsetY; col++)
		{
			// суть 1 & 1 = 1 и 1 & 0 = 0 
			cols = cols && (map[col][row] == choice);
			rows = rows && (map[row][col] == choice);
			if (cols)
				counter_col++;
			if (rows)
				counter_row++;
		}

		if (counter_row == 5 || counter_col == 5)
			return true;
		counter_row = 0;
		counter_col = 0;
	}

	return false;
}

/* Логика победы для работы с любым размером поля. */
bool checkWin()
{
	for (int row = 0; row < 26; row++)
	{
		for (int col = 0; col < 26; col++)
		{
			// Вызываем методы проверки и если хоть один блок заполнен,
			// то игрок, который проставляет это символ, выиграл
			// иначе продолжаем для другого смещения
			if (checkDiagonal(row, col, step) || checkLanes(row, col, step))
			{
				if (step == 1)
				{
					winner = 1;
				}
				else winner = 2;
				return true;
			}
		}
	}
	// Все подквадраты в квадрате проверены. 5-х последовательностей
	// подряд не выявлено. Значит еще не победа.
	return false;
}

//функція, що обробляє всі повідомлення
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int count;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rt;
	hdc = GetDC(hWnd);
	Graphics graphics(hdc);
	Pen bluePen (Color(255, 0, 0, 255), 2);
	Pen redPen1 (Color(255, 255, 0, 0), 2);
	SolidBrush Solid(Color(255, 255, 255, 255));

	FontFamily fontFamily(L"Times New Roman");
	Font font(&fontFamily, 22, FontStyleRegular, UnitPixel);
	SolidBrush solidBrush(Color(255, 0, 0, 0));

	ReleaseDC(hWnd, hdc);
	POINT coord;
	static HDC memBit_menu;
	static HBITMAP hBmp, hBmp2, hBmp3;
	switch (msg)
	{
	case WM_CREATE: //Повідомлення приходить при створенні вікна
	{
		hdc = GetDC(hWnd);
		hBmp = (HBITMAP)LoadImage(NULL, L"Фон.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		hBmp2 = (HBITMAP)LoadImage(NULL, L"Выбор.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		hBmp3 = (HBITMAP)LoadImage(NULL, L"Поле.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hBmp, sizeof(BM), &BM);
		memBit_menu = CreateCompatibleDC(hdc);
		SelectObject(memBit_menu, hBmp);
		//Создание меню кнопок
		
			GetClientRect(hWnd, &rt); //Область для окна рисования
			MyButtons[0].id = 0;
			MyButtons[0].hButton = CreateWindow(L"button",
				L"Нова гра",
				WS_VISIBLE | WS_CHILD,
				100,
				100,
				200,
				50,
				hWnd,
				(HMENU)MyButtons[0].id,
				hInst,
				NULL);
			MyButtons[1].id = 1;
			MyButtons[1].hButton = CreateWindow(L"button",
				L"Попередні результати",
				WS_VISIBLE | WS_CHILD,
				100,
				200,
				200,
				50,
				hWnd,
				(HMENU)MyButtons[1].id,
				hInst,
				NULL);
			MyButtons[2].id = 2;
			MyButtons[2].hButton = CreateWindow(L"button",
				L"Вийти з гри",
				WS_VISIBLE | WS_CHILD,
				100,
				300,
				200,
				50,
				hWnd,
				(HMENU)MyButtons[2].id,
				hInst,
				NULL);
			MyButtons[3].id = 3;
			MyButtons[3].hButton = CreateWindow(L"button",
				L"Назад",
				WS_CHILD | BS_PUSHBUTTON,
				rt.right - 125, 
				rt.bottom - 50, 
				100, 
				20,
				hWnd,
				(HMENU)MyButtons[3].id,
				hInst,
				NULL);
			MyButtons[4].id = 4;
			MyButtons[4].hButton = CreateWindow(L"button",
				L"Хрестики",
				WS_CHILD,
				350,
				250,
				220,
				50,
				hWnd,
				(HMENU)MyButtons[4].id,
				hInst,
				NULL);
			MyButtons[5].id = 5;
			MyButtons[5].hButton = CreateWindow(L"button",
				L"Нулики",
				WS_CHILD,
				650,
				250,
				220,
				50,
				hWnd,
				(HMENU)MyButtons[5].id,
				hInst,
				NULL);
			MyButtons[6].id = 6;
			MyButtons[6].hButton = CreateWindow(L"listbox",
				L"Назад",
				WS_CHILD | LBS_STANDARD | LBS_HASSTRINGS,
				305,
				45,
				615,
				555,
				hWnd,
				(HMENU)MyButtons[6].id,
				hInst,
				NULL);
			MyButtons[7].id = 7;
			MyButtons[7].hButton = CreateWindow(L"button",
				L"Назад",
				WS_CHILD | BS_PUSHBUTTON,
				100,
				400,
				150,
				50,
				hWnd,
				(HMENU)MyButtons[7].id,
				hInst,
				NULL);
		
		// заполняет 0 крестики и нолики
		for (int i = 0; i < 540; i++)
		{
			place[i][4] = 0;
			place[i][5] = 0;
		}		
		Map_Null();
		break;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == MyButtons[0].id // При нажатии на эту кнопку начинает игру
			&& HIWORD(wParam) == BN_CLICKED)
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Dialog1); // Запрос имени
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, Dialog2); // Запрос имени
			ShowWindow(MyButtons[0].hButton, SW_HIDE); // Скрываем кнопку Начало игры
			ShowWindow(MyButtons[1].hButton, SW_HIDE); // Скрываем кнопку Резульатов
			ShowWindow(MyButtons[2].hButton, SW_HIDE); // Скрываем кнопку Выйти из игры
			ShowWindow(MyButtons[4].hButton, SW_NORMAL); // Делает видимым кнопку Крестик
			ShowWindow(MyButtons[5].hButton, SW_NORMAL); // Делает видимым кнопку Нолик
			SelectObject(memBit_menu, hBmp2);// Замена обьектов
			hdc = GetDC(hWnd);
			InvalidateRect(hWnd, NULL, TRUE); // обновление клиенской области
			ReleaseDC(hWnd, hdc);
		}
		if (LOWORD(wParam) == MyButtons[1].id // При нажатии на эту кнопку записывает результаты
			&& HIWORD(wParam) == BN_CLICKED)
		{

			ShowWindow(MyButtons[0].hButton, SW_HIDE); // Скрываем кнопку Начало игры
			ShowWindow(MyButtons[1].hButton, SW_HIDE); // Скрываем кнопку Резульатов
			ShowWindow(MyButtons[2].hButton, SW_HIDE); // Скрываем кнопку Выйти из игры
			ShowWindow(MyButtons[7].hButton, SW_NORMAL); // Делает видимым кнопку Назад (из результатов)
			ShowWindow(MyButtons[6].hButton, SW_NORMAL); // Делает видимым список 
			count = SendMessage(MyButtons[6].hButton, LB_GETCOUNT, 0, 0);
			for (int i = 0; i < count; i++)
			{
				SendMessage(MyButtons[6].hButton, LB_DELETESTRING, 0, 0);
			}
			//Запись в таблицу результатов данных
			ifstream fin("TabelWinners.txt", fstream::app | fstream::out); // Открываем файл
			if (!fin.is_open())
				MessageBox(hWnd, L"Файл результатів не відкрився!", L"Error", MB_OK); // сообщить о том, что файл не открыт
			else
			{
				//Считываем данные до тех пор пока не дойдем до конца файла
				while (!fin.eof())
				{
					char buf[40];
					fin.getline(buf, 40);
					if (fin.eof())
						break;
					//Добавляем новую строку в список
					SendMessageA(MyButtons[6].hButton, LB_ADDSTRING, 0, (LPARAM)buf);
				}
				Name.clear(); //очистка поля имени
				Name2.clear(); //очистка поля имени
			}
			
		}
		if (LOWORD(wParam) == MyButtons[2].id // При нажатии на эту кнопку выходит с игры
			&& HIWORD(wParam) == BN_CLICKED)
		{
			PostQuitMessage(0); // Завершает работу программы
		}
		if (LOWORD(wParam) == MyButtons[3].id // При нажатии на эту кнопку возвращается назад
			&& HIWORD(wParam) == BN_CLICKED)
		{
			ShowWindow(MyButtons[0].hButton, SW_NORMAL); // Делает видимым кнопку Начало игры 
			ShowWindow(MyButtons[1].hButton, SW_NORMAL);// Делает видимым кнопку Результатов
			ShowWindow(MyButtons[2].hButton, SW_NORMAL);// Делает видимым кнопку Выйти из игры 
			ShowWindow(MyButtons[3].hButton, SW_HIDE);// Скрываем кнопку Назад (из поля)
			SelectObject(memBit_menu, hBmp);
			flagStep = 2;
			// придаем значение 0 крестику и нолику
			for (int i = 0; i < 540; i++)
			{
				place[i][4] = 0; 
				place[i][5] = 0;
			}
			winner = 0;
			Map_Null();
			game_stage = 0;
			
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (LOWORD(wParam) == MyButtons[4].id // При нажатии на эту кнопку меняется на крестик
 			&& HIWORD(wParam) == BN_CLICKED)
		{
			flagStep = 1;
			hdc = GetDC(hWnd);
			ShowWindow(MyButtons[4].hButton, SW_HIDE); // Скрываем кнопку Крестик
			ShowWindow(MyButtons[5].hButton, SW_HIDE); // Скрываем кнопку Нолик
			ShowWindow(MyButtons[3].hButton, SW_NORMAL); // Делает видимым кнопку Назад (из поля)
			SelectObject(memBit_menu, hBmp3);
			FlagInfo = 1;
			game_stage = 2;
			InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);

		}
		if (LOWORD(wParam) == MyButtons[5].id // При нажатии на эту кнопку меняется на нолик
			&& HIWORD(wParam) == BN_CLICKED)
		{
			flagStep = 0;
			hdc = GetDC(hWnd);
			ShowWindow(MyButtons[4].hButton, SW_HIDE); // Скрываем кнопку Крестик
			ShowWindow(MyButtons[5].hButton, SW_HIDE); // Скрываем кнопку Нолик
			ShowWindow(MyButtons[3].hButton, SW_NORMAL); // Делает видимым кнопку Назад (из поля)
			SelectObject(memBit_menu, hBmp3);
			FlagInfo = 2;
			game_stage = 2;
			InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);

		}
		if (LOWORD(wParam) == MyButtons[7].id // При нажатии на эту кнопку возвращается назад в таблицу результатов
			&& HIWORD(wParam) == BN_CLICKED)
		{
			ShowWindow(MyButtons[0].hButton, SW_NORMAL); // Делает видимым кнопку Начало игры 
			ShowWindow(MyButtons[1].hButton, SW_NORMAL); // Делает видимым кнопку Результатов
			ShowWindow(MyButtons[2].hButton, SW_NORMAL); // Делает видимым кнопку Выйти из игры 
			ShowWindow(MyButtons[3].hButton, SW_HIDE); // Скрываем кнопку Назад (из поля)
			ShowWindow(MyButtons[6].hButton, SW_HIDE); // Делает видимым список 
			ShowWindow(MyButtons[7].hButton, SW_HIDE); // Делает видимым кнопку Назад (из результатов)	
			flagStep = 0;
		}
		break;
	}
	case WM_SIZE:
	{
		if (game_stage == 1)
		{
			RECT rt;
			GetClientRect(hWnd, &rt);
			SetWindowPos(MyButtons[3].hButton, NULL, rt.right - 125, rt.bottom - 50, 100, 20, NULL); // Изменения размеров дочернего окна кнопки Назад (из поля)
			SetWindowPos(MyButtons[4].hButton, NULL, rt.right / 2 - 220, rt.bottom / 2 - 34, 150, 43, NULL); // Изменения размеров дочернего окна Крестик
			SetWindowPos(MyButtons[5].hButton, NULL, rt.right / 2 + 100, rt.bottom / 2 - 34, 150, 43, NULL); // Изменения размеров дочернего окна Нолик
			break;
		}
	}
	case WM_LBUTTONDOWN:
	{
		if (winner == 0)
		{
			GetClientRect(hWnd, &rt); //Область вікна для малювання
			HRGN hRegion = CreateRectRgn(rt.right - 150, 5, rt.right, 40);
			GetCursorPos(&coord);
			ScreenToClient(hWnd, &coord);
			int i;
			//Проверяем квадрат попапили мы в эту область
			for (i = 0; i < 539; i++)
			{
				if (coord.x > place[i][0] && coord.x < place[i][2]
					&& coord.y >place[i][1] && coord.y < place[i][3])
					break;
				else continue;
				break;
			}
			hdc = GetDC(hWnd);
			// если попал в регион, то рисует 
			if (PtInRegion(CreateEllipticRgn(place[i][0], place[i][1], place[i][2], place[i][3]), coord.x, coord.y))
			{
				// если у нас flagStep = 0  и поле = 0 рисует нолик
				if (flagStep == 0 && place[i][4] == 0)
				{
					graphics.DrawEllipse(&redPen1, place[i][0] + 3, place[i][1] + 2, place[i][2] - place[i][0] - 11, place[i][3] - place[i][1] - 5);
					InvalidateRgn(hWnd, hRegion, TRUE);
					FlagInfo = 1;
					place[i][4] = 1;
					place[i][5] = 1;
					flagStep = 1; // устанавливаем значение флага для крестика
					// перенос из одномерного в двумерный массив
					int tmp_i = i / 30; 
					int tmp_j = i - (30 * tmp_i);
					map[tmp_i][tmp_j] = place[i][4] + 1;
					step = 2;  // переменная, которая рисует 2 - как нолик
				}
				// если у нас flagStep = 1  и поле = 0 рисует крестик
				if (flagStep == 1 && place[i][4] == 0)
				{
					graphics.DrawLine(&bluePen, place[i][0], place[i][1], place[i][2] - 5, place[i][3] - 2); // координаты верхней левой точки и правой нижней точки
					graphics.DrawLine(&bluePen, place[i][0], place[i][3] - 2, place[i][2] - 5, place[i][1]); // координаты верхней правой точки и левой нижней точки
					InvalidateRgn(hWnd, hRegion, TRUE);
					flagStep = 0; // устанавливаем значение флага для нолика
					FlagInfo = 2;
					place[i][4] = 1;
					place[i][5] = 2;
					// перенос из одномерного в двумерный массив
					int tmp_i = i / 30;
					int tmp_j = i - (30 * tmp_i);
					map[tmp_i][tmp_j] = place[i][4];
					step = 1; // переменная, которая рисует 1 - как крестик
				}
				checkWin();// функция победы
				if (winner == 1 || winner == 2)
				{
					//победитель
					MessageBox(hWnd, (winner == 1) ?
						L"Перемогли ХРЕСТИКИ" :
						L"Перемогли НУЛИКИ",
						L"Гра завершилась",
						MB_OK | MB_ICONINFORMATION);
					ofstream fout("TabelWinners.txt", fstream::app | fstream::out); //открываем 
					if (!fout.is_open())
						MessageBox(hWnd, L"Файл результатів не відкрився!", L"Error", MB_OK);
					else
					{
						if (winner == 1)
						{
							fout << "Ім'я " << Name << " Переміг гравши за ХРЕСТИКІВ" << "\n";

						}
						if (winner == 2)
						{
							fout << "Ім'я " << Name2 << " Переміг гравши за НУЛИКІВ" << "\n";

						}
						fout.close(); // закрытие файла
						Name.clear(); // очистка поля имени
						Name2.clear(); // очистка поля имени
					}
				}
			}
		}
	}
	break;
	case WM_PAINT: //Перемалювати вікно
	{
		hdc = BeginPaint(hWnd, &ps); //Почати графічний вивід			
		GetClientRect(hWnd, &rt); //Область вікна для малювання
		PointF point(rt.right - 130, 20);
		BitBlt(hdc, 0, 0, BM.bmWidth, BM.bmHeight, memBit_menu, 0, 0, SRCCOPY);
		if ((game_stage == 1 && FlagInfo == 0) || game_stage == 2)
		{
			draw = 0;
			vertical = 4;
			game_stage = 1;
			// рисую поле игры
			for (int x = 0; x < colsY; x++)
			{
				height = 10;
				for (int y = 0; y < rowsX; ++y)
				{
					place[draw][0] = rt.left / rowsX + height;
					place[draw][1] = rt.top / colsY + vertical;
					place[draw][2] = rt.right / rowsX + height;
					place[draw][3] = rt.bottom / colsY + vertical;
					draw++;
					height += rt.right / rowsX - 4.5;
				}
				vertical += rt.bottom / colsY;
			}
			DrawRect(hdc, hWnd);
			DrawSymbols(hdc, hWnd);
		}
		if (FlagInfo == 1)
		{
			graphics.DrawString(L"ХРЕСТИКИ", -1, &font, point, &solidBrush);
			FlagInfo = 0;

		}
		if (FlagInfo == 2)
		{
			graphics.DrawString(L"НУЛИКИ", -1, &font, point, &solidBrush);
			FlagInfo = 0;
		}
		EndPaint(hWnd, &ps); //Закінчити графічний вивід
	}
	break;
	case WM_DESTROY: //Завершення роботи
		PostQuitMessage(0);
		break;
	default:
		//Обробка повідомлень, які не оброблені користувачем
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

// рисует поле
void DrawRect(HDC hdc, HWND hWnd)
{

	RECT rt;
	hdc = GetDC(hWnd);
	GetClientRect(hWnd, &rt);

	draw = 0;
	for (int x = 0; x < colsY; x++)
	{
		for (int y = 0; y < rowsX; ++y)
		{	
			Rectangle(hdc, place[draw][0], place[draw][1], place[draw][2], place[draw][3]);
			draw++;
		}
	}
}

//перерисовывает на Крестик или Нолик при изменении экрана
void DrawSymbols(HDC hdc, HWND hWnd) 
{
	Graphics graphics(hdc);
	Pen bluePen(Color(255, 0, 0, 255), 2);
	Pen redPen1(Color(255, 255, 0, 0), 2);
	SolidBrush Solid(Color(255, 255, 255, 255));

	int i;
	for (i = 0; i < 539; i++)
	{
		if (place[i][4] == 1 && place[i][5] == 1)
		{
			graphics.DrawEllipse(&redPen1, place[i][0] + 3, place[i][1] + 2, place[i][2] - place[i][0] - 11, place[i][3] - place[i][1] - 5);
		}
		if (place[i][4] == 1 && place[i][5] == 2)
		{
			graphics.DrawLine(&bluePen, place[i][0], place[i][1], place[i][2] - 5, place[i][3] - 2);
			graphics.DrawLine(&bluePen, place[i][0], place[i][3] - 2, place[i][2] - 5, place[i][1]);
		}
	}
}

INT_PTR CALLBACK Dialog1(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR Tbuffer[20];
	WORD Wbuffer;
	int buff;

	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:// считываем имя пользователя

			Wbuffer = (WORD)SendDlgItemMessage(hWnd,
				IDC_EDIT1,
				EM_LINELENGTH,
				(WPARAM)0,
				(LPARAM)0);
			*((LPWORD)Tbuffer) = Wbuffer;
			buff = Wbuffer;
			SendDlgItemMessage(hWnd,
				IDC_EDIT1,
				EM_GETLINE,
				(WPARAM)0,       // line 0 
				(LPARAM)Tbuffer);
				for (int i = 0; i < buff; i++)
				{
					Name += Tbuffer[i];
				}

			EndDialog(hWnd, 0);
		}
	}
	return 0;
}

INT_PTR CALLBACK Dialog2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR Tbuffer[20];
	WORD Wbuffer;
	int buff;

	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:// считываем имя пользователя

			Wbuffer = (WORD)SendDlgItemMessage(hWnd,
				IDC_EDIT2,
				EM_LINELENGTH,
				(WPARAM)0,
				(LPARAM)0);
			*((LPWORD)Tbuffer) = Wbuffer;
			buff = Wbuffer;
			SendDlgItemMessage(hWnd,
				IDC_EDIT2,
				EM_GETLINE,
				(WPARAM)0,       // line 0 
				(LPARAM)Tbuffer);
				for (int i = 0; i < buff; i++)
				{
					Name2 += Tbuffer[i];
				}
			EndDialog(hWnd, 0);
		}
	}
	return 0;
}
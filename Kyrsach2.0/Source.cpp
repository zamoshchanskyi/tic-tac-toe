#include <windows.h>
#include <GdiPlus.h>
#include <fstream>
#include <string>
#include "resource.h"

//�������� ����
HINSTANCE hInst;
LPCTSTR szWindowClass = L"QWERTY";
LPCTSTR szTitle = L"��������-������";

using namespace Gdiplus;
#pragma comment (lib, "Gdiplus.lib")
using namespace std;

int fiveWin = 5; // ��� ���� ������ ��������� ��� �������
int draw = 0; //��������� �����
int vertical = 4;
int height = 0; //������
int width = 0;	//������
int rowsX = 30; //����
int colsY = 18; //�������
const int X = 18;
const int Y = 30;
int place[540][6];
int map[X][Y]; // ������ ��� ���������� �����
int flagStep = 0;
int winner = 0; // ���� ������������ ����������
static int step = 0; // ���������� ������� ��� �����
HBITMAP hBitmap_menu;
static BITMAP BM;
string Name, Name2;
bool player = 0;
int game_stage = 0;
int FlagInfo = 0;

//��������� ���� �������
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

//������� �������
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

//��������� ����� ����
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcx;
	wcx.cbSize = sizeof(WNDCLASSEX);

	wcx.style = CS_HREDRAW | CS_VREDRAW; //����� ����
	wcx.lpfnWndProc = (WNDPROC)WndProc; //������ ���������
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance; //���������� ��������
	wcx.hIcon = LoadIcon(NULL, IDI_QUESTION); //���������� ������
	wcx.hCursor = LoadCursor(NULL, IDC_SIZE); //���������� �������
	wcx.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH); //��������� ����
	wcx.lpszMenuName = NULL; //���������� ����
	wcx.lpszClassName = szWindowClass; //��'� �����
	wcx.hIconSm = NULL;

	return RegisterClassEx(&wcx); //�������� ����� ����
}

//������� ���� ��������
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hInst = hInstance;
	hWnd = CreateWindow(szWindowClass, //��'� ����� ����
		szTitle, //����� ��������
		WS_OVERLAPPEDWINDOW, //����� ����
		0, //��������� �� X
		0, //��������� �� Y
		1280, //����� �� X
		750, //����� �� Y
		NULL, //���������� ������������ ����
		NULL, // ���������� ���� ����
		hInstance, //���������� ��������
		NULL); //��������� ���������

	if (!hWnd)
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);  //�������� ����
	UpdateWindow(hWnd); //������� ����

	return TRUE;
}

//���������� ���� ������
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

/* ��������� ��������� */
bool checkDiagonal(int offsetX, int offsetY, int choice)
{
	bool toright; // ������ ����� ���� �������
	bool toleft; // ������ ������ ���� ������.
	toright = true;
	toleft = true;
	int counter_right = 0, counter_left = 0; // �������� ����������
	int j = offsetX;
	for (int i = offsetY; i < fiveWin + offsetY; i++, j++)
	{
		// ���� 1 & 1 = 1 � 1 & 0 = 0 
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

/* ��������� �������������� � ������������ ����� */
bool checkLanes(int offsetX, int offsetY, int choice)
{
	bool cols;// ������������ �����
	bool rows;// �������������� �����
	int counter_row = 0, counter_col = 0; // �������
	for (int row = offsetX; row < fiveWin + offsetX; row++)
	{
		cols = true;
		rows = true;
		for (int col = offsetY; col < fiveWin + offsetY; col++)
		{
			// ���� 1 & 1 = 1 � 1 & 0 = 0 
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

/* ������ ������ ��� ������ � ����� �������� ����. */
bool checkWin()
{
	for (int row = 0; row < 26; row++)
	{
		for (int col = 0; col < 26; col++)
		{
			// �������� ������ �������� � ���� ���� ���� ���� ��������,
			// �� �����, ������� ����������� ��� ������, �������
			// ����� ���������� ��� ������� ��������
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
	// ��� ����������� � �������� ���������. 5-� �������������������
	// ������ �� ��������. ������ ��� �� ������.
	return false;
}

//�������, �� �������� �� �����������
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
	case WM_CREATE: //����������� ��������� ��� �������� ����
	{
		hdc = GetDC(hWnd);
		hBmp = (HBITMAP)LoadImage(NULL, L"���.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		hBmp2 = (HBITMAP)LoadImage(NULL, L"�����.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		hBmp3 = (HBITMAP)LoadImage(NULL, L"����.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hBmp, sizeof(BM), &BM);
		memBit_menu = CreateCompatibleDC(hdc);
		SelectObject(memBit_menu, hBmp);
		//�������� ���� ������
		
			GetClientRect(hWnd, &rt); //������� ��� ���� ���������
			MyButtons[0].id = 0;
			MyButtons[0].hButton = CreateWindow(L"button",
				L"���� ���",
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
				L"�������� ����������",
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
				L"����� � ���",
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
				L"�����",
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
				L"��������",
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
				L"������",
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
				L"�����",
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
				L"�����",
				WS_CHILD | BS_PUSHBUTTON,
				100,
				400,
				150,
				50,
				hWnd,
				(HMENU)MyButtons[7].id,
				hInst,
				NULL);
		
		// ��������� 0 �������� � ������
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
		if (LOWORD(wParam) == MyButtons[0].id // ��� ������� �� ��� ������ �������� ����
			&& HIWORD(wParam) == BN_CLICKED)
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Dialog1); // ������ �����
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, Dialog2); // ������ �����
			ShowWindow(MyButtons[0].hButton, SW_HIDE); // �������� ������ ������ ����
			ShowWindow(MyButtons[1].hButton, SW_HIDE); // �������� ������ ����������
			ShowWindow(MyButtons[2].hButton, SW_HIDE); // �������� ������ ����� �� ����
			ShowWindow(MyButtons[4].hButton, SW_NORMAL); // ������ ������� ������ �������
			ShowWindow(MyButtons[5].hButton, SW_NORMAL); // ������ ������� ������ �����
			SelectObject(memBit_menu, hBmp2);// ������ ��������
			hdc = GetDC(hWnd);
			InvalidateRect(hWnd, NULL, TRUE); // ���������� ��������� �������
			ReleaseDC(hWnd, hdc);
		}
		if (LOWORD(wParam) == MyButtons[1].id // ��� ������� �� ��� ������ ���������� ����������
			&& HIWORD(wParam) == BN_CLICKED)
		{

			ShowWindow(MyButtons[0].hButton, SW_HIDE); // �������� ������ ������ ����
			ShowWindow(MyButtons[1].hButton, SW_HIDE); // �������� ������ ����������
			ShowWindow(MyButtons[2].hButton, SW_HIDE); // �������� ������ ����� �� ����
			ShowWindow(MyButtons[7].hButton, SW_NORMAL); // ������ ������� ������ ����� (�� �����������)
			ShowWindow(MyButtons[6].hButton, SW_NORMAL); // ������ ������� ������ 
			count = SendMessage(MyButtons[6].hButton, LB_GETCOUNT, 0, 0);
			for (int i = 0; i < count; i++)
			{
				SendMessage(MyButtons[6].hButton, LB_DELETESTRING, 0, 0);
			}
			//������ � ������� ����������� ������
			ifstream fin("TabelWinners.txt", fstream::app | fstream::out); // ��������� ����
			if (!fin.is_open())
				MessageBox(hWnd, L"���� ���������� �� ��������!", L"Error", MB_OK); // �������� � ���, ��� ���� �� ������
			else
			{
				//��������� ������ �� ��� ��� ���� �� ������ �� ����� �����
				while (!fin.eof())
				{
					char buf[40];
					fin.getline(buf, 40);
					if (fin.eof())
						break;
					//��������� ����� ������ � ������
					SendMessageA(MyButtons[6].hButton, LB_ADDSTRING, 0, (LPARAM)buf);
				}
				Name.clear(); //������� ���� �����
				Name2.clear(); //������� ���� �����
			}
			
		}
		if (LOWORD(wParam) == MyButtons[2].id // ��� ������� �� ��� ������ ������� � ����
			&& HIWORD(wParam) == BN_CLICKED)
		{
			PostQuitMessage(0); // ��������� ������ ���������
		}
		if (LOWORD(wParam) == MyButtons[3].id // ��� ������� �� ��� ������ ������������ �����
			&& HIWORD(wParam) == BN_CLICKED)
		{
			ShowWindow(MyButtons[0].hButton, SW_NORMAL); // ������ ������� ������ ������ ���� 
			ShowWindow(MyButtons[1].hButton, SW_NORMAL);// ������ ������� ������ �����������
			ShowWindow(MyButtons[2].hButton, SW_NORMAL);// ������ ������� ������ ����� �� ���� 
			ShowWindow(MyButtons[3].hButton, SW_HIDE);// �������� ������ ����� (�� ����)
			SelectObject(memBit_menu, hBmp);
			flagStep = 2;
			// ������� �������� 0 �������� � ������
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
		if (LOWORD(wParam) == MyButtons[4].id // ��� ������� �� ��� ������ �������� �� �������
 			&& HIWORD(wParam) == BN_CLICKED)
		{
			flagStep = 1;
			hdc = GetDC(hWnd);
			ShowWindow(MyButtons[4].hButton, SW_HIDE); // �������� ������ �������
			ShowWindow(MyButtons[5].hButton, SW_HIDE); // �������� ������ �����
			ShowWindow(MyButtons[3].hButton, SW_NORMAL); // ������ ������� ������ ����� (�� ����)
			SelectObject(memBit_menu, hBmp3);
			FlagInfo = 1;
			game_stage = 2;
			InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);

		}
		if (LOWORD(wParam) == MyButtons[5].id // ��� ������� �� ��� ������ �������� �� �����
			&& HIWORD(wParam) == BN_CLICKED)
		{
			flagStep = 0;
			hdc = GetDC(hWnd);
			ShowWindow(MyButtons[4].hButton, SW_HIDE); // �������� ������ �������
			ShowWindow(MyButtons[5].hButton, SW_HIDE); // �������� ������ �����
			ShowWindow(MyButtons[3].hButton, SW_NORMAL); // ������ ������� ������ ����� (�� ����)
			SelectObject(memBit_menu, hBmp3);
			FlagInfo = 2;
			game_stage = 2;
			InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);

		}
		if (LOWORD(wParam) == MyButtons[7].id // ��� ������� �� ��� ������ ������������ ����� � ������� �����������
			&& HIWORD(wParam) == BN_CLICKED)
		{
			ShowWindow(MyButtons[0].hButton, SW_NORMAL); // ������ ������� ������ ������ ���� 
			ShowWindow(MyButtons[1].hButton, SW_NORMAL); // ������ ������� ������ �����������
			ShowWindow(MyButtons[2].hButton, SW_NORMAL); // ������ ������� ������ ����� �� ���� 
			ShowWindow(MyButtons[3].hButton, SW_HIDE); // �������� ������ ����� (�� ����)
			ShowWindow(MyButtons[6].hButton, SW_HIDE); // ������ ������� ������ 
			ShowWindow(MyButtons[7].hButton, SW_HIDE); // ������ ������� ������ ����� (�� �����������)	
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
			SetWindowPos(MyButtons[3].hButton, NULL, rt.right - 125, rt.bottom - 50, 100, 20, NULL); // ��������� �������� ��������� ���� ������ ����� (�� ����)
			SetWindowPos(MyButtons[4].hButton, NULL, rt.right / 2 - 220, rt.bottom / 2 - 34, 150, 43, NULL); // ��������� �������� ��������� ���� �������
			SetWindowPos(MyButtons[5].hButton, NULL, rt.right / 2 + 100, rt.bottom / 2 - 34, 150, 43, NULL); // ��������� �������� ��������� ���� �����
			break;
		}
	}
	case WM_LBUTTONDOWN:
	{
		if (winner == 0)
		{
			GetClientRect(hWnd, &rt); //������� ���� ��� ���������
			HRGN hRegion = CreateRectRgn(rt.right - 150, 5, rt.right, 40);
			GetCursorPos(&coord);
			ScreenToClient(hWnd, &coord);
			int i;
			//��������� ������� �������� �� � ��� �������
			for (i = 0; i < 539; i++)
			{
				if (coord.x > place[i][0] && coord.x < place[i][2]
					&& coord.y >place[i][1] && coord.y < place[i][3])
					break;
				else continue;
				break;
			}
			hdc = GetDC(hWnd);
			// ���� ����� � ������, �� ������ 
			if (PtInRegion(CreateEllipticRgn(place[i][0], place[i][1], place[i][2], place[i][3]), coord.x, coord.y))
			{
				// ���� � ��� flagStep = 0  � ���� = 0 ������ �����
				if (flagStep == 0 && place[i][4] == 0)
				{
					graphics.DrawEllipse(&redPen1, place[i][0] + 3, place[i][1] + 2, place[i][2] - place[i][0] - 11, place[i][3] - place[i][1] - 5);
					InvalidateRgn(hWnd, hRegion, TRUE);
					FlagInfo = 1;
					place[i][4] = 1;
					place[i][5] = 1;
					flagStep = 1; // ������������� �������� ����� ��� ��������
					// ������� �� ����������� � ��������� ������
					int tmp_i = i / 30; 
					int tmp_j = i - (30 * tmp_i);
					map[tmp_i][tmp_j] = place[i][4] + 1;
					step = 2;  // ����������, ������� ������ 2 - ��� �����
				}
				// ���� � ��� flagStep = 1  � ���� = 0 ������ �������
				if (flagStep == 1 && place[i][4] == 0)
				{
					graphics.DrawLine(&bluePen, place[i][0], place[i][1], place[i][2] - 5, place[i][3] - 2); // ���������� ������� ����� ����� � ������ ������ �����
					graphics.DrawLine(&bluePen, place[i][0], place[i][3] - 2, place[i][2] - 5, place[i][1]); // ���������� ������� ������ ����� � ����� ������ �����
					InvalidateRgn(hWnd, hRegion, TRUE);
					flagStep = 0; // ������������� �������� ����� ��� ������
					FlagInfo = 2;
					place[i][4] = 1;
					place[i][5] = 2;
					// ������� �� ����������� � ��������� ������
					int tmp_i = i / 30;
					int tmp_j = i - (30 * tmp_i);
					map[tmp_i][tmp_j] = place[i][4];
					step = 1; // ����������, ������� ������ 1 - ��� �������
				}
				checkWin();// ������� ������
				if (winner == 1 || winner == 2)
				{
					//����������
					MessageBox(hWnd, (winner == 1) ?
						L"��������� ��������" :
						L"��������� ������",
						L"��� �����������",
						MB_OK | MB_ICONINFORMATION);
					ofstream fout("TabelWinners.txt", fstream::app | fstream::out); //��������� 
					if (!fout.is_open())
						MessageBox(hWnd, L"���� ���������� �� ��������!", L"Error", MB_OK);
					else
					{
						if (winner == 1)
						{
							fout << "��'� " << Name << " ������ ������ �� ������ʲ�" << "\n";

						}
						if (winner == 2)
						{
							fout << "��'� " << Name2 << " ������ ������ �� ����ʲ�" << "\n";

						}
						fout.close(); // �������� �����
						Name.clear(); // ������� ���� �����
						Name2.clear(); // ������� ���� �����
					}
				}
			}
		}
	}
	break;
	case WM_PAINT: //������������ ����
	{
		hdc = BeginPaint(hWnd, &ps); //������ ��������� ����			
		GetClientRect(hWnd, &rt); //������� ���� ��� ���������
		PointF point(rt.right - 130, 20);
		BitBlt(hdc, 0, 0, BM.bmWidth, BM.bmHeight, memBit_menu, 0, 0, SRCCOPY);
		if ((game_stage == 1 && FlagInfo == 0) || game_stage == 2)
		{
			draw = 0;
			vertical = 4;
			game_stage = 1;
			// ����� ���� ����
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
			graphics.DrawString(L"��������", -1, &font, point, &solidBrush);
			FlagInfo = 0;

		}
		if (FlagInfo == 2)
		{
			graphics.DrawString(L"������", -1, &font, point, &solidBrush);
			FlagInfo = 0;
		}
		EndPaint(hWnd, &ps); //�������� ��������� ����
	}
	break;
	case WM_DESTROY: //���������� ������
		PostQuitMessage(0);
		break;
	default:
		//������� ����������, �� �� �������� ������������
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

// ������ ����
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

//�������������� �� ������� ��� ����� ��� ��������� ������
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
		case IDOK:// ��������� ��� ������������

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
		case IDOK:// ��������� ��� ������������

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
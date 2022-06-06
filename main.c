#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <MMsystem.h>
#include "robot.h"
#include "weapon.h"
#include "item.h"
#pragma comment(lib,"winmm.lib") 

#define GBOARD_ORIGIN_X 5
#define GBOARD_ORIGIN_Y 3
#define RIGHT 77
#define LEFT 75
#define DOWN 80
#define UP 72
#define BACKSPACE 8
#define ESC 27
#define SPACE 32

typedef struct weapon {
	int x, y; // ��ġ�� ��ġ
	int model; // �� ��ȣ(0~4)
	int durability; // ������
	int bullettime; // �߻� ����
} WEAPON;

typedef struct bullet {
	int x, y; // ���� ��ġ
	int model; // �� ��ȣ(0~4)
} BULLET;

typedef struct robot {
	int x, y; // ���� ��ġ
	int model; // �� ��ȣ(0~4)
	int hp; // ü��
	int attack; // ���ݷ�
} ROBOT;

typedef struct item {
	int x, y; // ��ġ�� ��ġ
	int gold; // ���
	int assist; // ��ý�Ʈ
	int time; // ������ ���� �ð�
} ITEM;

// ��������
int gameBoardInfo[21][46] = { 0, }; // ���Ӻ��������迭
WEAPON* w[200]; // ���� ����ü ������ �迭
BULLET* b[1000]; // �Ѿ� ����ü ������ �迭
ROBOT* r[200]; // �κ� ����ü ������ �迭
ITEM* it[200]; // ������ ����ü ������ �迭
int t = 0; // �ð�(����Ŭ Ƚ��)
COORD indicatorPos = { GBOARD_ORIGIN_X + 4, GBOARD_ORIGIN_Y }; // �ε��������� ��ġ     
int hcnt = 1, vcnt = 1; // �ε��������� ����, ���� ��ġ�� ��Ÿ���� ��ǥ
int stage = 1; // ��������(1 - NORMAL, 2 - HARD, 3- HELL)
int speed = 30; // ���� �ӵ�
int gold = 300, assist[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // ���, ��ý�Ʈ ����
int nr = 0, nw = 0, nb = 0, ni = 0; // �κ�, ����, �Ѿ�, ������ ��
int robot = 10; // ���� �κ� ��

// �Լ� ���� ����
void init();
void titleDraw();
int menuDraw();
void helpPageDraw();
void endDraw();
void gameOverDraw();
void gameClearDraw();

void textcolor(int color_number);
void RemoveCursor(); // Ŀ�� ���ִ� �Լ�
COORD GetCurrentCursorPos(); // ���� Ŀ�� ��ġ ��ȯ�ϴ� �Լ�
void Gotoxy(int x, int y); // Ŀ�� ��ġ �����ϴ� �Լ�
void DrawBoard(); // �ʱ� ���Ӻ��� ����ϴ� �Լ�

void GenerateRobot(); // �κ� �����ϴ� �Լ�
void PrintRobot(char robotInfo[4][4], int color); // �κ� ����ϴ� �Լ�
void AddRobotToBoard(int model, int curPosX, int curPosY); // ���Ӻ��������迭�� �κ� ���� �߰��ϴ� �Լ�
void CheckRobot(); // ��� �κ��� ���� �浹 �˻��ϴ� �Լ�
void DeleteRobot(int index, char robotInfo[4][4]);
void MoveRobot(ROBOT* r); // �κ� �������� �� ĭ �̵��ϴ� �Լ�
void AttackWeapon(ROBOT* r, int idx); // �κ��� ���� �����ϴ� �Լ�
int DetectCollision(int n, int x, int y); // �� �κ��� ���� �浹 �˻��ϴ� �Լ�

void GenerateWeapon(int n); // ���� �����ϴ� �Լ�
void PrintWeapon(char weaponInfo[4][4], int color); // ���� ����ϴ� �Լ�
void AddWeaponToBoard(int model, int curPosX, int curPosY); // ���Ӻ��������迭�� ���� ���� �߰��ϴ� �Լ�
void CheckWeapon(); // ��� ���⿡ ���� ������ �˻��ϴ� �Լ�
void DeleteWeapon(int index, char weaponInfo[4][4], int f); // ���� �����ϴ� �Լ�
int WhichWeapon(int x, int y); // ĭ�� ��ġ�� ������ �迭 �ε����� ��ȯ�ϴ� �Լ�

void ProcessKeyInput(); // Ű �Է¹޴� �Լ�
int isEmpty(int curPosX, int curPosY); // �� ĭ �˻��ϴ� �Լ�
int isGameOver(); // ���� ���� �Լ�
void StageUp(); // �������� �� �Լ�

void GenerateBullet(); // �Ѿ� �����ϴ� �Լ�
void MoveBullet(); // �Ѿ� �����̴� �Լ�
void DeleteBullet(int idx); // �Ѿ� ���� �Լ�
void CheckBullet();// �Ѿ� ������ ���� �浹�ϴ��� Ȯ���ϴ� �Լ�

void GenerateItem(); // ������ �����ϴ� �Լ�
void PrintItem(); // ������ ����ϴ� �Լ�
void AddItemToBoard(int curPosX, int curPosY); // ���Ӻ��������迭�� ������ ���� �߰��ϴ� �Լ�
void CheckItem(); // ��� �����ۿ� ���� �ð� �˻��ϴ� �Լ� 
void DeleteItem(int index, int f); // ������ �����ϴ� �Լ�
int WhichItem(int x, int y); // ĭ�� ��ġ�� �������� �迭 �ε����� ��ȯ�ϴ� �Լ� 

void PrintJet(); // ��Ʈ�� ����ϴ� �Լ�
void DeleteJet(); // ��Ʈ�� �����ϴ� �Լ�
void RedrawBoard(); // ���Ӻ��� �ٽ� ����ϴ� �Լ�

int main()
{
	int i, j;
	int menu;

	PlaySound(TEXT("./sound/start.wav"), NULL, SND_ASYNC);
	init();
	titleDraw();
	menu = menuDraw();

	switch (menu) {
	case 0: //���ӽ��� 
	gameStart:
		PlaySound(NULL, NULL, NULL);
		DrawBoard();
		RemoveCursor();
		while (1)
		{
			if (isGameOver()) { // ���� ���� ������ ���� ����
				PlaySound(TEXT("./sound/gameover.wav"), NULL, SND_ASYNC);
				gameOverDraw();
				Sleep(3000);
				break;
			}

			if (robot <= 0) // �������� �� or ���� Ŭ����
			{
				if (stage == 3)
				{
					PlaySound(TEXT("./sound/gameclear.wav"), NULL, SND_ASYNC);
					gameClearDraw();
					Sleep(5000);
					break;
				}
				StageUp();
			}

			if (t != 0 && t % 10 == 0) // �κ� ���� ����
				GenerateRobot(); // �κ��� ������ ���� ������

			if (t == 3 || t != 0 && t % 12 == 0) // ������ ���� ����
				GenerateItem(); // �������� ������ ��ġ�� ������

			GenerateBullet(); // �Ѿ� ����
			CheckBullet(); // �Ѿ� ������ �浹 üũ
			MoveBullet(); // �Ѿ� �����̱�

			CheckItem(); // ������ �ð� üũ
			CheckWeapon(); // ���� ������ üũ, bullettime--
			CheckRobot(); // �κ� ü��, �浹 üũ

			ProcessKeyInput(); // Ű �Է� �˻�

			RedrawBoard(); // ���� �ٽ� �׸���

			t++;
		}

		break;
	case 1: //���Ӽ���
		system("cls");
		helpPageDraw();

		//hlep page���� space bar ������ ���� ����
		int key;
		while (1) {
			if (_kbhit() != 0) {
				key = _getch();

				if (key == SPACE)
					goto gameStart;
			}
		}
		break;

	case 2: //��������
		system("cls");
		endDraw();
		exit(-1);
	}

	/*	Gotoxy(0, 30);
	for (i = 0; i <= 20; i++)
	{
	for (j = 0; j <= 45; j++)
	{
	if (gameBoardInfo[i][j] < 10)
	{
	printf("%d ", gameBoardInfo[i][j]);
	}
	else
	{
	printf("%d", gameBoardInfo[i][j]);
	}
	}
	printf("\n");
	}*/

	return 0;
}

void init() {
	system("mode con cols=110 lines=30 | title ��0620");
	return;
}

void titleDraw() {

	textcolor(14); //31
	Gotoxy(28, 4);
	printf("                                                    \n");
	Gotoxy(28, 5);
	printf("        #        #                                  \n");
	Gotoxy(28, 6);
	printf("        ###    # #     #####  #####  #####  #####      \n");
	Gotoxy(28, 7);
	printf("      ## ## ### #     #   #  #          #  #   #      \n");
	Gotoxy(28, 8);
	printf("     ##   ##  # #    # # #  #####  #####  # # #      \n");
	Gotoxy(28, 9);
	printf("       ##           #   #  #   #  #      #   #      \n");
	Gotoxy(28, 10);
	printf("        #######    #####  #####  #####  #####      \n");
	Gotoxy(28, 11);
	printf("                                                    \n");

	textcolor(15);
	Gotoxy(30, 13);
	printf("���Ű�� �����̰� space bar�� �޴��� �����ϼ���");

	return;
}

int menuDraw() {
	int x = 25 + 24;
	int y = 4 + 16;
	Gotoxy(x - 2, y);
	printf("> �� �� �� ��");
	Gotoxy(x, y + 1);
	printf("�� �� �� ��");
	Gotoxy(x, y + 2);
	printf("   �� ��   ");

	Gotoxy(x - 2, y);
	while (1) {
		RemoveCursor();
		char key = _getch();

		//if (_kbhit() != 0) {
		if (key) {
			switch (key) {
			case UP:
				Gotoxy(x - 2, y);
				printf(" ");
				y -= 1;
				if (y < 4 + 16)
					y = 4 + 16;
				else;
				Gotoxy(x - 2, y);
				printf(">");
				break;

			case DOWN:
				Gotoxy(x - 2, y);
				printf(" ");
				y += 1;
				if (y > 4 + 18)
					y = 4 + 18;
				else;
				Gotoxy(x - 2, y);
				printf(">");
				break;

			case SPACE:
				return y - (4 + 16);

			}
		}
	}
}

void helpPageDraw() {
	int s = 6;
	int key;

	Gotoxy(35, 10);
	textcolor(12);
	printf("2120�� 6�� 20��...");
	textcolor(15);
	Gotoxy(35, 12);
	printf("�κ� �δ밡 �����뿡 �ĵ�����,");
	Gotoxy(35, 14);
	printf("�б� �ǹ����� �ϳ��� �Զ��ȴ�.");
	Gotoxy(35, 16);
	printf("�л����� ������ ���� �ǹ��� ���AI���ͷ� �ǽ��Ѵ�.");
	Gotoxy(35, 18);
	printf("�κ����� ���͸� ���� ���� �ٰ����� �ִ�...");
	Gotoxy(85, 27);
	textcolor(13);
	printf("press 'SPACE BAR'");
	
	while (1) {
		if (_kbhit() != 0) {
			key = _getch();

			if (key == SPACE)
				break;
		}
	}
	system("cls");

	Gotoxy(10, 4);
	textcolor(12);
	printf("<���� ����>");
	textcolor(15);
	Gotoxy(10, 7);
	printf("- �� ������ ������ ���� ���ݸ��� �κ��� �ϳ��� ��Ÿ����.");
	Gotoxy(12, 8);
	printf("�÷��̾�� ���⸦ ��ġ�ϰ� ��ų�� ����ϸ� �ٰ����� �κ��� �ı��ؾ� �Ѵ�.");
	Gotoxy(12, 9);
	printf("�� �� ���� �κ��̶� �� ���� ���� �ٴٸ��� ���� ����!");
	Gotoxy(10, 11);
	printf("- ���� ���⸦ ������ �� �ְ�, ��ý�Ʈ�� ��ų�� ����� �� �ִ�.");
	Gotoxy(10, 13);
	printf("- 'Target Robot'�� �������� Ŭ��� ���� ���� ���� �κ� ���� ���Ѵ�.");
	Gotoxy(10, 15);
	printf("- ���������� NORMAL, HARD, HELL�� ������.");
	Gotoxy(12, 16);
	printf("���������� ����Ҽ��� �ı��ؾ� �� �κ��� ���� �þ��,");
	Gotoxy(12, 17);
	printf("��� ������ ����� ��ų�� ������ �þ��, ��Ÿ���� �κ��� ������ �þ��.");
	Gotoxy(10, 19);
	printf("- ����Ű�� �ε������͸� �̵��Ͽ� Ű �Է��� ���� ���� ����� �����Ѵ�.");
	Gotoxy(10, 21);
	printf("- ����� �Ѿ��� �� �� �߻��� ������ �������� ���δ�.");
	Gotoxy(10, 23);
	printf("- �������� ���� ȹ������ ������ �������.");
	Gotoxy(85, 27);
	textcolor(13);
	printf("press 'SPACE BAR'");

	while (1) {
		if (_kbhit() != 0) {
			key = _getch();

			if (key == SPACE)
				break;
		}
	}
	system("cls");

	Gotoxy(26, 2);
	textcolor(3);
	printf("Tip. �������� �κ��� ��ó�� ���� ���� ���� ȹ���ϴ� ���� ����");
	Gotoxy(10, 5);
	textcolor(12);
	printf("<�κ� ����>");
	Gotoxy(12, 7);
	PrintRobot(robotModel[0], 15);
	Gotoxy(22, 9);
	printf("<�⺻�� �κ�1>  ���� ���� �κ�");
	Gotoxy(12, 11);
	PrintRobot(robotModel[1], 15);
	Gotoxy(22, 13);
	printf("<�⺻�� �κ�2>  �⺻�� �κ�1���� ü�¸鿡�� ���׷��̵�� �κ�");
	Gotoxy(10, 15);
	PrintRobot(robotModel[2], 15);
	Gotoxy(22, 17);
	printf(" <����� �κ�>  �� Ưȭ�� �κ�");
	Gotoxy(12, 20);
	PrintRobot(robotModel[3], 15);
	Gotoxy(22, 21);
	printf(" <������ �κ�>  ���ݿ� Ưȭ�� �κ�");
	Gotoxy(10, 24);
	PrintRobot(robotModel[4], 15);
	Gotoxy(22, 25);
	printf("   <���� �κ�>  ���°� ���ݷ� �η� ���� �κ�");
	Gotoxy(85, 27);
	textcolor(13);
	printf("press 'SPACE BAR'");

	while (1) {
		if (_kbhit() != 0) {
			key = _getch();

			if (key == SPACE)
				break;
		}
	}
	system("cls");

	Gotoxy(26, 2);
	textcolor(3);
	printf("Tip. ����� �κ��� �ָ� ������ ���� ��ġ�ϴ� ���� ����");
	Gotoxy(10, 5);
	textcolor(12);
	printf("<���� ����>");
	Gotoxy(11, 7);
	PrintWeapon(weaponModel[0], 15);
	Gotoxy(22, 9);
	printf("    <����>  ���� �⺻���� ����");
	Gotoxy(11, 11);
	PrintWeapon(weaponModel[1], 15);
	Gotoxy(22, 13);
	printf("<���ݼ���>  ���Ѻ��� ���ݷ°� �������� ���� ����");
	Gotoxy(11, 15);
	PrintWeapon(weaponModel[2], 15);
	Gotoxy(22, 17);
	printf("<�������>  �� �߿��� ���ݷ°� �������� ���� ���� ����");
	Gotoxy(11, 19);
	PrintWeapon(weaponModel[3], 15);
	Gotoxy(22, 21);
	printf("    <����>  �Ѻ��� ���ݷ°� �������� ������ ����ӵ��� ���� ����");
	Gotoxy(11, 24);
	PrintWeapon(weaponModel[4], 15);
	Gotoxy(22, 25);
	printf("    <��ũ>  ���ݷ°� �������� ���� ���� ����ӵ��� ���� ����");
	Gotoxy(85, 27);
	textcolor(13);
	printf("press 'SPACE BAR'");

	while (1) {
		if (_kbhit() != 0) {
			key = _getch();

			if (key == SPACE)
				break;
		}
	}
	system("cls");

	Gotoxy(35, 2);
	textcolor(12);
	printf("              <����Ű ����>");
	Gotoxy(10, s++);
	textcolor(15);
	printf("�� ���� ��ġ						�� �⺻"); //TAB X 6
	Gotoxy(10, s++);
	printf("\n");
	Gotoxy(10, s++);
	printf("< 1 >	: ����						   ��");
	Gotoxy(10, s++);
	printf("< 2 >	: ���ݼ���					�� �� ��	: �ε������� �̵�");
	Gotoxy(10, s++);
	printf("< 3 >	: �������");
	Gotoxy(10, s++);
	printf("< 4 >	: ����						BACKSPACE	: ���� ����");
	Gotoxy(10, s++);
	printf("< 5 >	: ��ũ						SPACE BAR	: ������ ȹ��");
	Gotoxy(10, s++);
	printf("							ESC		: ���� ����         ");
	Gotoxy(10, s++);
	printf("\n");
	Gotoxy(10, s++);
	printf("\n");
	Gotoxy(10, s++);
	printf("�� ��ý�Ʈ ���");
	Gotoxy(10, s++);
	printf("\n");
	Gotoxy(10, s++);
	printf("< Q >	: ���� - ��Ʈ��					");
	Gotoxy(10, s++);
	printf("	�� �ش� ������ �κ����� ���� �������� ������");
	Gotoxy(10, s++);
	printf("< W >	: ���������� - ����ġ��");
	Gotoxy(10, s++);
	printf("	�� ��ġ�Ǿ� �ִ� ��繫���� �������� ���� ȸ����Ų��");
	Gotoxy(10, s++);
	printf("< E >	: ���а� - ����");
	Gotoxy(10, s++);
	printf("	�� ���忡 �ִ� ��� �κ����� ���� �������� ������");
	Gotoxy(75, 27);
	textcolor(13);
	printf("press 'SPACE BAR' to start game");
	textcolor(15);
}

void endDraw() {
	Gotoxy(25, 5); //���Ⱑ ��ġ�� ���� ������ ���Ƽ� ������ġ�� ���س���! ���ٸ��� y��ǥ�� �÷������ָ��!
	textcolor(11); //���ξ� �ϴ� ���� 11�� ���س��µ� ��ü������ �̻������ص����� ���پ� Ȥ�� �׳� ���ξ� ����� �ٲ㵵 ����!

	Gotoxy(25, 5);
	printf("      ####   #####   #   #   #####  #####  #####  #####      \n");
	Gotoxy(25, 6);
	printf("     #      #       ##  #   #   #  #          #  #   #      \n");
	Gotoxy(25, 7);
	printf("    ####   #####   # # #   # # #  #####  #####  # # #      \n");
	Gotoxy(25, 8);
	printf("       #   #      #  ##   #   #  #   #  #      #   #      \n");
	Gotoxy(25, 9);
	printf("   ####   #####  #   ##  #####  #####  #####  #####      \n");

	Gotoxy(33, 15);
	textcolor(8);
	printf("������ : �����, ���ä, ������, ������\n");
	Gotoxy(33, 16);
	printf("    BGM ��ó : www.bensound.com\n");
	Gotoxy(33, 17);
	printf(" ȿ���� ��ó : http://soundbible.com/\n");

	Gotoxy(70, 25);
	textcolor(7); //����Ϸ���~
	return;
}

void gameOverDraw() {
	system("cls");
	Gotoxy(25, 5);
	textcolor(12);

	printf("        ####    #    #   #  #####    ##### #   # ##### #### 	    \n");
	Gotoxy(25, 6);
	printf("      #       # #  # # # # #        #   # #   # #     #   #     \n");
	Gotoxy(25, 7);
	printf("     #  ### ##### #  #  # #####    #   # #   # ##### ####	    \n");
	Gotoxy(25, 8);
	printf("    #   #  #   # #     # #        #   #  # #  #     #  ##   \n");
	Gotoxy(25, 9);
	printf("    ####  #   # #     # #####    #####   #   ##### #   ###	   \n");

	Gotoxy(70, 25);
	textcolor(7); //����Ϸ���~

	return;
}

void gameClearDraw() {
	system("cls");
	Gotoxy(20, 5);
	textcolor(14);

	printf("        ####    #    #    #  #####     ####  #     #####    #    #### 	    \n");
	Gotoxy(20, 6);
	printf("       #       # #  # # # # #         #     #     #       # #   #   #     \n");
	Gotoxy(20, 7);
	printf("      #  ##  ##### #  #  # #####     #     #     #####  #####  ####	    \n");
	Gotoxy(20, 8);
	printf("     #   #  #   # #     # #         #     #     #      #   #  #  ##   \n");
	Gotoxy(20, 9);
	printf("    ####  #   #  #     # #####     ####  ##### #####  #   #  #   ###    \n");

	Gotoxy(34, 15);
	textcolor(8);
	printf("������ : �����, ���ä, ������, ������\n");
	Gotoxy(34, 16);
	printf("    BGM ��ó : www.bensound.com\n");
	Gotoxy(34, 17);
	printf(" ȿ���� ��ó : http://soundbible.com/\n");

	Gotoxy(70, 25);
	textcolor(8); //����Ϸ���~

	return;
}

void textcolor(int color_number) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_number);
}

void RemoveCursor() // Ŀ�� ���ִ� �Լ�
{
	CONSOLE_CURSOR_INFO curInfo;

	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

COORD GetCurrentCursorPos() // ���� Ŀ�� ��ġ ��ȯ�ϴ� �Լ�
{
	COORD curPoint;
	CONSOLE_SCREEN_BUFFER_INFO curInfo;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curPoint.X = curInfo.dwCursorPosition.X;
	curPoint.Y = curInfo.dwCursorPosition.Y;

	return curPoint;
}

void Gotoxy(int x, int y) // Ŀ�� ��ġ �����ϴ� �Լ�(SetCurrentCursorPos()�� �ʹ� �� Gotoxy�� ��ü��)
{
	COORD pos = { x, y };

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void DrawBoard() // �ʱ� ���� �׸��� �Լ�(stage == 1)
{
	int i, j, tmp;
	int x, y;
	system("cls");
	// ��ý�Ʈ
	Gotoxy(GBOARD_ORIGIN_X + 1, GBOARD_ORIGIN_Y - 1);
	printf("Assist ");
	for (i = 0; i < 15; i++)
	{
		printf("��");
	}

	// ���
	Gotoxy(GBOARD_ORIGIN_X + 45, GBOARD_ORIGIN_Y - 1);
	printf("Gold 300");

	// ��������
	Gotoxy(GBOARD_ORIGIN_X + 79, GBOARD_ORIGIN_Y - 1);
	printf("STAGE NORMAL");

	// ���� �κ� ��
	Gotoxy(GBOARD_ORIGIN_X + 92, GBOARD_ORIGIN_Y + 10);
	printf("Target Robot");
	Gotoxy(GBOARD_ORIGIN_X + 97, GBOARD_ORIGIN_Y + 12);
	printf("%d", robot);

	// ���� ����
	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y);
	printf("��");
	for (i = 1; i < 90; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X + i, GBOARD_ORIGIN_Y);
		printf("��");
	}
	printf("��");

	for (i = 1; i < 20; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + i);
		printf("��");
	}

	for (i = 1; i < 20; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X + 90, GBOARD_ORIGIN_Y + i);
		printf("��");
	}

	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 20);
	printf("��");
	for (i = 1; i < 90; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X + i, GBOARD_ORIGIN_Y + 20);
		printf("��");
	}
	printf("��");

	for (i = 1; i <= 3; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 5 * i);
		printf("��");
		for (j = 1; j < 90; j++)
		{
			Gotoxy(GBOARD_ORIGIN_X + j, GBOARD_ORIGIN_Y + 5 * i);
			if (j % 10 == 0)
				printf("��");
			else
				printf("��");
		}
		printf("��");
	}

	// ���� ����
	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 21);
	printf("��");
	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 21);
	for (i = 1; i < 55; i++) {
		Gotoxy(GBOARD_ORIGIN_X + i, GBOARD_ORIGIN_Y + 21);
		printf("��");
	}
	printf("��");
	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 23);
	for (i = 1; i < 55; i++) {
		Gotoxy(GBOARD_ORIGIN_X + i, GBOARD_ORIGIN_Y + 23);
		printf("��");
	}

	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 22);
	tmp = 0;
	for (j = 0; j < 6; j++) {
		Gotoxy(GBOARD_ORIGIN_X + tmp + 3, GBOARD_ORIGIN_Y + 22);
		if (j == 0)
		{
			printf("50 Gold"); //%d , generateWeapon[j].sellPricegold
			Gotoxy(GBOARD_ORIGIN_X + tmp + 6, GBOARD_ORIGIN_Y + 22 + 2);
			printf("1");
		}
		else if (j == 1)
		{
			printf("75 Gold");
			Gotoxy(GBOARD_ORIGIN_X + tmp + 6, GBOARD_ORIGIN_Y + 22 + 2);
			printf("2");
		}

		for (i = 0; i < 3; i++) {
			Gotoxy(GBOARD_ORIGIN_X + tmp, GBOARD_ORIGIN_Y + 22 + i);
			printf("��");
		}

		tmp += 11;
	}

	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 25);
	printf("��");
	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 25);
	for (i = 1; i < 55; i++) {
		Gotoxy(GBOARD_ORIGIN_X + i, GBOARD_ORIGIN_Y + 25);
		printf("��");
	}
	printf("��");


	// ��ų ����
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 21);
	printf("��");
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 21);
	for (i = 1; i < 33; i++) {
		Gotoxy(GBOARD_ORIGIN_X + 57 + i, GBOARD_ORIGIN_Y + 21);
		printf("��");
	}
	printf("��");
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 23);
	for (i = 1; i < 33; i++) {
		Gotoxy(GBOARD_ORIGIN_X + 57 + i, GBOARD_ORIGIN_Y + 23);
		printf("��");
	}
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 22);
	tmp = 0;
	for (j = 0; j < 4; j++) {
		Gotoxy(GBOARD_ORIGIN_X + 57 + tmp + 2, GBOARD_ORIGIN_Y + 22);
		if (j == 0) {
			printf("10 Assist"); //%d , generateAssist[j].useAssistGage
			Gotoxy(GBOARD_ORIGIN_X + 57 + tmp + 5, GBOARD_ORIGIN_Y + 22 + 2);
			printf(" Q");
		}

		for (i = 0; i < 3; i++) {
			Gotoxy(GBOARD_ORIGIN_X + 57 + tmp, GBOARD_ORIGIN_Y + 22 + i);
			printf("��");
		}

		tmp += 11;
	}
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 25);
	printf("��");
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 25);
	for (i = 1; i < 33; i++) {
		Gotoxy(GBOARD_ORIGIN_X + 57 + i, GBOARD_ORIGIN_Y + 25);
		printf("��");
	}
	printf("��");


	// gameBoardInfo�� ���� �߰�
	for (y = 0; y <= 20; y++)
	{
		gameBoardInfo[y][0] = 1;
		gameBoardInfo[y][45] = 1;
	}

	for (x = 0; x <= 45; x++)
	{
		gameBoardInfo[0][x] = 1;
		gameBoardInfo[5][x] = 1;
		gameBoardInfo[10][x] = 1;
		gameBoardInfo[15][x] = 1;
		gameBoardInfo[20][x] = 1;
	}
}

void GenerateRobot()
{
	int line;
	int x, y;

	srand((unsigned int)time(NULL));
	line = rand() % 4; // ���° �ٿ� �κ��� �������� �������� ����

	x = GBOARD_ORIGIN_X + 82;
	y = GBOARD_ORIGIN_Y + 1 + 5 * line;

	if (isEmpty(x, y)) // ��ĭ �˻�
	{
		r[nr] = (ROBOT*)malloc(sizeof(ROBOT)); // �κ� �����Ҵ�

		if (stage == 1)
			r[nr]->model = rand() % 2;
		else if (stage == 2)
			r[nr]->model = rand() % 4;
		else if (stage == 3)
			r[nr]->model = rand() % 5;

		r[nr]->x = x;
		r[nr]->y = y;

		switch (r[nr]->model)
		{
		case 0: r[nr]->attack = 1; r[nr]->hp = 5; break;
		case 1: r[nr]->attack = 2; r[nr]->hp = 8; break;
		case 2: r[nr]->attack = 5; r[nr]->hp = 25; break;
		case 3: r[nr]->attack = 8; r[nr]->hp = 15; break;
		case 4: r[nr]->attack = 6; r[nr]->hp = 20; break;
		default: break;
		}

		Gotoxy(r[nr]->x, r[nr]->y); // �ڱ� ��ġ�� �̵�
		PrintRobot(robotModel[r[nr]->model], 15);
		AddRobotToBoard(r[nr]->model, r[nr]->x, r[nr]->y); // ���� ������ �κ��� �߰�

		nr++; // �κ� ���� �߰�
	}
}

void PrintRobot(char robotInfo[4][4], int color) // �κ� �׸��� �Լ�
{
	int x, y;
	COORD curPos = GetCurrentCursorPos();
	textcolor(color);
	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			Gotoxy(curPos.X + (x * 2), curPos.Y + y);

			if (robotInfo[y][x] == 0)
			{
				printf("  ");
			}
			else if (robotInfo[y][x] == 2)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 3)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 4)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 5)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 6)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 7)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 8)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 9)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 10)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 11)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 12)
			{
				printf("�� ");
			}
			else if (robotInfo[y][x] == 13)
			{
				printf("�� ");
			}
			else if (robotInfo[y][x] == 14)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 15)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 16)
			{
				printf("��");
			}
			else if (robotInfo[y][x] == 17)
			{
				printf("��");
			}
		}
	}
	Gotoxy(curPos.X, curPos.Y);
	textcolor(15);
}

void AddRobotToBoard(int model, int curPosX, int curPosY) // ���� ������ �κ��� �߰��ϴ� �Լ�
{
	int x, y, arrCurX, arrCurY;

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			arrCurX = (curPosX - GBOARD_ORIGIN_X) / 2;
			arrCurY = curPosY - GBOARD_ORIGIN_Y;

			if (robotModel[model][y][x] >= 2 && robotModel[model][y][x] <= 17)
				gameBoardInfo[arrCurY + y][arrCurX + x] = robotModel[model][y][x];
		}
	}
}

void CheckRobot()
{
	int i;
	int collision;

	for (i = 0; i < nr; i++) // �κ� Ž��
	{
		if (r[i]->hp <= 0) // hp �˻�
		{
			DeleteRobot(i, robotModel[r[i]->model]);
			i--;
			continue;
		}

		else // ü���� 0 �̻��̸� �浹 üũ
		{
			collision = DetectCollision(i, r[i]->x, r[i]->y);

			if (collision == -1) // �浹���� ������
			{
				MoveRobot(r[i]); // �κ� �����̱�
			}
			else if (collision >= 0) // ����� �浹�ϸ�
			{
				AttackWeapon(r[i], collision); // ���� ����
			}
		}
	}
}

void DeleteRobot(int index, char robotInfo[4][4])
{
	int x, y;
	int arrCurX, arrCurY;
	int i;
	int g, a;

	PlaySound(TEXT("./sound/robotdie.wav"), NULL, SND_ASYNC);

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			arrCurX = (r[index]->x - GBOARD_ORIGIN_X) / 2;
			arrCurY = r[index]->y - GBOARD_ORIGIN_Y;

			if (robotInfo[y][x] >= 2 && robotInfo[y][x] <= 17)
			{
				gameBoardInfo[arrCurY + y][arrCurX + x] = 0;
				printf("  ");
			}
		}
	}

	robot--;

	// ��ȭ
	if (r[index]->model == 0)
	{
		g = 50; a = 0;
	}
	else if (r[index]->model == 1)
	{
		g = 70; a = 1;
	}
	else if (r[index]->model == 2)
	{
		g = 100; a = 2;
	}
	else if (r[index]->model == 3)
	{
		g = 150; a = 2;
	}
	else if (r[index]->model == 4)
	{
		g = 200; a = 3;
	}

	gold += g;
	for (i = 0; i < 15; i++)
	{
		if (a == 0)
			break;

		if (assist[i] != 1)
		{
			assist[i] = 1;
			a--;
		}
	}

	free(r[index]);
	r[index] = (ROBOT*)malloc(sizeof(ROBOT));
	for (i = index; i < nr - 1; i++)
	{
		*r[i] = *r[i + 1];
	}
	r[nr - 1] = NULL;
	nr--;
}

void MoveRobot(ROBOT* r) // 1116 ����
{
	int i;
	int x, y;
	int arrCurX = (r->x - GBOARD_ORIGIN_X) / 2, arrCurY = r->y - GBOARD_ORIGIN_Y;

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			if (robotModel[r->model][y][x] != 0)
			{
				gameBoardInfo[arrCurY + y][arrCurX + x - 1] = gameBoardInfo[arrCurY + y][arrCurX + x];
				gameBoardInfo[arrCurY + y][arrCurX + x] = 0;
			}
		}
	}
	r->x -= 2;
}

// 1116 �κ��� ���⸦ �����ϴ� �Լ�
void AttackWeapon(ROBOT* r, int idx)
{
	int x, y;

	w[idx]->durability -= r->attack;

	// ���Ⱑ ���� ������ �� ������ �� �� �����̵���(����ȭ)
	Gotoxy(w[idx]->x, w[idx]->y);
	PrintWeapon(weaponModel[w[idx]->model], 4);

	// ���ݴ��� ������ �������� 0 ���ϰ� �Ǹ� ����
	if (w[idx]->durability <= 0)
		DeleteWeapon(idx, weaponModel[w[idx]->model], 2);
}

int DetectCollision(int n, int x, int y)
{
	int i;
	int arrX = (x - GBOARD_ORIGIN_X) / 2;
	int arrY = (y - GBOARD_ORIGIN_Y);
	int idx;
	int flag = -1;

	// �κ��� �Ѿ� �浹�� ������ ���� �� �˻� �� �������� 0 �̻��̸� �ٸ� �浹 �˻�(���� �ʿ�)
	for (i = 0; i < nb; i++)
	{
		if (b[i]->x >= x && (b[i]->y - 2 == y || b[i]->y - 1 == y))	// �κ� �տ� �Ѿ��� ������
		{
			switch (b[i]->model)
			{
			case 0: r[n]->hp -= 1; break;
			case 1: r[n]->hp -= 2; break;
			case 2: r[n]->hp -= 3; break;
			case 3: r[n]->hp -= 5; break;
			case 4: r[n]->hp -= 8; break;
			default: break;
			}
			PlaySound(TEXT("./sound/robothurts.wav"), NULL, SND_ASYNC);
			// �κ��� ���� ������ �� ������ �� �� �����̵���(����ȭ)
			Gotoxy(r[n]->x, r[n]->y);
			PrintRobot(robotModel[r[n]->model], 4);
			DeleteBullet(i);
			i--;
		}
	}
	// �κ��� ������ �浹 �� ������ ���� �� �ٸ� �浹 �˻�
	for (i = 0; i < 4; i++)
	{
		if (gameBoardInfo[arrY + i][arrX - 1] == 37)
		{
			idx = WhichItem(x - 8, y);
			Gotoxy(it[idx]->x, it[idx]->y);
			DeleteItem(idx, 0);
			break;
		}
	}

	// �κ��� �κ� �浹 �˻�
	for (i = 0; i < 4; i++)
	{
		if (gameBoardInfo[arrY + i][arrX - 1] >= 2 && gameBoardInfo[arrY + i][arrX - 1] <= 17)
		{
			flag = -2;
			break;
		}
	}

	if (flag != -2) // �κ� �տ� �κ��� ���ٸ�
	{
		// �κ��� ���� �浹�ϸ� ���� �迭 �ε��� ��ȯ
		for (i = 0; i < nw; i++)
		{
			if (w[i]->y == y)
			{
				if (x - w[i]->x == 8)
				{
					flag = WhichWeapon(x - 8, y);
					break;
				}
			}
		}
	}

	return flag; // �浹 ���� Ȯ���ؼ� ����
}

void GenerateWeapon(int n)
{
	int flag = 0;
	COORD curPos = GetCurrentCursorPos();

	switch (n)
	{
	case 0:
		if (gold - 50 >= 0)
			flag = 1;
		break;
	case 1:
		if (gold - 75 >= 0)
			flag = 1;
		break;
	case 2:
		if (gold - 125 >= 0)
			flag = 1;
		break;
	case 3:
		if (gold - 200 >= 0)
			flag = 1;
		break;
	case 4:
		if (gold - 500 >= 0)
			flag = 1;
		break;
	default: break;
	}
	if (flag) {
		w[nw] = (WEAPON*)malloc(sizeof(WEAPON));

		w[nw]->model = n;
		w[nw]->x = curPos.X;
		w[nw]->y = curPos.Y;
		switch (n)
		{
		case 0: w[nw]->durability = 7;
			w[nw]->bullettime = 2;
			gold -= 50;
			break;
		case 1:	w[nw]->durability = 10;
			w[nw]->bullettime = 2;
			gold -= 75;
			break;
		case 2: w[nw]->durability = 15;
			w[nw]->bullettime = 2;
			gold -= 125;
			break;
		case 3: w[nw]->durability = 20;
			w[nw]->bullettime = 2;
			gold -= 200;
			break;
		case 4: w[nw]->durability = 30;
			w[nw]->bullettime = 2;
			gold -= 500;
			break;
		default: break;
		}
		PrintWeapon(weaponModel[w[nw]->model], 15);
		AddWeaponToBoard(w[nw]->model, w[nw]->x, w[nw]->y); // ���� ������ �κ��� �߰�

		nw++; // ���� ���� �߰�

		PlaySound(TEXT("./sound/weapon.wav"), NULL, SND_ASYNC);
	}
	else
		PlaySound(TEXT("./sound/dontweapon.wav"), NULL, SND_ASYNC);
}

void PrintWeapon(char weaponInfo[4][4], int color) // ���� �׸��� �Լ�
{
	int x, y;
	COORD curPos = GetCurrentCursorPos();
	textcolor(color);
	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			Gotoxy(curPos.X + (x * 2), curPos.Y + y);

			if (weaponInfo[y][x] == 0)
			{
				printf("  ");
			}
			else if (weaponInfo[y][x] == 20)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 21)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 22)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 23)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 24)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 25)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 26)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 27)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 28)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 29)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 30)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 31)
			{
				printf("��");
			}
			else if (weaponInfo[y][x] == 32)
			{
				printf("��");
			}
		}
	}
	Gotoxy(curPos.X, curPos.Y);
	textcolor(15);
}

void AddWeaponToBoard(int model, int curPosX, int curPosY) // ���� ������ ����� �߰��ϴ� �Լ�
{
	int x, y, arrCurX, arrCurY;

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			arrCurX = (curPosX - GBOARD_ORIGIN_X) / 2;
			arrCurY = curPosY - GBOARD_ORIGIN_Y;

			if (weaponModel[model][y][x] >= 20 && weaponModel[model][y][x] <= 32)
				gameBoardInfo[arrCurY + y][arrCurX + x] = weaponModel[model][y][x];
		}
	}
}

void CheckWeapon()
{
	int i;

	for (i = 0; i < nw; i++)
	{
		if (w[i]->durability <= 0)
		{
			DeleteWeapon(i, weaponModel[w[i]->model], 2);
			i--;
			continue;
		}
		w[i]->bullettime--;
	}
}

void DeleteWeapon(int index, char weaponInfo[4][4], int f) // DeleteWeapon�Լ��� �� ������ �̿�Ǵµ� ��ü���� Ʋ�� ���� �������� �Ϻθ� �޶� �÷��� = 1 or 2�� ����
{														   // f = 1: BackSpace �޾��� �� ���� f = 2: �������� 0 ���ϰ� ���� �� ����
	int i;
	int x, y;
	int arrCurX, arrCurY;
	COORD curPos = GetCurrentCursorPos();

	PlaySound(TEXT("./sound/removeweapon.wav"), NULL, SND_ASYNC);
	if (f == 1)
	{
		for (y = 0; y < 4; y++)
		{
			for (x = 0; x < 4; x++)
			{
				Gotoxy(curPos.X + (x * 2), curPos.Y + y);

				arrCurX = (curPos.X - GBOARD_ORIGIN_X) / 2;
				arrCurY = curPos.Y - GBOARD_ORIGIN_Y;
				gameBoardInfo[arrCurY + y][arrCurX + x] = 0;

				if (weaponInfo[y][x] >= 20 && weaponInfo[y][x] <= 32)
				{
					printf("  ");
				}
			}
		}
		Gotoxy(curPos.X, curPos.Y);
	}

	else if (f == 2)
	{
		for (y = 0; y < 4; y++)
		{
			for (x = 0; x < 4; x++)
			{
				arrCurX = (w[index]->x - GBOARD_ORIGIN_X) / 2;
				arrCurY = w[index]->y - GBOARD_ORIGIN_Y;
				gameBoardInfo[arrCurY + y][arrCurX + x] = 0;
				if (weaponInfo[y][x] >= 20 && weaponInfo[y][x] <= 32)
				{
					printf("  ");
				}
			}
		}
	}

	free(w[index]);
	w[index] = (WEAPON*)malloc(sizeof(WEAPON));
	for (i = index; i < nw - 1; i++)
	{
		*w[i] = *w[i + 1];
	}
	w[nw - 1] = NULL;

	nw--;
}

int WhichWeapon(int x, int y)
{
	int i;

	if (isEmpty(x, y)) // ��ĭ�̸� -1 ��ȯ
		return -1;

	for (i = 0; i < nw; i++) // ���� ����ü ������ �迭�� Ž��
	{
		if (w[i]->x == x && w[i]->y == y) // ���� Ŀ�� ��ġ�� ��ġ�� ���� ã��
		{
			return i; // ���� �� �ε��� ��ȯ
		}
	}

	return -1; // �κ� ���̸� -1 ��ȯ
}

void ProcessKeyInput()
{
	int key;
	int i, n, index = -1, line;
	int useSkill, countUse;
	COORD curPos, curPos2;

	Gotoxy(indicatorPos.X, indicatorPos.Y); // ����� �ε��������� ��ġ�� Ŀ�� ����
	curPos = GetCurrentCursorPos(); // ���� ��ġ ����
	textcolor(4); // �۾� ���������� ����
	printf("��������");

	for (i = 0; i < 20; i++)
	{
		if (_kbhit() != 0)
		{
			key = _getch();

			// �ε������� �̵�
			if (key == RIGHT) // ������ ����Ű
			{
				if (hcnt == 7)
					PlaySound(TEXT("./sound/indicator.wav"), NULL, SND_ASYNC);
				if (hcnt != 7)
				{
					Gotoxy(indicatorPos.X, indicatorPos.Y);
					textcolor(15);
					printf("��������");  // ���� �ε������� �����(������� �����)
					Gotoxy(indicatorPos.X + 10, indicatorPos.Y);
					curPos = GetCurrentCursorPos();
					textcolor(4);
					printf("��������");
					indicatorPos = curPos;
					hcnt++;
				}
			}

			else if (key == LEFT) // ���� ����Ű
			{
				if (hcnt == 1)
					PlaySound(TEXT("./sound/indicator.wav"), NULL, SND_ASYNC);
				if (hcnt != 1)
				{
					Gotoxy(indicatorPos.X, indicatorPos.Y);
					textcolor(15);
					printf("��������");
					Gotoxy(indicatorPos.X - 10, indicatorPos.Y);
					curPos = GetCurrentCursorPos();
					textcolor(4);
					printf("��������");
					indicatorPos = curPos;
					hcnt--;
				}
			}

			else if (key == UP) // �� ����Ű
			{
				if (vcnt == 1)
					PlaySound(TEXT("./sound/indicator.wav"), NULL, SND_ASYNC);
				if (vcnt != 1)
				{
					Gotoxy(indicatorPos.X, indicatorPos.Y);
					textcolor(15);
					printf("��������");
					Gotoxy(indicatorPos.X, indicatorPos.Y - 5);
					curPos = GetCurrentCursorPos();
					textcolor(4);
					printf("��������");
					indicatorPos = curPos;
					vcnt--;
				}
			}

			else if (key == DOWN) // �Ʒ� ����Ű
			{
				if (vcnt == 4)
					PlaySound(TEXT("./sound/indicator.wav"), NULL, SND_ASYNC);
				if (vcnt != 4)
				{
					Gotoxy(indicatorPos.X, indicatorPos.Y);
					textcolor(15);
					printf("��������");
					Gotoxy(indicatorPos.X, indicatorPos.Y + 5);
					curPos = GetCurrentCursorPos();
					textcolor(4);
					printf("��������");
					indicatorPos = curPos;
					vcnt++;
				}
			}


			// ���� ��ġ
			else if ((49 <= key && key <= 50) && stage == 1) // 1~2�� - ��������1
			{
				n = key - '0' - 1;
				curPos2.X = curPos.X - 2;
				curPos2.Y = curPos.Y + 1;

				if (isEmpty(curPos2.X, curPos2.Y))
				{
					Gotoxy(curPos2.X, curPos2.Y);
					textcolor(7);
					GenerateWeapon(n);
					Gotoxy(curPos2.X + 3, curPos2.Y - 1);
				}
				else
					PlaySound(TEXT("./sound/dontweapon.wav"), NULL, SND_ASYNC);
			}

			else if ((49 <= key && key <= 52) && stage == 2) // 1~4�� - ��������2
			{
				n = key - '0' - 1;
				curPos2.X = curPos.X - 2;
				curPos2.Y = curPos.Y + 1;

				if (isEmpty(curPos2.X, curPos2.Y))
				{
					Gotoxy(curPos2.X, curPos2.Y);
					textcolor(7);
					GenerateWeapon(n);
					Gotoxy(curPos2.X + 3, curPos2.Y - 1);
				}
				else
					PlaySound(TEXT("./sound/dontweapon.wav"), NULL, SND_ASYNC);
			}

			else if ((49 <= key && key <= 53) && stage == 3) // 1~5�� - ��������3
			{
				n = key - '0' - 1;
				curPos2.X = curPos.X - 2;
				curPos2.Y = curPos.Y + 1;

				if (isEmpty(curPos2.X, curPos2.Y))
				{
					Gotoxy(curPos2.X, curPos2.Y);
					textcolor(7);
					GenerateWeapon(n);
					Gotoxy(curPos2.X + 3, curPos2.Y - 1);
				}
				else
					PlaySound(TEXT("./sound/dontweapon.wav"), NULL, SND_ASYNC);
			}

			if (51 <= key && key <= 53 && stage == 1)
				PlaySound(TEXT("./sound/dontweapon.wav"), NULL, SND_ASYNC);
			if (key == 53 && stage == 2)
				PlaySound(TEXT("./sound/dontweapon.wav"), NULL, SND_ASYNC);


			// ���� ����
			else if (key == BACKSPACE) // �齺���̽�
			{
				curPos2.X = curPos.X - 2;
				curPos2.Y = curPos.Y + 1;

				index = WhichWeapon(curPos2.X, curPos2.Y); // ����ü ������ �迭 �ε���
				if (!isEmpty(curPos2.X, curPos2.Y) && index != -1)
				{
					Gotoxy(curPos2.X, curPos2.Y);
					DeleteWeapon(index, weaponModel[w[index]->model], 1);

					Gotoxy(curPos2.X + 3, curPos2.Y - 1);
				}
			}


			// ��ų

			if (key == 113 && assist[9] != 1)
				PlaySound(TEXT("./sound/dontuseskill.wav"), NULL, SND_ASYNC);
			if (key == 119 && assist[11] != 1)
				PlaySound(TEXT("./sound/dontuseskill.wav"), NULL, SND_ASYNC);
			if (key == 101 && assist[14] != 1)
				PlaySound(TEXT("./sound/dontuseskill.wav"), NULL, SND_ASYNC);
			if (stage == 1 && (key == 119 || key == 101))
				PlaySound(TEXT("./sound/dontuseskill.wav"), NULL, SND_ASYNC);
			if (stage == 2 && key == 101)
				PlaySound(TEXT("./sound/dontuseskill.wav"), NULL, SND_ASYNC);


			else if (key == 113 && assist[9] == 1) // Q(��Ʈ��) 
			{
				countUse = 0;
				useSkill = 10;
				for (i = 0; i < 15; i++) {
					if (assist[i] == 1)
						countUse++;
				}

				if (countUse >= useSkill) {
					countUse -= useSkill;
					for (i = 0; i < 15; i++) {
						if (countUse) {
							assist[i] = 1;
							countUse--;
						}
						else
							assist[i] = 0;
					}
					PlaySound(TEXT("./sound/skilljet.wav"), NULL, SND_ASYNC);
					line = vcnt - 1; // �� ��° ����(0~3)
					// �ش� ������ ù��° ĭ���� �̵�
					curPos2.X = GBOARD_ORIGIN_X + 2;
					curPos2.Y = GBOARD_ORIGIN_Y + 1 + 5 * line;


					for (i = 0; i < 40; i++) // ��ų ����Ʈ
					{
						Gotoxy(curPos2.X + 2 * i, curPos2.Y);
						PrintJet();
						Sleep(10);
						DeleteJet();
						if (_kbhit() != 0) // ��ų�� �ߵ��ϴ� ���� Ű���� �Է� ����
							key = _getch();
					}

					for (i = 0; i < nr; i++)
					{
						if (r[i]->y == GBOARD_ORIGIN_Y + 1 + 5 * line)
						{
							Gotoxy(r[i]->x + 2, r[i]->y);
							PrintRobot(robotModel[r[i]->model], 4);
							textcolor(4);
							Gotoxy(r[i]->x + 2, r[i]->y);
							printf("-5");
							r[i]->hp -= 5; // �κ� ü�� ���
						}
					}

					textcolor(15);
				}
			}

			else if (key == 119 && stage >= 2) // W(����ġ��)
			{
				countUse = 0;
				useSkill = 12;
				for (i = 0; i < 15; i++) {
					if (assist[i] == 1)
						countUse++;
				}
				if (countUse >= useSkill) {

					countUse -= useSkill;
					for (i = 0; i < 15; i++) {
						if (countUse) {
							assist[i] = 1;
							countUse--;
						}
						else
							assist[i] = 0;
					}
					PlaySound(TEXT("./sound/skillheal.wav"), NULL, SND_ASYNC);
					for (i = 0; i < nw; i++)
					{
						Gotoxy(w[i]->x, w[i]->y);
						PrintWeapon(weaponModel[w[i]->model], 10);
						textcolor(10);
						Gotoxy(w[i]->x, w[i]->y);
						printf("+5");
						w[i]->durability += 5; // ���� ������ ȸ��
					}
					textcolor(15);
				}
			}

			else if (key == 101 && stage == 3) // E(����)
			{
				countUse = 0;
				useSkill = 15;
				for (i = 0; i < 15; i++) {
					if (assist[i] == 1)
						countUse++;
				}
				if (countUse >= useSkill) {
					countUse -= useSkill;
					for (i = 0; i < 15; i++) {
						if (countUse) {
							assist[i] = 1;
							countUse--;
						}
						else
							assist[i] = 0;
					}
					PlaySound(TEXT("./sound/skilljudge.wav"), NULL, SND_ASYNC);
					for (i = 0; i < nr; i++)
					{
						Gotoxy(r[i]->x + 2, r[i]->y);
						PrintRobot(robotModel[r[i]->model], 4);
						textcolor(4);
						Gotoxy(r[i]->x + 2, r[i]->y);
						printf("-5");
						r[i]->hp -= 5; // �κ� ü�� ���
					}
					textcolor(15);
				}
			}


			// ������ ȹ��
			else if (key == SPACE) // �����̽���
			{
				curPos2.X = curPos.X - 2;
				curPos2.Y = curPos.Y + 1;

				index = WhichItem(curPos2.X, curPos2.Y); // ����ü ������ �迭 �ε���
				if (!isEmpty(curPos2.X, curPos2.Y) && index != -1)
				{
					Gotoxy(curPos2.X, curPos2.Y);
					PlaySound(TEXT("./sound/eatitem.wav"), NULL, SND_ASYNC);
					DeleteItem(index, 1);

					Gotoxy(curPos2.X + 3, curPos2.Y - 1);
				}
			}


			// ���� ����
			else if (key == ESC) // ESC
			{
				textcolor(7);
				system("cls");
				endDraw();
				exit(-1);
			}
		}

		Sleep(speed); // ���ǵ� ����
	}
}

int isEmpty(int curPosX, int curPosY) // ��ĭ üũ �Լ�(1 - ��ĭ, 0 - ��ĭX)
{
	int x, y;
	int arrCurX = (curPosX - GBOARD_ORIGIN_X) / 2;
	int arrCurY = curPosY - GBOARD_ORIGIN_Y;

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			if (gameBoardInfo[arrCurY + y][arrCurX + x] != 0)
				return 0;
		}
	}

	return 1;
}

int isGameOver()  // 1116 ������ �κ�: �κ��� ���� �հ��淡 �� �հ� ���� �����س���!
{
	int i;

	for (i = 0; i < nr; i++)
	{
		if (r[i]->x <= GBOARD_ORIGIN_X + 2)
			return 1;
	}
	return 0;
}

void StageUp()
{
	COORD curPos = GetCurrentCursorPos();
	int i;

	if (stage == 1)
		robot = 15;
	else if (stage == 2)
		robot = 20;

	PlaySound(TEXT("./sound/stageup.wav"), NULL, SND_ASYNC);
	for (i = 0; i < 2; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X + 67, GBOARD_ORIGIN_Y - 1);
		textcolor(9);
		printf("STAGE UP!!");
		Sleep(800);
		Gotoxy(GBOARD_ORIGIN_X + 67, GBOARD_ORIGIN_Y - 1);
		printf("          ");
		Sleep(800);
	}
	textcolor(15);
	Gotoxy(curPos.X, curPos.Y);
	stage++;
	RedrawBoard();
}

void GenerateBullet()
{
	int i;

	for (i = 0; i < nw; i++)
	{
		if (w[i]->bullettime == 0)
		{
			b[nb] = (BULLET*)malloc(sizeof(BULLET)); // �Ѿ� �ϳ� �����Ҵ�
			w[i]->durability--;
			if (w[i]->model == 0) // 1�� ����
			{
				b[nb]->x = w[i]->x + 6;
				b[nb]->y = w[i]->y + 2;
				b[nb]->model = 0;
				Gotoxy(b[nb]->x, b[nb]->y);
				w[i]->bullettime = 5;
			}

			else if (w[i]->model == 1) // 2�� ����
			{
				b[nb]->x = w[i]->x + 6;
				b[nb]->y = w[i]->y + 2;
				b[nb]->model = 1;
				Gotoxy(b[nb]->x, b[nb]->y);
				w[i]->bullettime = 5;
			}

			else if (w[i]->model == 2) // 3�� ����
			{
				b[nb]->x = w[i]->x + 6;
				b[nb]->y = w[i]->y + 2;
				b[nb]->model = 2;
				Gotoxy(b[nb]->x, b[nb]->y);
				w[i]->bullettime = 5;
			}

			else if (w[i]->model == 3) // 4�� ����
			{
				b[nb]->x = w[i]->x + 6;
				b[nb]->y = w[i]->y + 1;
				b[nb]->model = 3;
				Gotoxy(b[nb]->x, b[nb]->y);
				w[i]->bullettime = 7;
			}

			else if (w[i]->model == 4) // 5�� ����
			{
				b[nb]->x = w[i]->x + 6;
				b[nb]->y = w[i]->y + 1;
				b[nb]->model = 4;
				Gotoxy(b[nb]->x, b[nb]->y);
				w[i]->bullettime = 3;
			}

			nb++;
		}
	}
}

void MoveBullet()
{
	int i;

	for (i = 0; i < nb; i++)
	{
		b[i]->x += 2;
		Gotoxy(b[i]->x, b[i]->y);

		if (b[i]->model == 0)
			printf("��");
		else if (b[i]->model == 1)
			printf("��");
		else if (b[i]->model == 2)
			printf("��");
		else if (b[i]->model == 3)
			printf("��");
		else if (b[i]->model == 4)
			printf("��");

	}
}

void DeleteBullet(int idx)
{
	int i;

	free(b[idx]);
	b[idx] = (BULLET*)malloc(sizeof(BULLET));
	for (i = idx; i < nb - 1; i++)
	{
		*b[i] = *b[i + 1];
	}
	b[nb - 1] = NULL;
	nb--;
}

void CheckBullet()
{
	int i;
	int arrX, arrY;
	for (i = 0; i < nb; i++)
	{
		arrX = (b[i]->x + 2 - GBOARD_ORIGIN_X) / 2;
		arrY = b[i]->y - GBOARD_ORIGIN_Y;
		if (gameBoardInfo[arrY][arrX] == 1)
		{
			DeleteBullet(i);
			i--;
		}
	}
}

void GenerateItem()
{
	COORD curPos = GetCurrentCursorPos();
	int line, kan;
	int x, y;

	while (1)
	{
		srand((unsigned int)time(NULL));
		line = rand() % 4; // ���° �ٿ� 
		kan = rand() % 7; // ���° ĭ�� �������� �������� ����

		x = GBOARD_ORIGIN_X + 2 + 10 * kan;
		y = GBOARD_ORIGIN_Y + 1 + 5 * line;

		if (isEmpty(x, y)) //��ĭ�̸� 
			break;
	}

	it[ni] = (ITEM*)malloc(sizeof(ITEM)); // ������ �����Ҵ�
	it[ni]->x = x;
	it[ni]->y = y;

	while (1)
	{
		it[ni]->gold = rand() % 50;
		it[ni]->assist = rand() % 3;

		if ((it[ni]->gold) != 0 && (it[ni]->assist) != 0)
			break;
	}

	it[ni]->time = 8; // ������ 8�� ������ �����

	Gotoxy(it[ni]->x, it[ni]->y); // �ڱ� ��ġ�� �̵�
	PrintItem();
	AddItemToBoard(it[ni]->x, it[ni]->y); // ���� ������ ������ �� �߰�
	textcolor(15);
	ni++; // ������ ���� �߰�
}

void PrintItem() // ������ �׸��� �Լ�
{
	int x, y;
	COORD curPos = GetCurrentCursorPos();
	textcolor(14);
	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			Gotoxy(curPos.X + (x * 2), curPos.Y + y);

			if (itemModel[y][x] == 0)
			{
				printf("  ");
			}
			else if (itemModel[y][x] == 35)
			{
				printf("��");
			}
			else if (itemModel[y][x] == 36)
			{
				printf("??");
			}
			else if (itemModel[y][x] == 37)
			{
				printf("��");
			}
		}
	}
	Gotoxy(curPos.X, curPos.Y);
}

void AddItemToBoard(int curPosX, int curPosY) // ���� ������ ������ �� �߰��ϴ� �Լ�
{
	int x, y, arrCurX, arrCurY;

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			arrCurX = (curPosX - GBOARD_ORIGIN_X) / 2;
			arrCurY = curPosY - GBOARD_ORIGIN_Y;

			if (itemModel[y][x] >= 35 && itemModel[y][x] <= 37)
				gameBoardInfo[arrCurY + y][arrCurX + x] = itemModel[y][x];
		}
	}
}

void CheckItem() // �ð� ������ ������ ������� �Լ�
{
	int i;
	int index;

	for (i = 0; i < ni; i++)
	{
		if ((it[i]->time) == 0)
		{
			index = WhichItem(it[i]->x, it[i]->y); // ����ü ������ �迭 �ε���
			if (!isEmpty(it[i]->x, it[i]->y) && index != -1)
			{
				Gotoxy(it[i]->x, it[i]->y);
				DeleteItem(index, 0);
				i--;
				continue;
			}
		}

		if (t % 1 == 0) //1�ʸ��� �˻�
		{
			for (i = 0; i < ni; i++)
				(it[i]->time) -= 1;
		}
	}
}

void DeleteItem(int index, int f) //1. space�ٷ� ������ ��� �� ��ý�Ʈ ���ŵ�  2. �����ð� ������ �����   3. �κ��� �浹�ϸ� �����
{                                 // 1�� -> f = 1   2��, 3�� -> f = 0     3���� �ϴ� �κ��浹ó������ ��� ������ ó���Ǵ� ���� ����!
	int i;
	int x, y;
	int arrCurX, arrCurY;
	COORD curPos = GetCurrentCursorPos();

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			Gotoxy(curPos.X + (x * 2), curPos.Y + y);

			arrCurX = (curPos.X - GBOARD_ORIGIN_X) / 2;
			arrCurY = curPos.Y - GBOARD_ORIGIN_Y;
			if (itemModel[y][x] >= 35 && itemModel[y][x] <= 37)
			{
				gameBoardInfo[arrCurY + y][arrCurX + x] = 0;
				printf("  ");
			}
		}
	}
	Gotoxy(curPos.X, curPos.Y);

	if (f == 1)
	{
		gold += (it[index]->gold);
		for (i = 0; i < 15; i++) {
			if ((it[index]->assist) == 0)
				break;

			if (assist[i] != 1) {
				assist[i] = 1;
				(it[index]->assist) -= 1;
			}
		}
	}

	free(it[index]);
	it[index] = (ITEM*)malloc(sizeof(ITEM));
	for (i = index; i < ni - 1; i++)
	{
		*it[i] = *it[i + 1];
	}
	it[ni - 1] = NULL;

	ni--;
}

int WhichItem(int x, int y)
{
	int i;

	if (isEmpty(x, y))  // ��ĭ�̸� -1 ��ȯ
		return -1;

	for (i = 0; i < ni; i++) // ������ ����ü ������ �迭�� Ž��
	{
		if (it[i]->x == x && it[i]->y == y) // ���� Ŀ�� ��ġ�� ��ġ�� ������ ã��
		{
			return i; // ������ �� �ε��� ��ȯ
		}
	}

	return -1; // ������ �� �ƴϸ� -1 ��ȯ
}

void PrintJet()
{
	COORD curPos = GetCurrentCursorPos();
	int x, y;
	textcolor(9);
	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			Gotoxy(curPos.X + (x * 2), curPos.Y + y);

			if (jetModel[y][x] == 0)
			{
				printf("  ");
			}
			else if (jetModel[y][x] == 1)
			{
				printf("��");
			}
			else if (jetModel[y][x] == 2)
			{
				printf("��");
			}
			else if (jetModel[y][x] == 3)
			{
				printf("=");
			}
		}
	}
	Gotoxy(curPos.X, curPos.Y);
	textcolor(15);
}

void DeleteJet()
{
	COORD curPos = GetCurrentCursorPos();
	int x, y;

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			Gotoxy(curPos.X + (x * 2), curPos.Y + y);

			if (jetModel[y][x] != 0)
			{
				printf("  ");
			}
		}
	}
	Gotoxy(curPos.X, curPos.Y);
}

void RedrawBoard() // ������Ʈ�� ���� ������ ���� ���� �ٽ� �׸��� �Լ�
{
	int i;
	int x, y;
	int cursX, cursY;

	textcolor(15);
	// ��ý�Ʈ
	Gotoxy(GBOARD_ORIGIN_X + 8, GBOARD_ORIGIN_Y - 1);
	for (i = 0; i < 15; i++) { // Redraw �ؾߵ�
		if (assist[i] == 1)
			printf("��");
		else
			printf("��");
	}

	// ���
	Gotoxy(GBOARD_ORIGIN_X + 45, GBOARD_ORIGIN_Y - 1);
	if (gold < 10)
		printf("Gold %d   ", gold);
	else if (gold < 100)
		printf("Gold %d  ", gold);
	else if (gold < 1000)
		printf("Gold %d ", gold);
	else if (gold < 10000)
		printf("Gold %d", gold);


	// ��������
	Gotoxy(GBOARD_ORIGIN_X + 79, GBOARD_ORIGIN_Y - 1);
	printf("STAGE");
	if (stage == 2)
		printf(" HARD  ");
	else if (stage == 3)
		printf(" HELL  ");

	// ���� �κ� ��
	Gotoxy(GBOARD_ORIGIN_X + 92, GBOARD_ORIGIN_Y + 10);
	printf("Target Robot");
	Gotoxy(GBOARD_ORIGIN_X + 97, GBOARD_ORIGIN_Y + 12);
	if (robot < 10)
		printf("%d ", robot);
	else if (robot < 100)
		printf("%d", robot);

	// ����, ��ų ����
	if (stage == 2 || stage == 3)
	{
		Gotoxy(GBOARD_ORIGIN_X + 25, GBOARD_ORIGIN_Y + 22);
		printf("125 Gold");
		Gotoxy(GBOARD_ORIGIN_X + 28, GBOARD_ORIGIN_Y + 24);
		printf("3");
		Gotoxy(GBOARD_ORIGIN_X + 36, GBOARD_ORIGIN_Y + 22);
		printf("200 Gold");
		Gotoxy(GBOARD_ORIGIN_X + 39, GBOARD_ORIGIN_Y + 24);
		printf("4");

		Gotoxy(GBOARD_ORIGIN_X + 70, GBOARD_ORIGIN_Y + 22);
		printf("12 Assist");
		Gotoxy(GBOARD_ORIGIN_X + 73, GBOARD_ORIGIN_Y + 24);
		printf(" W");

		if (stage == 3)
		{
			Gotoxy(GBOARD_ORIGIN_X + 47, GBOARD_ORIGIN_Y + 22);
			printf("500 Gold");
			Gotoxy(GBOARD_ORIGIN_X + 50, GBOARD_ORIGIN_Y + 24);
			printf("5");

			Gotoxy(GBOARD_ORIGIN_X + 81, GBOARD_ORIGIN_Y + 22);
			printf("15 Assist");
			Gotoxy(GBOARD_ORIGIN_X + 84, GBOARD_ORIGIN_Y + 24);
			printf(" E");
		}
	}

	for (y = 1; y < 20; y++)
	{
		for (x = 1; x < 45; x++)
		{
			cursX = x * 2 + GBOARD_ORIGIN_X;
			cursY = y + GBOARD_ORIGIN_Y;
			Gotoxy(cursX, cursY);

			if (y == 5 || y == 10 || y == 15) // ���̻��̿� �ִ� ���μ��� �ٽ� �׸��� ����
				continue;

			if (gameBoardInfo[y][x] == 0)
			{
				printf("  ");
			}
			else if (gameBoardInfo[y][x] == 2)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 3)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 4)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 5)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 6)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 7)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 8)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 9)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 10)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 11)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 12)
			{
				printf("�� ");
			}
			else if (gameBoardInfo[y][x] == 13)
			{
				printf("�� ");
			}
			else if (gameBoardInfo[y][x] == 14)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 15)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 16)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 17)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 20)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 21)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 22)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 23)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 24)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 25)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 26)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 27)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 28)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 29)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 30)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 31)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 32)
			{
				printf("��");
			}
			else if (gameBoardInfo[y][x] == 35)
			{
				textcolor(14);
				printf("��");
				textcolor(15);
			}
			else if (gameBoardInfo[y][x] == 36)
			{
				textcolor(14);
				printf("??");
				textcolor(15);
			}
			else if (gameBoardInfo[y][x] == 37)
			{
				textcolor(14);
				printf("��");
				textcolor(15);
			}
		}
	}
}


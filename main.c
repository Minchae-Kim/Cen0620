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
	int x, y; // 배치된 위치
	int model; // 모델 번호(0~4)
	int durability; // 내구도
	int bullettime; // 발사 간격
} WEAPON;

typedef struct bullet {
	int x, y; // 현재 위치
	int model; // 모델 번호(0~4)
} BULLET;

typedef struct robot {
	int x, y; // 현재 위치
	int model; // 모델 번호(0~4)
	int hp; // 체력
	int attack; // 공격력
} ROBOT;

typedef struct item {
	int x, y; // 배치된 위치
	int gold; // 골드
	int assist; // 어시스트
	int time; // 아이템 유지 시간
} ITEM;

// 전역변수
int gameBoardInfo[21][46] = { 0, }; // 게임보드정보배열
WEAPON* w[200]; // 무기 구조체 포인터 배열
BULLET* b[1000]; // 총알 구조체 포인터 배열
ROBOT* r[200]; // 로봇 구조체 포인터 배열
ITEM* it[200]; // 아이템 구조체 포인터 배열
int t = 0; // 시간(사이클 횟수)
COORD indicatorPos = { GBOARD_ORIGIN_X + 4, GBOARD_ORIGIN_Y }; // 인디케이터의 위치     
int hcnt = 1, vcnt = 1; // 인디케이터의 수평, 수직 위치를 나타내는 지표
int stage = 1; // 스테이지(1 - NORMAL, 2 - HARD, 3- HELL)
int speed = 30; // 게임 속도
int gold = 300, assist[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 골드, 어시스트 정보
int nr = 0, nw = 0, nb = 0, ni = 0; // 로봇, 무기, 총알, 아이템 수
int robot = 10; // 남은 로봇 수

// 함수 원형 선언
void init();
void titleDraw();
int menuDraw();
void helpPageDraw();
void endDraw();
void gameOverDraw();
void gameClearDraw();

void textcolor(int color_number);
void RemoveCursor(); // 커서 없애는 함수
COORD GetCurrentCursorPos(); // 현재 커서 위치 반환하는 함수
void Gotoxy(int x, int y); // 커서 위치 설정하는 함수
void DrawBoard(); // 초기 게임보드 출력하는 함수

void GenerateRobot(); // 로봇 생성하는 함수
void PrintRobot(char robotInfo[4][4], int color); // 로봇 출력하는 함수
void AddRobotToBoard(int model, int curPosX, int curPosY); // 게임보드정보배열에 로봇 정보 추가하는 함수
void CheckRobot(); // 모든 로봇에 대해 충돌 검사하는 함수
void DeleteRobot(int index, char robotInfo[4][4]);
void MoveRobot(ROBOT* r); // 로봇 왼쪽으로 한 칸 이동하는 함수
void AttackWeapon(ROBOT* r, int idx); // 로봇이 무기 공격하는 함수
int DetectCollision(int n, int x, int y); // 한 로봇에 대해 충돌 검사하는 함수

void GenerateWeapon(int n); // 무기 생성하는 함수
void PrintWeapon(char weaponInfo[4][4], int color); // 무기 출력하는 함수
void AddWeaponToBoard(int model, int curPosX, int curPosY); // 게임보드정보배열에 무기 정보 추가하는 함수
void CheckWeapon(); // 모든 무기에 대해 내구도 검사하는 함수
void DeleteWeapon(int index, char weaponInfo[4][4], int f); // 무기 삭제하는 함수
int WhichWeapon(int x, int y); // 칸에 위치한 무기의 배열 인덱스를 반환하는 함수

void ProcessKeyInput(); // 키 입력받는 함수
int isEmpty(int curPosX, int curPosY); // 빈 칸 검사하는 함수
int isGameOver(); // 게임 오버 함수
void StageUp(); // 스테이지 업 함수

void GenerateBullet(); // 총알 생성하는 함수
void MoveBullet(); // 총알 움직이는 함수
void DeleteBullet(int idx); // 총알 삭제 함수
void CheckBullet();// 총알 오른쪽 벽과 충돌하는지 확인하는 함수

void GenerateItem(); // 아이템 생성하는 함수
void PrintItem(); // 아이템 출력하는 함수
void AddItemToBoard(int curPosX, int curPosY); // 게임보드정보배열에 아이템 정보 추가하는 함수
void CheckItem(); // 모든 아이템에 대해 시간 검사하는 함수 
void DeleteItem(int index, int f); // 아이템 삭제하는 함수
int WhichItem(int x, int y); // 칸에 위치한 아이템의 배열 인덱스를 반환하는 함수 

void PrintJet(); // 제트기 출력하는 함수
void DeleteJet(); // 제트기 삭제하는 함수
void RedrawBoard(); // 게임보드 다시 출력하는 함수

int main()
{
	int i, j;
	int menu;

	PlaySound(TEXT("./sound/start.wav"), NULL, SND_ASYNC);
	init();
	titleDraw();
	menu = menuDraw();

	switch (menu) {
	case 0: //게임시작 
	gameStart:
		PlaySound(NULL, NULL, NULL);
		DrawBoard();
		RemoveCursor();
		while (1)
		{
			if (isGameOver()) { // 왼쪽 벽에 닿으면 게임 오버
				PlaySound(TEXT("./sound/gameover.wav"), NULL, SND_ASYNC);
				gameOverDraw();
				Sleep(3000);
				break;
			}

			if (robot <= 0) // 스테이지 업 or 게임 클리어
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

			if (t != 0 && t % 10 == 0) // 로봇 생성 간격
				GenerateRobot(); // 로봇이 오른쪽 끝에 생성됨

			if (t == 3 || t != 0 && t % 12 == 0) // 아이템 생성 간격
				GenerateItem(); // 아이템이 랜덤한 위치에 생성됨

			GenerateBullet(); // 총알 생성
			CheckBullet(); // 총알 벽과의 충돌 체크
			MoveBullet(); // 총알 움직이기

			CheckItem(); // 아이템 시간 체크
			CheckWeapon(); // 무기 내구도 체크, bullettime--
			CheckRobot(); // 로봇 체력, 충돌 체크

			ProcessKeyInput(); // 키 입력 검사

			RedrawBoard(); // 보드 다시 그리기

			t++;
		}

		break;
	case 1: //게임설명
		system("cls");
		helpPageDraw();

		//hlep page에서 space bar 누르면 게임 시작
		int key;
		while (1) {
			if (_kbhit() != 0) {
				key = _getch();

				if (key == SPACE)
					goto gameStart;
			}
		}
		break;

	case 2: //게임종료
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
	system("mode con cols=110 lines=30 | title 센0620");
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
	printf("↑↓키로 움직이고 space bar로 메뉴를 선택하세요");

	return;
}

int menuDraw() {
	int x = 25 + 24;
	int y = 4 + 16;
	Gotoxy(x - 2, y);
	printf("> 게 임 시 작");
	Gotoxy(x, y + 1);
	printf("게 임 설 명");
	Gotoxy(x, y + 2);
	printf("   종 료   ");

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
	printf("2120년 6월 20일...");
	textcolor(15);
	Gotoxy(35, 12);
	printf("로봇 부대가 세종대에 쳐들어오고,");
	Gotoxy(35, 14);
	printf("학교 건물들이 하나씩 함락된다.");
	Gotoxy(35, 16);
	printf("학생들은 마지막 남은 건물인 대양AI센터로 피신한다.");
	Gotoxy(35, 18);
	printf("로봇들은 센터를 향해 점점 다가오고 있다...");
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
	printf("<게임 설명>");
	textcolor(15);
	Gotoxy(10, 7);
	printf("- 맵 오른쪽 끝에서 일정 간격마다 로봇이 하나씩 나타난다.");
	Gotoxy(12, 8);
	printf("플레이어는 무기를 배치하고 스킬을 사용하며 다가오는 로봇을 파괴해야 한다.");
	Gotoxy(12, 9);
	printf("단 한 대의 로봇이라도 맵 왼쪽 끝에 다다르면 게임 오버!");
	Gotoxy(10, 11);
	printf("- 골드로 무기를 구매할 수 있고, 어시스트로 스킬을 사용할 수 있다.");
	Gotoxy(10, 13);
	printf("- 'Target Robot'은 스테이지 클리어를 위한 현재 남은 로봇 수를 말한다.");
	Gotoxy(10, 15);
	printf("- 스테이지는 NORMAL, HARD, HELL로 나뉜다.");
	Gotoxy(12, 16);
	printf("스테이지가 상승할수록 파괴해야 할 로봇의 수가 늘어나고,");
	Gotoxy(12, 17);
	printf("사용 가능한 무기와 스킬의 종류가 늘어나고, 나타나는 로봇의 종류가 늘어난다.");
	Gotoxy(10, 19);
	printf("- 방향키로 인디케이터를 이동하여 키 입력을 통해 여러 기능을 실행한다.");
	Gotoxy(10, 21);
	printf("- 무기는 총알을 한 번 발사할 때마다 내구도가 깎인다.");
	Gotoxy(10, 23);
	printf("- 아이템을 빨리 획득하지 않으면 사라진다.");
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
	printf("Tip. 아이템은 로봇이 근처에 가기 전에 빨리 획득하는 것이 좋다");
	Gotoxy(10, 5);
	textcolor(12);
	printf("<로봇 정보>");
	Gotoxy(12, 7);
	PrintRobot(robotModel[0], 15);
	Gotoxy(22, 9);
	printf("<기본형 로봇1>  가장 약한 로봇");
	Gotoxy(12, 11);
	PrintRobot(robotModel[1], 15);
	Gotoxy(22, 13);
	printf("<기본형 로봇2>  기본형 로봇1보다 체력면에서 업그레이드된 로봇");
	Gotoxy(10, 15);
	PrintRobot(robotModel[2], 15);
	Gotoxy(22, 17);
	printf(" <방어형 로봇>  방어에 특화된 로봇");
	Gotoxy(12, 20);
	PrintRobot(robotModel[3], 15);
	Gotoxy(22, 21);
	printf(" <공격형 로봇>  공격에 특화된 로봇");
	Gotoxy(10, 24);
	PrintRobot(robotModel[4], 15);
	Gotoxy(22, 25);
	printf("   <대형 로봇>  방어력과 공격력 두루 갖춘 로봇");
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
	printf("Tip. 무기는 로봇과 멀리 떨어진 곳에 배치하는 것이 좋다");
	Gotoxy(10, 5);
	textcolor(12);
	printf("<무기 정보>");
	Gotoxy(11, 7);
	PrintWeapon(weaponModel[0], 15);
	Gotoxy(22, 9);
	printf("    <권총>  가장 기본적인 무기");
	Gotoxy(11, 11);
	PrintWeapon(weaponModel[1], 15);
	Gotoxy(22, 13);
	printf("<저격소총>  권총보다 공격력과 내구도가 높은 무기");
	Gotoxy(11, 15);
	PrintWeapon(weaponModel[2], 15);
	Gotoxy(22, 17);
	printf("<기관단총>  총 중에서 공격력과 내구도가 가장 높은 무기");
	Gotoxy(11, 19);
	PrintWeapon(weaponModel[3], 15);
	Gotoxy(22, 21);
	printf("    <대포>  총보다 공격력과 내구도가 높으나 연사속도가 느린 무기");
	Gotoxy(11, 24);
	PrintWeapon(weaponModel[4], 15);
	Gotoxy(22, 25);
	printf("    <탱크>  공격력과 내구도가 가장 높고 연사속도가 빠른 무기");
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
	printf("              <조작키 설명>");
	Gotoxy(10, s++);
	textcolor(15);
	printf("◇ 무기 설치						◇ 기본"); //TAB X 6
	Gotoxy(10, s++);
	printf("\n");
	Gotoxy(10, s++);
	printf("< 1 >	: 권총						   △");
	Gotoxy(10, s++);
	printf("< 2 >	: 저격소총					◁ ▽ ▷	: 인디케이터 이동");
	Gotoxy(10, s++);
	printf("< 3 >	: 기관단총");
	Gotoxy(10, s++);
	printf("< 4 >	: 대포						BACKSPACE	: 무기 삭제");
	Gotoxy(10, s++);
	printf("< 5 >	: 탱크						SPACE BAR	: 아이템 획득");
	Gotoxy(10, s++);
	printf("							ESC		: 게임 종료         ");
	Gotoxy(10, s++);
	printf("\n");
	Gotoxy(10, s++);
	printf("\n");
	Gotoxy(10, s++);
	printf("◇ 어시스트 사용");
	Gotoxy(10, s++);
	printf("\n");
	Gotoxy(10, s++);
	printf("< Q >	: 기항 - 제트기					");
	Gotoxy(10, s++);
	printf("	ㄴ 해당 라인의 로봇에게 일정 데미지를 입힌다");
	Gotoxy(10, s++);
	printf("< W >	: 세종스포츠 - 물리치료");
	Gotoxy(10, s++);
	printf("	ㄴ 배치되어 있는 모든무기의 내구도를 일정 회복시킨다");
	Gotoxy(10, s++);
	printf("< E >	: 법학과 - 심판");
	Gotoxy(10, s++);
	printf("	ㄴ 보드에 있는 모든 로봇에게 일정 데미지를 입힌다");
	Gotoxy(75, 27);
	textcolor(13);
	printf("press 'SPACE BAR' to start game");
	textcolor(15);
}

void endDraw() {
	Gotoxy(25, 5); //여기가 위치가 제일 좋은것 같아서 시작위치로 정해놨어! 한줄마다 y좌표만 플러스해주면됌!
	textcolor(11); //경인아 일단 색은 11로 정해놨는데 전체적으로 이색으로해도좋고 한줄씩 혹은 그냥 경인쓰 맘대로 바꿔도 좋고!

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
	printf("만든이 : 김경인, 김민채, 심지혜, 윤금제\n");
	Gotoxy(33, 16);
	printf("    BGM 출처 : www.bensound.com\n");
	Gotoxy(33, 17);
	printf(" 효과음 출처 : http://soundbible.com/\n");

	Gotoxy(70, 25);
	textcolor(7); //계속하려면~
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
	textcolor(7); //계속하려면~

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
	printf("만든이 : 김경인, 김민채, 심지혜, 윤금제\n");
	Gotoxy(34, 16);
	printf("    BGM 출처 : www.bensound.com\n");
	Gotoxy(34, 17);
	printf(" 효과음 출처 : http://soundbible.com/\n");

	Gotoxy(70, 25);
	textcolor(8); //계속하려면~

	return;
}

void textcolor(int color_number) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_number);
}

void RemoveCursor() // 커서 없애는 함수
{
	CONSOLE_CURSOR_INFO curInfo;

	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

COORD GetCurrentCursorPos() // 현재 커서 위치 반환하는 함수
{
	COORD curPoint;
	CONSOLE_SCREEN_BUFFER_INFO curInfo;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curPoint.X = curInfo.dwCursorPosition.X;
	curPoint.Y = curInfo.dwCursorPosition.Y;

	return curPoint;
}

void Gotoxy(int x, int y) // 커서 위치 설정하는 함수(SetCurrentCursorPos()는 너무 길어서 Gotoxy로 대체함)
{
	COORD pos = { x, y };

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void DrawBoard() // 초기 보드 그리는 함수(stage == 1)
{
	int i, j, tmp;
	int x, y;
	system("cls");
	// 어시스트
	Gotoxy(GBOARD_ORIGIN_X + 1, GBOARD_ORIGIN_Y - 1);
	printf("Assist ");
	for (i = 0; i < 15; i++)
	{
		printf("□");
	}

	// 골드
	Gotoxy(GBOARD_ORIGIN_X + 45, GBOARD_ORIGIN_Y - 1);
	printf("Gold 300");

	// 스테이지
	Gotoxy(GBOARD_ORIGIN_X + 79, GBOARD_ORIGIN_Y - 1);
	printf("STAGE NORMAL");

	// 남은 로봇 수
	Gotoxy(GBOARD_ORIGIN_X + 92, GBOARD_ORIGIN_Y + 10);
	printf("Target Robot");
	Gotoxy(GBOARD_ORIGIN_X + 97, GBOARD_ORIGIN_Y + 12);
	printf("%d", robot);

	// 게임 보드
	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y);
	printf("┌");
	for (i = 1; i < 90; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X + i, GBOARD_ORIGIN_Y);
		printf("─");
	}
	printf("┐");

	for (i = 1; i < 20; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + i);
		printf("│");
	}

	for (i = 1; i < 20; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X + 90, GBOARD_ORIGIN_Y + i);
		printf("│");
	}

	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 20);
	printf("└");
	for (i = 1; i < 90; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X + i, GBOARD_ORIGIN_Y + 20);
		printf("─");
	}
	printf("┘");

	for (i = 1; i <= 3; i++)
	{
		Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 5 * i);
		printf("├");
		for (j = 1; j < 90; j++)
		{
			Gotoxy(GBOARD_ORIGIN_X + j, GBOARD_ORIGIN_Y + 5 * i);
			if (j % 10 == 0)
				printf("┼");
			else
				printf("─");
		}
		printf("┤");
	}

	// 무기 정보
	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 21);
	printf("┌");
	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 21);
	for (i = 1; i < 55; i++) {
		Gotoxy(GBOARD_ORIGIN_X + i, GBOARD_ORIGIN_Y + 21);
		printf("─");
	}
	printf("┐");
	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 23);
	for (i = 1; i < 55; i++) {
		Gotoxy(GBOARD_ORIGIN_X + i, GBOARD_ORIGIN_Y + 23);
		printf("─");
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
			printf("│");
		}

		tmp += 11;
	}

	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 25);
	printf("└");
	Gotoxy(GBOARD_ORIGIN_X, GBOARD_ORIGIN_Y + 25);
	for (i = 1; i < 55; i++) {
		Gotoxy(GBOARD_ORIGIN_X + i, GBOARD_ORIGIN_Y + 25);
		printf("─");
	}
	printf("┘");


	// 스킬 정보
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 21);
	printf("┌");
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 21);
	for (i = 1; i < 33; i++) {
		Gotoxy(GBOARD_ORIGIN_X + 57 + i, GBOARD_ORIGIN_Y + 21);
		printf("─");
	}
	printf("┐");
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 23);
	for (i = 1; i < 33; i++) {
		Gotoxy(GBOARD_ORIGIN_X + 57 + i, GBOARD_ORIGIN_Y + 23);
		printf("─");
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
			printf("│");
		}

		tmp += 11;
	}
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 25);
	printf("└");
	Gotoxy(GBOARD_ORIGIN_X + 57, GBOARD_ORIGIN_Y + 25);
	for (i = 1; i < 33; i++) {
		Gotoxy(GBOARD_ORIGIN_X + 57 + i, GBOARD_ORIGIN_Y + 25);
		printf("─");
	}
	printf("┘");


	// gameBoardInfo에 정보 추가
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
	line = rand() % 4; // 몇번째 줄에 로봇이 생성될지 랜덤으로 결정

	x = GBOARD_ORIGIN_X + 82;
	y = GBOARD_ORIGIN_Y + 1 + 5 * line;

	if (isEmpty(x, y)) // 빈칸 검사
	{
		r[nr] = (ROBOT*)malloc(sizeof(ROBOT)); // 로봇 동적할당

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

		Gotoxy(r[nr]->x, r[nr]->y); // 자기 위치로 이동
		PrintRobot(robotModel[r[nr]->model], 15);
		AddRobotToBoard(r[nr]->model, r[nr]->x, r[nr]->y); // 보드 정보에 로봇모델 추가

		nr++; // 로봇 개수 추가
	}
}

void PrintRobot(char robotInfo[4][4], int color) // 로봇 그리는 함수
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
				printf("○");
			}
			else if (robotInfo[y][x] == 3)
			{
				printf("□");
			}
			else if (robotInfo[y][x] == 4)
			{
				printf("△");
			}
			else if (robotInfo[y][x] == 5)
			{
				printf("◀");
			}
			else if (robotInfo[y][x] == 6)
			{
				printf("●");
			}
			else if (robotInfo[y][x] == 7)
			{
				printf("■");
			}
			else if (robotInfo[y][x] == 8)
			{
				printf("▲");
			}
			else if (robotInfo[y][x] == 9)
			{
				printf("◁");
			}
			else if (robotInfo[y][x] == 10)
			{
				printf("∠");
			}
			else if (robotInfo[y][x] == 11)
			{
				printf("≪");
			}
			else if (robotInfo[y][x] == 12)
			{
				printf("┘ ");
			}
			else if (robotInfo[y][x] == 13)
			{
				printf("│ ");
			}
			else if (robotInfo[y][x] == 14)
			{
				printf("≫");
			}
			else if (robotInfo[y][x] == 15)
			{
				printf("＾");
			}
			else if (robotInfo[y][x] == 16)
			{
				printf("▣");
			}
			else if (robotInfo[y][x] == 17)
			{
				printf("∇");
			}
		}
	}
	Gotoxy(curPos.X, curPos.Y);
	textcolor(15);
}

void AddRobotToBoard(int model, int curPosX, int curPosY) // 보드 정보에 로봇모델 추가하는 함수
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

	for (i = 0; i < nr; i++) // 로봇 탐색
	{
		if (r[i]->hp <= 0) // hp 검사
		{
			DeleteRobot(i, robotModel[r[i]->model]);
			i--;
			continue;
		}

		else // 체력이 0 이상이면 충돌 체크
		{
			collision = DetectCollision(i, r[i]->x, r[i]->y);

			if (collision == -1) // 충돌하지 않으면
			{
				MoveRobot(r[i]); // 로봇 움직이기
			}
			else if (collision >= 0) // 무기와 충돌하면
			{
				AttackWeapon(r[i], collision); // 무기 공격
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

	// 재화
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

void MoveRobot(ROBOT* r) // 1116 수정
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

// 1116 로봇이 무기를 공격하는 함수
void AttackWeapon(ROBOT* r, int idx)
{
	int x, y;

	w[idx]->durability -= r->attack;

	// 무기가 공격 당했을 때 색상이 한 번 깜빡이도록(가시화)
	Gotoxy(w[idx]->x, w[idx]->y);
	PrintWeapon(weaponModel[w[idx]->model], 4);

	// 공격당한 무기의 내구도가 0 이하가 되면 삭제
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

	// 로봇과 총알 충돌시 내구도 감소 및 검사 후 내구도가 0 이상이면 다른 충돌 검사(구현 필요)
	for (i = 0; i < nb; i++)
	{
		if (b[i]->x >= x && (b[i]->y - 2 == y || b[i]->y - 1 == y))	// 로봇 앞에 총알이 닿으면
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
			// 로봇이 공격 당했을 때 색상이 한 번 깜빡이도록(가시화)
			Gotoxy(r[n]->x, r[n]->y);
			PrintRobot(robotModel[r[n]->model], 4);
			DeleteBullet(i);
			i--;
		}
	}
	// 로봇과 아이템 충돌 시 아이템 삭제 후 다른 충돌 검사
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

	// 로봇과 로봇 충돌 검사
	for (i = 0; i < 4; i++)
	{
		if (gameBoardInfo[arrY + i][arrX - 1] >= 2 && gameBoardInfo[arrY + i][arrX - 1] <= 17)
		{
			flag = -2;
			break;
		}
	}

	if (flag != -2) // 로봇 앞에 로봇이 없다면
	{
		// 로봇과 무기 충돌하면 무기 배열 인덱스 반환
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

	return flag; // 충돌 상태 확인해서 리턴
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
		AddWeaponToBoard(w[nw]->model, w[nw]->x, w[nw]->y); // 보드 정보에 로봇모델 추가

		nw++; // 무기 개수 추가

		PlaySound(TEXT("./sound/weapon.wav"), NULL, SND_ASYNC);
	}
	else
		PlaySound(TEXT("./sound/dontweapon.wav"), NULL, SND_ASYNC);
}

void PrintWeapon(char weaponInfo[4][4], int color) // 무기 그리는 함수
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
				printf("■");
			}
			else if (weaponInfo[y][x] == 21)
			{
				printf("√");
			}
			else if (weaponInfo[y][x] == 22)
			{
				printf("＝");
			}
			else if (weaponInfo[y][x] == 23)
			{
				printf("≤");
			}
			else if (weaponInfo[y][x] == 24)
			{
				printf("┴");
			}
			else if (weaponInfo[y][x] == 25)
			{
				printf("≪");
			}
			else if (weaponInfo[y][x] == 26)
			{
				printf("∠");
			}
			else if (weaponInfo[y][x] == 27)
			{
				printf("〃");
			}
			else if (weaponInfo[y][x] == 28)
			{
				printf("┌");
			}
			else if (weaponInfo[y][x] == 29)
			{
				printf("∈");
			}
			else if (weaponInfo[y][x] == 30)
			{
				printf("∋");
			}
			else if (weaponInfo[y][x] == 31)
			{
				printf("─");
			}
			else if (weaponInfo[y][x] == 32)
			{
				printf("▲");
			}
		}
	}
	Gotoxy(curPos.X, curPos.Y);
	textcolor(15);
}

void AddWeaponToBoard(int model, int curPosX, int curPosY) // 보드 정보에 무기모델 추가하는 함수
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

void DeleteWeapon(int index, char weaponInfo[4][4], int f) // DeleteWeapon함수가 두 군데서 이용되는데 전체적인 틀은 같고 세부적인 일부만 달라서 플래그 = 1 or 2로 구분
{														   // f = 1: BackSpace 받았을 시 삭제 f = 2: 내구도가 0 이하가 됐을 시 삭제
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

	if (isEmpty(x, y)) // 빈칸이면 -1 반환
		return -1;

	for (i = 0; i < nw; i++) // 무기 구조체 포인터 배열을 탐색
	{
		if (w[i]->x == x && w[i]->y == y) // 현재 커서 위치에 배치된 무기 찾기
		{
			return i; // 무기 모델 인덱스 반환
		}
	}

	return -1; // 로봇 모델이면 -1 반환
}

void ProcessKeyInput()
{
	int key;
	int i, n, index = -1, line;
	int useSkill, countUse;
	COORD curPos, curPos2;

	Gotoxy(indicatorPos.X, indicatorPos.Y); // 저장된 인디케이터의 위치로 커서 설정
	curPos = GetCurrentCursorPos(); // 현재 위치 저장
	textcolor(4); // 글씨 빨간색으로 설정
	printf("────");

	for (i = 0; i < 20; i++)
	{
		if (_kbhit() != 0)
		{
			key = _getch();

			// 인디케이터 이동
			if (key == RIGHT) // 오른쪽 방향키
			{
				if (hcnt == 7)
					PlaySound(TEXT("./sound/indicator.wav"), NULL, SND_ASYNC);
				if (hcnt != 7)
				{
					Gotoxy(indicatorPos.X, indicatorPos.Y);
					textcolor(15);
					printf("────");  // 이전 인디케이터 지우기(흰색으로 덮어쓰기)
					Gotoxy(indicatorPos.X + 10, indicatorPos.Y);
					curPos = GetCurrentCursorPos();
					textcolor(4);
					printf("────");
					indicatorPos = curPos;
					hcnt++;
				}
			}

			else if (key == LEFT) // 왼쪽 방향키
			{
				if (hcnt == 1)
					PlaySound(TEXT("./sound/indicator.wav"), NULL, SND_ASYNC);
				if (hcnt != 1)
				{
					Gotoxy(indicatorPos.X, indicatorPos.Y);
					textcolor(15);
					printf("────");
					Gotoxy(indicatorPos.X - 10, indicatorPos.Y);
					curPos = GetCurrentCursorPos();
					textcolor(4);
					printf("────");
					indicatorPos = curPos;
					hcnt--;
				}
			}

			else if (key == UP) // 위 방향키
			{
				if (vcnt == 1)
					PlaySound(TEXT("./sound/indicator.wav"), NULL, SND_ASYNC);
				if (vcnt != 1)
				{
					Gotoxy(indicatorPos.X, indicatorPos.Y);
					textcolor(15);
					printf("────");
					Gotoxy(indicatorPos.X, indicatorPos.Y - 5);
					curPos = GetCurrentCursorPos();
					textcolor(4);
					printf("────");
					indicatorPos = curPos;
					vcnt--;
				}
			}

			else if (key == DOWN) // 아래 방향키
			{
				if (vcnt == 4)
					PlaySound(TEXT("./sound/indicator.wav"), NULL, SND_ASYNC);
				if (vcnt != 4)
				{
					Gotoxy(indicatorPos.X, indicatorPos.Y);
					textcolor(15);
					printf("────");
					Gotoxy(indicatorPos.X, indicatorPos.Y + 5);
					curPos = GetCurrentCursorPos();
					textcolor(4);
					printf("────");
					indicatorPos = curPos;
					vcnt++;
				}
			}


			// 무기 배치
			else if ((49 <= key && key <= 50) && stage == 1) // 1~2번 - 스테이지1
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

			else if ((49 <= key && key <= 52) && stage == 2) // 1~4번 - 스테이지2
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

			else if ((49 <= key && key <= 53) && stage == 3) // 1~5번 - 스테이지3
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


			// 무기 삭제
			else if (key == BACKSPACE) // 백스페이스
			{
				curPos2.X = curPos.X - 2;
				curPos2.Y = curPos.Y + 1;

				index = WhichWeapon(curPos2.X, curPos2.Y); // 구조체 포인터 배열 인덱스
				if (!isEmpty(curPos2.X, curPos2.Y) && index != -1)
				{
					Gotoxy(curPos2.X, curPos2.Y);
					DeleteWeapon(index, weaponModel[w[index]->model], 1);

					Gotoxy(curPos2.X + 3, curPos2.Y - 1);
				}
			}


			// 스킬

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


			else if (key == 113 && assist[9] == 1) // Q(제트기) 
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
					line = vcnt - 1; // 몇 번째 라인(0~3)
					// 해당 라인의 첫번째 칸으로 이동
					curPos2.X = GBOARD_ORIGIN_X + 2;
					curPos2.Y = GBOARD_ORIGIN_Y + 1 + 5 * line;


					for (i = 0; i < 40; i++) // 스킬 이펙트
					{
						Gotoxy(curPos2.X + 2 * i, curPos2.Y);
						PrintJet();
						Sleep(10);
						DeleteJet();
						if (_kbhit() != 0) // 스킬이 발동하는 동안 키보드 입력 무시
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
							r[i]->hp -= 5; // 로봇 체력 깎기
						}
					}

					textcolor(15);
				}
			}

			else if (key == 119 && stage >= 2) // W(물리치료)
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
						w[i]->durability += 5; // 무기 내구도 회복
					}
					textcolor(15);
				}
			}

			else if (key == 101 && stage == 3) // E(심판)
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
						r[i]->hp -= 5; // 로봇 체력 깎기
					}
					textcolor(15);
				}
			}


			// 아이템 획득
			else if (key == SPACE) // 스페이스바
			{
				curPos2.X = curPos.X - 2;
				curPos2.Y = curPos.Y + 1;

				index = WhichItem(curPos2.X, curPos2.Y); // 구조체 포인터 배열 인덱스
				if (!isEmpty(curPos2.X, curPos2.Y) && index != -1)
				{
					Gotoxy(curPos2.X, curPos2.Y);
					PlaySound(TEXT("./sound/eatitem.wav"), NULL, SND_ASYNC);
					DeleteItem(index, 1);

					Gotoxy(curPos2.X + 3, curPos2.Y - 1);
				}
			}


			// 게임 종료
			else if (key == ESC) // ESC
			{
				textcolor(7);
				system("cls");
				endDraw();
				exit(-1);
			}
		}

		Sleep(speed); // 스피드 조정
	}
}

int isEmpty(int curPosX, int curPosY) // 빈칸 체크 함수(1 - 빈칸, 0 - 빈칸X)
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

int isGameOver()  // 1116 수정한 부분: 로봇이 벽을 뚫고가길래 안 뚫고 가게 수정해놨음!
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
			b[nb] = (BULLET*)malloc(sizeof(BULLET)); // 총알 하나 동적할당
			w[i]->durability--;
			if (w[i]->model == 0) // 1번 무기
			{
				b[nb]->x = w[i]->x + 6;
				b[nb]->y = w[i]->y + 2;
				b[nb]->model = 0;
				Gotoxy(b[nb]->x, b[nb]->y);
				w[i]->bullettime = 5;
			}

			else if (w[i]->model == 1) // 2번 무기
			{
				b[nb]->x = w[i]->x + 6;
				b[nb]->y = w[i]->y + 2;
				b[nb]->model = 1;
				Gotoxy(b[nb]->x, b[nb]->y);
				w[i]->bullettime = 5;
			}

			else if (w[i]->model == 2) // 3번 무기
			{
				b[nb]->x = w[i]->x + 6;
				b[nb]->y = w[i]->y + 2;
				b[nb]->model = 2;
				Gotoxy(b[nb]->x, b[nb]->y);
				w[i]->bullettime = 5;
			}

			else if (w[i]->model == 3) // 4번 무기
			{
				b[nb]->x = w[i]->x + 6;
				b[nb]->y = w[i]->y + 1;
				b[nb]->model = 3;
				Gotoxy(b[nb]->x, b[nb]->y);
				w[i]->bullettime = 7;
			}

			else if (w[i]->model == 4) // 5번 무기
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
			printf("①");
		else if (b[i]->model == 1)
			printf("②");
		else if (b[i]->model == 2)
			printf("③");
		else if (b[i]->model == 3)
			printf("④");
		else if (b[i]->model == 4)
			printf("⑤");

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
		line = rand() % 4; // 몇번째 줄에 
		kan = rand() % 7; // 몇번째 칸에 아이템이 생성될지 결정

		x = GBOARD_ORIGIN_X + 2 + 10 * kan;
		y = GBOARD_ORIGIN_Y + 1 + 5 * line;

		if (isEmpty(x, y)) //빈칸이면 
			break;
	}

	it[ni] = (ITEM*)malloc(sizeof(ITEM)); // 아이템 동적할당
	it[ni]->x = x;
	it[ni]->y = y;

	while (1)
	{
		it[ni]->gold = rand() % 50;
		it[ni]->assist = rand() % 3;

		if ((it[ni]->gold) != 0 && (it[ni]->assist) != 0)
			break;
	}

	it[ni]->time = 8; // 아이템 8초 지나면 사라짐

	Gotoxy(it[ni]->x, it[ni]->y); // 자기 위치로 이동
	PrintItem();
	AddItemToBoard(it[ni]->x, it[ni]->y); // 보드 정보에 아이템 모델 추가
	textcolor(15);
	ni++; // 아이템 개수 추가
}

void PrintItem() // 아이템 그리는 함수
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
				printf("【");
			}
			else if (itemModel[y][x] == 36)
			{
				printf("??");
			}
			else if (itemModel[y][x] == 37)
			{
				printf("】");
			}
		}
	}
	Gotoxy(curPos.X, curPos.Y);
}

void AddItemToBoard(int curPosX, int curPosY) // 보드 정보에 아이템 모델 추가하는 함수
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

void CheckItem() // 시간 지나면 아이템 사라지는 함수
{
	int i;
	int index;

	for (i = 0; i < ni; i++)
	{
		if ((it[i]->time) == 0)
		{
			index = WhichItem(it[i]->x, it[i]->y); // 구조체 포인터 배열 인덱스
			if (!isEmpty(it[i]->x, it[i]->y) && index != -1)
			{
				Gotoxy(it[i]->x, it[i]->y);
				DeleteItem(index, 0);
				i--;
				continue;
			}
		}

		if (t % 1 == 0) //1초마다 검사
		{
			for (i = 0; i < ni; i++)
				(it[i]->time) -= 1;
		}
	}
}

void DeleteItem(int index, int f) //1. space바로 먹으면 골드 및 어시스트 갱신됨  2. 일정시간 지나면 사라짐   3. 로봇과 충돌하면 사라짐
{                                 // 1번 -> f = 1   2번, 3번 -> f = 0     3번은 일단 로봇충돌처리에서 어떻게 어디까지 처리되는 보고 결정!
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

	if (isEmpty(x, y))  // 빈칸이면 -1 반환
		return -1;

	for (i = 0; i < ni; i++) // 아이템 구조체 포인터 배열을 탐색
	{
		if (it[i]->x == x && it[i]->y == y) // 현재 커서 위치에 배치된 아이템 찾기
		{
			return i; // 아이템 모델 인덱스 반환
		}
	}

	return -1; // 아이템 모델 아니면 -1 반환
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
				printf("＞");
			}
			else if (jetModel[y][x] == 2)
			{
				printf("▶");
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

void RedrawBoard() // 업데이트된 보드 정보에 따라 보드 다시 그리는 함수
{
	int i;
	int x, y;
	int cursX, cursY;

	textcolor(15);
	// 어시스트
	Gotoxy(GBOARD_ORIGIN_X + 8, GBOARD_ORIGIN_Y - 1);
	for (i = 0; i < 15; i++) { // Redraw 해야됨
		if (assist[i] == 1)
			printf("■");
		else
			printf("□");
	}

	// 골드
	Gotoxy(GBOARD_ORIGIN_X + 45, GBOARD_ORIGIN_Y - 1);
	if (gold < 10)
		printf("Gold %d   ", gold);
	else if (gold < 100)
		printf("Gold %d  ", gold);
	else if (gold < 1000)
		printf("Gold %d ", gold);
	else if (gold < 10000)
		printf("Gold %d", gold);


	// 스테이지
	Gotoxy(GBOARD_ORIGIN_X + 79, GBOARD_ORIGIN_Y - 1);
	printf("STAGE");
	if (stage == 2)
		printf(" HARD  ");
	else if (stage == 3)
		printf(" HELL  ");

	// 남은 로봇 수
	Gotoxy(GBOARD_ORIGIN_X + 92, GBOARD_ORIGIN_Y + 10);
	printf("Target Robot");
	Gotoxy(GBOARD_ORIGIN_X + 97, GBOARD_ORIGIN_Y + 12);
	if (robot < 10)
		printf("%d ", robot);
	else if (robot < 100)
		printf("%d", robot);

	// 무기, 스킬 정보
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

			if (y == 5 || y == 10 || y == 15) // 사이사이에 있는 가로선은 다시 그리지 않음
				continue;

			if (gameBoardInfo[y][x] == 0)
			{
				printf("  ");
			}
			else if (gameBoardInfo[y][x] == 2)
			{
				printf("○");
			}
			else if (gameBoardInfo[y][x] == 3)
			{
				printf("□");
			}
			else if (gameBoardInfo[y][x] == 4)
			{
				printf("△");
			}
			else if (gameBoardInfo[y][x] == 5)
			{
				printf("◀");
			}
			else if (gameBoardInfo[y][x] == 6)
			{
				printf("●");
			}
			else if (gameBoardInfo[y][x] == 7)
			{
				printf("■");
			}
			else if (gameBoardInfo[y][x] == 8)
			{
				printf("▲");
			}
			else if (gameBoardInfo[y][x] == 9)
			{
				printf("◁");
			}
			else if (gameBoardInfo[y][x] == 10)
			{
				printf("∠");
			}
			else if (gameBoardInfo[y][x] == 11)
			{
				printf("≪");
			}
			else if (gameBoardInfo[y][x] == 12)
			{
				printf("┘ ");
			}
			else if (gameBoardInfo[y][x] == 13)
			{
				printf("│ ");
			}
			else if (gameBoardInfo[y][x] == 14)
			{
				printf("≫");
			}
			else if (gameBoardInfo[y][x] == 15)
			{
				printf("＾");
			}
			else if (gameBoardInfo[y][x] == 16)
			{
				printf("▣");
			}
			else if (gameBoardInfo[y][x] == 17)
			{
				printf("∇");
			}
			else if (gameBoardInfo[y][x] == 20)
			{
				printf("■");
			}
			else if (gameBoardInfo[y][x] == 21)
			{
				printf("√");
			}
			else if (gameBoardInfo[y][x] == 22)
			{
				printf("＝");
			}
			else if (gameBoardInfo[y][x] == 23)
			{
				printf("≤");
			}
			else if (gameBoardInfo[y][x] == 24)
			{
				printf("┴");
			}
			else if (gameBoardInfo[y][x] == 25)
			{
				printf("≪");
			}
			else if (gameBoardInfo[y][x] == 26)
			{
				printf("∠");
			}
			else if (gameBoardInfo[y][x] == 27)
			{
				printf("〃");
			}
			else if (gameBoardInfo[y][x] == 28)
			{
				printf("┌");
			}
			else if (gameBoardInfo[y][x] == 29)
			{
				printf("∈");
			}
			else if (gameBoardInfo[y][x] == 30)
			{
				printf("∋");
			}
			else if (gameBoardInfo[y][x] == 31)
			{
				printf("─");
			}
			else if (gameBoardInfo[y][x] == 32)
			{
				printf("▲");
			}
			else if (gameBoardInfo[y][x] == 35)
			{
				textcolor(14);
				printf("【");
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
				printf("】");
				textcolor(15);
			}
		}
	}
}


/* C语言代码原作者：Toby-DWT
C++修改者：东风九号
使用AI辅助 */ 

#include <iostream>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <conio.h>
#include <windows.h>
#include <ctime>
using namespace std;

#define ROW 17
#define COL 82
#define middle_col 40
#define left_col 16
#define right_col 65
#define $_left 35
#define $_middle 40
#define $_right 46

#define running 0
#define jump 1
#define down 2
#define middle 3

#define none 0
#define invincible 4
#define flying 5
#define super_jump 6
#define magnet 7
#define double_score 8
#define double_coin 9

// 控制台颜色常量定义
#define WHITE 7
#define PINK 13        // ^ 粉色
#define LIGHT_BLUE 11  // # 浅蓝色
#define LIGHT_GREEN 10 // _ 浅绿色
#define RED 12         // x 红色
#define YELLOW 14      // $ 金币黄色
#define PURPLE 5       // 道具紫色

struct Node {
	int material_row;
	int material_col;
	int if_eaten;
	char hinder_type;
	int prop_type;
	Node* next;
};

struct LinkedList {
	Node* head;
	Node* tail;
	LinkedList() : head(nullptr), tail(nullptr) {}
};

char map[ROW][COL];
int head_col;
int head_row;
long long score;
long long coin_nums;
int score_add;
int run;
int speed;
int speed_limit;
int speed_hinder_happen;
int speed_prop_happen;
int lastLane = -1;
char lastHinder = 0;
// 金币均衡控制
int lastCoinLane = -1;    // 上一次金币车道 0左 1中 2右
int coinCycleCnt = 0;     // 轮换计数器，强制三条轮流
const int COIN_SPAWN_STEP = 3; // 每3轮强制换车道

LinkedList list_coin;
LinkedList list_hinder;
LinkedList list_prop;

int move_state;
int move_state_time;
int move_state_time_limit;
int prop_state;
int prop_state_time;
int prop_state_time_limit;

int $_place[3] = {$_left, $_middle, $_right};
char hinder[4] = {'^','_','#','x'};
int prop[6] = {invincible,flying,super_jump,magnet,double_score,double_coin};

bool lose;
bool replay = true;

void ClearScreen()
{
    COORD coordScreen = { 0, 0 };
    DWORD dwCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 获取缓冲区总字符数
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // 用空格填满整个缓冲区
    FillConsoleOutputCharacter(hConsole, ' ', dwConSize, coordScreen, &dwCharsWritten);
    FillConsoleOutputAttribute(hConsole, 0, dwConSize, coordScreen, &dwCharsWritten);

    // 光标放回左上角
    SetConsoleCursorPosition(hConsole, coordScreen);
}

// 控制台光标定位
void gotoxy(int x, int y) {
	COORD pos = { (SHORT)x,(SHORT)y };
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(handle, pos);
}

// 隐藏光标
void HideCursor() {
	CONSOLE_CURSOR_INFO cursor_info = { 1,0 };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

// 设置文字颜色
void SetColor(int color) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
}
// 恢复默认白色
void ResetColor() {
	SetColor(WHITE);
}

// 绘制人物
void PrintSmallGuy() {
	map[head_row][head_col - 1]='(';
	map[head_row][head_col]='*';
	map[head_row][head_col + 1]=')';
	map[head_row + 1][head_col - 1]='/';
	map[head_row + 1][head_col]='|';
	map[head_row + 1][head_col + 1]='\\';
	map[head_row + 2][head_col - 1]='/';
	map[head_row + 2][head_col + 1]='\\';
}

// 清除人物
void ClearSmallGuy() {
	map[head_row][head_col] = ' ';
	map[head_row][head_col - 1] = ' ';
	map[head_row][head_col + 1] = ' ';
	map[head_row + 1][head_col - 1] = ' ';
	map[head_row + 1][head_col] = ' ';
	map[head_row + 1][head_col + 1] = ' ';
	map[head_row + 2][head_col - 1] = ' ';
	map[head_row + 2][head_col + 1] = ' ';
}

// 跑动动画
void PrintRun() {
	if(run == 0) {
		run = 1;
		map[head_row + 2][head_col - 1] = ' ';
		map[head_row + 2][head_col + 1] = '\\';
		map[head_row + 1][head_col + 1] = ' ';
		map[head_row + 1][head_col - 1] = '/';
	} else if(run == 1) {
		run = 0;
		map[head_row + 2][head_col + 1] = ' ';
		map[head_row + 2][head_col - 1] = '/';
		map[head_row + 1][head_col + 1] = '\\';
		map[head_row + 1][head_col - 1] = ' ';
	}
}

// 跳跃绘制
void PrintJump() {
	PrintSmallGuy();
	map[head_row + 2][head_col - 1] = '-';
	map[head_row + 2][head_col + 1] = '-';
}

// 下蹲绘制
void PrintSquat() {
	PrintSmallGuy();
	map[head_row + 1][head_col - 1] = '-';
	map[head_row + 1][head_col + 1] = '-';
	map[head_row + 2][head_col - 1] = '-';
	map[head_row + 2][head_col + 1] = '-';
}

// 滑翔绘制
void PaintGliding() {
	PrintSmallGuy();
	map[head_row + 1][head_col] = '&';
	map[head_row + 1][head_col - 1] = '/';
	map[head_row + 1][head_col + 1] = '\\';
	map[head_row + 2][head_col - 1] = ' ';
	map[head_row + 2][head_col + 1] = ' ';
}

// 获取 [min,max] 均匀随机整数
int RandInt(int min, int max) {
	return rand() % (max - min + 1) + min;
}

void ClearCoinHinder(const Node* node, char letter) {
	if(node->material_row <= 15 && map[node->material_row][node->material_col] == letter) {
		map[node->material_row][node->material_col] = ' ';
	}
}

void ClearProp(const Node* node) {
	if(node->material_row <= 15) {
		map[node->material_row][node->material_col] = ' ';
		map[node->material_row][node->material_col + 1] = ' ';
	}
}

void PaintProp(const Node* node, int p) {
	if(node->material_row <= 15) {
		if (p == invincible) {
			map[node->material_row][node->material_col] = '@';
			map[node->material_row][node->material_col + 1] = '@';
		} else if (p == flying) {
			map[node->material_row][node->material_col] = '>';
			map[node->material_row][node->material_col + 1] = '<';
		} else if (p == super_jump) {
			map[node->material_row][node->material_col] = 'd';
			map[node->material_row][node->material_col + 1] = 'b';
		} else if (p == magnet) {
			map[node->material_row][node->material_col] = 'U';
			map[node->material_row][node->material_col + 1] = 'U';
		} else if (p == double_score) {
			map[node->material_row][node->material_col] = 'X';
			map[node->material_row][node->material_col + 1] = '2';
		} else if (p == double_coin) {
			map[node->material_row][node->material_col] = 'X';
			map[node->material_row][node->material_col + 1] = '$';
		}
	}
}

void ClearHinderPerspective(const Node* node) {
	if(node->material_row == 2 || node->material_row == 3 || node->material_row == 4) {
		map[node->material_row][node->material_col-1] = ' ';
	} else if(node->material_row == 5 || node->material_row == 6 || node->material_row == 7) {
		map[node->material_row][node->material_col-2] = ' ';
		map[node->material_row][node->material_col-1] = ' ';
	} else if(node->material_row == 8 || node->material_row == 9 || node->material_row == 10) {
		map[node->material_row][node->material_col-2] = ' ';
		map[node->material_row][node->material_col-1] = ' ';
		map[node->material_row][node->material_col+2] = ' ';
		map[node->material_row][node->material_col+3] = ' ';
	} else if(node->material_row >= 11 && node->material_row <= 15) {
		map[node->material_row][node->material_col-3] = ' ';
		map[node->material_row][node->material_col-2] = ' ';
		map[node->material_row][node->material_col-1] = ' ';
		map[node->material_row][node->material_col+2] = ' ';
		map[node->material_row][node->material_col+3] = ' ';
	}
	if(map[node->material_row][node->material_col+1] != '@' && map[node->material_row][node->material_col+1] != '<' &&
	        map[node->material_row][node->material_col+1] != 'b' && map[node->material_row][node->material_col+1] != 'U' &&
	        map[node->material_row][node->material_col+1] != 'X') {
		map[node->material_row][node->material_col+1] = ' ';
	}
}

void PaintCoinHinder(const Node* node, char letter) {
	if(node->material_row <= 15) {
		map[node->material_row][node->material_col] = letter;
	}
}

void PaintHinderPerspective(const Node* node, char letter) {
	if(node->material_row == 2 || node->material_row == 3 || node->material_row == 4) {
		map[node->material_row][node->material_col-1] = letter;
		map[node->material_row][node->material_col+1] = letter;
	} else if(node->material_row == 5 || node->material_row == 6 || node->material_row == 7) {
		map[node->material_row][node->material_col-2] = letter;
		map[node->material_row][node->material_col-1] = letter;
		map[node->material_row][node->material_col+1] = letter;
	} else if(node->material_row == 8 || node->material_row == 9 || node->material_row == 10) {
		map[node->material_row][node->material_col-2] = letter;
		map[node->material_row][node->material_col-1] = letter;
		map[node->material_row][node->material_col+1] = letter;
		map[node->material_row][node->material_col+2] = letter;
		map[node->material_row][node->material_col+3] = letter;
	} else if(node->material_row >= 11 && node->material_row <= 15) {
		map[node->material_row][node->material_col-3] = letter;
		map[node->material_row][node->material_col-2] = letter;
		map[node->material_row][node->material_col-1] = letter;
		map[node->material_row][node->material_col+1] = letter;
		map[node->material_row][node->material_col+2] = letter;
		map[node->material_row][node->material_col+3] = letter;
	}
}

// 超出屏幕销毁节点
void BottomClear(LinkedList* list) {
	Node *cur = nullptr;
	Node *prev = nullptr;
	for (cur = list->head; cur != nullptr; cur = cur->next,prev = cur) {
		if(cur->material_row > 15)
			break;
	}
	if(cur == nullptr) return;
	if(prop_state == magnet) {
		score+=3;
		coin_nums++;
	}
	if(cur == list->head && cur == list->tail) {
		list->head = nullptr;
		list->tail = nullptr;
		delete cur;
		return;
	} else if(cur == list->head) {
		list->head = cur->next;
		delete cur;
		return;
	} else if(cur == list->tail) {
		list->tail = prev;
		prev->next = nullptr;
		delete cur;
		return;
	}
	prev->next = cur->next;
	delete cur;
}

void SpeedRun() {
	PrintSmallGuy();
	PrintRun();
	if((score >= 150 && score < 300 && speed_limit == 5)
	        || (score >= 300 && score < 600 && speed_limit == 4)
	        || (score >= 600 && score < 1000 && speed_limit == 3)
	        || (score >= 1000 && speed_limit == 2)) {
		speed_limit--;
	}
}

// 链表尾插
void PutIn(Node* node, LinkedList* list) {
	if(list->head == nullptr) {
		list->head = node;
		list->tail = node;
		return;
	}
	list->tail->next = node;
	list->tail = node;
}

void MoveStateTimeCountdown() {
	if(move_state_time++ < move_state_time_limit);
	else {
		move_state_time = 1;
		move_state = running;
	}
	if(speed_limit == 5) {
		move_state_time_limit = 20 + 2;
	} else if(speed_limit == 4) {
		move_state_time_limit = 16 + 2;
	} else if(speed_limit == 3) {
		move_state_time_limit = 12 + 2;
	} else if(speed_limit == 2) {
		move_state_time_limit = 8 + 2;
	} else if(speed_limit == 1) {
		move_state_time_limit = 6;
	}
}

void PropStateTimeCountdown() {
	if(prop_state_time < prop_state_time_limit) {
		prop_state_time++;
		if(prop_state_time == 28 * speed_limit || prop_state_time == 26 * speed_limit) {
			ClearScreen();
		}
	} else {
		prop_state_time = 1;
		ClearScreen();
		prop_state = none;
	}
	if(speed_limit == 4) {
		prop_state_time_limit = 120;
	} else if(speed_limit == 3) {
		prop_state_time_limit = 90;
	} else if(speed_limit == 2) {
		prop_state_time_limit = 60;
	} else if(speed_limit == 1) {
		prop_state_time_limit = 30;
	}
}

// 生成金币
void CoinHappen() {
	// 收集当前空闲车道 0左($_left) 1中($_middle) 2右($_right)
	int emptyLanes[3] = {-1,-1,-1};
	int emptyCnt = 0;
	if(map[1][$_left] == ' ') emptyLanes[emptyCnt++] = 0;
	if(map[1][$_middle] == ' ') emptyLanes[emptyCnt++] = 1;
	if(map[1][$_right] == ' ') emptyLanes[emptyCnt++] = 02;

	if(emptyCnt == 0) return; // 三条全被占用，不生成

	int selectLane;
	coinCycleCnt++;

	// 规则1：连续2次同车道，强制换一条
	if(coinCycleCnt >= 2) {
		do {
			int idx = RandInt(0, emptyCnt - 1);
			selectLane = emptyLanes[idx];
		} while(selectLane == lastCoinLane);
		coinCycleCnt = 0;
	}
	// 规则2：普通随机，但避开上一条
	else {
		int idx = RandInt(0, emptyCnt - 1);
		selectLane = emptyLanes[idx];
		if(selectLane == lastCoinLane && emptyCnt > 1) {
			do {
				idx = RandInt(0, emptyCnt - 1);
				selectLane = emptyLanes[idx];
			} while(selectLane == lastCoinLane);
		}
	}
	lastCoinLane = selectLane;

	// 映射对应列坐标
	int colTarget;
	if(selectLane == 0) colTarget = $_left;
	else if(selectLane == 1) colTarget = $_middle;
	else colTarget = $_right;

	// 新建金币节点
	Node* node = new Node;
	node->material_col = colTarget;
	node->material_row = 1;
	node->if_eaten = 0;
	node->next = nullptr;
	PutIn(node, &list_coin);
	PaintCoinHinder(node, '$');
}

void CoinMove() {
	for (Node *node = list_coin.head; node != nullptr; node = node->next) {
		if(node->material_col <= $_left) {
			ClearCoinHinder(node,'$');
			if(node->material_row % 2 == 1) {
				node->material_col-=2;
			} else if(node->material_row % 2 == 0) {
				node->material_col--;
			}
			node->material_row++;
			PaintCoinHinder(node, '$');
		}
		if(node->material_col == $_middle) {
			ClearCoinHinder(node,'$');
			node->material_row++;
			PaintCoinHinder(node, '$');
		}
		if(node->material_col >= $_right) {
			ClearCoinHinder(node,'$');
			if(node->material_row % 2 == 1) {
				node->material_col+=2;
			} else if(node->material_row % 2 == 0) {
				node->material_col++;
			}
			node->material_row++;
			PaintCoinHinder(node, '$');
		}
	}
}

int CoinEat() {
	Node *cur = list_coin.head;
	for (; cur != nullptr; cur = cur->next) {
		if(cur->material_row == 15
		        && cur->material_col >= head_col - 2
		        && cur->material_col <= head_col + 2
		        && move_state != jump && move_state != middle) {
			cur->if_eaten = 1;
			return 1;
		}
	}
	return 0;
}

void CoinClear() {
	Node *cur;
	Node *prev = nullptr;
	for (cur = list_coin.head; cur != nullptr; cur = cur->next) {
		if(cur->if_eaten == 1) {
			ClearCoinHinder(cur,'$');
			break;
		}
		prev = cur;
	}
	if(cur == nullptr) return;
	if(prop_state == double_coin) {
		score+=score_add;
		coin_nums++;
	}
	score+=score_add;
	coin_nums++;
	if(cur == list_coin.head && cur == list_coin.tail) {
		list_coin.head = nullptr;
		list_coin.tail = nullptr;
		delete cur;
		return;
	} else if(cur == list_coin.head) {
		list_coin.head = cur->next;
		delete cur;
		return;
	} else if(cur == list_coin.tail) {
		list_coin.tail = prev;
		prev->next = nullptr;
		delete cur;
		return;
	}
	prev->next = cur->next;
	delete cur;
}

void HinderHappen() {
	int possible[3] = {-1,-1,-1};
	int availCnt = 0;
	// 遍历三条轨道，收集空位
	if(map[1][$_left] == ' ') possible[availCnt++] = 0;
	if(map[1][$_middle] == ' ') possible[availCnt++] = 1;
	if(map[1][$_right] == ' ') possible[availCnt++] = 02;

	if(availCnt == 0) {
		speed_hinder_happen = 4 * speed_limit;
		return;
	}

	int selLaneIdx;
	int targetLane;
	// 均衡逻辑：优先不选上一次轨道
	if(availCnt > 1) {
		do {
			selLaneIdx = RandInt(0, availCnt - 1);
			targetLane = possible[selLaneIdx];
		} while(targetLane == lastLane);
	} else {
		selLaneIdx = 0;
		targetLane = possible[selLaneIdx];
	}
	lastLane = targetLane;

	// 映射轨道坐标
	int colTarget;
	if(targetLane == 0) colTarget = $_left;
	else if(targetLane == 1) colTarget = $_middle;
	else colTarget = $_right;

	// 防连续同种障碍
	char selHind;
	do {
		int t = RandInt(0,3);
		selHind = hinder[t];
	} while(selHind == lastHinder);
	lastHinder = selHind;

	// 生成节点
	Node *node = new Node;
	node->material_col = colTarget;
	node->material_row = 1;
	node->if_eaten = 0;
	node->hinder_type = selHind;
	node->next = nullptr;
	PutIn(node, &list_hinder);
	PaintCoinHinder(node, node->hinder_type);
}

void HinderMove() {
	for (Node *node = list_hinder.head; node != nullptr; node = node->next) {
		if(node->material_col <= $_left) {
			ClearCoinHinder(node,node->hinder_type);
			ClearHinderPerspective(node);
			if(node->material_row % 2 == 1) {
				node->material_col-=2;
			} else if(node->material_row % 2 == 0) {
				node->material_col--;
			}
			node->material_row++;
			PaintCoinHinder(node, node->hinder_type);
			PaintHinderPerspective(node, node->hinder_type);
		}
		if(node->material_col == $_middle) {
			ClearCoinHinder(node,node->hinder_type);
			ClearHinderPerspective(node);
			node->material_row++;
			PaintCoinHinder(node, node->hinder_type);
			PaintHinderPerspective(node, node->hinder_type);
		}
		if(node->material_col >= $_right) {
			ClearCoinHinder(node,node->hinder_type);
			ClearHinderPerspective(node);
			if(node->material_row % 2 == 1) {
				node->material_col+=2;
			} else if(node->material_row % 2 == 0) {
				node->material_col++;
			}
			node->material_row++;
			PaintCoinHinder(node, node->hinder_type);
			PaintHinderPerspective(node, node->hinder_type);
		}
	}
}

void IfHinderClash() {
	Node *cur = list_hinder.head;
	for (; cur != nullptr; cur = cur->next) {
		if(cur->material_row == 15) break;
	}
	if(cur == nullptr) return;
	if(cur->material_col <= head_col + 2 && cur->material_col >= head_col - 2) {
		if(cur->hinder_type == '_') {
			if(move_state == jump || move_state == middle) return;
			lose = true;
		} else if(cur->hinder_type == '^') {
			if(move_state == down || (move_state == jump && prop_state == super_jump)) return;
			lose = true;
		} else if(cur->hinder_type == 'x') {
			if(move_state == jump && prop_state == super_jump) return;
			lose = true;
		} else if(cur->hinder_type == '#') {
			if(move_state == middle || (move_state == jump && prop_state == super_jump)) return;
			lose = true;
		}
		if(prop_state == invincible) {
			lose = false;
			score+=3;
			ClearHinderPerspective(cur);
			return;
		}
	}
}

void PropHappen() {
	int possible[2] = {0};
	int i = 0;
	if(map[1][$_left] == ' ') {
		possible[i] = $_left;
		i++;
	}
	if(map[1][$_middle] == ' ' && i <= 1) {
		possible[i] = $_middle;
		i++;
	}
	if(map[1][$_right] == ' ' && i <= 1) {
		possible[i] = $_right;
		i++;
	}
	if(i == 0) {
		speed_prop_happen = 45 * speed_limit;
		return;
	} else if(i == 1) {
		//int random2 = (int )fabs(5.23 * sin((double )score));
		int random2 = RandInt(0,5);
		Node *node = new Node;
		node->material_col = possible[0];
		node->material_row = 1;
		node->if_eaten = 0;
		node->prop_type = prop[random2];
		node->next = nullptr;
		PutIn(node, &list_prop);
		PaintProp(node, node->prop_type);
	} else if(i == 2) {
		//int random1 = (int )fabs(1.414 * sin((double )score));
		int random1 = RandInt(0,i-1);
		//int random2 = (int )fabs(5.23 * sin((double )score));
		int random2 = RandInt(0,5);
		Node *node = new Node;
		node->material_col = possible[random1];
		node->material_row = 1;
		node->if_eaten = 0;
		node->prop_type = prop[random2];
		node->next = nullptr;
		PutIn(node, &list_prop);
		PaintProp(node, node->prop_type);
	}
}

void PropMove() {
	for (Node *node = list_prop.head; node != nullptr; node = node->next) {
		if(node->material_col <= $_left) {
			ClearProp(node);
			if(node->material_row % 2 == 1) {
				node->material_col-=2;
			} else if(node->material_row % 2 == 0) {
				node->material_col--;
			}
			node->material_row++;
			PaintProp(node, node->prop_type);
		}
		if(node->material_col == $_middle) {
			ClearProp(node);
			node->material_row++;
			PaintProp(node, node->prop_type);
		}
		if(node->material_col >= $_right) {
			ClearProp(node);
			if(node->material_row % 2 == 1) {
				node->material_col+=2;
			} else if(node->material_row % 2 == 0) {
				node->material_col++;
			}
			node->material_row++;
			PaintProp(node, node->prop_type);
		}
	}
}

int PropEat() {
	Node *cur = list_prop.head;
	for (; cur != nullptr; cur = cur->next) {
		if(cur->material_row == 15 && cur->material_col >= head_col - 2
		        && cur->material_col <= head_col + 2 && move_state != jump && move_state != middle) {
			ClearScreen();
			cur->if_eaten = 1;
			return 1;
		}
	}
	return 0;
}

void PropClear() {
	Node *cur = list_prop.head;
	ClearProp(cur);
	if(cur == nullptr) return;
	if(cur->prop_type == invincible) prop_state = invincible;
	else if(cur->prop_type == flying) prop_state = flying;
	else if(cur->prop_type == super_jump) prop_state = super_jump;
	else if(cur->prop_type == magnet) prop_state = magnet;
	else if(cur->prop_type == double_score) prop_state = double_score;
	else if(cur->prop_type == double_coin) prop_state = double_coin;
	list_prop.head = nullptr;
	list_prop.tail = nullptr;
	delete cur;
}

// C++ 内存释放（delete替代free）
void Free(LinkedList* list) {
	while(list->head != nullptr) {
		Node *cur = list->head;
		list->head = cur->next;
		delete cur;
	}
}

void StartUp() {
	for (int i = 0; i <= COL-1; ++i) {
		cout << '-';
		if(i == COL-1) cout << endl;
	}
	for (int i = 1; i <= 5; ++i) {
		cout << "|";
		for (int j = 0; j < COL - 2; ++j) cout << " ";
		cout << "|" << endl;
	}
	cout << "|";
	for (int j = 0; j < 28; ++j) cout << " ";
	cout << "Welcome to Metro Running";
	for (int j = 0; j < 28; ++j) cout << " ";
	cout << "|" << endl << "|";
	for (int j = 0; j < 29; ++j) cout << " ";
	cout << "Press 'Enter' to start";
	for (int j = 0; j < 29; ++j) cout << " ";
	cout << "|" << endl;
	for (int i = 8; i <= 15; ++i) {
		cout << "|";
		for (int j = 0; j < COL - 2; ++j) cout << " ";
		cout << "|" << endl;
	}
	for (int i = 0; i <= COL-1; ++i) {
		cout << '-';
		if(i == COL) cout << endl;
	}
	HideCursor();

	while(_getch() != '\r');
	ClearScreen();

	// 清空所有残留输入，避免影响游戏内按键
	while(_kbhit()) _getch();
}

void Initial() {
	for (int i = 0; i < ROW; ++i) {
		for (int j = 0; j < COL; ++j) {
			map[i][j] = ' ';
		}
	}
	for (int i = 0; i < ROW; ++i) {
		map[i][0] = '|';
		map[i][COL-1] = '|';
	}
	for (int i = 0; i < COL; ++i) {
		map[0][i] = '-';
		map[ROW-1][i] = '-';
	}
	for (int row = 15, col1 = 4, col2 = 77; row >= 1; row--,col1 += 2,col2 -= 2) {
		map[row][col1] = '/';
		map[row][col2] = '\\';
	}
	for (int row = 15, col1 = 24, col2 = 57; row >= 1; row--,col1++,col2--) {
		map[row][col1] = '/';
		map[row][col1 + 1] = '/';
		map[row][col2] = '\\';
		map[row][col2 - 1] = '\\';
	}
	score = 0;
	coin_nums = 0;
	score_add = 3;
	head_col = middle_col;
	head_row = 13;
	PrintSmallGuy();
	run = 0;
	speed_limit = 7;
	speed = 1;
	move_state = running;
	move_state_time = 1;
	move_state_time_limit = 24;
	prop_state = none;
	prop_state_time = 1;
	prop_state_time_limit = 150;
	speed_hinder_happen = 1;
	speed_prop_happen = 1;
	lose = false;
	replay = false;
}

void Show() {
	gotoxy(0,0);
	cout << "Score:" << score << "         Coin:" << coin_nums << endl;

	for (int i = 0; i < ROW; ++i) {
		for (int j = 0; j < COL; ++j) {
			char ch = map[i][j];
			// 根据字符切换对应颜色
			if(ch == '$')
				SetColor(YELLOW);
			else if(ch == '^')
				SetColor(PINK);
			else if(ch == '#')
				SetColor(LIGHT_BLUE);
			else if(ch == '_')
				SetColor(LIGHT_GREEN);
			else if(ch == 'x')
				SetColor(RED);
			// 所有道具字符统一紫色
			else if(ch == '@' || ch == '>' || ch == '<' || ch == 'd' || ch == 'b' || ch == 'U' || ch == '2')
				SetColor(PURPLE);
			else
				ResetColor(); // 边框、人物、空格默认白色

			cout << ch;
			ResetColor(); // 打印单个字符后立刻恢复白色，避免污染下一个字符
		}
		cout << endl;
	}
}

// 已整合：大小写兼容 + 方向键支持
void UpdateWithIn() {
	int input;
	if(_kbhit()) {
		input = _getch();
		// 方向键扩展键
		if(input == 224) {
			int key = _getch();
			switch(key) {
				case 75: // 左←
					if(head_col == middle_col) {
						ClearSmallGuy();
						head_col = left_col;
					}
					if(head_col == right_col) {
						ClearSmallGuy();
						head_col = middle_col;
					}
					break;
				case 77: // 右→
					if(head_col == middle_col) {
						ClearSmallGuy();
						head_col = right_col;
					}
					if(head_col == left_col) {
						ClearSmallGuy();
						head_col = middle_col;
					}
					break;
				case 72: // 上↑跳跃
					if((move_state != jump && move_state != middle) || prop_state == flying) {
						move_state = jump;
						move_state_time = 1;
					}
					break;
				case 80: // 下↓下蹲
					if(move_state == jump) {
						move_state = middle;
						move_state_time = 1;
					} else {
						move_state = down;
						move_state_time = 1;
					}
					break;
			}
			return;
		}
		// 空格暂停
		if(input == ' ') {
			cout << "Press space to continue";
			char input2 = 0;
			while(input2 != ' ') {
				input2 = (char) _getch();
			}
			ClearScreen();
			return;
		}
		// 字母大小写统一识别
		char ch = tolower((unsigned char)input);
		if(ch == 'a') {
			if(head_col == middle_col) {
				ClearSmallGuy();
				head_col = left_col;
			}
			if(head_col == right_col) {
				ClearSmallGuy();
				head_col = middle_col;
			}
		} else if(ch == 'd') {
			if(head_col == middle_col) {
				ClearSmallGuy();
				head_col = right_col;
			}
			if(head_col == left_col) {
				ClearSmallGuy();
				head_col = middle_col;
			}
		} else if(ch == 'w') {
			if((move_state != jump && move_state != middle) || prop_state == flying) {
				move_state = jump;
				move_state_time = 1;
			}
		} else if(ch == 's') {
			if(move_state == jump) {
				move_state = middle;
				move_state_time = 1;
			} else {
				move_state = down;
				move_state_time = 1;
			}
		}
	}
}

void UpdateWithoutIn() {
	static int coinSpawnTick = 0;
	if(speed < speed_limit);
	else {
		CoinMove();
		coinSpawnTick++;
		// 多少个刷新周期生成1枚
		if(coinSpawnTick >= 2) {
			CoinHappen();
			coinSpawnTick = 0;
		}
		PropMove();
		HinderMove();
		BottomClear(&list_hinder);
	}
	if(speed_hinder_happen < 4 * speed_limit) {
		speed_hinder_happen++;
	} else {
		speed_hinder_happen = 1;
		HinderHappen();
	}
	if(speed_prop_happen < 45 * speed_limit) {
		speed_prop_happen++;
	} else {
		speed_prop_happen = 1;
		PropHappen();
	}
	MoveStateTimeCountdown();
	if(prop_state != none) PropStateTimeCountdown();
	IfHinderClash();
	if(CoinEat()) CoinClear();
	else BottomClear(&list_coin);
	if(PropEat()) PropClear();
	else BottomClear(&list_prop);
	if(speed < speed_limit) {
		speed++;
	} else {
		speed = 1;
		score++;
		if(prop_state == double_score) score++;
		if(move_state == running) SpeedRun();
		else if(move_state == jump) PrintJump();
		else if(move_state == down) PrintSquat();
		else if(move_state == middle) PaintGliding();
	}
}

void End() {
	Free(&list_coin);
	Free(&list_hinder);
	Free(&list_prop);
	cout << "LOSE" << endl;
	cout << "If you want to try again, press 'y'" << endl;
	cout << "Else, press 'n' to exit" << endl;
	while(1) {
		char input = (char )_getch();
		if(input == 'y' || 'Y') {
			replay = true;
			ClearScreen();
			return;
		} else if(input == 'n' || 'N') {
			return;
		}
	}
}

int main() {
	srand((unsigned)time(NULL));
	while(replay) {
		StartUp();
		Initial();
		while(_kbhit()) _getch(); // 清空所有残留按键
		while (1) {
			Show();
			UpdateWithIn();
			UpdateWithoutIn();
			if (lose) break;
		}
		End();
	}
	return 0;
}

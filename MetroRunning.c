#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define ROW 17
#define COL 82
//人物所在点
#define middle_col 40
#define left_col 16
#define right_col 65
//物品生成点
#define $_left 35
#define $_middle 40
#define $_right 46
//四个状态常量
#define running 0
#define jump 1
#define down 2
#define middle 3
//四个道具状态量和none
#define none 0
#define invincible 4
#define flying 5
#define super_jump 6
#define magnet 7
#define double_score 8
#define double_coin 9

typedef struct Node {
    int material_row;
    int material_col;
    int if_eaten;
    char hinder_type;
    int prop_type;
    struct Node *next;
} Node;
typedef struct LinkedList{
    Node *head;
    Node *tail;
}LinkedList;

char map[ROW][COL];
//人物坐标
int head_col;
int head_row;
//计分
long long score;
long long coin_nums;
int score_add;
//用来画人物跑动
int run;
//速度控制与帧率
int speed;
int speed_limit;
//生成间隙判断
int speed_hinder_happen;
int speed_prop_happen;
//各种链子
LinkedList list_coin = {
        .head = NULL,
        .tail = NULL
};//准备好金币链子
LinkedList list_hinder = {
        .head = NULL,
        .tail = NULL
};//障碍物链子
LinkedList list_prop = {
        .head = NULL,
        .tail = NULL
};//道具链子(实际上每这条链上永远只会有一个道具)
//这是运动状态
int move_state;
int move_state_time;
int move_state_time_limit;
//这是道具状态
int prop_state;
int prop_state_time;
int prop_state_time_limit;
//金币生成位置，障碍物类型，道具类型，为了random故用数组存好
int $_place[3] = {$_left, $_middle, $_right};
char hinder[4] = {'^','_','#','x'};
int prop[6] = {invincible,flying,super_jump,magnet,double_score,double_coin};
//输赢判据
bool lose;
bool replay = true;

//清屏与隐藏光标函数
void gotoxy(int x, int y){//清屏函数
    COORD pos = { (SHORT)x,(SHORT)y };
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(handle, pos);
}
void HideCursor(){//光标隐藏函数
    CONSOLE_CURSOR_INFO cursor_info = { 1,0 };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}
//画画函数
void PrintSmallGuy(){
    map[head_row][head_col - 1]='('; map[head_row][head_col]='*'; map[head_row][head_col + 1]=')';//头
    map[head_row + 1][head_col - 1]='/'; map[head_row + 1][head_col]='|'; map[head_row + 1][head_col + 1]='\\';//身体
    map[head_row + 2][head_col - 1]='/'; map[head_row + 2][head_col + 1]='\\';//脚
}
void ClearSmallGuy(){
    map[head_row][head_col] = ' ';
    map[head_row][head_col - 1] = ' ';
    map[head_row][head_col + 1] = ' ';
    map[head_row + 1][head_col - 1] = ' ';
    map[head_row + 1][head_col] = ' ';
    map[head_row + 1][head_col + 1] = ' ';
    map[head_row + 2][head_col - 1] = ' ';
    map[head_row + 2][head_col + 1] = ' ';
}
void PrintRun(){
    if(run == 0){//run等于0时迈右腿
        run = 1;
        map[head_row + 2][head_col - 1] = ' ';
        map[head_row + 2][head_col + 1] = '\\';
        map[head_row + 1][head_col + 1] = ' ';
        map[head_row + 1][head_col - 1] = '/';
    } else if(run == 1){//run等于1时迈左腿
        run = 0;
        map[head_row + 2][head_col + 1] = ' ';
        map[head_row + 2][head_col - 1] = '/';
        map[head_row + 1][head_col + 1] = '\\';
        map[head_row + 1][head_col - 1] = ' ';
    }
}
void PrintJump(){
    PrintSmallGuy();
    map[head_row + 2][head_col - 1] = '-';
    map[head_row + 2][head_col + 1] = '-';
}
void PrintSquat(){
    PrintSmallGuy();
    map[head_row + 1][head_col - 1] = '-';
    map[head_row + 1][head_col + 1] = '-';
    map[head_row + 2][head_col - 1] = '-';
    map[head_row + 2][head_col + 1] = '-';
}
void PaintGliding(){
    PrintSmallGuy();
    map[head_row + 1][head_col] = '&';
    map[head_row + 1][head_col - 1] = '/';
    map[head_row + 1][head_col + 1] = '\\';
    map[head_row + 2][head_col - 1] = ' ';
    map[head_row + 2][head_col + 1] = ' ';
}
//物品移动函数
void ClearCoinHinder(Node const *node,char letter){
    if(node->material_row <= 15 && map[node->material_row][node->material_col] == letter) {
        map[node->material_row][node->material_col] = ' ';
    }
}
void PaintCoinHinder(Node const *node,char letter){
    if(node->material_row <= 15) {
        map[node->material_row][node->material_col] = letter;
    }
}
void ClearProp(Node const *node){
    if(node->material_row <= 15) {
        map[node->material_row][node->material_col] = ' ';
        map[node->material_row][node->material_col + 1] = ' ';
    }
}
void PaintProp(Node const *node, int p){
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
        } else if (p == double_score){
            map[node->material_row][node->material_col] = 'X';
            map[node->material_row][node->material_col + 1] = '2';
        } else if (p == double_coin){
            map[node->material_row][node->material_col] = 'X';
            map[node->material_row][node->material_col + 1] = '$';
        }
    }
}
//障碍物透视绘画函数
void ClearHinderPerspective(Node const *node){
    if(node->material_row == 2 || node->material_row == 3 || node->material_row == 4){
        map[node->material_row][node->material_col-1] = ' ';
    }else if(node->material_row == 5 || node->material_row == 6 || node->material_row == 7){
        map[node->material_row][node->material_col-2] = ' ';
        map[node->material_row][node->material_col-1] = ' ';
    }else if(node->material_row == 8 || node->material_row == 9 || node->material_row == 10){
        map[node->material_row][node->material_col-2] = ' ';
        map[node->material_row][node->material_col-1] = ' ';
        map[node->material_row][node->material_col+2] = ' ';
        map[node->material_row][node->material_col+3] = ' ';
    }else if(node->material_row >= 11 && node->material_row <= 15){
        map[node->material_row][node->material_col-3] = ' ';
        map[node->material_row][node->material_col-2] = ' ';
        map[node->material_row][node->material_col-1] = ' ';
        map[node->material_row][node->material_col+2] = ' ';
        map[node->material_row][node->material_col+3] = ' ';
    }
    if(map[node->material_row][node->material_col+1] != '@' && map[node->material_row][node->material_col+1] != '<' &&
        map[node->material_row][node->material_col+1] != 'b' && map[node->material_row][node->material_col+1] != 'U' &&
        map[node->material_row][node->material_col+1] != 'X'){
        map[node->material_row][node->material_col+1] = ' ';//防止清除道具
    }
}
void PaintHinderPerspective(Node const *node,char letter){
    if(node->material_row == 2 || node->material_row == 3 || node->material_row == 4){
        map[node->material_row][node->material_col-1] = letter;
        map[node->material_row][node->material_col+1] = letter;
    }else if(node->material_row == 5 || node->material_row == 6 || node->material_row == 7){
        map[node->material_row][node->material_col-2] = letter;
        map[node->material_row][node->material_col-1] = letter;
        map[node->material_row][node->material_col+1] = letter;
    }else if(node->material_row == 8 || node->material_row == 9 || node->material_row == 10){
        map[node->material_row][node->material_col-2] = letter;
        map[node->material_row][node->material_col-1] = letter;
        map[node->material_row][node->material_col+1] = letter;
        map[node->material_row][node->material_col+2] = letter;
        map[node->material_row][node->material_col+3] = letter;
    }else if(node->material_row >= 11 && node->material_row <= 15){
        map[node->material_row][node->material_col-3] = letter;
        map[node->material_row][node->material_col-2] = letter;
        map[node->material_row][node->material_col-1] = letter;
        map[node->material_row][node->material_col+1] = letter;
        map[node->material_row][node->material_col+2] = letter;
        map[node->material_row][node->material_col+3] = letter;
    }
}
//底部清除函数
void BottomClear(LinkedList *list){
    Node *cur = NULL;
    Node *prev = NULL;
    for (cur = list->head; cur != NULL; cur = cur->next,prev = cur) {
        if(cur->material_row > 15)
            break;
    }
    //如果没有
    if(cur == NULL) {
        return;
    }
    if(prop_state == magnet){
        score+=3;
        coin_nums++;
    }
    //如果只有一个
    if(cur == list->head && cur == list->tail){
        list->head = NULL;
        list->tail = NULL;
        free(cur);
        return;
    }
    //如果是第一个
    else if(cur == list->head){
        list->head = cur->next;
        free(cur);
        return;
    }
    //如果是最后一个
    else if(cur == list->tail){
        list->tail = prev;
        prev->next = NULL;
        free(cur);
        return;
    }
    prev->next = cur->next;
    free(cur);
}
//调速函数
void SpeedRun(){
    PrintSmallGuy();
    PrintRun();
    if((score >= 150 && score < 300 && speed_limit == 5)
    || (score >= 300 && score < 600 && speed_limit == 4)
    || (score >= 600 && score < 1000 && speed_limit == 3)
    || (score >= 1000 && speed_limit == 2)){
        speed_limit--;
    }
}
//链表插入函数
void PutIn(Node *node, LinkedList *list){
    //空链表情况
    if(list->head == NULL){
        list->head = node;
        list->tail = node;
        return;
    }
    list->tail->next = node;
    list->tail = node;
}
//两个状态倒计时函数
void MoveStateTimeCountdown(){
    if(move_state_time++ < move_state_time_limit);
    else {
        move_state_time = 1;
        move_state = running;
    }
    if(speed_limit == 5){//这是动作持续时间
        move_state_time_limit = 20 + 2;//加2是为了优化状态末过障易死的问题，但不会解决
    } else if(speed_limit == 4) {
        move_state_time_limit = 16 + 2;
    } else if(speed_limit == 3){
        move_state_time_limit = 12 + 2;
    } else if(speed_limit == 2){
        move_state_time_limit = 8 + 2;
    } else if(speed_limit == 1){
        move_state_time_limit = 6;
    }
}
void PropStateTimeCountdown(){
    if(prop_state_time < prop_state_time_limit){
        prop_state_time++;
        if(prop_state_time == 28 * speed_limit || prop_state_time == 26 * speed_limit){//道具结束前三次屏闪
            system("cls");
        }
    } else {
        prop_state_time = 1;
        system("cls");
        prop_state = none;
    }
    if(speed_limit == 4){
        prop_state_time_limit = 120;
    } else if(speed_limit == 3){
        prop_state_time_limit = 90;
    } else if(speed_limit == 2){
        prop_state_time_limit = 60;
    } else if(speed_limit == 1){
        prop_state_time_limit = 30;
    }

}
//Coin相关函数
void CoinHappen(){
    Node *node = (Node*)malloc(sizeof(*node));
    int random = (int )fabs(2.3 * sin((double)score));//这里解决了随机函数不那么随机的问题
    node->material_col = $_place[random];
    node->material_row = 1;
    node->if_eaten = 0;
    node->next = NULL;
    PutIn(node,&list_coin);
    PaintCoinHinder(node, '$');
}
void CoinMove(){
    for (Node *node = list_coin.head; node != NULL; node = node->next) {
        if(node->material_col <= $_left){//左边的金币
            ClearCoinHinder(node,'$');
            if(node->material_row % 2 == 1){
                node->material_col-=2;
            } else if(node->material_row % 2 == 0){
                node->material_col--;
            }
            node->material_row++;
            PaintCoinHinder(node, '$');
        }
        if(node->material_col == $_middle){//中间的金币
            ClearCoinHinder(node,'$');
            node->material_row++;
            PaintCoinHinder(node, '$');
        }
        if(node->material_col >= $_right){//右边的金币
            ClearCoinHinder(node,'$');
            if(node->material_row % 2 == 1){
                node->material_col+=2;
            } else if(node->material_row % 2 == 0){
                node->material_col++;
            }
            node->material_row++;
            PaintCoinHinder(node, '$');
        }
    }
}
int CoinEat(){
    Node *cur = list_coin.head;
    for (; cur != NULL;cur = cur->next) {
        if(cur->material_row == 15
        && cur->material_col >= head_col - 2
        && cur->material_col <= head_col + 2
        && move_state != jump && move_state != middle){
            cur->if_eaten = 1;
            return 1;
        }
    }
    return 0;
}
void CoinClear(){
    Node *cur;
    Node *prev = NULL;
    for (cur = list_coin.head; cur != NULL; cur = cur->next) {
        if(cur->if_eaten == 1){
            ClearCoinHinder(cur,'$');
            break;
        }
        prev = cur;
    }
    //如果没有
    if(cur == NULL) {
        return;
    }
    if(prop_state == double_coin){
        score+=score_add;
        coin_nums++;
    }
    score+=score_add;
    coin_nums++;
    //如果只有一个
    if(cur == list_coin.head && cur == list_coin.tail){
        list_coin.head = NULL;
        list_coin.tail = NULL;
        free(cur);
        return;
    }
    //如果是第一个
    else if(cur == list_coin.head){
        list_coin.head = cur->next;
        free(cur);
        return;
    }
    //如果是最后一个
    else if(cur == list_coin.tail){
        list_coin.tail = prev;
        prev->next = NULL;
        free(cur);
        return;
    }
    prev->next = cur->next;
    free(cur);
}
//Hinder相关函数
void HinderHappen(){
    int possible[2] = {0};//拿到两个空位置
    int i = 0;
    if(map[1][$_left] == ' '){
        possible[i] = $_left;
        i++;
    }
    if(map[1][$_middle] == ' ' && i <= 1){
        possible[i] = $_middle;
        i++;
    }
    if(map[1][$_right] == ' ' && i <= 1){
        possible[i] = $_right;
        i++;
    }
    if(i == 0){
        speed_hinder_happen = 4 * speed_limit;
    } else if(i == 1){
        int random2 = (int )fabs(3.15 * sin((double )score));
        Node *node = (Node *) malloc(sizeof(*node));
        node->material_col = possible[0];
        node->material_row = 1;node->if_eaten = 0;
        node->hinder_type = hinder[random2];
        node->next = NULL;
        PutIn(node, &list_hinder);
        PaintCoinHinder(node, node->hinder_type);
    } else if (i == 2){
        int random1 = (int )fabs(1.414 * sin((double )score));//从两个空位置里选一个
        int random2 = (int )fabs(3.15 * sin((double )score));//4的随机数函数
        Node *node = (Node *) malloc(sizeof(*node));
        node->material_col = possible[random1];
        node->material_row = 1;
        node->if_eaten = 0;
        node->hinder_type = hinder[random2];
        node->next = NULL;
        PutIn(node, &list_hinder);
        PaintCoinHinder(node, node->hinder_type);
    }
}
void HinderMove(){
    for (Node *node = list_hinder.head; node != NULL; node = node->next) {
        if(node->material_col <= $_left){//左边的障碍
            ClearCoinHinder(node,node->hinder_type);
            ClearHinderPerspective(node);
            if(node->material_row % 2 == 1){
                node->material_col-=2;
            } else if(node->material_row % 2 == 0){
                node->material_col--;
            }
            node->material_row++;
            PaintCoinHinder(node, node->hinder_type);
            PaintHinderPerspective(node, node->hinder_type);
        }
        if(node->material_col == $_middle){//中间的障碍
            ClearCoinHinder(node,node->hinder_type);
            ClearHinderPerspective(node);
            node->material_row++;
            PaintCoinHinder(node, node->hinder_type);
            PaintHinderPerspective(node, node->hinder_type);
        }
        if(node->material_col >= $_right){//右边的障碍
            ClearCoinHinder(node,node->hinder_type);
            ClearHinderPerspective(node);
            if(node->material_row % 2 == 1){
                node->material_col+=2;
            } else if(node->material_row % 2 == 0){
                node->material_col++;
            }
            node->material_row++;
            PaintCoinHinder(node, node->hinder_type);
            PaintHinderPerspective(node, node->hinder_type);
        }
    }
}
void IfHinderClash(){
    Node *cur = list_hinder.head;
    for (; cur != NULL; cur = cur->next) {
        if(cur->material_row == 15){
            break;
        }
    }
    if(cur == NULL){
        return;
    }
    if(cur->material_col <= head_col + 2 && cur->material_col >= head_col - 2){
        //对于up障碍
        if(cur->hinder_type == '_'){
            if(move_state == jump || move_state == middle){
                return;
            } else {
                lose = true;
            }
        }
        //对于down障碍
        else if(cur->hinder_type == '^'){
            if(move_state == down || (move_state == jump && prop_state == super_jump)){
                return;
            } else {
                lose = true;
            }
        }
        //对于x障碍
        else if(cur->hinder_type == 'x'){
            if(move_state == jump && prop_state == super_jump){
                return;
            }
            lose = true;
        }
        //对于middle障碍
        else if(cur->hinder_type == '#'){
            if(move_state == middle || (move_state == jump && prop_state == super_jump)){
                return;
            } else {
                lose = true;
            }
        }
        if(prop_state == invincible){
            lose = false;
            score+=3;
            ClearHinderPerspective(cur);
            return;
        }
    }
}
//Prop相关函数
void PropHappen(){
    int possible[2] = {0};//拿到两个空位置
    int i = 0;
    if(map[1][$_left] == ' '){
        possible[i] = $_left;
        i++;
    }
    if(map[1][$_middle] == ' ' && i <= 1){
        possible[i] = $_middle;
        i++;
    }
    if(map[1][$_right] == ' ' && i <= 1){
        possible[i] = $_right;
        i++;
    }
    if(i == 0){    //加上如若为无空位的判断
        speed_prop_happen = 45 * speed_limit;
        return;
    } else if(i == 1){
        int random2 = (int )fabs(5.23 * sin((double )score));
        Node *node = (Node *) malloc(sizeof(*node));
        node->material_col = possible[0];
        node->material_row = 1;
        node->if_eaten = 0;
        node->prop_type = prop[random2];
        node->next = NULL;
        PutIn(node, &list_prop);
        PaintProp(node, node->prop_type);
    } else if(i == 2) {
        int random1 = (int )fabs(1.414 * sin((double )score));//从两个空位置里选一个
        int random2 = (int )fabs(5.23 * sin((double )score));
        Node *node = (Node *) malloc(sizeof(*node));
        node->material_col = possible[random1];
        node->material_row = 1;
        node->if_eaten = 0;
        node->prop_type = prop[random2];
        node->next = NULL;
        PutIn(node, &list_prop);
        PaintProp(node, node->prop_type);
    }
}
void PropMove(){
    for (Node *node = list_prop.head; node != NULL; node = node->next) {
        if(node->material_col <= $_left){//左边的障碍
            ClearProp(node);
            if(node->material_row % 2 == 1){
                node->material_col-=2;
            } else if(node->material_row % 2 == 0){
                node->material_col--;
            }
            node->material_row++;
            PaintProp(node, node->prop_type);
        }
        if(node->material_col == $_middle){//中间的障碍
            ClearProp(node);
            node->material_row++;
            PaintProp(node, node->prop_type);
        }
        if(node->material_col >= $_right){//右边的障碍
            ClearProp(node);
            if(node->material_row % 2 == 1){
                node->material_col+=2;
            } else if(node->material_row % 2 == 0){
                node->material_col++;
            }
            node->material_row++;
            PaintProp(node, node->prop_type);
        }
    }
}
int PropEat(){
    //吃掉
    Node *cur = list_prop.head;
    for (; cur != NULL;cur = cur->next) {
        if(cur->material_row == 15 && cur->material_col >= head_col - 2
            && cur->material_col <= head_col + 2 && move_state != jump && move_state != middle){
            system("cls");//吃到时会屏闪
            cur->if_eaten = 1;
            return 1;
        }
    }
    return 0;
}
void PropClear(){
    Node *cur = list_prop.head;
    ClearProp(cur);
    //如果没有
    if(cur == NULL) {
        return;
    }
    //接下来开始道具状态的改变
    if(cur->prop_type == invincible){
        prop_state = invincible;
    } else if(cur->prop_type == flying){
        prop_state = flying;
    } else if(cur->prop_type == super_jump){
        prop_state = super_jump;
    } else if(cur->prop_type == magnet){
        prop_state = magnet;
    } else if(cur->prop_type == double_score){
        prop_state = double_score;
    } else if(cur->prop_type == double_coin){
        prop_state = double_coin;
    }
    //只会只有一个
    list_prop.head = NULL;
    list_prop.tail = NULL;
    free(cur);
}
//链子内存释放函数
void Free(LinkedList *list){
    while(list->head != NULL) {
        Node *cur = list->head;
        list->head = cur->next;
        free(cur);
    }
}

void StartUp(){
    //画开始界面,里面的数字只是为了更好地展现效果，故没有宏定义
    for (int i = 0; i <= COL + 1; ++i) {
        printf("%c","-\n"[i == 0 || i == COL + 1]);
    }
    for (int i = 1; i <= 5; ++i) {
        printf("|");
        for (int j = 0; j < COL - 2; ++j) {
            printf(" ");
        }
        printf("|\n");
    }
    printf("|");
    for (int j = 0; j < 28; ++j) {
        printf(" ");
    }
    printf("Welcome to Metro Running");
    for (int j = 0; j < 28; ++j) {
        printf(" ");
    }
    printf("|\n|");
    for (int j = 0; j < 29; ++j) {
        printf(" ");
    }
    printf("Press 'Enter' to start");
    for (int j = 0; j < 29; ++j) {
        printf(" ");
    }
    printf("|\n");
    for (int i = 8; i <= 15; ++i) {
        printf("|");
        for (int j = 0; j < COL - 2; ++j) {
            printf(" ");
        }
        printf("|\n");
    }
    for (int i = 0; i <= COL; ++i) {
        printf("%c","-\n"[i == COL]);
    }
    HideCursor();

    //开始游戏
    char c;
    scanf("%c", &c);
    system("cls");
}
void Initial(){
    //先将空格安排上
    for (int i = 0; i < ROW; ++i) {
        for (int j = 0; j < COL; ++j) {
            map[i][j] = ' ';
        }
    }
    //这是在画边框
    for (int i = 0; i < ROW; ++i) {
        map[i][0] = '|';
        map[i][COL-1] = '|';
    }
    for (int i = 0; i < COL; ++i) {
        map[0][i] = '-';
        map[ROW-1][i] = '-';
    }
    //这是画道路
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
    head_row = 13;//脑袋所在的位置
    PrintSmallGuy();
    run = 0;
    speed_limit = 5;
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
void Show(){
    gotoxy(0,0);
    printf("Score:%4lld         Coin:%4lld\n",score,coin_nums);
    for (int i = 0; i < ROW; ++i) {
        for (int j = 0; j < COL; ++j) {
            printf("%c",map[i][j]);
        }
        printf("\n");
    }
}
void UpdateWithIn(){
    char input;
    //开始键入
    if(_kbhit()){
        input = (char)_getch();
        if(input == 'a'){
            if(head_col == middle_col){
                ClearSmallGuy();
                head_col = left_col;
            }
            if(head_col == right_col){
                ClearSmallGuy();
                head_col = middle_col;
            }
        }
        else if(input == 'd'){
            if(head_col == middle_col){
                ClearSmallGuy();
                head_col = right_col;
            }
            if(head_col == left_col){
                ClearSmallGuy();
                head_col = middle_col;
            }
        }
        else if(input == 'w' && ((move_state != jump && move_state != middle) || prop_state == flying)){
            move_state = jump;
            move_state_time = 1;
        }
        else if(input == 's'){
            if(move_state == jump){//进入middle的条件
                move_state = middle;
                move_state_time = 1;
            } else {
                move_state = down;
                move_state_time = 1;
            }
        }

        //这个是暂停
        if(input == ' '){
            printf("Press space to continue");
            char input2 = 0;
            while(input2 != ' ') {
                input2 = (char) _getch();
            }
            system("cls");
        }
    }
}
void UpdateWithoutIn(){
    //物品移动
    if(speed < speed_limit);
    else{
        CoinMove();
        CoinHappen();

        PropMove();

        HinderMove();
        BottomClear(&list_hinder);
    }
    //障碍物生成
    if(speed_hinder_happen < 4 * speed_limit){//每四行一个障碍
        speed_hinder_happen++;
    } else {
        speed_hinder_happen = 1;
        HinderHappen();
    }
    //道具生成
    if(speed_prop_happen < 45 * speed_limit){//每45行一个道具，道具持续30行
        speed_prop_happen++;
    } else {
        speed_prop_happen = 1;
        PropHappen();
    }

    //运动状态倒计时
    MoveStateTimeCountdown();
    if(prop_state != none) {
        PropStateTimeCountdown();
    }

    //这里是采用逐帧检验，以获得更好的游戏体验
    IfHinderClash();//障碍物碰撞检验
    if(CoinEat()){//金币碰撞检验
        CoinClear();
    } else {
        BottomClear(&list_coin);
    }
    if(PropEat()){//道具碰撞检验
        PropClear();
    } else {
        BottomClear(&list_prop);
    }


    //刷小人   放在最后必须，因为透视
    if(speed < speed_limit){//帧数卡在这里共用
        speed++;
    } else {
        speed = 1;
        score++;
        if(prop_state == double_score){
            score++;
        }
        if(move_state == running) {
            SpeedRun();
        }else if(move_state == jump){
            PrintJump();
        }else if(move_state == down){
            PrintSquat();
        }else if(move_state == middle){
            PaintGliding();
        }
    }
}
void End(){
    Free(&list_coin);
    Free(&list_hinder);
    Free(&list_prop);

    printf("LOSE\n");
    printf("If you want to try again, press 'y'\n");
    printf("Else, press 'n' to exit\n");
    while(1){
        char input = (char )_getch();
        if(input == 'y'){
            replay = true;
            system("cls");
            return;
        } else if(input == 'n'){
            return;
        }
    }
}

int main() {
    while(replay) {
        StartUp();
        Initial();
        while (1) {
            Show();
            UpdateWithIn();
            UpdateWithoutIn();
            //输判据
            if (lose) {
                break;
            }
        }
        End();
    }
    return 0;
}
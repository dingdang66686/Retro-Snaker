#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>
#include <WinBase.h>
#include <conio.h>
#include <locale.h>
#include <time.h>

const struct
{
    int width;
    int height;
} map = { 28,16 };

int gameover;
int score;

HANDLE winHandle;

enum Direction { Left, Up, Right, Down };

struct SnakeBody
{
    int X;
    int Y;
    enum Direction direction;
    struct SnakeBody* prev;
};

struct
{
    struct SnakeBody* tail;
    struct SnakeBody* head;
    struct SnakeBody temp;
} snake;

void snake_add(int x, int y, enum Direction dir)
{
    if (!snake.tail)
        snake.tail = snake.head = (struct SnakeBody*)malloc(sizeof(struct SnakeBody));
    else
    {
        snake.head->prev = (struct SnakeBody*)malloc(sizeof(struct SnakeBody));
        snake.head = snake.head->prev;
    }
    snake.head->X = x;
    snake.head->Y = y;
    snake.head->direction = dir;
    snake.head->prev = 0;
}

struct SnakeBody snake_pop()
{
    if (!snake.tail) return;
    struct SnakeBody temp = { snake.tail->X, snake.tail->Y, snake.tail->direction };
    struct SnakeBody* m = snake.tail;
    snake.tail = snake.tail->prev;
    if (!snake.tail) snake.head = snake.tail;
    free(m);
    return temp;
}

struct
{
    int X;
    int Y;
} food;

void genfood()
{
    int s = 1;
    int i;
    while (s)
    {
        s = 0;
        food.X = rand() % map.width;
        food.Y = rand() % map.height;
        for (struct SnakeBody* i = snake.tail; i; i = i -> prev)
        {
            if (food.X == i->X && food.Y == i->Y)
            {
                s = 1;
                break;
            }
        }
    }
}

int init()
{
    gameover = 0;
    snake_add(0.6 * (rand() % map.width) + 0.2 * map.width, 0.6 * (rand() % map.height) + 0.2 * map.height, rand() % 4);
    score = 0;
    genfood();
    return 0;
}

void update()
{
    int i;
    struct SnakeBody* j;
    snake_add(snake.head->X, snake.head->Y, snake.head->direction);
    switch (snake.head->direction)
    {
    case Left:
        if (!snake.head->X) snake.head->X += map.width;
        snake.head->X -= 1;
        break;
    case Right:
        snake.head->X += 1;
        snake.head->X %= map.width;
        break;
    case Up:
        if (!snake.head->Y) snake.head->Y += map.height;
        snake.head->Y -= 1;
        break;
    case Down:
        snake.head->Y += 1;
        snake.head->Y %= map.height;
        break;
    }
    for (j = snake.tail->prev;; j = j->prev)
    {
        if (j == snake.head) break;
        if (snake.head->X == j->X && snake.head->Y == j->Y) { gameover = 1; return; }
    }
    if (snake.head->X == food.X && snake.head->Y == food.Y)
    {
        score += 1;
        genfood();
    }
    else
        snake.temp = snake_pop();
}

void put(int x, int y, char* e)
{
    COORD pos = { x * 2, y };
    SetConsoleCursorPosition(winHandle, pos);
    SetConsoleTextAttribute(winHandle, 7 + 0 * 0x10);
    printf(e);
}

void putcolor(int x, int y, char* e, int color)
{
    COORD pos = { x * 2, y };
    SetConsoleCursorPosition(winHandle, pos);
    SetConsoleTextAttribute(winHandle, color);
    printf(e);
    SetConsoleTextAttribute(winHandle, 7 + 0 * 0x10);
}

void display()
{
    char score_s[8];
    sprintf_s(score_s, 8, "%7d", score);
    put(3, map.height + 2, score_s);
    put(snake.temp.X, snake.temp.Y, "  ");
    putcolor(food.X, food.Y, "  ", 4*0x10);
    putcolor(snake.tail->X, snake.tail->Y, "  ",2*0x10);
    putcolor(snake.head->X, snake.head->Y, "  ", 6*0x10);
}

void changedir()
{
    if (!_kbhit()) return;
    enum Direction dir;
    switch (_getch())
    {
    case 'w': dir = Up; break;
    case 'a': dir = Left; break;
    case 's': dir = Down; break;
    case 'd': dir = Right; break;
    case 32: put(max(map.width - 4, 12), map.height + 2, "暂停"); while (_getch() != 32); put(max(map.width - 4, 12), map.height + 2, "运行");
    default: return;
    }
    if ((dir + snake.head->direction) % 2)
        snake.head->direction = dir;
}

int main()
{
    setlocale(LC_ALL, "ZH-CN");
    SetConsoleTitle(u"贪吃蛇");
    winHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cursor_info = { 1, 0 };
    SetConsoleCursorInfo(winHandle, &cursor_info);

    srand(time(0));

    printf("按空格键开始游戏...");

    while (_getch() == 32)
    {
        char cmd[64];
        sprintf_s(cmd, 64, "mode con cols=%d lines=%d", map.width * 2, map.height + 4);
        system(cmd);

        for (int i = 0; i < map.width; ++i) putcolor(i, map.height, "  ", 0 + 9 * 0x10);
        printf("\n\n分数：");
        put(max(map.width - 4, 12), map.height + 2, "运行");

        init();
        while (!gameover)
        {
            changedir();
            update();
            display();
            Sleep(100);
        }
        while (snake.tail) snake_pop();
        system("cls");
        sprintf_s(cmd, 64, "mode con cols=%d lines=%d", 90, 28);
        system(cmd);
        printf("游戏结束！分数：%d\n按空格键重新开始，其他键退出...", score);
    }
}

#pragma once

#pragma region 头文件引用

// Windows 头文件: 
#include <windows.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>
#include <fstream>  
#include <string> 
#include<mmsystem.h>


// 资源头文件
#include "resource.h"
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "Winmm.lib")  

#include <vector>
#include <math.h>

#pragma endregion

#pragma region 宏定义

#define WINDOW_TITLEBARHEIGHT	32
#define WINDOW_WIDTH			1024
#define WINDOW_HEIGHT			768

// 场景定义
#define STAGE_STARTMENU			0
#define STAGE_HELP				1
#define STAGE_1					2
#define STAGE_2					3
#define STAGE_3					4
#define STAGE_4					5   
#define STAGE_5					6 
#define STAGE_PAUSE				7
#define STAGE_GAMEOVER			8
#define STAGE_VICTORY			9
#define STAGE_LEVEL_INFO        12  // 关卡信息界面

// 地图相关
#define MAP_WIDTH               1024
#define MAP_HEIGHT              768



// 单位尺寸
#define UNIT_SIZE_X				64
#define UNIT_SIZE_Y				64
#define UNIT_LAST_FRAME			11

// 鱼的类型定义
#define UNIT_SIDE_PLAYER     	0		// 玩家
#define UNIT_SIDE_PLAYER2       1
#define UNIT_SIDE_ENEMY			2		// 敌人
#define UNIT_SIDE_NEUTRAL		3		// 中立

// 鱼的种类
#define FISH_TYPE_SMALL			0		// 小型鱼
#define FISH_TYPE_MEDIUM		1		// 中型鱼
#define FISH_TYPE_LARGE			2		// 大型鱼
#define FISH_TYPE_BOSS			3		// BOSS鱼

// 鱼的状态
#define UNIT_STATE_HOLD			0
#define UNIT_STATE_WALK			1
#define UNIT_STATE_ATTACK		2
#define UNIT_STATE_DEAD			3

// 方向
#define UNIT_DIRECT_RIGHT		0
#define UNIT_DIRECT_LEFT		1

// 道具类型
#define ITEM_SPEEDUP            0
#define ITEM_HEAL               1
#define ITEM_SHIELD             2
#define ITEM_SHRINK             3
#define ITEM_STAR               4

// 最大数量
#define MAX_ITEMS               5


// 按钮定义
#define BUTTON_STARTGAME		1001
#define BUTTON_HELP				1002
#define BUTTON_BACK				1003
#define BUTTON_RESUME			1004
#define BUTTON_RESTART			1005
#define BUTTON_MAINMENU			1006
#define BUTTON_NEXTLEVEL		1007
#define BUTTON_START			1012

#define BUTTON_WIDTH			212
#define BUTTON_HEIGHT			76

// 角色选择
#define STAGE_CHARACTER_SELECT 10
#define BUTTON_PLAYER1 1008
#define BUTTON_PLAYER2 1009
#define BUTTON_CONTINUE 1010

#define STAGE_ABOUT             11
#define BUTTON_ABOUT            1011 

// 特殊鱼类效果
#define FISH_EFFECT_NONE 0
#define FISH_EFFECT_GOLD 1
#define FISH_EFFECT_POISON 2
#define FISH_EFFECT_BOMB 3

// 玩家类型
// 玩家类型
#define PLAYER_TYPE_RED    0  // 红色
#define PLAYER_TYPE_YELLOW 1  // 黄色


// 定时器
#define TIMER_GAMETIMER			1
#define TIMER_GAMETIMER_ELAPSE	30
#define UNIT_SPEED				3.0

// 游戏参数
#define MAX_ENEMIES				10		// 最大敌人数
#define COLLISION_DISTANCE		40		// 碰撞距离
#define EAT_SIZE_RATIO			0.8		// 可吃掉的体型比例

#pragma endregion

#pragma region 结构体声明

// 场景结构体
struct Stage
{
	int stageID;
	HBITMAP bg;
	int timeCountDown;
	bool timerOn;
	int levelGoal;			// 关卡目标分数
	wchar_t levelDesc[256];	// 关卡描述
};

// 按钮结构体
struct Button
{
	int buttonID;
	bool visible;
	HBITMAP img;
	int x;
	int y;
	int width;
	int height;
};

// 单位结构体
struct Unit
{
	HBITMAP img;
	HBITMAP img_left;   // 新增：向左的图片
	HBITMAP img_right;  // 新增：向右的图片
	int frame_row;
	int frame_column;
	int* frame_sequence;
	int frame_count;
	int frame_id;

	int side;			// 阵营
	int type;			// 类型
	int state;			// 状态
	int direction;		// 方向

	double x;			// 坐标
	double y;
	double vx;			// 速度
	double vy;

	double size;		// 体型大小（1.0为正常）
	int health;			// 生命值
	bool alive;			// 是否存活

	// AI相关
	int aiType;			// AI类型：0-随机游走，1-追逐玩家，2-逃离玩家
	double targetX;		// 目标点
	double targetY;
	int aiTimer;		// AI计时器

	int effectType; // 新增：特殊效果类型
};

// 粒子效果结构体
struct Particle
{
	double x, y;
	double vx, vy;
	int life;
	int maxLife;
	COLORREF color;
};


// 浮动文字结构体
struct FloatingText
{
    double x, y;
    double vy;          // 上升速度
    int life;
    int maxLife;
    wchar_t text[32];   // 显示的文字
    COLORREF color;
    int fontSize;
};


// 游戏数据结构体
struct GameData
{
	int score;				// 得分
	int level;				// 当前关卡
	int lives;				// 生命数
	double playerSize;		// 玩家体型
	bool hasKey;			// 是否有钥匙等道具
	int itemCount;			// 道具数量
	int characterType;           // 
	int speedBoostTimer;         // 
	int shieldTimer;             // 
	int fishEaten;               // 
	int itemsCollected;          // 
	int hiddenStarsCollected;    // 

	bool perfectRun;       // 完美通关（不掉血）
	int totalTime;         // 总用时
	bool secretUnlocked;   // 是否解锁秘密

};


// 相机结构体
struct Camera
{
	double x, y;
	double targetX, targetY;
};

// 道具结构体
struct Item
{
	int type;
	double x, y;
	bool active;
	int lifeTime;
	HBITMAP img;
};

// 存档数据结构体
struct SaveData
{
	int level;
	int score;
	int lives;
	double playerSize;
	int characterType;
	int fishEaten;
	int itemsCollected;
	int hiddenStarsCollected;
};




#pragma endregion

#pragma region 函数声明

// 初始化和事件处理
void InitGame(HWND hWnd, WPARAM wParam, LPARAM lParam);
void KeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
void KeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
void MouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
void LButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
void LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
void TimerUpdate(HWND hWnd, WPARAM wParam, LPARAM lParam);

// 游戏逻辑
Button* CreateButton(int buttonID, HBITMAP img, int width, int height, int x, int y);
Unit* CreateUnit(int side, int type, double x, double y, double size);
void InitStage(HWND hWnd, int stageID);
void UpdateUnits(HWND hWnd);
void UpdatePlayer(Unit* player);
void UpdateEnemy(Unit* enemy, Unit* player);
void CheckCollisions(HWND hWnd);
void SpawnEnemy(HWND hWnd);
void RemoveDeadUnits();
bool CheckBoundary(Unit* unit);
void CreateParticles(double x, double y, COLORREF color);
void UpdateParticles();

// 游戏状态
void PauseGame(HWND hWnd);
void ResumeGame(HWND hWnd);
void GameOver(HWND hWnd, bool victory);
void NextLevel(HWND hWnd);
void ResetGame(HWND hWnd);

// 浮动文字
void CreateFloatingText(double x, double y, const wchar_t* text, COLORREF color, int fontSize);
void UpdateFloatingTexts();
void DrawFloatingTexts(HDC hdc);


// 音频
void PlayBGM();
void StopBGM();
void PlayGameSound(int soundID);

// 绘图
void Paint(HWND hWnd);
HBITMAP InitBackGround(HWND hWnd, HBITMAP bmp_src);
void DrawHelpScreen(HDC hdc);
void DrawAboutScreen(HDC hdc);
void DrawCharacterSelect(HDC hdc);
void DrawLevelInfoScreen(HDC hdc);
void DrawPauseScreen(HDC hdc);
void DrawVictoryScreen(HDC hdc);
void DrawGameOverScreen(HDC hdc);
void DrawUI(HDC hdc, HDC hdc_load);
void DrawParticles(HDC hdc);
void DrawLevelInfo(HDC hdc);

// 道具相关
Item* CreateItem(int type, double x, double y);
void SpawnItem(HWND hWnd);
void UpdateItems(HWND hWnd);

// 存档相关
void SaveGame();
bool LoadGame();
bool HasSaveFile();

void SpawnEnemyCluster(HWND hWnd, int count);



#pragma endregion

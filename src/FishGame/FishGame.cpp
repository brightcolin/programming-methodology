// FishGame.cpp 
#include "FishGame.h"
using namespace std;

#pragma region 全局变量

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// 图像资源
HBITMAP bmp_start_background;
HBITMAP bmp_help_background;
HBITMAP bmp_game_background;
HBITMAP bmp_StartButton;
HBITMAP bmp_HelpButton;
HBITMAP bmp_BackButton;
HBITMAP bmp_ResumeButton;
HBITMAP bmp_RestartButton;
HBITMAP bmp_MenuButton;
HBITMAP bmp_NextButton;

HBITMAP bmp_AboutButton;
HBITMAP bmp_GameTitle;

// 鱼类图片
HBITMAP bmp_Fish_Small_Left[3];
HBITMAP bmp_Fish_Small_Right[3];
HBITMAP bmp_Fish_Medium_Left[3];
HBITMAP bmp_Fish_Medium_Right[3];
HBITMAP bmp_Fish_Large_Left[3];
HBITMAP bmp_Fish_Large_Right[3];

HBITMAP bmp_Player;
HBITMAP bmp_Player2;

HBITMAP bmp_ContinueButton;
HBITMAP bmp_Player1Icon;
HBITMAP bmp_Player2Icon;

HBITMAP bmp_Item_Speed;
HBITMAP bmp_Item_Heal;
HBITMAP bmp_Item_Shield;
HBITMAP bmp_Item_Shrink;
HBITMAP bmp_Item_Star;

HBITMAP bmp_Stage_BG[5];

// 游戏状态
Stage* currentStage = NULL;
vector<Unit*> units;
vector<Button*> buttons;
vector<Particle*> particles;
vector<FloatingText*> floatingTexts;
vector<Item*> items;
SaveData saveData;

GameData gameData;
Camera camera = { 0, 0, 0, 0 };

// 暂停状态保存
struct PauseState {
	vector<Unit*> savedUnits;
	vector<Item*> savedItems;
	int savedSpawnTimer;
	int savedItemSpawnTimer;
	bool isValid;
} pauseState;

// 输入状态
int mouseX = 0, mouseY = 0;
bool mouseDown = false;
bool keyUpDown = false, keyDownDown = false;
bool keyLeftDown = false, keyRightDown = false;
bool keyEscDown = false;
bool keySpaceDown = false;

// 游戏变量
int spawnTimer = 0;
int itemSpawnTimer = 0;
int levelStartTime = 0;
int gameStartTime = 0;
int clusterSpawnTimer = 0;  // 新增：集群刷新计时器
Unit* player = NULL;

// 帧动画
int FRAMES_HOLD[] = { 0 };
int FRAMES_HOLD_COUNT = 1;
int FRAMES_WALK[] = { 0, 0, 0, 0, 1, 1, 1, 1 };
int FRAMES_WALK_COUNT = 8;
int FRAMES_ATTACK[] = { 2, 2, 2, 2, 3, 3, 3, 3 };
int FRAMES_ATTACK_COUNT = 8;

#pragma endregion

#pragma region Win32框架

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CONTRAGAME, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) return FALSE;

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CONTRAGAME));
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CONTRAGAME);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	HWND hWnd = CreateWindow(szWindowClass, szTitle,
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WINDOW_WIDTH, WINDOW_HEIGHT + WINDOW_TITLEBARHEIGHT,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		InitGame(hWnd, wParam, lParam);
		break;
	case WM_KEYDOWN:
		KeyDown(hWnd, wParam, lParam);
		break;
	case WM_KEYUP:
		KeyUp(hWnd, wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		MouseMove(hWnd, wParam, lParam);
		break;
	case WM_LBUTTONDOWN:
		LButtonDown(hWnd, wParam, lParam);
		break;
	case WM_LBUTTONUP:
		LButtonUp(hWnd, wParam, lParam);
		break;
	case WM_TIMER:
		TimerUpdate(hWnd, wParam, lParam);
		break;
	case WM_PAINT:
		Paint(hWnd);
		break;
	case WM_DESTROY:
		StopBGM();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#pragma endregion

#pragma region 初始化和事件处理

void InitGame(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	srand((unsigned)time(NULL));

	// 加载图像资源
	bmp_start_background = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_START_BG));
	bmp_help_background = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_HELP_BG));
	bmp_game_background = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_STAGE_BG));
	bmp_GameTitle = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_TITLE));

	bmp_StartButton = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_START));
	bmp_HelpButton = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_HELP));
	bmp_BackButton = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_BACK));
	bmp_ResumeButton = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_RESUME));
	bmp_RestartButton = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_RESTART));
	bmp_MenuButton = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_MENU));
	bmp_NextButton = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_NEXT));
	bmp_AboutButton = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_ABOUT));

	bmp_Player = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_PLAYER));
	bmp_Player2 = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_PLAYER2));

	bmp_ContinueButton = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_CONTINUE));
	bmp_Player1Icon = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_PLAYER1_ICON));
	bmp_Player2Icon = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_PLAYER2_ICON));

	// 加载鱼类图片
	bmp_Fish_Small_Left[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_S1_Left));
	bmp_Fish_Small_Left[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_S2_Left));
	bmp_Fish_Small_Left[2] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_S3_Left));

	bmp_Fish_Small_Right[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_S1_Right));
	bmp_Fish_Small_Right[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_S2_Right));
	bmp_Fish_Small_Right[2] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_S3_Right));

	bmp_Fish_Medium_Left[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_M1_Left));
	bmp_Fish_Medium_Left[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_M2_Left));
	bmp_Fish_Medium_Left[2] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_M3_Left));

	bmp_Fish_Medium_Right[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_M1_Right));
	bmp_Fish_Medium_Right[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_M2_Right));
	bmp_Fish_Medium_Right[2] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_M3_Right));

	bmp_Fish_Large_Left[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_L1_Left));
	bmp_Fish_Large_Left[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_L2_Left));
	bmp_Fish_Large_Left[2] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_L3_Left));

	bmp_Fish_Large_Right[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_L1_Right));
	bmp_Fish_Large_Right[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_L2_Right));
	bmp_Fish_Large_Right[2] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_FISH_L3_Right));

	// 加载道具图片
	bmp_Item_Speed = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_ITEM_SPEED));
	bmp_Item_Heal = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_ITEM_HEAL));
	bmp_Item_Shield = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_ITEM_SHIELD));
	bmp_Item_Shrink = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_ITEM_SHRINK));
	bmp_Item_Star = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_ITEM_STAR));

	// 加载关卡背景
	bmp_Stage_BG[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_STAGE1_BG));
	bmp_Stage_BG[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_STAGE2_BG));
	bmp_Stage_BG[2] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_STAGE3_BG));
	bmp_Stage_BG[3] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_STAGE4_BG));
	bmp_Stage_BG[4] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP_STAGE5_BG));

	// 创建按钮
	buttons.push_back(CreateButton(BUTTON_STARTGAME, bmp_StartButton, 200, 75,
		(WINDOW_WIDTH - 200) / 2, 425));
	buttons.push_back(CreateButton(BUTTON_HELP, bmp_HelpButton, 210, 73,
		(WINDOW_WIDTH - 200) / 2-250, 425));
	buttons.push_back(CreateButton(BUTTON_BACK, bmp_BackButton, 200, 80,
		(WINDOW_WIDTH - 200) / 2, 575));
	buttons.push_back(CreateButton(BUTTON_RESUME, bmp_ResumeButton, 200, 78,
		(WINDOW_WIDTH - 200) / 2, 350));
	buttons.push_back(CreateButton(BUTTON_RESTART, bmp_RestartButton, 200, 69,
		(WINDOW_WIDTH - 200) / 2, 450));
	buttons.push_back(CreateButton(BUTTON_MAINMENU, bmp_MenuButton, 200, 72,
		(WINDOW_WIDTH - 200) / 2, 550));
	buttons.push_back(CreateButton(BUTTON_NEXTLEVEL, bmp_NextButton, 200, 71,
		(WINDOW_WIDTH - 200) / 2, 450));  
	buttons.push_back(CreateButton(BUTTON_CONTINUE, bmp_ContinueButton, 200, 75,
		(WINDOW_WIDTH - 200) / 2, 325));
	buttons.push_back(CreateButton(BUTTON_PLAYER1, bmp_Player1Icon, 128, 128,
		(WINDOW_WIDTH - 200) / 2 - 150, 300));
	buttons.push_back(CreateButton(BUTTON_PLAYER2, bmp_Player2Icon, 128, 128,
		(WINDOW_WIDTH - 200) / 2 + 150, 300));
	buttons.push_back(CreateButton(BUTTON_ABOUT, bmp_AboutButton, 200, 75,
		(WINDOW_WIDTH - 200) / 2+250, 425));

	// 初始化游戏数据
	gameData.score = 0;
	gameData.level = 1;
	gameData.lives = 3;
	gameData.playerSize = 1.0;
	gameData.hasKey = false;
	gameData.itemCount = 0;
	gameData.characterType = PLAYER_TYPE_RED;
	gameData.speedBoostTimer = 0;
	gameData.shieldTimer = 0;
	gameData.fishEaten = 0;
	gameData.itemsCollected = 0;
	gameData.hiddenStarsCollected = 0;
	gameData.perfectRun = true;
	gameData.secretUnlocked = false;

	pauseState.isValid = false;

	InitStage(hWnd, STAGE_STARTMENU);
	SetTimer(hWnd, TIMER_GAMETIMER, TIMER_GAMETIMER_ELAPSE, NULL);
	PlayBGM();
}

void KeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_UP:
	case 'W':
		keyUpDown = true;
		break;
	case VK_DOWN:
	case 'S':
		keyDownDown = true;
		break;
	case VK_LEFT:
	case 'A':
		keyLeftDown = true;
		break;
	case VK_RIGHT:
	case 'D':
		keyRightDown = true;
		break;
	case VK_ESCAPE:
		if (!keyEscDown && currentStage->stageID >= STAGE_1 && currentStage->stageID <= STAGE_5)
		{
			PauseGame(hWnd);
		}
		keyEscDown = true;
		break;
	case VK_SPACE:
		if (!keySpaceDown && currentStage->stageID >= STAGE_1 && currentStage->stageID <= STAGE_5)
		{
			PauseGame(hWnd);
		}
		keySpaceDown = true;
		break;
	}
}

void KeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_UP:
	case 'W':
		keyUpDown = false;
		break;
	case VK_DOWN:
	case 'S':
		keyDownDown = false;
		break;
	case VK_LEFT:
	case 'A':
		keyLeftDown = false;
		break;
	case VK_RIGHT:
	case 'D':
		keyRightDown = false;
		break;
	case VK_ESCAPE:
		keyEscDown = false;
		break;
	case VK_SPACE:
		keySpaceDown = false;
		break;
	}
}

void MouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	mouseX = LOWORD(lParam);
	mouseY = HIWORD(lParam);
}

void LButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	mouseX = LOWORD(lParam);
	mouseY = HIWORD(lParam);
	mouseDown = true;

	for (int i = 0; i < buttons.size(); i++)
	{
		Button* button = buttons[i];
		if (button->visible &&
			mouseX >= button->x && mouseX <= button->x + button->width &&
			mouseY >= button->y && mouseY <= button->y + button->height)
		{
			PlayGameSound(0);

			switch (button->buttonID)
			{
			case BUTTON_STARTGAME:
				if (currentStage->stageID == STAGE_STARTMENU) {
					InitStage(hWnd, STAGE_CHARACTER_SELECT);
					break;
				}

				if (gameData.level == 1){
					InitStage(hWnd, STAGE_1);
				}
				else if (gameData.level == 2) {
					InitStage(hWnd, STAGE_2);
				}
				else if (gameData.level == 3) {
					InitStage(hWnd, STAGE_3);
				}
				else if (gameData.level == 4) {
					InitStage(hWnd, STAGE_4);
				}
				else if (gameData.level == 5) {
					InitStage(hWnd, STAGE_5);
				}
				break;

			case BUTTON_HELP:
				InitStage(hWnd, STAGE_HELP);
				break;
			case BUTTON_ABOUT:
				InitStage(hWnd, STAGE_ABOUT);
				break;
			case BUTTON_BACK:
				InitStage(hWnd, STAGE_STARTMENU);
				break;
			case BUTTON_RESUME:
				ResumeGame(hWnd);
				break;
			case BUTTON_RESTART:
				ResetGame(hWnd);
				InitStage(hWnd, STAGE_LEVEL_INFO);  // 从关卡信息开始
				break;
			case BUTTON_MAINMENU:
				pauseState.isValid = false;  // 清除暂停状态
				InitStage(hWnd, STAGE_STARTMENU);
				break;
			case BUTTON_NEXTLEVEL:
				NextLevel(hWnd);
				break;
			case BUTTON_CONTINUE:
				if (HasSaveFile())
				{
					LoadGame();
					InitStage(hWnd, STAGE_LEVEL_INFO);  // 显示关卡信息
				}
				break;
			case BUTTON_PLAYER1:
				gameData.characterType = PLAYER_TYPE_RED;
				ResetGame(hWnd);
				InitStage(hWnd, STAGE_LEVEL_INFO);  // 显示关卡信息
				break;
			case BUTTON_PLAYER2:
				gameData.characterType = PLAYER_TYPE_YELLOW;
				ResetGame(hWnd);
				InitStage(hWnd, STAGE_LEVEL_INFO);  // 显示关卡信息
				break;
			}
		}
	}
}

void LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	mouseX = LOWORD(lParam);
	mouseY = HIWORD(lParam);
	mouseDown = false;
}

void TimerUpdate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (currentStage->stageID >= STAGE_1 && currentStage->stageID <= STAGE_5)
	{
		UpdateUnits(hWnd);
		UpdateItems(hWnd);
		UpdateFloatingTexts();
		CheckCollisions(hWnd);
		RemoveDeadUnits();
		UpdateParticles();

		spawnTimer++;
		// 难度递增：关卡越高，刷新越快
		int spawnInterval = 80 - (gameData.level * 10);  // 80, 70, 60, 50, 40
		if (spawnTimer > spawnInterval)
		{
			SpawnEnemy(hWnd);
			spawnTimer = 0;
		}

		// 集群刷新系统
		clusterSpawnTimer++;
		int clusterInterval = 600 - (gameData.level * 50);  // 每关卡减少间隔
		if (clusterSpawnTimer > clusterInterval)
		{
			int clusterSize = 3 + gameData.level;  // 集群大小随关卡增加
			SpawnEnemyCluster(hWnd, clusterSize);
			clusterSpawnTimer = 0;
		}

		// 隐藏任务1：收集3个星星
		if (gameData.hiddenStarsCollected >= 3 && !gameData.secretUnlocked)
		{
			gameData.secretUnlocked = true;
			gameData.playerSize += 0.02;
			player->size = gameData.playerSize;
			gameData.score += 200;
			CreateFloatingText(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 50,
				L"SECRET UNLOCKED!", RGB(255, 215, 0), 40);
			CreateFloatingText(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2,
				L"+0.5 Size & +200 Points!", RGB(255, 255, 0), 24);
			PlayGameSound(9);
		}

		// 隐藏任务2：吃掉100条鱼获得额外生命
		if (gameData.fishEaten >= 100 && gameData.lives < 5)
		{
			gameData.lives++;
			gameData.fishEaten = 0;
			CreateFloatingText(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2,
				L"EXTRA LIFE!", RGB(0, 255, 0), 30);
			PlayGameSound(9);
		}

		itemSpawnTimer++;
		int itemInterval = 200 - (gameData.level * 20);  // 道具刷新也加快
		if (itemSpawnTimer > itemInterval)
		{
			SpawnItem(hWnd);
			itemSpawnTimer = 0;
		}

		if (gameData.speedBoostTimer > 0)
			gameData.speedBoostTimer--;
		if (gameData.shieldTimer > 0)
			gameData.shieldTimer--;

		if (gameData.score >= currentStage->levelGoal)
		{
			GameOver(hWnd, true);
		}

		if (player && !player->alive)
		{
			gameData.lives--;
			gameData.perfectRun = false;
			if (gameData.lives <= 0)
			{
				GameOver(hWnd, false);
			}
			else
			{
				player->alive = true;
				player->health = 100;
				player->x = WINDOW_WIDTH / 2;
				player->y = WINDOW_HEIGHT / 2;
				CreateFloatingText(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 50,
					L"RESPAWN!", RGB(0, 255, 255), 30);
			}
		}
	}

	InvalidateRect(hWnd, NULL, FALSE);
}

#pragma endregion

#pragma region 游戏逻辑

Button* CreateButton(int buttonID, HBITMAP img, int width, int height, int x, int y)
{
	Button* button = new Button();
	button->buttonID = buttonID;
	button->img = img;
	button->width = width;
	button->height = height;
	button->x = x;
	button->y = y;
	button->visible = false;
	return button;
}

Unit* CreateUnit(int side, int type, double x, double y, double size)
{
	Unit* unit = new Unit();
	unit->side = side;
	unit->type = type;
	unit->state = UNIT_STATE_WALK;
	unit->effectType = FISH_EFFECT_NONE;

	bool fromLeft = (x < WINDOW_WIDTH / 2);
	unit->direction = fromLeft ? UNIT_DIRECT_RIGHT : UNIT_DIRECT_LEFT;

	if (side == UNIT_SIDE_PLAYER)
	{
		if (gameData.characterType == PLAYER_TYPE_YELLOW)
		{
			unit->img_left = bmp_Player2;
			unit->img_right = bmp_Player2;
		}
		else
		{
			unit->img_left = bmp_Player;
			unit->img_right = bmp_Player;
		}
		unit->img = unit->img_right;
	}
	else
	{
		int idx = rand() % 3;
		if (type == FISH_TYPE_SMALL)
		{
			unit->img_left = bmp_Fish_Small_Left[idx];
			unit->img_right = bmp_Fish_Small_Right[idx];
			if (idx == 1 && rand() % 100 < 10)
				unit->effectType = FISH_EFFECT_GOLD;
		}
		else if (type == FISH_TYPE_MEDIUM)
		{
			unit->img_left = bmp_Fish_Medium_Left[idx];
			unit->img_right = bmp_Fish_Medium_Right[idx];
			if (idx == 1 && rand() % 100 < 15)
				unit->effectType = FISH_EFFECT_POISON;
		}
		else if (type == FISH_TYPE_LARGE)
		{
			unit->img_left = bmp_Fish_Large_Left[idx];
			unit->img_right = bmp_Fish_Large_Right[idx];
			if (idx == 2 && rand() % 100 < 10)
				unit->effectType = FISH_EFFECT_BOMB;
		}

		if (unit->direction == UNIT_DIRECT_LEFT)
			unit->img = unit->img_left;
		else
			unit->img = unit->img_right;
	}

	unit->frame_row = 0;
	unit->frame_column = 0;
	unit->frame_sequence = FRAMES_WALK;
	unit->frame_count = FRAMES_WALK_COUNT;
	unit->frame_id = 0;

	unit->x = x;
	unit->y = y;

	if (side == UNIT_SIDE_ENEMY)
	{
		// 随机AI类型
		int rnd = rand() % 100;
		if (rnd < 60)
			unit->aiType = 0;  // 直线游走
		else if (rnd < 85)
			unit->aiType = 1;  // 追逐玩家
		else
			unit->aiType = 2;  // 逃离玩家

		if (unit->direction == UNIT_DIRECT_RIGHT)
			unit->vx = 2.5;
		else
			unit->vx = -2.5;
		unit->vy = 0;
	}
	else
	{
		unit->vx = 0;
		unit->vy = 0;
	}

	unit->size = size;
	unit->health = 100;
	unit->alive = true;
	unit->aiTimer = 0;
	unit->targetX = 0;
	unit->targetY = 0;

	return unit;
}

void InitStage(HWND hWnd, int stageID)
{
	// 如果不是从暂停恢复，清除暂停状态
	if (stageID != STAGE_1 && stageID != STAGE_2 && stageID != STAGE_3 &&
		stageID != STAGE_4 && stageID != STAGE_5)
	{
		pauseState.isValid = false;
	}

	if (currentStage != NULL) delete currentStage;
	currentStage = new Stage();
	currentStage->stageID = stageID;
	currentStage->timerOn = false;

	for (int i = 0; i < buttons.size(); i++)
		buttons[i]->visible = false;

	// 只有在非暂停恢复时才清除单位
	if (!pauseState.isValid || stageID == STAGE_PAUSE)
	{
		for (int i = 0; i < units.size(); i++)
			delete units[i];
		units.clear();
		for (int i = 0; i < particles.size(); i++)
			delete particles[i];
		particles.clear();
		for (int i = 0; i < floatingTexts.size(); i++)
			delete floatingTexts[i];
		floatingTexts.clear();
		for (int i = 0; i < items.size(); i++)
			delete items[i];
		items.clear();
	}

	player = NULL;
	spawnTimer = 0;

	switch (stageID)
	{
	case STAGE_STARTMENU:
		currentStage->bg = bmp_start_background;
		buttons[0]->visible = true;
		buttons[1]->visible = true;
		buttons[10]->visible = true;
		if (HasSaveFile())
			buttons[7]->visible = true;
		break;

	case STAGE_HELP:
		currentStage->bg = bmp_help_background;
		buttons[2]->visible = true;
		break;

	case STAGE_ABOUT:
		currentStage->bg = bmp_help_background;
		buttons[2]->visible = true;
		break;

	case STAGE_CHARACTER_SELECT:
		currentStage->bg = bmp_start_background;
		currentStage->timerOn = false;
		buttons[8]->visible = true;
		buttons[9]->visible = true;
		buttons[2]->visible = true;
		break;

	case STAGE_LEVEL_INFO:
		currentStage->bg = bmp_help_background;
		currentStage->timerOn = false;
		// 显示开始按钮，而不是自动跳转
		buttons[0]->visible = true;  // 复用开始按钮作为"开始关卡"按钮
		buttons[2]->visible = true;  // 返回按钮
		break;

	case STAGE_1:
	case STAGE_2:
	case STAGE_3:
	case STAGE_4:
	case STAGE_5:
	{
		int level = stageID - STAGE_1;
		currentStage->bg = bmp_Stage_BG[level];
		currentStage->timerOn = true;
		if (gameData.level == 1) currentStage->levelGoal = 100;
		else if (gameData.level == 2) currentStage->levelGoal = 200;
		else if (gameData.level == 3) currentStage->levelGoal = 350;
		else if (gameData.level == 4) currentStage->levelGoal = 550;
		else if (gameData.level == 5) currentStage->levelGoal = 800;

		wsprintf(currentStage->levelDesc, L"Level %d: Eat %d points!", level + 1, currentStage->levelGoal);

		player = CreateUnit(UNIT_SIDE_PLAYER, FISH_TYPE_MEDIUM,
			WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, gameData.playerSize);
		units.push_back(player);

		// 初始敌人数量随关卡增加
		for (int i = 0; i < 3 + level; i++)
		{
			SpawnEnemy(hWnd);
		}

		if (stageID == STAGE_1)
		{
			gameStartTime = GetTickCount();
			gameData.perfectRun = true;
		}
		levelStartTime = GetTickCount();
		clusterSpawnTimer = 0;
		break;
	}

	case STAGE_PAUSE:
		currentStage->bg = bmp_help_background;
		currentStage->timerOn = false;
		buttons[3]->visible = true;
		buttons[4]->visible = true;
		buttons[5]->visible = true;
		break;

	case STAGE_VICTORY:
		currentStage->bg = bmp_help_background;
		currentStage->timerOn = false;
		if (gameData.level < 5)
			buttons[6]->visible = true;
		buttons[5]->visible = true;
		break;

	case STAGE_GAMEOVER:
		currentStage->bg = bmp_help_background;
		currentStage->timerOn = false;
		buttons[4]->visible = true;
		buttons[5]->visible = true;
		break;
	}

	InvalidateRect(hWnd, NULL, FALSE);
}

void UpdateUnits(HWND hWnd)
{
	for (int i = 0; i < units.size(); i++)
	{
		Unit* unit = units[i];
		if (!unit->alive) continue;

		if (unit->side == UNIT_SIDE_PLAYER)
		{
			UpdatePlayer(unit);
		}
		else
		{
			UpdateEnemy(unit, player);
		}

		if (unit->frame_id >= unit->frame_count)
    		unit->frame_id = 0;

		int column = unit->frame_sequence[unit->frame_id];
		unit->frame_column = column;

		unit->x += unit->vx;
		unit->y += unit->vy;

		CheckBoundary(unit);
	}
}

void UpdatePlayer(Unit* player)
{
	if (!player) return;

	double speed = UNIT_SPEED;
	if (gameData.speedBoostTimer > 0)
		speed *= 1.5;
	else if (gameData.speedBoostTimer < 0)
		speed *= 0.6;  // 中毒减速

	double dirX = 0, dirY = 0;

	if (keyUpDown) dirY = -1;
	if (keyDownDown) dirY = 1;
	if (keyLeftDown) dirX = -1;
	if (keyRightDown) dirX = 1;

	double dirLen = sqrt(dirX * dirX + dirY * dirY);

	int next_state = player->state;

	if (dirLen > 0.01)
	{
		next_state = UNIT_STATE_WALK;
		dirX /= dirLen;
		dirY /= dirLen;

		player->vx = dirX * speed;
		player->vy = dirY * speed;

		if (dirX > 0)
		{
			player->direction = UNIT_DIRECT_RIGHT;
			player->img = player->img_right;
		}
		else if (dirX < 0)
		{
			player->direction = UNIT_DIRECT_LEFT;
			player->img = player->img_left;
		}
	}
	else
	{
		next_state = UNIT_STATE_HOLD;
		player->vx = 0;
		player->vy = 0;
	}

	if (next_state != player->state)
	{
		player->state = next_state;
		player->frame_id = 0;

		switch (player->state)
		{
		case UNIT_STATE_HOLD:
			player->frame_sequence = FRAMES_HOLD;
			player->frame_count = FRAMES_HOLD_COUNT;
			break;
		case UNIT_STATE_WALK:
			player->frame_sequence = FRAMES_WALK;
			player->frame_count = FRAMES_WALK_COUNT;
			break;
		}
	}
}

void UpdateEnemy(Unit* enemy, Unit* player)
{
	if (!enemy || !player) return;

	enemy->aiTimer++;

	// AI类型0: 直线游走
	if (enemy->aiType == 0)
	{
		if (enemy->direction == UNIT_DIRECT_RIGHT)
		{
			enemy->vx = 2.5;
			enemy->img = enemy->img_right;
		}
		else
		{
			enemy->vx = -2.5;
			enemy->img = enemy->img_left;
		}

		enemy->vy = sin((double)GetTickCount() / 500.0) * 0.3;

		if (enemy->x < -100 || enemy->x > WINDOW_WIDTH + 100 ||
			enemy->y < -100 || enemy->y > WINDOW_HEIGHT + 100)
		{
			enemy->alive = false;
		}
		return;
	}

	// 定期更新目标
	if (enemy->aiTimer > 60)
	{
		enemy->aiTimer = 0;

		// AI类型1: 追逐玩家（体型相近或更小时）
		if (enemy->aiType == 1)
		{
			double sizeRatio = enemy->size / player->size;
			// 体型相近时会互相攻击
			if (sizeRatio > 0.8 && sizeRatio < 1.2)
			{
				enemy->targetX = player->x;
				enemy->targetY = player->y;
			}
			else if (enemy->size < player->size * EAT_SIZE_RATIO)
			{
				// 太小时逃跑
				enemy->targetX = 2 * enemy->x - player->x;
				enemy->targetY = 2 * enemy->y - player->y;
			}
			else
			{
				// 更大时追逐
				enemy->targetX = player->x;
				enemy->targetY = player->y;
			}
		}
		// AI类型2: 逃离玩家（体型更小时）
		else if (enemy->aiType == 2)
		{
			if (enemy->size < player->size)
			{
				enemy->targetX = 2 * enemy->x - player->x;
				enemy->targetY = 2 * enemy->y - player->y;
			}
			else
			{
				// 如果变得更大，转为追逐
				enemy->targetX = player->x;
				enemy->targetY = player->y;
			}
		}
	}

	double dx = enemy->targetX - enemy->x;
	double dy = enemy->targetY - enemy->y;
	double dist = sqrt(dx * dx + dy * dy);

	if (dist > 10)
	{
		double speed = UNIT_SPEED * 0.7;
		// 体型越大速度越慢
		speed *= (2.0 / (1.0 + enemy->size));

		enemy->vx = dx / dist * speed;
		enemy->vy = dy / dist * speed;

		if (enemy->vx > 0)
		{
			enemy->direction = UNIT_DIRECT_RIGHT;
			enemy->img = enemy->img_right;
		}
		else if (enemy->vx < 0)
		{
			enemy->direction = UNIT_DIRECT_LEFT;
			enemy->img = enemy->img_left;
		}
	}
}

void CheckCollisions(HWND hWnd)
{
	if (!player || !player->alive) return;

	for (int i = 0; i < units.size(); i++)
	{
		Unit* enemy = units[i];
		if (enemy == player || !enemy->alive) continue;

		double dx = player->x - enemy->x;
		double dy = player->y - enemy->y;
		double dist = sqrt(dx * dx + dy * dy);

		if (dist < COLLISION_DISTANCE * (player->size + enemy->size) / 2)
		{
			// 玩家可以吃掉敌人
			if (player->size > enemy->size * EAT_SIZE_RATIO)
			{
				enemy->alive = false;
				int points = (int)(enemy->size * 10);

				switch (enemy->effectType)
				{
				case FISH_EFFECT_GOLD:
					points *= 2;
					CreateFloatingText(enemy->x, enemy->y - 40, L"GOLD FISH!", RGB(255, 215, 0), 24);
					CreateFloatingText(enemy->x, enemy->y - 10, L"x2 BONUS", RGB(255, 255, 0), 16);
					PlayGameSound(6);
					break;

				case FISH_EFFECT_POISON:
					gameData.speedBoostTimer = -180;
					CreateFloatingText(enemy->x, enemy->y - 40, L"POISON!", RGB(0, 255, 0), 24);
					CreateFloatingText(enemy->x, enemy->y - 10, L"Slow down...", RGB(100, 255, 100), 16);
					PlayGameSound(7);
					break;

				case FISH_EFFECT_BOMB:
					player->health -= 20;
					gameData.perfectRun = false;
					CreateFloatingText(enemy->x, enemy->y - 40, L"BOMB!", RGB(255, 0, 0), 24);
					CreateFloatingText(enemy->x, enemy->y - 10, L"-20 HP", RGB(255, 100, 100), 16);
					PlayGameSound(3);
					break;

				default:
					CreateFloatingText(enemy->x, enemy->y - 30, L"Yummy!", RGB(0, 255, 0), 16);
					PlayGameSound(1);
					break;
				}

				gameData.score += points;
				gameData.fishEaten++;
				double growthRate = 0.01 / (gameData.playerSize *100+0.01);
				if (growthRate < 0.00001) growthRate = 0.00001;
				gameData.playerSize += growthRate;
				player->size = gameData.playerSize;

				wchar_t text[32];
				wsprintf(text, L"+%d", points);
				CreateFloatingText(enemy->x, enemy->y - 30, text, RGB(255, 215, 0), 24);

				CreateParticles(enemy->x, enemy->y, RGB(255, 200, 100));
			}
			// 敌人可以吃掉玩家
			else if (enemy->size > player->size * EAT_SIZE_RATIO)
			{
				if (gameData.shieldTimer > 0)
				{
					CreateFloatingText(player->x, player->y - 30, L"PROTECTED!", RGB(0, 191, 255), 20);
					enemy->alive = false;
					CreateParticles(enemy->x, enemy->y, RGB(0, 191, 255));
				}
				else
				{
					player->health -= 30;
					gameData.perfectRun = false;

					// 击退效果
					double knockbackDist = 80.0;
					double dx = player->x - enemy->x;
					double dy = player->y - enemy->y;
					double dist = sqrt(dx * dx + dy * dy) + 0.001;

					player->x += (dx / dist) * knockbackDist;
					player->y += (dy / dist) * knockbackDist;

					CheckBoundary(player);

					CreateFloatingText(player->x, player->y - 30, L"-30 HP", RGB(255, 0, 0), 20);
					CreateFloatingText(player->x, player->y + 10, L"Knocked Back!", RGB(255, 100, 100), 16);
					PlayGameSound(3);

					CreateParticles(player->x, player->y, RGB(255, 0, 0));

					if (player->health <= 0)
					{
						player->alive = false;
						CreateFloatingText(player->x, player->y, L"WASTED!", RGB(255, 0, 0), 30);
						PlayGameSound(2);
					}
				}
			}
			// 体型相近时互相攻击
			else
			{
				// 双方都受伤
				player->health -= 10;
				enemy->health -= 10;
				gameData.perfectRun = false;

				// 互相击退
				double knockbackDist = 50.0;
				double repelDist = sqrt(dx * dx + dy * dy) + 0.001;

				player->x += (dx / repelDist) * knockbackDist;
				player->y += (dy / repelDist) * knockbackDist;
				enemy->x -= (dx / repelDist) * knockbackDist;
				enemy->y -= (dy / repelDist) * knockbackDist;

				CheckBoundary(player);
				CheckBoundary(enemy);

				CreateFloatingText(player->x, player->y - 30, L"-10 HP", RGB(255, 100, 0), 16);
				CreateFloatingText(enemy->x, enemy->y - 30, L"-10 HP", RGB(255, 100, 0), 16);
				PlayGameSound(3);

				CreateParticles(player->x, player->y, RGB(255, 128, 0));
				CreateParticles(enemy->x, enemy->y, RGB(255, 128, 0));

				if (enemy->health <= 0)
				{
					enemy->alive = false;
				}

				if (player->health <= 0)
				{
					player->alive = false;
					CreateFloatingText(player->x, player->y, L"WASTED!", RGB(255, 0, 0), 30);
					PlayGameSound(2);
				}
			}
		}
	}
}

void SpawnEnemy(HWND hWnd)
{
	if (units.size() >= MAX_ENEMIES + 1) return;  // +1 for player

	double x, y;
	bool fromLeft = (rand() % 2 == 0);

	if (fromLeft)
		x = -50;
	else
		x = WINDOW_WIDTH + 50;

	y = 100 + rand() % (WINDOW_HEIGHT - 200);

	// 难度递增：后期关卡大鱼比例增加
	int typeRoll = rand() % 100;
	int type;
	if (typeRoll < 40 - gameData.level * 5)
		type = FISH_TYPE_SMALL;
	else if (typeRoll < 70)
		type = FISH_TYPE_MEDIUM;
	else
		type = FISH_TYPE_LARGE;

	// 体型范围随关卡调整
	double minSize = 0.5 + gameData.level * 0.05;
	double maxSize = 2.0 + gameData.level * 0.1;
	double size = minSize + (rand() % 100) / 100.0 * (maxSize - minSize);

	Unit* enemy = CreateUnit(UNIT_SIDE_ENEMY, type, x, y, size);
	units.push_back(enemy);
}

void SpawnEnemyCluster(HWND hWnd, int count)
{
	int edge = rand() % 4;
	double centerX, centerY;

	if (edge == 0)
	{
		centerX = -100;
		centerY = WINDOW_HEIGHT / 2;
	}
	else if (edge == 1)
	{
		centerX = WINDOW_WIDTH + 100;
		centerY = WINDOW_HEIGHT / 2;
	}
	else if (edge == 2)
	{
		centerX = WINDOW_WIDTH / 2;
		centerY = -100;
	}
	else
	{
		centerX = WINDOW_WIDTH / 2;
		centerY = WINDOW_HEIGHT + 100;
	}

	int type = rand() % 3;
	for (int i = 0; i < count; i++)
	{
		double offsetX = (rand() % 200) - 100;
		double offsetY = (rand() % 200) - 100;
		double x = centerX + offsetX;
		double y = centerY + offsetY;

		double size = 0.6 + (rand() % 80) / 100.0;
		Unit* enemy = CreateUnit(UNIT_SIDE_ENEMY, type, x, y, size);
		units.push_back(enemy);
	}

	CreateFloatingText(WINDOW_WIDTH / 2, 100, L"Enemy Swarm!", RGB(255, 0, 0), 30);
	PlayGameSound(8);
}

void RemoveDeadUnits()
{
	for (int i = units.size() - 1; i >= 0; i--)
	{
		if (!units[i]->alive && units[i] != player)
		{
			delete units[i];
			units.erase(units.begin() + i);
		}
	}
}

bool CheckBoundary(Unit* unit)
{
	if (unit->side == UNIT_SIDE_PLAYER)
	{
		if (unit->x < 32) unit->x = 32;
		if (unit->x > WINDOW_WIDTH - 32) unit->x = WINDOW_WIDTH - 32;
		if (unit->y < 32) unit->y = 32;
		if (unit->y > WINDOW_HEIGHT - 32) unit->y = WINDOW_HEIGHT - 32;
	}
	return true;
}

void CreateParticles(double x, double y, COLORREF color)
{
	for (int i = 0; i < 10; i++)
	{
		Particle* p = new Particle();
		p->x = x;
		p->y = y;
		double angle = (rand() % 360) * 3.14159 / 180.0;
		double speed = 2 + rand() % 4;
		p->vx = cos(angle) * speed;
		p->vy = sin(angle) * speed;
		p->life = 30;
		p->maxLife = 30;
		p->color = color;
		particles.push_back(p);
	}
}

void UpdateParticles()
{
	for (int i = particles.size() - 1; i >= 0; i--)
	{
		Particle* p = particles[i];
		p->x += p->vx;
		p->y += p->vy;
		p->life--;

		if (p->life <= 0)
		{
			delete p;
			particles.erase(particles.begin() + i);
		}
	}
}

// 保存暂停状态
void SavePauseState()
{
	// 清除旧状态
	for (auto u : pauseState.savedUnits)
		delete u;
	pauseState.savedUnits.clear();

	for (auto it : pauseState.savedItems)
		delete it;
	pauseState.savedItems.clear();

	// 深拷贝单位
	for (auto unit : units)
	{
		Unit* copy = new Unit(*unit);
		pauseState.savedUnits.push_back(copy);
	}

	// 深拷贝道具
	for (auto item : items)
	{
		Item* copy = new Item(*item);
		pauseState.savedItems.push_back(copy);
	}

	pauseState.savedSpawnTimer = spawnTimer;
	pauseState.savedItemSpawnTimer = itemSpawnTimer;
	pauseState.isValid = true;
}

// 恢复暂停状态
void RestorePauseState()
{
	if (!pauseState.isValid) return;

	// 清除当前状态
	for (auto u : units)
		delete u;
	units.clear();

	for (auto it : items)
		delete it;
	items.clear();

	// 恢复保存的状态
	for (auto unit : pauseState.savedUnits)
	{
		Unit* copy = new Unit(*unit);
		units.push_back(copy);
		if (copy->side == UNIT_SIDE_PLAYER)
			player = copy;
	}

	for (auto item : pauseState.savedItems)
	{
		Item* copy = new Item(*item);
		items.push_back(copy);
	}

	spawnTimer = pauseState.savedSpawnTimer;
	itemSpawnTimer = pauseState.savedItemSpawnTimer;
}

void PauseGame(HWND hWnd)
{
	SavePauseState();
	InitStage(hWnd, STAGE_PAUSE);
}

void ResumeGame(HWND hWnd)
{
	RestorePauseState();
	InitStage(hWnd, STAGE_1 + gameData.level - 1);
	currentStage->timerOn = true;
}

void GameOver(HWND hWnd, bool victory)
{
	if (victory)
	{
		SaveGame();
		gameData.totalTime = (GetTickCount() - gameStartTime) / 1000;

		// 通关奖励系统
		if (gameData.level < 5)
		{
			// 每关卡奖励
			gameData.playerSize += 0.02;  // 体型增加
			gameData.lives++;           // 额外生命
			CreateFloatingText(WINDOW_WIDTH / 2, 250,
				L"LEVEL COMPLETE!", RGB(255, 255, 0), 36);
			CreateFloatingText(WINDOW_WIDTH / 2, 300,
				L"+1 Life & Size Boost!", RGB(0, 255, 0), 24);
		}

		InitStage(hWnd, STAGE_VICTORY);
		PlayGameSound(4);
	}
	else
	{
		InitStage(hWnd, STAGE_GAMEOVER);
		PlayGameSound(5);
	}

	pauseState.isValid = false;  // 清除暂停状态
}

void NextLevel(HWND hWnd)
{
	gameData.level++;
	if (gameData.level <= 5)
	{
		SaveGame();
		InitStage(hWnd, STAGE_LEVEL_INFO);
	}
	else
	{
		CreateFloatingText(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2,
			L"ALL LEVELS CLEARED!", RGB(255, 215, 0), 40);
		InitStage(hWnd, STAGE_VICTORY);
	}

	pauseState.isValid = false;  // 清除暂停状态
}

void ResetGame(HWND hWnd)
{
	gameData.score = 0;
	gameData.level = 1;
	gameData.lives = 3;
	gameData.playerSize = 1.0;
	gameData.hasKey = false;
	gameData.itemCount = 0;
	gameData.speedBoostTimer = 0;
	gameData.shieldTimer = 0;
	gameData.fishEaten = 0;
	gameData.itemsCollected = 0;
	gameData.hiddenStarsCollected = 0;
	gameData.perfectRun = true;
	gameData.secretUnlocked = false;

	pauseState.isValid = false;  // 清除暂停状态
}

#pragma endregion

#pragma region 道具系统

Item* CreateItem(int type, double x, double y)
{
	Item* item = new Item();
	item->type = type;
	item->x = x;
	item->y = y;
	item->active = true;
	item->lifeTime = 300;

	switch (type)
	{
	case ITEM_SPEEDUP:
		item->img = bmp_Item_Speed;
		break;
	case ITEM_HEAL:
		item->img = bmp_Item_Heal;
		break;
	case ITEM_SHIELD:
		item->img = bmp_Item_Shield;
		break;
	case ITEM_SHRINK:
		item->img = bmp_Item_Shrink;
		break;
	case ITEM_STAR:
		item->img = bmp_Item_Star;
		break;
	}

	return item;
}

void SpawnItem(HWND hWnd)
{
	if (items.size() >= MAX_ITEMS) return;

	int type;
	int rnd = rand() % 100;
	if (rnd < 40)
		type = ITEM_SPEEDUP;
	else if (rnd < 70)
		type = ITEM_HEAL;
	else if (rnd < 85)
		type = ITEM_SHIELD;
	else if (rnd < 95)
		type = ITEM_SHRINK;
	else
		type = ITEM_STAR;

	double x = 100 + rand() % (MAP_WIDTH - 200);
	double y = 100 + rand() % (MAP_HEIGHT - 200);

	Item* item = CreateItem(type, x, y);
	items.push_back(item);
}

void UpdateItems(HWND hWnd)
{
	if (!player || !player->alive) return;

	for (int i = items.size() - 1; i >= 0; i--)
	{
		Item* item = items[i];

		double dx = player->x - item->x;
		double dy = player->y - item->y;
		double dist = sqrt(dx * dx + dy * dy);

		if (dist < 50)
		{
			switch (item->type)
			{
			case ITEM_SPEEDUP:
				gameData.speedBoostTimer = 300;
				CreateFloatingText(item->x, item->y - 20, L"SPEED UP!", RGB(255, 255, 0), 20);
				CreateFloatingText(item->x, item->y + 10, L"10 seconds", RGB(255, 215, 0), 14);
				break;

			case ITEM_HEAL:
				player->health = 100;
				CreateFloatingText(item->x, item->y - 20, L"FULL HP!", RGB(255, 0, 127), 20);
				CreateFloatingText(item->x, item->y + 10, L"+100 HP", RGB(255, 100, 180), 14);
				break;

			case ITEM_SHIELD:
				gameData.shieldTimer = 600;
				CreateFloatingText(item->x, item->y - 20, L"SHIELD!", RGB(0, 191, 255), 20);
				CreateFloatingText(item->x, item->y + 10, L"20 seconds", RGB(100, 200, 255), 14);
				break;

			case ITEM_SHRINK:
				gameData.playerSize *= 0.8;
				if (gameData.playerSize < 0.5) gameData.playerSize = 0.5;
				player->size = gameData.playerSize;
				CreateFloatingText(item->x, item->y - 20, L"SHRINK!", RGB(128, 128, 128), 20);
				CreateFloatingText(item->x, item->y + 10, L"Oh no...", RGB(150, 150, 150), 14);
				break;

			case ITEM_STAR:
				gameData.hiddenStarsCollected++;
				gameData.score += 50;
				gameData.itemsCollected++;
				CreateFloatingText(item->x, item->y - 20, L"STAR!", RGB(255, 215, 0), 24);
				CreateFloatingText(item->x, item->y + 10, L"+50 BONUS", RGB(255, 255, 0), 16);
				break;
			}

			CreateParticles(item->x, item->y, RGB(255, 255, 0));
			PlayGameSound(10);

			delete item;
			items.erase(items.begin() + i);
			continue;
		}

		item->lifeTime--;
		if (item->lifeTime <= 0)
		{
			delete item;
			items.erase(items.begin() + i);
		}
	}
}

#pragma endregion

#pragma region 浮动文字系统

void CreateFloatingText(double x, double y, const wchar_t* text, COLORREF color, int fontSize)
{
	FloatingText* ft = new FloatingText();
	ft->x = x;
	ft->y = y;
	ft->vy = -1.5;
	ft->life = 60;
	ft->maxLife = 60;
	wcscpy_s(ft->text, 32, text);
	ft->color = color;
	ft->fontSize = fontSize;
	floatingTexts.push_back(ft);
}

void UpdateFloatingTexts()
{
	for (int i = floatingTexts.size() - 1; i >= 0; i--)
	{
		FloatingText* ft = floatingTexts[i];
		ft->y += ft->vy;
		ft->life--;

		if (ft->life <= 0)
		{
			delete ft;
			floatingTexts.erase(floatingTexts.begin() + i);
		}
	}
}

void DrawFloatingTexts(HDC hdc)
{
	for (int i = 0; i < floatingTexts.size(); i++)
	{
		FloatingText* ft = floatingTexts[i];

		int alpha = (ft->life * 255) / ft->maxLife;

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, ft->color);

		HFONT hFont = CreateFont(ft->fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

		TextOut(hdc, (int)ft->x, (int)ft->y, ft->text, wcslen(ft->text));

		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
	}
}

#pragma endregion

#pragma region 音频系统

void PlayBGM()
{
	PlaySoundW(L"audio\\bgm.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP | SND_NODEFAULT);
}

void StopBGM()
{
	PlaySoundW(NULL, NULL, 0 | SND_NODEFAULT);
}

void PlayGameSound(int soundID)
{
	// 播放音效 =====
	switch (soundID)
	{
	case 0: // 按钮点击
		PlaySoundW(L"audio\\click.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	case 1: // 吃掉
		PlaySoundW(L"audio\\eat.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	case 2: // 死亡
		PlaySoundW(L"audio\\death.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	case 3: // 受伤
		PlaySoundW(L"audio\\hurt.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	case 4: // 胜利
		PlaySoundW(L"audio\\victory.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	case 5: // 失败
		PlaySoundW(L"audio\\gameover.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	case 6: // 金鱼
		PlaySoundW(L"audio\\gold.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	case 7: // 中毒
		PlaySoundW(L"audio\\poison.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	case 8: // 集群
		PlaySoundW(L"audio\\warning.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	case 9: // 解锁
		PlaySoundW(L"audio\\unlock.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	case 10: // 拾取道具
		PlaySoundW(L"audio\\item.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		break;
	}
}

#pragma endregion

#pragma region 存档系统

void SaveGame()
{
	std::ofstream file("fishgame.sav", std::ios::binary);
	if (file.is_open())
	{
		saveData.level = gameData.level;
		saveData.score = gameData.score;
		saveData.lives = gameData.lives;
		saveData.playerSize = gameData.playerSize;
		saveData.characterType = gameData.characterType;
		saveData.fishEaten = gameData.fishEaten;
		saveData.itemsCollected = gameData.itemsCollected;
		saveData.hiddenStarsCollected = gameData.hiddenStarsCollected;

		file.write((char*)&saveData, sizeof(SaveData));
		file.close();
	}
}

bool LoadGame()
{
	std::ifstream file("fishgame.sav", std::ios::binary);
	if (file.is_open())
	{
		file.read((char*)&saveData, sizeof(SaveData));
		file.close();

		gameData.level = saveData.level;
		gameData.score = saveData.score;
		gameData.lives = saveData.lives;
		gameData.playerSize = saveData.playerSize;
		gameData.characterType = saveData.characterType;
		gameData.fishEaten = saveData.fishEaten;
		gameData.itemsCollected = saveData.itemsCollected;
		gameData.hiddenStarsCollected = saveData.hiddenStarsCollected;

		return true;
	}
	return false;
}

bool HasSaveFile()
{
	std::ifstream file("fishgame.sav");
	return file.good();
}

#pragma endregion

// 绘图系统 - 关卡信息界面增强版

void Paint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc_window = BeginPaint(hWnd, &ps);
	HDC hdc_memBuffer = CreateCompatibleDC(hdc_window);
	HDC hdc_loadBmp = CreateCompatibleDC(hdc_window);

	HBITMAP blankBmp = CreateCompatibleBitmap(hdc_window, WINDOW_WIDTH, WINDOW_HEIGHT);
	SelectObject(hdc_memBuffer, blankBmp);

	// 绘制背景
	if (currentStage->bg)
	{
		SelectObject(hdc_loadBmp, currentStage->bg);
		BitBlt(hdc_memBuffer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hdc_loadBmp, 0, 0, SRCCOPY);
	}

	if (currentStage->stageID == STAGE_STARTMENU)
	{
		if (bmp_GameTitle)
		{
			SelectObject(hdc_loadBmp, bmp_GameTitle);
			TransparentBlt(hdc_memBuffer,
				(WINDOW_WIDTH - 400) / 2, 70, 400, 200,
				hdc_loadBmp, 0, 0, 400, 200,
				RGB(255, 255, 255));
		}
	}

	// 游戏主界面
	if (currentStage->stageID >= STAGE_1 && currentStage->stageID <= STAGE_5)
	{
		// 绘制单位
		for (int i = 0; i < units.size(); i++)
		{
			Unit* unit = units[i];
			if (!unit->alive) continue;

			SelectObject(hdc_loadBmp, unit->img);
			int w = (int)(UNIT_SIZE_X * unit->size);
			int h = (int)(UNIT_SIZE_Y * unit->size);

			int srcX = 0;
			TransparentBlt(hdc_memBuffer,
				(int)(unit->x - w / 2), (int)(unit->y - h / 2), w, h,
				hdc_loadBmp,
				UNIT_SIZE_X * unit->frame_column, UNIT_SIZE_Y * unit->frame_row,
				UNIT_SIZE_X, UNIT_SIZE_Y,
				RGB(255, 255, 255));
		}

		// 绘制道具
		for (int i = 0; i < items.size(); i++)
		{
			Item* item = items[i];
			if (!item->active) continue;

			SelectObject(hdc_loadBmp, item->img);
			TransparentBlt(hdc_memBuffer,
				(int)(item->x - 16), (int)(item->y - 16), 64, 64,
				hdc_loadBmp, 0, 0, 64, 64,
				RGB(255, 255, 255));
		}

		DrawParticles(hdc_memBuffer);
		DrawUI(hdc_memBuffer, hdc_loadBmp);
		DrawFloatingTexts(hdc_memBuffer);
	}
	else if (currentStage->stageID == STAGE_HELP)
	{
		DrawHelpScreen(hdc_memBuffer);
	}
	else if (currentStage->stageID == STAGE_ABOUT)
	{
		DrawAboutScreen(hdc_memBuffer);
	}
	else if (currentStage->stageID == STAGE_CHARACTER_SELECT)
	{
		DrawCharacterSelect(hdc_memBuffer);
	}
	else if (currentStage->stageID == STAGE_LEVEL_INFO)
	{
		DrawLevelInfoScreen(hdc_memBuffer);
	}
	else if (currentStage->stageID == STAGE_VICTORY)
	{
		DrawVictoryScreen(hdc_memBuffer);
	}
	else if (currentStage->stageID == STAGE_GAMEOVER)
	{
		DrawGameOverScreen(hdc_memBuffer);
	}
	else if (currentStage->stageID == STAGE_PAUSE)
	{
		DrawPauseScreen(hdc_memBuffer);
	}

	// 绘制按钮
	for (int i = 0; i < buttons.size(); i++)
	{
		Button* button = buttons[i];
		if (button->visible)
		{
			SelectObject(hdc_loadBmp, button->img);
			TransparentBlt(hdc_memBuffer,
				button->x, button->y, button->width, button->height,
				hdc_loadBmp, 0, 0, button->width, button->height,
				RGB(255, 255, 255));
		}
	}

	BitBlt(hdc_window, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hdc_memBuffer, 0, 0, SRCCOPY);

	DeleteObject(blankBmp);
	DeleteDC(hdc_memBuffer);
	DeleteDC(hdc_loadBmp);
	EndPaint(hWnd, &ps);
}

// 绘制帮助界面（含道具和敌人说明）
void DrawHelpScreen(HDC hdc)
{
	SetBkMode(hdc, TRANSPARENT);

	// 标题
	SetTextColor(hdc, RGB(255, 215, 0));
	HFONT hFontTitle = CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFontTitle);
	TextOut(hdc, WINDOW_WIDTH / 2 - 120, 30, L"游戏说明", 4);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFontTitle);

	// 正文
	SetTextColor(hdc, RGB(255, 255, 255));
	HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	hOldFont = (HFONT)SelectObject(hdc, hFont);

	const wchar_t* helpText[] = {
		L"【基本操作】",
		L"  • WASD/方向键 - 控制移动",
		L"  • ESC/空格 - 暂停游戏",
		L"",
		L"【游戏规则】",
		L"  • 吃掉比自己小的鱼来增长体型",
		L"  • 躲避比自己大的鱼",
		L"  • 体型相近时会互相攻击，双方受伤",
		L"  • 达到目标分数即可过关",
		L"",
		L"【道具说明】",
		L"  闪电 - 加速10秒 | 红心 - 恢复满血",
		L"  盾牌 - 无敌20秒 | 星星 - 隐藏奖励",
		L"",
		L"【特殊鱼类】",
		L"  金鱼 - 双倍分数 | 绿鱼 - 中毒减速",
		L"  爆炸鱼 - 扣除20血",
		L"",
		L"【隐藏任务】",
		L"  • 收集3颗星星解锁秘密奖励",
		L"  • 吃掉100条鱼获得额外生命",
	};

	int y = 100;
	for (int i = 0; i < 20; i++)
	{
		if (wcschr(helpText[i], L'【'))
			SetTextColor(hdc, RGB(255, 255, 0));  // 标题黄色
		else if (wcschr(helpText[i], L'•'))
			SetTextColor(hdc, RGB(200, 200, 200));  // 列表灰色
		else
			SetTextColor(hdc, RGB(180, 180, 255));  // 内容浅蓝

		TextOut(hdc, 100, y, helpText[i], wcslen(helpText[i]));
		y += 28;
	}

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}

// 绘制关于界面
void DrawAboutScreen(HDC hdc)
{
	SetBkMode(hdc, TRANSPARENT);

	SetTextColor(hdc, RGB(255, 215, 0));
	HFONT hFontTitle = CreateFont(50, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFontTitle);
	TextOut(hdc, WINDOW_WIDTH / 2 - 150, 100, L"关于游戏", 4);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFontTitle);

	SetTextColor(hdc, RGB(255, 255, 255));
	HFONT hFont = CreateFont(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	hOldFont = (HFONT)SelectObject(hdc, hFont);

	const wchar_t* aboutText[] = {
		L"游戏名称: 贪吃鱼大冒险",
		L"",
		L"开发者: Guanlin Shang",
		L"",
		L"游戏简介：",
		L"  这是一款休闲益智的吃鱼游戏。",
		L"  在海洋世界中，你将扮演一条小鱼，",
		L"  通过吃掉比自己小的鱼来不断成长。",
		L"  小心那些体型比你大的鱼！",
		L"",
		L"版本: 2.0",
		L"发布日期: 2025"
	};

	for (int i = 0; i < 12; i++)
	{
		TextOut(hdc, 180, 200 + i * 40, aboutText[i], wcslen(aboutText[i]));
	}

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}

// 绘制角色选择界面
void DrawCharacterSelect(HDC hdc)
{
	SetBkMode(hdc, TRANSPARENT);

	SetTextColor(hdc, RGB(255, 255, 0));
	HFONT hFont = CreateFont(50, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
	TextOut(hdc, WINDOW_WIDTH / 2 - 180, 150, L"选择你的角色", 6);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);

	// 角色1介绍
	SetTextColor(hdc, RGB(255, 100, 100));
	hFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	hOldFont = (HFONT)SelectObject(hdc, hFont);
	TextOut(hdc, 280, 450, L"红色鱼", 3);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);

	hFont = CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	hOldFont = (HFONT)SelectObject(hdc, hFont);
	TextOut(hdc, 280, 500, L"均衡型", 3);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);

	// 角色2介绍
	SetTextColor(hdc, RGB(255, 255, 100));
	hFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	hOldFont = (HFONT)SelectObject(hdc, hFont);
	TextOut(hdc, 600, 450, L"黄色鱼", 3);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);

	hFont = CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	hOldFont = (HFONT)SelectObject(hdc, hFont);
	TextOut(hdc, 600, 500, L"速度型", 3);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}

// 绘制关卡信息界面（详细版）
void DrawLevelInfoScreen(HDC hdc)
{
	SetBkMode(hdc, TRANSPARENT);

	// 关卡标题
	SetTextColor(hdc, RGB(255, 215, 0));
	HFONT hFontTitle = CreateFont(60, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFontTitle);

	wchar_t levelTitle[50];
	wsprintf(levelTitle, L"第 %d 关", gameData.level);
	SIZE titleSize;
	GetTextExtentPoint32(hdc, levelTitle, wcslen(levelTitle), &titleSize);
	TextOut(hdc, (WINDOW_WIDTH - titleSize.cx) / 2, 80, levelTitle, wcslen(levelTitle));

	SelectObject(hdc, hOldFont);
	DeleteObject(hFontTitle);

	// 关卡描述
	SetTextColor(hdc, RGB(255, 255, 255));
	HFONT hFont = CreateFont(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	hOldFont = (HFONT)SelectObject(hdc, hFont);

	// 通关条件
	wchar_t info[100];
	int goalScore;
	if (gameData.level == 1) goalScore = 100;
	else if (gameData.level == 2) goalScore = 200;
	else if (gameData.level == 3) goalScore = 350;
	else if (gameData.level == 4) goalScore = 550;
	else if (gameData.level == 5) goalScore = 800;

	wsprintf(info, L"目标分数: %d", goalScore);
	GetTextExtentPoint32(hdc, info, wcslen(info), &titleSize);
	TextOut(hdc, (WINDOW_WIDTH - titleSize.cx) / 2, 180, info, wcslen(info));

	// 难度说明
	const wchar_t* difficulty;
	COLORREF diffColor;
	if (gameData.level <= 2)
	{
		difficulty = L"难度: ★☆☆☆☆ 简单";
		diffColor = RGB(100, 255, 100);
	}
	else if (gameData.level <= 4)
	{
		difficulty = L"难度: ★★★☆☆ 中等";
		diffColor = RGB(255, 255, 100);
	}
	else
	{
		difficulty = L"难度: ★★★★★ 困难";
		diffColor = RGB(255, 100, 100);
	}

	SetTextColor(hdc, diffColor);
	GetTextExtentPoint32(hdc, difficulty, wcslen(difficulty), &titleSize);
	TextOut(hdc, (WINDOW_WIDTH - titleSize.cx) / 2, 220, difficulty, wcslen(difficulty));

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);

	// 关卡特性说明
	HFONT hFontSmall = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	hOldFont = (HFONT)SelectObject(hdc, hFontSmall);
	SetTextColor(hdc, RGB(200, 200, 255));

	const wchar_t* levelDesc[5][3] = {
		{L"• 新手关卡，敌人较少", L"• 小鱼为主，容易吞食", L"• 熟悉游戏操作"},
		{L"• 敌人数量增加", L"• 出现中型鱼", L"• 注意躲避大鱼"},
		{L"• 大量敌人集群出现", L"• 金鱼和毒鱼增多", L"• 善用道具"},
		{L"• 大型鱼群密集", L"• 爆炸鱼频繁出现", L"• 考验走位技巧"},
		{L"• 最终挑战！", L"• 所有类型敌人", L"• 达成完美结局吧！"}
	};

	for (int i = 0; i < 3; i++)
	{
		GetTextExtentPoint32(hdc, levelDesc[gameData.level - 1][i],
			wcslen(levelDesc[gameData.level - 1][i]), &titleSize);
		TextOut(hdc, (WINDOW_WIDTH - titleSize.cx) / 2, 280 + i * 35,
			levelDesc[gameData.level - 1][i], wcslen(levelDesc[gameData.level - 1][i]));
	}

	// 提示
	SetTextColor(hdc, RGB(255, 255, 0));
	hFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	SelectObject(hdc, hFont);
	const wchar_t* tip = L"点击开始进入关卡";
	GetTextExtentPoint32(hdc, tip, wcslen(tip), &titleSize);
	TextOut(hdc, (WINDOW_WIDTH - titleSize.cx) / 2, 500, tip, wcslen(tip));

	SelectObject(hdc, hOldFont);
	DeleteObject(hFontSmall);
	DeleteObject(hFont);
}

void DrawPauseScreen(HDC hdc)
{
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 0, 0));

	HFONT hFont = CreateFont(60, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	wchar_t text[100];
	wsprintf(text, L"游戏暂停！");
	SIZE textSize;
	GetTextExtentPoint32(hdc, text, wcslen(text), &textSize);
	TextOut(hdc, (WINDOW_WIDTH - textSize.cx) / 2, 200, text, wcslen(text));

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}

// 绘制胜利界面
void DrawVictoryScreen(HDC hdc)
{
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 255, 0));

	HFONT hFont = CreateFont(60, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	wchar_t text[100];
	wsprintf(text, L"恭喜过关！");
	SIZE textSize;
	GetTextExtentPoint32(hdc, text, wcslen(text), &textSize);
	TextOut(hdc, (WINDOW_WIDTH - textSize.cx) / 2, 150, text, wcslen(text));

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);

	// 统计信息
	hFont = CreateFont(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	hOldFont = (HFONT)SelectObject(hdc, hFont);
	SetTextColor(hdc, RGB(255, 255, 255));

	wsprintf(text, L"最终得分: %d", gameData.score);
	TextOut(hdc, WINDOW_WIDTH / 2 - 100, 250, text, wcslen(text));

	wsprintf(text, L"吃掉鱼类: %d", gameData.fishEaten);
	TextOut(hdc, WINDOW_WIDTH / 2 - 100, 290, text, wcslen(text));

	wsprintf(text, L"收集道具: %d", gameData.itemsCollected);
	TextOut(hdc, WINDOW_WIDTH / 2 - 100, 330, text, wcslen(text));

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);

	DrawFloatingTexts(hdc);
}

// 绘制游戏失败界面
void DrawGameOverScreen(HDC hdc)
{
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 0, 0));

	HFONT hFont = CreateFont(60, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	wchar_t text[100];
	wsprintf(text, L"游戏失败！");
	SIZE textSize;
	GetTextExtentPoint32(hdc, text, wcslen(text), &textSize);
	TextOut(hdc, (WINDOW_WIDTH - textSize.cx) / 2, 200, text, wcslen(text));

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);

	// 鼓励文字
	hFont = CreateFont(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	hOldFont = (HFONT)SelectObject(hdc, hFont);
	SetTextColor(hdc, RGB(200, 200, 200));

	const wchar_t* encouragement = L"不要放弃，再试一次吧！";
	GetTextExtentPoint32(hdc, encouragement, wcslen(encouragement), &textSize);
	TextOut(hdc, (WINDOW_WIDTH - textSize.cx) / 2, 300, encouragement, wcslen(encouragement));

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}


void DrawUI(HDC hdc, HDC hdc_load)
{
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));

	HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	wchar_t text[100];

	wsprintf(text, L"得分: %d / %d", gameData.score, currentStage->levelGoal);
	TextOut(hdc, 20, 20, text, wcslen(text));

	wsprintf(text, L"生命: %d", gameData.lives);
	TextOut(hdc, 20, 50, text, wcslen(text));

	const wchar_t* sizeRank = L"S";
	if (gameData.playerSize > 2.0) sizeRank = L"XXX_L";
	else if (gameData.playerSize > 1.5) sizeRank = L"XL";
	else if (gameData.playerSize > 1.2) sizeRank = L"L";
	else if (gameData.playerSize > 1.1) sizeRank = L"M";
	wsprintf(text, L"体型: %s", sizeRank);
	TextOut(hdc, 20, 80, text, wcslen(text));

	wsprintf(text, L"关卡: %d", gameData.level);
	TextOut(hdc, WINDOW_WIDTH - 120, 20, text, wcslen(text));

	// 修复：血量条显示
	if (player && player->alive)
	{
		int barWidth = 200;
		int barHeight = 25;
		int barX = WINDOW_WIDTH - barWidth - 20;
		int barY = 50;

		// 绘制血量条标签
		wsprintf(text, L"血量");
		TextOut(hdc, barX - 60, barY + 2, text, wcslen(text));

		// 绘制血量条边框
		HBRUSH brushBorder = CreateSolidBrush(RGB(0, 0, 0));
		RECT rectBorder = { barX, barY, barX + barWidth, barY + barHeight };
		FrameRect(hdc, &rectBorder, brushBorder);
		DeleteObject(brushBorder);

		// 绘制血量条填充
		int filledWidth = (barWidth * player->health) / 100;
		if (filledWidth > 0)
		{
			RECT filledRect = { barX + 2, barY + 2, barX + filledWidth - 2, barY + barHeight - 2 };

			// 根据血量选择颜色
			COLORREF barColor;
			if (player->health > 60)
				barColor = RGB(0, 255, 0);    // 绿色
			else if (player->health > 30)
				barColor = RGB(255, 255, 0);  // 黄色
			else
				barColor = RGB(255, 0, 0);    // 红色

			HBRUSH brushFill = CreateSolidBrush(barColor);
			FillRect(hdc, &filledRect, brushFill);
			DeleteObject(brushFill);
		}

		// 绘制血量数值（在血量条内部）
		wsprintf(text, L"%d%%", player->health);
		SetTextColor(hdc, RGB(255, 255, 255));

		// 计算文字居中位置
		SIZE textSize;
		GetTextExtentPoint32(hdc, text, wcslen(text), &textSize);
		int textX = barX + (barWidth - textSize.cx) / 2;
		int textY = barY + (barHeight - textSize.cy) / 2;

		// 绘制文字阴影效果
		SetTextColor(hdc, RGB(0, 0, 0));
		TextOut(hdc, textX + 1, textY + 1, text, wcslen(text));
		SetTextColor(hdc, RGB(255, 255, 255));
		TextOut(hdc, textX, textY, text, wcslen(text));
	}

	// 显示道具效果
	int effectY = 110;
	SetTextColor(hdc, RGB(255, 255, 255));

	if (gameData.speedBoostTimer > 0)
	{
		wsprintf(text, L"Speed: %d s", gameData.speedBoostTimer / 30);
		SetTextColor(hdc, RGB(255, 255, 0));
		TextOut(hdc, 20, effectY, text, wcslen(text));
		effectY += 30;
	}

	if (gameData.shieldTimer > 0)
	{
		wsprintf(text, L"Shield: %d s", gameData.shieldTimer / 30);
		SetTextColor(hdc, RGB(0, 191, 255));
		TextOut(hdc, 20, effectY, text, wcslen(text));
		effectY += 30;
	}

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}

void DrawParticles(HDC hdc)
{
	for (int i = 0; i < particles.size(); i++)
	{
		Particle* p = particles[i];
		int alpha = (p->life * 255) / p->maxLife;

		HBRUSH brush = CreateSolidBrush(p->color);
		HPEN pen = CreatePen(PS_SOLID, 1, p->color);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);

		Ellipse(hdc, (int)(p->x - 3), (int)(p->y - 3), (int)(p->x + 3), (int)(p->y + 3));

		SelectObject(hdc, oldBrush);
		SelectObject(hdc, oldPen);
		DeleteObject(brush);
		DeleteObject(pen);
	}
}

void DrawLevelInfo(HDC hdc)
{
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));

	HFONT hFont = CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	wchar_t text[256];
	wsprintf(text, L"最终得分: %d", gameData.score);
	TextOut(hdc, WINDOW_WIDTH / 2 - 100, 300, text, wcslen(text));

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}

HBITMAP InitBackGround(HWND hWnd, HBITMAP bmp_src)
{
	PAINTSTRUCT ps;
	HDC hdc_window = BeginPaint(hWnd, &ps);
	HDC hdc_memBuffer = CreateCompatibleDC(hdc_window);
	HDC hdc_loadBmp = CreateCompatibleDC(hdc_window);

	HBITMAP bmp_output = CreateCompatibleBitmap(hdc_window, WINDOW_WIDTH, WINDOW_HEIGHT);
	SelectObject(hdc_memBuffer, bmp_output);
	SelectObject(hdc_loadBmp, bmp_src);

	StretchBlt(hdc_memBuffer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
		hdc_loadBmp, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SRCCOPY);

	BitBlt(hdc_window, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hdc_memBuffer, 0, 0, SRCCOPY);

	DeleteDC(hdc_memBuffer);
	DeleteDC(hdc_loadBmp);
	EndPaint(hWnd, &ps);

	return bmp_output;
}

#pragma endregion

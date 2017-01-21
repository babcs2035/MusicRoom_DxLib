#include "DxLib.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Room.h"
#include <string.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#define MAX_LOAD_MUSIC			50
#define MUSIC_COMMENT_HEIGHT	15
#define MUSIC_COMMENT_WEIGHT	25
#define DIRECTORY_PASS_GRAPH	"data\\graph\\"
#define DIRECTORY_PASS_MUSIC	"data\\music\\"

// 構造体
typedef struct Music_s {
	char name[10000];
	int image;
	char creator[15];
	char comment[MUSIC_COMMENT_HEIGHT*MUSIC_COMMENT_WEIGHT];
	int sound;
}Music_data;

// グローバル変数
Music_data music[MAX_LOAD_MUSIC];
int G_main, G_frame, G_button[9], G_noimage, G_nowloading;
int NowMusicNum = 0;
int FirstTime, NowTime;
char MusicNumStr[5];
int MusicNum;
bool ChangeImage_flag = false;
int ChangeImage_for = 0;
int Music_TotalTime = -1;
int Music_NowTime = -1;
long long int Music_TotalSample = -1;
long long int Music_NowSample = -1;
int g_frametime = 0;
int g_lasttime = 0;
int g_starttime = 0;

// 初期化
void Room_Init()
{
	TCHAR Filename[10005];
	int FP_music_list = FileRead_open("data\\music_list.txt");
	FileRead_gets(MusicNumStr, sizeof(MusicNumStr), FP_music_list);
	MusicNum = atoi(MusicNumStr);
	G_nowloading = LoadGraph(DIRECTORY_PASS_GRAPH "system\\nowloading.png", TRUE);
	G_noimage = LoadGraph(DIRECTORY_PASS_GRAPH "system\\noimage.png", TRUE);
	for (int i = 0; i < MusicNum; ++i)
	{
		FileRead_gets(music[i].name, sizeof(music[i].name), FP_music_list);
		sprintfDx(Filename, DIRECTORY_PASS_GRAPH "%s.png", music[i].name);
		music[i].image = LoadGraph(Filename);
		if (music[i].image == -1) { music[i].image = G_noimage; }
		SetUseASyncLoadFlag(TRUE);
		sprintfDx(Filename, "data\\comment\\%s.txt", music[i].name);
		int FP_comment = FileRead_open(Filename, FALSE);
		FileRead_gets(music[i].creator, sizeof(music[i].creator), FP_comment);
		auto strlength = FileRead_read(music[i].comment, sizeof(music[i].comment) - 1, FP_comment);
		if (0 <= strlength) { music[i].comment[strlength] = '\0'; }
		FileRead_close(FP_comment);
		const char*const extensions[] = { ".wav",".ogg",".mp3" };
		music[i].sound = -1;
		for (int j = 0; j < sizeof(extensions) / sizeof(extensions[0]); ++j)
		{
			sprintfDx(Filename, DIRECTORY_PASS_MUSIC "%s%s", music[i].name, extensions[j]);
			if (PathFileExists(Filename) == TRUE)
			{
				music[i].sound = LoadBGM(Filename);
				break;
			}
		}
		SetUseASyncLoadFlag(FALSE);
	}
	FileRead_close(FP_music_list);
	G_main = LoadGraph(DIRECTORY_PASS_GRAPH "system\\main.png");
	G_frame = LoadGraph(DIRECTORY_PASS_GRAPH "system\\frame.png");
	LoadDivGraph(DIRECTORY_PASS_GRAPH "system\\button.png", 9, 3, 3, 40, 40, G_button);
	G_button[8] = LoadGraph(DIRECTORY_PASS_GRAPH "system\\time.png");
	FirstTime = GetNowCount();
}

// 更新
void Room_Update()
{
	g_lasttime = GetNowCount()&INT_MAX;
	int curtime = GetNowCount()&INT_MAX;
	g_frametime = (curtime - g_lasttime)&INT_MAX;
	g_lasttime = curtime;

	// 読み込み終わっているか確認
	for (int i = 0; i < MusicNum; ++i)
	{
		if ((CheckHandleASyncLoad(music[i].image)) == -1)
		{
			music[i].image = G_noimage;
		}
	}

	// 音楽選択
	NowTime = GetNowCount() - FirstTime;
	if (Keyboard_Get(KEY_INPUT_LEFT) != 0 && ChangeImage_flag == false)
	{
		StopSoundMem(music[NowMusicNum].sound);
		Music_TotalTime = 1;
		Music_NowTime = 0;
		Music_NowSample = 0;
		ChangeImage_for = 1;
		ChangeImage_flag = true;
		g_starttime = g_lasttime;
	}
	else if (Keyboard_Get(KEY_INPUT_RIGHT) != 0 && ChangeImage_flag == false)
	{
		StopSoundMem(music[NowMusicNum].sound);
		Music_TotalTime = 1;
		Music_NowTime = 0;
		Music_NowSample = 0;
		ChangeImage_for = 2;
		ChangeImage_flag = true;
		g_starttime = g_lasttime;
	}

	// 音楽再生
	PlayMusic_Update();
}

static const int DRAW_X_START_POINT = -75;						// 画面外から登場させる
static const int DRAW_HEIGHT = 200;								// 画像の高さ
static const int DRAW_WIDTH_S = 50;								// 画像の幅（小さいほう）
static const int DRAW_WIDTH_L = 200;							// 画像の幅（大きいほう）
static const int DRAW_Y_TOP = 25;								// yの変形（上）
static const int DRAW_BOTTOM = DRAW_Y_TOP + DRAW_HEIGHT;		// yの変形（下）
static const int Y_DEFAULT_DIFF = 50;							// yの沈み込み度
static const int DRAW_X_DISTANCE = 50;							// 画像と次の画像のx座標の間隔
static const int DRAW_TIME_COST = 500;							// 何ミリ秒かけて画像の移動・変形を行うか

static void DrawMI_L(int gr_handle, int draw_x, int y2_diff = 0, int x2_diff = 0, bool flag = false)
{
	int Draw_Width = (flag == true ? DRAW_WIDTH_L : DRAW_WIDTH_S);
	DrawModiGraph(
		draw_x, DRAW_Y_TOP,
		draw_x + Draw_Width + x2_diff, DRAW_Y_TOP + Y_DEFAULT_DIFF - y2_diff,
		draw_x + Draw_Width + x2_diff, DRAW_BOTTOM - Y_DEFAULT_DIFF + y2_diff,
		draw_x, DRAW_BOTTOM,
		gr_handle, TRUE
	);
}

static void DrawMI_R(int gr_handle, int draw_x, int y1_diff = 0, int x1_diff = 0, bool flag = false)
{
	int Draw_Width = (flag == true ? DRAW_WIDTH_L : DRAW_WIDTH_S);
	DrawModiGraph(
		draw_x + x1_diff, DRAW_Y_TOP + Y_DEFAULT_DIFF - y1_diff,
		draw_x + Draw_Width, DRAW_Y_TOP,
		draw_x + Draw_Width, DRAW_BOTTOM,
		draw_x + x1_diff, DRAW_BOTTOM - Y_DEFAULT_DIFF + y1_diff,
		gr_handle, TRUE
	);
}

// 描画
void Room_Draw()
{
	// 画像
	DrawGraph(0, 0, G_main, TRUE);
	if (ChangeImage_flag == true)
	{
		ChangeMusicImageGraph();
	}
	else if (ChangeImage_flag == false)
	{
		int draw_x = DRAW_X_START_POINT;
		for (int i = 0; i < 7; ++i)
		{
			auto& music_data = music[(MusicNum - (3 - i) + NowMusicNum) % MusicNum];
			int image_handle = music_data.image;
			if ((CheckHandleASyncLoad(music_data.sound)) == TRUE)
			{
				image_handle = G_nowloading;
			}
			if (i < 3)
			{
				DrawMI_L(image_handle, draw_x);
				draw_x += DRAW_WIDTH_S + DRAW_X_DISTANCE;
			}
			else if (i == 3)
			{
				DrawModiGraph(draw_x, DRAW_Y_TOP, draw_x + 200, DRAW_Y_TOP, draw_x + 200, 225, draw_x, 225, image_handle, TRUE);
				draw_x += 200 + DRAW_X_DISTANCE;
			}
			else
			{
				DrawMI_R(image_handle, draw_x);
				draw_x += DRAW_WIDTH_S + DRAW_X_DISTANCE;
			}
		}
	}
	DrawGraph(0, 0, G_frame, TRUE);

	// 音楽再生
	if (CheckHandleASyncLoad(music[NowMusicNum].sound) == FALSE)
	{
		PlayMusic_Draw();
	}
	else
	{
		DrawBox(15, 268, 625, 308, GetColor(32, 32, 32), TRUE);
		DrawString(27, 280, "Now Loadling... Please WAIT!", GetColor(255, 255, 255));
	}
}

// 音楽イメージ画像の変化描画
void ChangeMusicImageGraph()
{
	int per = (g_lasttime - g_starttime) * 100 / DRAW_TIME_COST;
	if (ChangeImage_for == 1)
	{
		int draw_x = DRAW_X_START_POINT + 6 * (DRAW_X_DISTANCE + DRAW_WIDTH_S);
		for (int i = 6; i >= 0; --i)
		{
			auto& music_data = music[(MusicNum - (3 - i) + NowMusicNum) % MusicNum];
			int image_handle = music_data.image;
			if ((CheckHandleASyncLoad(music_data.sound)) == TRUE)
			{
				image_handle = G_nowloading;
			}
			if (i < 2)
			{
				DrawMI_L(image_handle, draw_x + (DRAW_WIDTH_S + DRAW_X_DISTANCE)*per / 100);
			}
			else if (i == 2)
			{
				DrawMI_L(image_handle, draw_x + (DRAW_WIDTH_S + DRAW_X_DISTANCE)*per / 100, Y_DEFAULT_DIFF * per / 100, (DRAW_WIDTH_L - DRAW_X_DISTANCE)*per / 100);
			}
			else if (i == 3)
			{
				DrawMI_R(image_handle, draw_x + (DRAW_WIDTH_S + DRAW_X_DISTANCE)*per / 100, Y_DEFAULT_DIFF * abs(per - 100) / 100, (DRAW_WIDTH_L - DRAW_X_DISTANCE)*per / 100, TRUE);
			}
			else if (i > 3)
			{
				DrawMI_R(image_handle, draw_x + 150 + (DRAW_WIDTH_S + DRAW_X_DISTANCE)*per / 100);
			}
			draw_x -= DRAW_X_DISTANCE + DRAW_WIDTH_S;
		}
	}
	else
	{
		int draw_x = DRAW_X_START_POINT + 6 * (DRAW_X_DISTANCE + DRAW_WIDTH_S);
		for (int i = 6; i >= 0; --i)
		{
			auto& music_data = music[(MusicNum - (3 - i) + NowMusicNum) % MusicNum];
			int image_handle = music_data.image;
			if ((CheckHandleASyncLoad(music_data.sound)) == TRUE)
			{
				image_handle = G_nowloading;
			}
			if (i < 3)
			{
				DrawMI_L(image_handle, draw_x - (DRAW_WIDTH_S + DRAW_X_DISTANCE)*per / 100);
			}
			else if (i == 3)
			{
				DrawMI_L(image_handle, draw_x - (DRAW_WIDTH_S + DRAW_X_DISTANCE)*per / 100, Y_DEFAULT_DIFF * abs(per - 100) / 100, -(DRAW_WIDTH_L - DRAW_WIDTH_S)*per / 100, TRUE);
			}
			else if (i == 4)
			{
				DrawMI_R(image_handle, draw_x + (DRAW_WIDTH_L - DRAW_WIDTH_S) - (DRAW_WIDTH_S + DRAW_X_DISTANCE) *per / 100, Y_DEFAULT_DIFF * per / 100, -(DRAW_WIDTH_L - DRAW_WIDTH_S)*per / 100);
			}
			else if (i > 4)
			{
				DrawMI_R(image_handle, draw_x + (DRAW_WIDTH_L - DRAW_WIDTH_S) - (DRAW_WIDTH_S + DRAW_X_DISTANCE)*per / 100);
			}
			draw_x -= DRAW_X_DISTANCE + DRAW_WIDTH_S;
		}
	}
	if (g_lasttime - g_starttime >= DRAW_TIME_COST)
	{
		ChangeImage_flag = false; g_starttime = 0;
		if (ChangeImage_for == 1)
		{
			NowMusicNum--;
			if (NowMusicNum < 0)
			{
				NowMusicNum = MusicNum - 1;
			}
		}
		else
		{
			NowMusicNum++;
			if (NowMusicNum >= MusicNum)
			{
				NowMusicNum = 0;
			}
		}
		ChangeImage_for = 0;
		Music_TotalTime = -1;
		Music_NowTime = -1;
	}
}

// 音楽再生（更新）
void PlayMusic_Update()
{
	if (CheckHandleASyncLoad(music[NowMusicNum].sound) == FALSE)
	{
		Music_TotalTime = GetSoundTotalTime(music[NowMusicNum].sound);
		if (ChangeImage_flag == false)
		{
			if (CheckSoundMem(music[NowMusicNum].sound) == 1)
			{
				Music_NowTime = GetSoundCurrentTime(music[NowMusicNum].sound);
				Music_NowSample = GetCurrentPositionSoundMem(music[NowMusicNum].sound);
			}
		}
	}

	// コントロールボタン
	if (CheckMouseClick(15, 268, 55, 308) == true)
	{
		if (Music_NowTime == -1)
		{	// 最初から
			PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_LOOP, TRUE);
			Music_NowTime = GetSoundCurrentTime(music[NowMusicNum].sound);
			Music_NowSample = GetCurrentPositionSoundMem(music[NowMusicNum].sound);
		}
		else
		{	// 途中から
			SetSoundCurrentTime(Music_NowTime, music[NowMusicNum].sound);
			PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_LOOP, FALSE);
			Music_NowTime = GetSoundCurrentTime(music[NowMusicNum].sound);
			Music_NowSample = GetCurrentPositionSoundMem(music[NowMusicNum].sound);
		}
	}
	else if (CheckMouseClick(60, 268, 100, 308) == true)
	{
		// 一時停止
		Music_NowTime = GetSoundCurrentTime(music[NowMusicNum].sound);
		Music_NowSample = GetCurrentPositionSoundMem(music[NowMusicNum].sound);
		StopSoundMem(music[NowMusicNum].sound);
	}
	else if (CheckMouseClick(585, 268, 625, 308) == true)
	{
		// 停止
		StopSoundMem(music[NowMusicNum].sound);
		Music_TotalTime = -1;
		Music_NowTime = -1;
		Music_NowSample = -1;
	}
	else if (CheckMouseClick(110, 278, 575, 300) == true)
	{	// 再生位置変更
		if (CheckSoundMem(music[NowMusicNum].sound) == 1)
		{
			{	// 再生バーのクリック位置から新しい再生位置を計算
				int clicked_x, clicked_y;
				GetMousePoint(&clicked_x, &clicked_y);
				Music_TotalSample = GetSoundTotalSample(music[NowMusicNum].sound);
				Music_NowSample = (Music_TotalSample*(clicked_x - 110)) / 465;
			}
			{	// 再生位置変更
				StopSoundMem(music[NowMusicNum].sound);
				SetCurrentPositionSoundMem((int)Music_NowSample, music[NowMusicNum].sound);
				Music_NowTime = GetSoundCurrentTime(music[NowMusicNum].sound);
				PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_LOOP, FALSE);
			}
		}
	}
}

// 音楽再生（描画）
void PlayMusic_Draw()
{
	// 再生バー
	DrawRoundRect(110, 278, 575, 300, 3, 3, GetColor(32, 32, 32), TRUE);
	if (ChangeImage_flag == false)
	{
		if (Music_NowTime != -1 && Music_TotalTime != -1)
		{
			DrawRoundRect(112, 281, 112 + 459 * Music_NowTime / Music_TotalTime, 297, 3, 3, GetColor(32, 110, 32), TRUE);
			DrawGraph(95 + 459 * Music_NowTime / Music_TotalTime, 268, G_button[6], TRUE);
		}
		else
		{
			DrawGraph(95, 268, G_button[7], TRUE);
		}
	}

	// コントロールボタン
	if (CheckMouseIn(15, 268, 55, 308) == true)
	{
		DrawGraph(15, 268, G_button[3], TRUE);
	}
	else
	{
		DrawGraph(15, 268, G_button[0], TRUE);
	}
	if (CheckMouseIn(60, 268, 100, 308) == true)
	{
		DrawGraph(60, 268, G_button[5], TRUE);
	}
	else
	{
		DrawGraph(60, 268, G_button[2], TRUE);
	}
	if (CheckMouseIn(585, 268, 625, 308) == true)
	{
		DrawGraph(585, 268, G_button[4], TRUE);
	}
	else
	{
		DrawGraph(585, 268, G_button[1], TRUE);
	}
	if (CheckMouseIn(95 + 459 * Music_NowTime / Music_TotalTime, 268, 95 + 459 * Music_NowTime / Music_TotalTime + 40, 308) == true)
	{
		if (CheckSoundMem(music[NowMusicNum].sound) == 1)
		{
			int minute = Music_NowTime / 1000 / 60;
			int second = Music_NowTime / 1000 - minute * 60;
			DrawGraph(75 + 459 * Music_NowTime / Music_TotalTime, 238, G_button[8], TRUE);
			DrawFormatString(93 + 459 * Music_NowTime / Music_TotalTime, 246, GetColor(0, 0, 0), "%02d:%02d", minute, second);
		}
	}
	else if (CheckMouseIn(110, 278, 575, 300) == true)
	{
		int minute = Music_TotalTime / 1000 / 60;
		int second = Music_TotalTime / 1000 - minute * 60;
		DrawFormatString(525, 282, GetColor(255, 255, 255), "%02d:%02d", minute, second);
	}
}
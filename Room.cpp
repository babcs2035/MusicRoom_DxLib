// include
#include "DxLib.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Room.h"
#include <string.h>
#include <Shlwapi.h>
#include <mbctype.h>
#pragma comment(lib, "shlwapi.lib")

// define
#define MAX_LOAD_MUSIC			50
#define MUSIC_COMMENT_MAX		114514

// 構造体
typedef struct Music_s {
	char name[10000];
	int image;
	char creator[100];
	char comment[MUSIC_COMMENT_MAX + 1];
	int sound;
}Music_data;

static const int DRAW_X_START_POINT = -75;						// 画面外から登場させる
static const int DRAW_HEIGHT = 200;								// 画像の高さ
static const int DRAW_WIDTH_S = 50;								// 画像の幅（小さいほう）
static const int DRAW_WIDTH_L = 200;							// 画像の幅（大きいほう）
static const int DRAW_Y_TOP = 25;								// yの変形（上）
static const int DRAW_BOTTOM = DRAW_Y_TOP + DRAW_HEIGHT;		// yの変形（下）
static const int Y_DEFAULT_DIFF = 50;							// yの沈み込み度
static const int DRAW_X_DISTANCE = 50;							// 画像と次の画像のx座標の間隔
static const int DRAW_TIME_COST = 500;							// 何ミリ秒かけて画像の移動・変形を行うか
static const int MUSIC_PLAYPOS_CHANGE_LENGTH = 5000;			// 早送りを何ミリ秒行うか

// グローバル変数
Music_data music[MAX_LOAD_MUSIC];
int G_main, G_mainFrame, G_button[11], G_noimage, G_nowloading, G_imageFrame[2], G_updown[2];
int F_Name, F_Creator, F_Comment;
int NowMusicNum = 0, ChangeMusicNum = -1;
int FirstTime, NowTime;
char MusicNumStr[5];
int MusicNum = -1;
bool ChangeImage_flag = false;
int ChangeImage_for = 0;
int Music_TotalTime = -1;
int Music_NowTime = -1;
long long int Music_TotalSample = -1;
long long int Music_NowSample = -1;
int draw_MusicName_x = 30, draw_MusicCreator_x = 400;
int g_frametime = 0;
int g_lasttime = 0;
int g_starttime = 0;
int g_MusicName_lasttime = 0;
int g_MusicName_starttime = 0;
int g_MusicCreator_lasttime = 0;
int g_MusicCreator_starttime = 0;
int now_musicComment_limitY = 0;
bool Is_musicComment_Y_over = false;
bool Is_mouse_on_musicImage[MAX_LOAD_MUSIC] = { false };
bool Is_music_loop = false;
bool music_changed_by_end = false;

// 初期化
void Room_Init()
{
	TCHAR Filename[10005];
	char F_name[10005];
	G_nowloading = LoadGraph("data\\nowloading.png");
	G_noimage = LoadGraph("data\\noimage.png");
	LoadDivGraph("data\\image_frame.png", 2, 2, 1, 200, 200, G_imageFrame);
	int FP_music_list = FileRead_open("music\\music_list.txt");
	for (int i = 0; i < MAX_LOAD_MUSIC; ++i)
	{
		F_name[0] = '\0';
		FileRead_gets(F_name, sizeof(F_name), FP_music_list);
		if (F_name[0] == '\0')
		{
			MusicNum = i;
			break;
		}
		sprintfDx(Filename, "music\\%s\\%s.png", F_name, F_name);
		music[i].image = LoadGraph(Filename);
		if (music[i].image == -1) { music[i].image = G_noimage; }
		sprintfDx(Filename, "music\\%s\\%s.txt", F_name, F_name);
		int FP_comment = FileRead_open(Filename, FALSE);
		FileRead_gets(music[i].name, sizeof(music[i].name), FP_comment);
		FileRead_gets(music[i].creator, sizeof(music[i].creator), FP_comment);
		auto strlength = FileRead_read(music[i].comment, sizeof(music[i].comment) - 1, FP_comment);
		if (0 <= strlength) { music[i].comment[strlength] = '\0'; }
		FileRead_close(FP_comment);
		SetUseASyncLoadFlag(TRUE);
		const char*const extensions[] = { ".wav",".ogg",".mp3" };
		music[i].sound = -1;
		for (int j = 0; j < sizeof(extensions) / sizeof(extensions[0]); ++j)
		{
			sprintfDx(Filename, "music\\%s\\%s%s", F_name, F_name, extensions[j]);
			if (PathFileExists(Filename) == TRUE)
			{
				music[i].sound = LoadBGM(Filename);
				break;
			}
		}
		SetUseASyncLoadFlag(FALSE);
	}
	MusicNum = (MusicNum == -1 ? MAX_LOAD_MUSIC : MusicNum);
	FileRead_close(FP_music_list);
	G_main = LoadGraph("data\\main.png", TRUE);
	G_mainFrame = LoadGraph("data\\main_frame.png", TRUE);
	LoadDivGraph("data\\button.png", 10, 3, 4, 40, 40, G_button);
	G_button[10] = LoadGraph("data\\time.png");
	LoadDivGraph("data\\updown.png", 2, 2, 1, 40, 10, G_updown);
	F_Name = CreateFontToHandle("メイリオ", 32, 9, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
	F_Creator = CreateFontToHandle("メイリオ", 28, 7, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
	F_Comment = CreateFontToHandle("Meiryo UI", 24, 5, DX_FONTTYPE_ANTIALIASING_8X8);
	FirstTime = GetNowCount();
}

// 更新
void Room_Update()
{
	g_lasttime = GetNowCount()&INT_MAX;
	g_MusicName_lasttime = GetNowCount()&INT_MAX;
	g_MusicCreator_lasttime = GetNowCount()&INT_MAX;
	NowTime = GetNowCount() - FirstTime;

	// 読み込み終わっているか確認
	for (int i = 0; i < MusicNum; ++i)
	{
		if ((CheckHandleASyncLoad(music[i].image)) == -1)
		{
			music[i].image = G_noimage;
		}
	}

	// 音楽選択
	if (Keyboard_Get(KEY_INPUT_LEFT) != 0 && ChangeImage_flag == false)
	{
		StopSoundMem(music[NowMusicNum].sound);
		Music_TotalTime = 1;
		Music_NowTime = 0;
		Music_NowSample = 0;
		ChangeImage_for = 1;
		ChangeImage_flag = true;
		g_starttime = g_lasttime;
		g_MusicName_starttime = g_MusicName_lasttime;
		g_MusicCreator_starttime = g_MusicCreator_lasttime;
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
		g_MusicName_starttime = g_MusicName_lasttime;
		g_MusicCreator_starttime = g_MusicCreator_lasttime;
	}

	// 音楽再生
	PlayMusic_Update();

	// 作者＆コメント（更新）
	CreatorAndComment_Update();

	// クリックで音楽変更
	int draw_x = DRAW_X_START_POINT;
	for (int i = 0; i < 7; ++i)
	{
		if (i < 3)
		{
			if (CheckMouseIn(draw_x - 5, DRAW_Y_TOP - 5, draw_x + DRAW_WIDTH_S + 5, DRAW_Y_TOP + DRAW_HEIGHT + 5) == true)
			{
				Is_mouse_on_musicImage[i] = true;
			}
			else
			{
				Is_mouse_on_musicImage[i] = false;
			}
			if (CheckMouseClick(draw_x - 5, DRAW_Y_TOP - 5, draw_x + DRAW_WIDTH_S + 5, DRAW_Y_TOP + DRAW_HEIGHT + 5) == true)
			{
				ChangeImage_flag = true;
				ChangeImage_for = 1;
				ChangeMusicNum = abs(NowMusicNum + MusicNum - (3 - i)) % MusicNum;
				StopSoundMem(music[NowMusicNum].sound);
				Music_TotalTime = 1;
				Music_NowTime = 0;
				Music_NowSample = 0;
				g_starttime = g_lasttime;
				g_MusicName_starttime = g_MusicName_lasttime;
				g_MusicCreator_starttime = g_MusicCreator_lasttime;
				break;
			}
			draw_x += DRAW_WIDTH_S + DRAW_X_DISTANCE;
		}
		else if (i == 3)
		{
			draw_x += 200 + DRAW_X_DISTANCE;
		}
		else
		{
			if (CheckMouseIn(draw_x - 5, DRAW_Y_TOP - 5, draw_x + DRAW_WIDTH_S + 5, DRAW_Y_TOP + DRAW_HEIGHT + 5) == true)
			{
				Is_mouse_on_musicImage[i] = true;
			}
			else
			{
				Is_mouse_on_musicImage[i] = false;
			}
			if (CheckMouseClick(draw_x - 5, DRAW_Y_TOP - 5, draw_x + DRAW_WIDTH_S + 5, DRAW_Y_TOP + DRAW_HEIGHT + 5) == true)
			{
				ChangeImage_flag = true;
				ChangeImage_for = 2;
				ChangeMusicNum = abs(NowMusicNum + (i - 3)) % MusicNum;
				StopSoundMem(music[NowMusicNum].sound);
				Music_TotalTime = 1;
				Music_NowTime = 0;
				Music_NowSample = 0;
				g_starttime = g_lasttime;
				g_MusicName_starttime = g_MusicName_lasttime;
				g_MusicCreator_starttime = g_MusicCreator_lasttime;
				break;
			}
			draw_x += DRAW_WIDTH_S + DRAW_X_DISTANCE;
		}
	}
}

static void DrawMI_L(int gr_handle, int draw_x, int y2_diff = 0, int x2_diff = 0, bool flag = false, bool mouse_on_flag = false)
{
	int Draw_Width = (flag == true ? DRAW_WIDTH_L : DRAW_WIDTH_S);
	if (mouse_on_flag == true)
	{
		DrawModiGraph(
			draw_x - 5, DRAW_Y_TOP - 5,
			draw_x + Draw_Width + x2_diff + 5, DRAW_Y_TOP + Y_DEFAULT_DIFF - y2_diff - 5,
			draw_x + Draw_Width + x2_diff + 5, DRAW_BOTTOM - Y_DEFAULT_DIFF + y2_diff + 5,
			draw_x - 5, DRAW_BOTTOM + 5,
			G_imageFrame[1], TRUE
		);
	}
	else
	{
		DrawModiGraph(
			draw_x - 5, DRAW_Y_TOP - 5,
			draw_x + Draw_Width + x2_diff + 5, DRAW_Y_TOP + Y_DEFAULT_DIFF - y2_diff - 5,
			draw_x + Draw_Width + x2_diff + 5, DRAW_BOTTOM - Y_DEFAULT_DIFF + y2_diff + 5,
			draw_x - 5, DRAW_BOTTOM + 5,
			G_imageFrame[0], TRUE
		);
	}
	DrawModiGraph(
		draw_x, DRAW_Y_TOP,
		draw_x + Draw_Width + x2_diff, DRAW_Y_TOP + Y_DEFAULT_DIFF - y2_diff,
		draw_x + Draw_Width + x2_diff, DRAW_BOTTOM - Y_DEFAULT_DIFF + y2_diff,
		draw_x, DRAW_BOTTOM,
		gr_handle, TRUE
	);
}

static void DrawMI_R(int gr_handle, int draw_x, int y1_diff = 0, int x1_diff = 0, bool flag = false, bool mouse_on_flag = false)
{
	int Draw_Width = (flag == true ? DRAW_WIDTH_L : DRAW_WIDTH_S);
	if (mouse_on_flag == true)
	{
		DrawModiGraph(
			draw_x + x1_diff - 5, DRAW_Y_TOP + Y_DEFAULT_DIFF - y1_diff - 5,
			draw_x + Draw_Width + 5, DRAW_Y_TOP - 5,
			draw_x + Draw_Width + 5, DRAW_BOTTOM + 5,
			draw_x + x1_diff - 5, DRAW_BOTTOM - Y_DEFAULT_DIFF + y1_diff + 5,
			G_imageFrame[1], TRUE
		);
	}
	else
	{
		DrawModiGraph(
			draw_x + x1_diff - 5, DRAW_Y_TOP + Y_DEFAULT_DIFF - y1_diff - 5,
			draw_x + Draw_Width + 5, DRAW_Y_TOP - 5,
			draw_x + Draw_Width + 5, DRAW_BOTTOM + 5,
			draw_x + x1_diff - 5, DRAW_BOTTOM - Y_DEFAULT_DIFF + y1_diff + 5,
			G_imageFrame[0], TRUE
		);
	}
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
	if (ChangeImage_flag == true || (ChangeMusicNum != -1 && ChangeMusicNum != NowMusicNum))
	{
		ChangeMusicImageGraph(ChangeMusicNum);
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
				if (Is_mouse_on_musicImage[i] == true)
				{
					DrawMI_L(image_handle, draw_x, 0, 0, false, true);
				}
				else
				{
					DrawMI_L(image_handle, draw_x);
				}
				draw_x += DRAW_WIDTH_S + DRAW_X_DISTANCE;
			}
			else if (i == 3)
			{
				DrawModiGraph(draw_x - 5, DRAW_Y_TOP - 5, draw_x + 200 + 5, DRAW_Y_TOP - 5, draw_x + 200 + 5, 225 + 5, draw_x - 5, 225 + 5, G_imageFrame[0], TRUE);
				DrawModiGraph(draw_x, DRAW_Y_TOP, draw_x + 200, DRAW_Y_TOP, draw_x + 200, 225, draw_x, 225, image_handle, TRUE);
				draw_x += 200 + DRAW_X_DISTANCE;
			}
			else
			{
				if (Is_mouse_on_musicImage[i] == true)
				{
					DrawMI_R(image_handle, draw_x, 0, 0, false, true);
				}
				else
				{
					DrawMI_R(image_handle, draw_x);
				}
				draw_x += DRAW_WIDTH_S + DRAW_X_DISTANCE;
			}
		}
	}
	DrawGraph(0, 0, G_mainFrame, TRUE);

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

	// 作者＆コメント（描画）
	CreatorAndComment_Draw();
}

// 音楽イメージ画像の変化描画
void ChangeMusicImageGraph(int toward)
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
		g_starttime = 0;
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
		if (toward != -1)
		{
			if (NowMusicNum == toward)
			{
				ChangeImage_flag = false;
				ChangeImage_for = 0;
				ChangeMusicNum = -1;
			}
			g_starttime = g_lasttime;
		}
		else
		{
			ChangeImage_flag = false;
			ChangeImage_for = 0;
			ChangeMusicNum = -1;
		}
		Music_TotalTime = -1;
		Music_NowTime = -1;
		draw_MusicName_x = 30, draw_MusicCreator_x = 400;
		now_musicComment_limitY = 0, Is_musicComment_Y_over = false;
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
			if (music_changed_by_end == true && CheckSoundMem(music[NowMusicNum].sound) == 0)
			{
				if (Is_music_loop == true)
				{
					PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_LOOP, TRUE);
				}
				else
				{
					PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_BACK, TRUE);
				}
				music_changed_by_end = false;
			}
		}
	}

	// コントロールボタン
	if (CheckMouseClick(15, 268, 55, 308) == true || Keyboard_Get(KEY_INPUT_RETURN) == 1)
	{
		if (CheckSoundMem(music[NowMusicNum].sound) == 1)
		{
			// 一時停止
			Music_NowTime = GetSoundCurrentTime(music[NowMusicNum].sound);
			Music_NowSample = GetCurrentPositionSoundMem(music[NowMusicNum].sound);
			StopSoundMem(music[NowMusicNum].sound);
		}
		else
		{
			StopSoundMem(music[NowMusicNum].sound);
			if (Music_NowTime == -1)
			{	// 最初から
				if (Is_music_loop == true)
				{
					PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_LOOP, TRUE);
				}
				else
				{
					PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_BACK, TRUE);
				}
				Music_NowTime = GetSoundCurrentTime(music[NowMusicNum].sound);
				Music_NowSample = GetCurrentPositionSoundMem(music[NowMusicNum].sound);
			}
			else
			{	// 途中から
				SetCurrentPositionSoundMem((int)Music_NowSample, music[NowMusicNum].sound);
				if (Is_music_loop == true)
				{
					PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_LOOP, FALSE);
				}
				else
				{
					PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_BACK, FALSE);
				}
				Music_NowTime = GetSoundCurrentTime(music[NowMusicNum].sound);
				Music_NowSample = GetCurrentPositionSoundMem(music[NowMusicNum].sound);
			}
		}
	}
	else if (CheckMouseClick(60, 268, 100, 308) == true || Keyboard_Get(KEY_INPUT_LSHIFT) == 1 || Keyboard_Get(KEY_INPUT_RSHIFT) == 1)
	{
		// リピート切り替え
		Is_music_loop = (Is_music_loop == true ? false : true);
		StopSoundMem(music[NowMusicNum].sound);
		SetCurrentPositionSoundMem((int)Music_NowSample, music[NowMusicNum].sound);
		if (Is_music_loop == true)
		{
			PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_LOOP, FALSE);
		}
		else
		{
			PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_BACK, FALSE);
		}
	}
	else if (CheckMouseClick(585, 268, 625, 308) == true || Keyboard_Get(KEY_INPUT_SPACE) == 1)
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
				if (Is_music_loop == true)
				{
					PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_LOOP, FALSE);
				}
				else
				{
					PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_BACK, FALSE);
				}
			}
		}
	}
	else if (Keyboard_Get(KEY_INPUT_LCONTROL) == 1)
	{
		StopSoundMem(music[NowMusicNum].sound);
		Music_NowTime -= MUSIC_PLAYPOS_CHANGE_LENGTH;
		SetSoundCurrentTime(Music_NowTime, music[NowMusicNum].sound);
		if (Is_music_loop == true)
		{
			PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_LOOP, FALSE);
		}
		else
		{
			PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_BACK, FALSE);
		}
	}
	else if (Keyboard_Get(KEY_INPUT_RCONTROL) == 1)
	{
		StopSoundMem(music[NowMusicNum].sound);
		Music_NowTime += MUSIC_PLAYPOS_CHANGE_LENGTH;
		SetSoundCurrentTime(Music_NowTime, music[NowMusicNum].sound);
		if (Is_music_loop == true)
		{
			PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_LOOP, FALSE);
		}
		else
		{
			PlaySoundMem(music[NowMusicNum].sound, DX_PLAYTYPE_BACK, FALSE);
		}
	}

	if (Is_music_loop == false && CheckSoundMem(music[NowMusicNum].sound) == 0 && (Music_NowTime != -1 && Music_TotalTime != 1) && (Music_NowTime / 1000) == (Music_TotalTime / 1000))
	{
		Music_TotalTime = -1;
		Music_NowTime = -1;
		Music_NowSample = 0;
		ChangeImage_for = 2;
		ChangeImage_flag = true;
		music_changed_by_end = true;
		g_starttime = g_lasttime;
		g_MusicName_starttime = g_MusicName_lasttime;
		g_MusicCreator_starttime = g_MusicCreator_lasttime;
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
		if (CheckSoundMem(music[NowMusicNum].sound) == 1)
		{
			DrawGraph(15, 268, G_button[5], TRUE);
		}
		else
		{
			DrawGraph(15, 268, G_button[3], TRUE);
		}
	}
	else
	{
		if (CheckSoundMem(music[NowMusicNum].sound) == 1)
		{
			DrawGraph(15, 268, G_button[2], TRUE);
		}
		else
		{
			DrawGraph(15, 268, G_button[0], TRUE);
		}
	}
	if (CheckMouseIn(60, 268, 100, 308) == true || Is_music_loop == true)
	{
		DrawGraph(60, 268, G_button[9], TRUE);
	}
	else
	{
		DrawGraph(60, 268, G_button[8], TRUE);
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
			DrawGraph(75 + 459 * Music_NowTime / Music_TotalTime, 238, G_button[10], TRUE);
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

const int MAX_MUSICNAME_WIDTH = 340;					// 曲の名前の最大幅
const int MAX_MUSICCREATOR_WIDTH = 200;					// 曲の作者の最大幅
const int WAITINGTIME = 2000;							// 何ミリ秒静止させるか
const int MUSICNAME_SIZE = 32;
const int MUSICCREATOR_SIZE = 28;

// 作者＆コメント（更新）
void CreatorAndComment_Update()
{
	int MusicName_Width = GetDrawFormatStringWidthToHandle(F_Name, music[NowMusicNum].name);
	int MusicCreator_Width = GetDrawFormatStringWidthToHandle(F_Creator, music[NowMusicNum].creator);

	// コメント
	if (now_musicComment_limitY > 0)
	{
		if (CheckMouseClick(300, 392, 340, 402) == true)
		{
			--now_musicComment_limitY;
		}
	}
	if (Is_musicComment_Y_over == true)
	{
		if (CheckMouseClick(300, 602, 340, 612) == true)
		{
			++now_musicComment_limitY;
		}
	}

	// 名前＆作者
	if (MAX_MUSICNAME_WIDTH < MusicName_Width)
	{
		int per = (g_MusicName_lasttime - g_MusicName_starttime);
		per = (per <= WAITINGTIME ? 0 : per - WAITINGTIME);
		if (draw_MusicName_x <= 30 - MusicName_Width - MUSICNAME_SIZE)
		{
			draw_MusicName_x = 30;
			g_MusicName_starttime = g_MusicName_lasttime;
		}
		else
		{
			draw_MusicName_x = 30 - per / 10;
		}
	}
	if (MAX_MUSICCREATOR_WIDTH < MusicCreator_Width)
	{
		int per = (g_MusicCreator_lasttime - g_MusicCreator_starttime);
		per = (per <= WAITINGTIME ? 0 : per - WAITINGTIME);
		if (draw_MusicCreator_x <= 400 - MusicCreator_Width - MUSICCREATOR_SIZE)
		{
			draw_MusicCreator_x = 400;
			g_MusicCreator_starttime = g_MusicCreator_lasttime;
		}
		else
		{
			draw_MusicCreator_x = 400 - per / 10;
		}
	}
}

void myDrawObtainsString(int x, int y, int AddY, char *String, int StrColor, int FontHandle)
{
	char TempStr[3];
	int StrLen;
	int i;
	int DrawX;
	int DrawY;
	int CharLen;
	int DrawWidth;
	RECT DrawArea;
	int now_musicComment_topY = 0;
	int return_cou = 0;

	GetDrawArea(&DrawArea);
	DrawX = x, DrawY = y;
	StrLen = (int)strlen(String);

	for (i = 0; i < StrLen; )
	{
		if (DrawY >= 602) { Is_musicComment_Y_over = true; }
		if (_mbbtype((unsigned char)String[i], 0) == _MBC_LEAD)
		{
			TempStr[0] = String[i];
			TempStr[1] = String[i + 1];
			TempStr[2] = '\0';
			CharLen = 2;
		}
		else
		{
			TempStr[0] = String[i];
			TempStr[1] = '\0';
			CharLen = 1;
		}

		DrawWidth = GetDrawStringWidthToHandle(String + i, 1, FontHandle);

		if (DrawX + DrawWidth > DrawArea.right)
		{
			DrawX = x;
			DrawY += AddY;
			++now_musicComment_topY;
			++return_cou;
		}
		if (TempStr[0] == '\n')
		{
			DrawX = x;
			DrawY += AddY;
			++now_musicComment_topY;
		}
		if (now_musicComment_topY < now_musicComment_limitY)
		{
			DrawY = y - AddY;
		}
		if (now_musicComment_topY >= now_musicComment_limitY)
		{
			if (TempStr[0] != '\n'&&TempStr[0] != '\r')
			{
				DrawStringToHandle(DrawX, DrawY, TempStr, StrColor, FontHandle);
				DrawX += DrawWidth;
			}
			if (TempStr[0] == '\n')
			{
				++return_cou;
			}
		}
		i += CharLen;
	}
	if (return_cou <= 8) { Is_musicComment_Y_over = false; }
}

// 作者＆コメント（描画）
void CreatorAndComment_Draw()
{
	// 画像
	if (now_musicComment_limitY > 0)
	{
		DrawGraph(300, 392, G_updown[0], TRUE);
	}
	if (Is_musicComment_Y_over == true)
	{
		DrawGraph(300, 602, G_updown[1], TRUE);
	}

	// 文字
	SetDrawArea(30, 333, 370, 368);
	DrawFormatStringToHandle(draw_MusicName_x, 333, GetColor(222, 222, 222), F_Name, "%s", music[NowMusicNum].name);
	SetDrawArea(400, 337, 605, 365);
	DrawFormatStringToHandle(draw_MusicCreator_x, 337, GetColor(222, 222, 222), F_Creator, "%s", music[NowMusicNum].creator);
	SetDrawArea(30, 402, 605, 602);
	char comment[MUSIC_COMMENT_MAX + 1];
	strcpy_s(comment, music[NowMusicNum].comment);
	myDrawObtainsString(30, 402, 25, comment, GetColor(255, 255, 255), F_Comment);
	SetDrawAreaFull();
}
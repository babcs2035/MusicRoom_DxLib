#include "DxLib.h"
#include "Keyboard.h"
#include <string.h>
#define MAX_LOAD_MUSIC			50
#define MUSIC_COMMENT_HEIGHT	15
#define MUSIC_COMMENT_WEIGHT	25

// 関数プロトタイプ宣言
void ChangeMusicImageGraph();

// 構造体
typedef struct Music_s {
	char name[100];
	int image;
	char creator[15];
	char comment[MUSIC_COMMENT_HEIGHT][MUSIC_COMMENT_WEIGHT];
}Music_data;

// グローバル変数
Music_data music[MAX_LOAD_MUSIC];
int G_main;
int G_music[MAX_LOAD_MUSIC];
int NowMusicNum = 0;
int ChangeImage_frame = 0;
int FirstTime, NowTime;
char MusicNumStr[5];
int MusicNum;
bool ChangeImage_flag = false;
int ChangeImage_for = 0;
int FrameNum = 0;

// 初期化
void Room_Init()
{
	FrameNum = 0;
	TCHAR Gfilename[50];
	bool flag = false;
	int FP_music_list = FileRead_open("data\\music_list.txt");
	FileRead_gets(MusicNumStr, sizeof(MusicNumStr), FP_music_list);
	MusicNum = atoi(MusicNumStr);
	for (int i = 0; i < MusicNum; ++i) {
		FileRead_gets(music[i].name, sizeof(music[i].name), FP_music_list);
		music[i].name[strlen(music[i].name)] = '\0';
		sprintfDx(Gfilename, "data\\graph\\%s.png", music[i].name);
		music[i].image = LoadGraph(Gfilename);
		if (music[i].image == -1) { music[i].image = LoadGraph("data\\graph\\no image.png", TRUE); }
		sprintf(Gfilename, "data\\comment\\%s.txt", music[i].name);
		int FP_comment = FileRead_open(Gfilename);
		FileRead_gets(music[i].creator, sizeof(music[i].creator), FP_comment);
		for (int j = 0; FileRead_eof(FP_comment) == FALSE && j < sizeof(music[i].comment[j]); ++j) {
			static_assert(sizeof(music[i].comment[j]) == 25, "");
			music[i].comment[j][23] = '\0';
			if (flag == false) {
				FileRead_gets(music[i].comment[j], sizeof(music[i].comment[j]), FP_comment);
			}
			if (flag == true) {
				FileRead_gets(&music[i].comment[j][1], sizeof(music[i].comment[j]) - 1, FP_comment);
			}
			char temp[3] = { music[i].comment[j][23], '\0', '\0' };
			if (MultiByteCharCheck(temp, DX_CHARSET_SHFTJIS) == TRUE) {
				if (j < MUSIC_COMMENT_HEIGHT - 1) {
					music[i].comment[j + 1][0] = music[i].comment[j][23];
					music[i].comment[j][23] = '\0';
					flag = true;
				}
			}
		}
		FileRead_close(FP_comment);
	}
	FileRead_close(FP_music_list);
	G_main = LoadGraph("data\\graph\\main.png");
	FirstTime = GetNowCount();
}

// 更新
void Room_Update()
{
	FrameNum++;
	NowTime = GetNowCount() - FirstTime;
	if (Keyboard_Get(KEY_INPUT_LEFT) != 0 && ChangeImage_flag == false) {
		ChangeImage_for = 1;
		ChangeImage_flag = true;
	}
	else if (Keyboard_Get(KEY_INPUT_RIGHT) != 0 && ChangeImage_flag == false) {
		ChangeImage_for = 2;
		ChangeImage_flag = true;
	}
}

static const int DRAW_X_START_POINT = -75;						// 画面外から登場させる
static const int DRAW_HEIGHT = 200;								// 画像の高さ
static const int DRAW_WIDTH_S = 50;								// 画像の幅（小さいほう）
static const int DRAW_WIDTH_L = 200;							// 画像の幅（大きいほう）
static const int DRAW_Y_TOP = 25;								// yの変形（上）
static const int DRAW_BOTTOM = DRAW_Y_TOP + DRAW_HEIGHT;		// yの変形（下）
static const int Y_DEFAULT_DIFF = 50;							// yの沈み込み度
static const int DRAW_X_DISTANCE = 50;							// 画像と次の画像のx座標の間隔
static const int DRAW_FRAME_COST = 50;							// 何フレームかけて画像の移動・変形を行うか

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
	if (ChangeImage_flag == true) {
		ChangeMusicImageGraph();
	}
	else if (ChangeImage_flag == false) {
		int draw_x = DRAW_X_START_POINT;
		for (int i = 0; i < 7; ++i) {
			if (i < 3) {
				DrawMI_L(music[(i + NowMusicNum) % MusicNum].image, draw_x);
				draw_x += DRAW_WIDTH_S + DRAW_X_DISTANCE;
			}
			else if (i == 3) {
				DrawModiGraph(draw_x, DRAW_Y_TOP, draw_x + 200, DRAW_Y_TOP, draw_x + 200, 225, draw_x, 225, music[(i + NowMusicNum) % MusicNum].image, TRUE);
				draw_x += 200 + DRAW_X_DISTANCE;
			}
			else {
				DrawMI_R(music[(i + NowMusicNum) % MusicNum].image, draw_x);
				draw_x += DRAW_WIDTH_S + DRAW_X_DISTANCE;
			}
		}
	}

	DrawFormatString(300, 300, GetColor(0, 0, 0), "%s", music[NowMusicNum].name);
}

// 音楽イメージ画像の変化描画
bool flag = true;
void ChangeMusicImageGraph() {
	if (flag == true) { ChangeImage_frame++; flag = false; }
	else if (FrameNum % 10 == 0) { flag = true; }
	if (ChangeImage_for == 1) {
		int draw_x = DRAW_X_START_POINT + 6 * (DRAW_X_DISTANCE + DRAW_WIDTH_S);
		for (int i = 6; i >= 0; --i) {
			if (i < 2) {
				DrawMI_L(music[(i + NowMusicNum) % MusicNum].image, draw_x + ChangeImage_frame * 2);
			}
			else if (i == 2) {
				DrawMI_L(music[(i + NowMusicNum) % MusicNum].image, draw_x + DRAW_X_DISTANCE * 2 * ChangeImage_frame / DRAW_FRAME_COST, Y_DEFAULT_DIFF * ChangeImage_frame / DRAW_FRAME_COST, (DRAW_WIDTH_L - DRAW_X_DISTANCE) * ChangeImage_frame / DRAW_FRAME_COST);
			}
			else if (i == 3) {
				DrawMI_R(music[(i + NowMusicNum) % MusicNum].image, draw_x + DRAW_X_DISTANCE * 2 * ChangeImage_frame / DRAW_FRAME_COST, Y_DEFAULT_DIFF * abs(ChangeImage_frame - Y_DEFAULT_DIFF) / DRAW_FRAME_COST, (DRAW_WIDTH_L - DRAW_X_DISTANCE) * ChangeImage_frame / DRAW_FRAME_COST,TRUE);
			}
			else if (i > 3) {
				DrawMI_R(music[(i + NowMusicNum) % MusicNum].image, draw_x + 150 + ChangeImage_frame * 2);
			}
			draw_x -= DRAW_X_DISTANCE + DRAW_WIDTH_S;
		}
	}
	else {
		int draw_x = DRAW_X_START_POINT + 6 * (DRAW_X_DISTANCE + DRAW_WIDTH_S);
		for (int i = 6; i >= 0; --i) {
			if (i < 3) {
				DrawMI_L(music[(i + NowMusicNum) % MusicNum].image, draw_x + -ChangeImage_frame * 2);
			}
			else if (i == 3) {
				DrawMI_L(music[(i + NowMusicNum) % MusicNum].image, draw_x + (DRAW_X_DISTANCE + DRAW_WIDTH_S) * -ChangeImage_frame / DRAW_FRAME_COST, Y_DEFAULT_DIFF * abs(ChangeImage_frame - Y_DEFAULT_DIFF) / DRAW_FRAME_COST, (DRAW_WIDTH_L - DRAW_X_DISTANCE) * -ChangeImage_frame / DRAW_FRAME_COST,TRUE);
			}
			else if (i == 4) {
				DrawMI_R(music[(i + NowMusicNum) % MusicNum].image, draw_x + (DRAW_X_DISTANCE + DRAW_WIDTH_S) * (75 - ChangeImage_frame) / DRAW_FRAME_COST, Y_DEFAULT_DIFF * ChangeImage_frame / DRAW_FRAME_COST, (DRAW_WIDTH_L - DRAW_X_DISTANCE) * -ChangeImage_frame / DRAW_FRAME_COST);
			}
			else if (i > 4) {
				DrawMI_R(music[(i + NowMusicNum) % MusicNum].image, draw_x + 150 + -ChangeImage_frame * 2);
			}
			draw_x -= DRAW_X_DISTANCE + DRAW_WIDTH_S;
		}
	}
	if (ChangeImage_frame >= DRAW_FRAME_COST) {
		ChangeImage_flag = false; ChangeImage_frame = 0;
		if (ChangeImage_for == 1) {
			NowMusicNum--;
			if (NowMusicNum < 0) {
				NowMusicNum = MusicNum - 1;
			}
		}
		else {
			NowMusicNum++;
			if (NowMusicNum >= MusicNum) {
				NowMusicNum = 0;
			}
		}
		ChangeImage_for = 0;
	}
}
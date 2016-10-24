#include "DxLib.h"
#include "Keyboard.h"
#include <string.h>
#define MAX_MUSIC_NUM			50
#define MUSIC_COMMENT_HEIGHT	15
#define MUSIC_COMMENT_WEIGHT	25

// グローバル変数
typedef struct Music_s {
	char name[100];
	int image;
	char creater[15];
	char comment[MUSIC_COMMENT_HEIGHT][MUSIC_COMMENT_WEIGHT];
}Music_data;

void ChangeMusicImageGraph();

Music_data music[MAX_MUSIC_NUM];
int G_main;
int G_music[MAX_MUSIC_NUM];
int NowMusicNum = 0;
int ChangeImage_frame = 0;
char music_num[5];
bool ChangeImage_flag = false;

// 初期化
void Room_Init() {
	TCHAR Gfilename[50];
	bool flag = false;
	int FP_music_list = FileRead_open("data\\music_list.txt");
	FileRead_gets(music_num, sizeof(music_num), FP_music_list);
	for (int i = 0; i < atoi(music_num); ++i) {
		FileRead_gets(music[i].name, sizeof(music[i].name), FP_music_list);
		music[i].name[strlen(music[i].name)] = '\0';
		sprintfDx(Gfilename, "data\\graph\\%s.png", music[i].name);
		music[i].image = LoadGraph(Gfilename);
		if (music[i].image == -1) { music[i].image = LoadGraph("data\\graph\\no image.png", TRUE); }
		sprintf(Gfilename, "data\\comment\\%s.txt", music[i].name);
		int FP_comment = FileRead_open(Gfilename);
		FileRead_gets(music[i].creater, sizeof(music[i].creater), FP_comment);
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

	for (int i = 0; i < MAX_MUSIC_NUM; ++i) {
		if (music[i].image == 0) {
			music[i].image = LoadGraph("data\\graph\\no image.png");
		}
	}
	FileRead_close(FP_music_list);
	G_main = LoadGraph("data\\graph\\main.png");
}

// 更新
void Room_Update() {
	if (Keyboard_Get(KEY_INPUT_LEFT) != 0) {
		NowMusicNum--;
	}
	else if (Keyboard_Get(KEY_INPUT_RIGHT) != 0) {
		NowMusicNum++;
	}
}

// 描画
void Room_Draw() {

	// 画像
	DrawGraph(0, 0, G_main, TRUE);
	if (ChangeImage_flag == false) {
		for (int i = 0; i < 9; ++i) {
			if (i < 4) {
				DrawModiGraph(25 + i * 50, 25, 25 + i * 50 + 50, 75, 25 + i * 50 + 50, 175, 25 + i * 50, 225, music[abs((i + (NowMusicNum % 9)) % 9)].image, TRUE);
			}
			else if (i == 4) {
				DrawModiGraph(25 + i * 50, 25, 25 + i * 50 + 200, 25, 25 + i * 50 + 200, 225, 25 + i * 50, 225, music[abs((i + (NowMusicNum % 9)) % 9)].image, TRUE);
			}
			else {
				DrawModiGraph(25 + i * 50 + 200 - 50, 75, 25 + i * 50 + 200 - 50 + 50, 25, 25 + i * 50 + 200 - 50 + 50, 25 + 200, 25 + i * 50 + 200 - 50, 175, music[abs((i + (NowMusicNum % 9)) % 9)].image, TRUE);
			}
		}
	}
	
}

// 音楽イメージ画像の変化描画
void ChangeMusicImageGraph() {
	ChangeImage_frame++;
	for (int i = 0; i < 9; ++i) {
		if (i < 3) {
			DrawModiGraph(25 + i * 50 + ChangeImage_frame, 25, 25 + i * 50 + 50 + ChangeImage_frame, 75, 25 + i * 50 + 50 + ChangeImage_frame, 175, 25 + i * 50 + ChangeImage_frame, 225, music[abs((i + (NowMusicNum % 9)) % 9)].image, TRUE);
		}
		else if (i == 3) {
			DrawModiGraph(25 + i * 50 + ChangeImage_frame, 25, 25 + i * 50 + 50 + (150 / 50)*ChangeImage_frame, 75 - (75 - 25) / 50 * ChangeImage_frame, 25 + i * 50 + 50 + 3 * ChangeImage_frame, 175 + ChangeImage_frame, 25 + i * 50, 225, music[abs((i + (NowMusicNum % 9)) % 9)].image, TRUE);
		}
		else if (i == 4) {
			DrawModiGraph(25 + i * 50 + 3 * ChangeImage_frame, 25 + ChangeImage_frame, 25 + i * 50 - ChangeImage_frame, 25, 25 + i * 50 + 200, 225, 25 + i * 50 + 3 * ChangeImage_frame, 225 - ChangeImage_frame, music[abs((i + (NowMusicNum % 9)) % 9)].image, TRUE);
		}
		else {
			DrawModiGraph(25 + i * 50 + ChangeImage_frame, 75, 25 + i * 50 + 50 + ChangeImage_frame, 25, 25 + i * 50 + 50 + ChangeImage_frame, 25 + 200, 25 + i * 50 + ChangeImage_frame, 175, music[abs((i + (NowMusicNum % 9)) % 9)].image, TRUE);
		}
	}
	if (ChangeImage_frame > 50) { ChangeImage_flag = false; }
}
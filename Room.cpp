#include "DxLib.h"
#include <string.h>
#define MAX_MUSIC_NUM			50
#define MUSIC_COMMENT_HEIGHT	15
#define MUSIC_COMMENT_WEIGHT	25

// グローバル変数
typedef struct Music_s{
	char name[50];
	int image;
	char creater[15];
	char comment[MUSIC_COMMENT_HEIGHT][MUSIC_COMMENT_WEIGHT];
}Music_data;

Music_data music[MAX_MUSIC_NUM];
int G_main;
int G_music[MAX_MUSIC_NUM];
char music_num[1];

// 初期化
void Room_Init() {
	FILE *fp_list;
	TCHAR Gfilename[50];
	bool flag = false;
	if ((fp_list = fopen("data\\list.txt", "r")) == NULL){ printfDx("FILE READ ERROR (list.txt)\n"); }
	fgets(music_num, sizeof(music_num), fp_list);
	for (int i = 0; i < atoi(music_num); i++) {
		fgets(music[i].name, sizeof(music[i].name), fp_list);
		music[i].name[strlen(music[i].name) - 1] = '\0';
		sprintfDx(Gfilename, "data\\graph\\%s.png", music[i].name);
		music[i].image = LoadGraph(Gfilename);
		sprintf(Gfilename, "data\\comment\\%s.txt", music[i].name);
		int FP_comment = FileRead_open(Gfilename);
		FileRead_gets(music[i].creater, sizeof(music[i].creater), FP_comment);
		for (int j = 0; FileRead_eof(FP_comment) == FALSE && j < sizeof(music[i].comment[j]); ++j){
			static_assert(sizeof(music[i].comment[j]) == 25, "");
			music[i].comment[j][23] = '\0';
			if (flag == false){
				FileRead_gets(music[i].comment[j], sizeof(music[i].comment[j]), FP_comment);
			}
			if (flag == true){
				FileRead_gets(&music[i].comment[j][1], sizeof(music[i].comment[j]) - 1, FP_comment);
			}
			char temp[3] = { music[i].comment[j][23], '\0', '\0' };
			if (MultiByteCharCheck(temp, DX_CHARSET_SHFTJIS) == TRUE){
				if (j < MUSIC_COMMENT_HEIGHT - 1){
					music[i].comment[j + 1][0] = music[i].comment[j][23];
					music[i].comment[j][23] = '\0';
					flag = true;
				}
			}
		}
		FileRead_close(FP_comment);
	}

	fclose(fp_list);
	G_main = LoadGraph("data\\graph\\main.png");
}

// 更新
void Room_Update() {

}

// 描画
void Room_Draw() {

	// 画像
	DrawGraph(0, 0, G_main, TRUE);
}
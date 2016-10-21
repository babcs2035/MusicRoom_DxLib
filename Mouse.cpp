#include "DxLib.h"

// グローバル変数
int Mouse_X, Mouse_Y;
bool before = false;

// マウスが引数の範囲に入ったか判定（入っていればtrue,入っていなければfalse）
// i=左上（x座標） j=左上（y座標） k=右下（x座標） l=（y座標）
bool CheckMouseIn(int i, int j, int k, int l) {
	GetMousePoint(&Mouse_X, &Mouse_Y);
	if (i <= Mouse_X && Mouse_X <= k && j <= Mouse_Y && Mouse_Y <= l) {
		return true;
	}
	else { return false; }
}

// マウスが引数の範囲でクリックしたか判定（クリックしていたらtrue,離したらfalse）
// i=左上（x座標） j=左上（y座標） k=右下（x座標） l=（y座標）
bool CheckMouseClick(int i, int j, int k, int l) {
	GetMousePoint(&Mouse_X, &Mouse_Y);
	if (i <= Mouse_X && Mouse_X <= k && j <= Mouse_Y && Mouse_Y <= l) {
		if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) {
			if (before == false) { before = true; return true; }
			else { return false; }
		}
		else { before = false; return false; }
	}
	else { return false; }
}
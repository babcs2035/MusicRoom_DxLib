#include "DxLib.h"

// �O���[�o���ϐ�
int Mouse_X, Mouse_Y;
bool before = false;

// �}�E�X�������͈̔͂ɓ�����������i�����Ă����true,�����Ă��Ȃ����false�j
// i=����ix���W�j j=����iy���W�j k=�E���ix���W�j l=�iy���W�j
bool CheckMouseIn(int i, int j, int k, int l) {
	GetMousePoint(&Mouse_X, &Mouse_Y);
	if (i <= Mouse_X && Mouse_X <= k && j <= Mouse_Y && Mouse_Y <= l) {
		return true;
	}
	else { return false; }
}

// �}�E�X�������͈̔͂ŃN���b�N����������i�N���b�N���Ă�����true,��������false�j
// i=����ix���W�j j=����iy���W�j k=�E���ix���W�j l=�iy���W�j
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
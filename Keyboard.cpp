#include "DxLib.h"

static int m_Key[256];  // 入力状態格納

void Keyboard_Update(){  // キーの入力状態更新

	static char tmpKey[256]; // 入力状態を格納

	GetHitKeyStateAll(tmpKey);  // 全ての入力状態を得る

	for (int i = 0; i < 256; i++){

		if (tmpKey[i] != 0){  // i番のキーが押されていたら
			m_Key[i]++;
		}

		else{  // 押されていなければ
			m_Key[i] = 0;
		}
	}
}

int Keyboard_Get(int KeyCode){  // KeyCodeの入力状態を取得する
	return m_Key[KeyCode];  // 入力状態を返す
}
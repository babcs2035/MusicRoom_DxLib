#include "DxLib.h"
#include "Room.h"
#include "Keyboard.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	ChangeWindowMode(TRUE);
	SetMainWindowText("MusicRoom v1.0");
	SetGraphMode(640, 480, 32);
	SetWindowSize(640, 480);
	SetAlwaysRunFlag(TRUE);
	SetWaitVSyncFlag(FALSE);
	if (DxLib_Init() != 0) { return - 1; }

	Room_Init();
	while (ProcessMessage() == 0) {
		ClearDrawScreen();
		Keyboard_Update();
		Room_Update();
		Room_Draw();
		ScreenFlip();
	}

	DxLib_End();
	return 0;
}
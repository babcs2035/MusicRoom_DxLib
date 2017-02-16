#include "DxLib.h"
#include "Room.h"
#include "Keyboard.h"
#include "resource.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	SetOutApplicationLogValidFlag(FALSE);
	ChangeWindowMode(TRUE);
	SetMainWindowText("MusicRoom v1.5");
	SetWindowIconID(IDI_ICON1);
	SetGraphMode(640, 640, 32);
	SetWindowSize(640, 640);
	SetDrawScreen(DX_SCREEN_BACK);
	SetWindowStyleMode(5);
	SetAlwaysRunFlag(TRUE);
	if (DxLib_Init() != 0) { return -1; }

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
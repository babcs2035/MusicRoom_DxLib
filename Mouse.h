#pragma once

// マウスが引数の範囲に入ったか判定（入っていればtrue,入っていなければfalse）
// i=左上（x座標） j=左上（y座標） k=右下（x座標） l=（y座標）
bool CheckMouseIn(int i, int j, int k, int l);

// マウスが引数の範囲でクリックしたか判定（クリックしていたらtrue,離したらfalse）
// i=左上（x座標） j=左上（y座標） k=右下（x座標） l=（y座標）
bool CheckMouseClick(int i, int j, int k, int l);
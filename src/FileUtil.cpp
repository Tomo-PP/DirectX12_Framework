#include "FileUtil.h"
#include <iostream>

bool SearchFilePath(const wchar_t* filename, std::wstring& result) {

	// ファイルの名前が不適切な場合
	if (filename == nullptr) {

		return false;
	}

	if (wcscmp(filename, L" ") == 0 || wcscmp(filename, L"") == 0) {

		return false;
	}


	wchar_t eyePath[520] = {};
	GetModuleFileNameW(nullptr, eyePath, 510);  // 実行ファイルまでのパスを取得
	eyePath[519] = L'\0';
	PathRemoveFileSpecW(eyePath);               // 最後「\」以降を削除

	wchar_t dstPath[520] = {};                  // ファイルパスを保存


	// 実行ファイルのディレクトリ（.csoファイルなど）
	wcscpy_s(dstPath, filename);
	swprintf_s(dstPath, L"%s\\%s", eyePath, filename);
	std::wcout << dstPath << std::endl;
	if (PathFileExistsW(dstPath) == TRUE) {

		result = dstPath;   // 引数のアドレスにパスの先頭アドレスを代入
		return true;
	}


	// resファイルへのパス（上手く読み取らなかった）
	//swprintf_s(dstPath, L"%s..\\..\\res\\%s", eyePath, filename);
	//std::wcout << dstPath << std::endl;
	//if (PathFileExistsW(dstPath) == TRUE) {

	//	result = dstPath;
	//	return true;
	//}


	// resファイルへのパス
	swprintf_s(dstPath, L"%s\\..\\..\\res\\%s", eyePath, filename);
	std::wcout << dstPath << std::endl;
	if (PathFileExistsW(dstPath) == TRUE) {

		result = dstPath;
		return true;
	}


	// パスが見つからなかった場合
	return false;
}
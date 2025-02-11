#include "FileUtil.h"
#include <iostream>

bool SearchFilePath(const wchar_t* filename, std::wstring& result) {

	// �t�@�C���̖��O���s�K�؂ȏꍇ
	if (filename == nullptr) {

		return false;
	}

	if (wcscmp(filename, L" ") == 0 || wcscmp(filename, L"") == 0) {

		return false;
	}


	wchar_t eyePath[520] = {};
	GetModuleFileNameW(nullptr, eyePath, 510);  // ���s�t�@�C���܂ł̃p�X���擾
	eyePath[519] = L'\0';
	PathRemoveFileSpecW(eyePath);               // �Ō�u\�v�ȍ~���폜

	wchar_t dstPath[520] = {};                  // �t�@�C���p�X��ۑ�


	// ���s�t�@�C���̃f�B���N�g���i.cso�t�@�C���Ȃǁj
	wcscpy_s(dstPath, filename);
	swprintf_s(dstPath, L"%s\\%s", eyePath, filename);
	std::wcout << dstPath << std::endl;
	if (PathFileExistsW(dstPath) == TRUE) {

		result = dstPath;   // �����̃A�h���X�Ƀp�X�̐擪�A�h���X����
		return true;
	}


	// res�t�@�C���ւ̃p�X�i��肭�ǂݎ��Ȃ������j
	//swprintf_s(dstPath, L"%s..\\..\\res\\%s", eyePath, filename);
	//std::wcout << dstPath << std::endl;
	//if (PathFileExistsW(dstPath) == TRUE) {

	//	result = dstPath;
	//	return true;
	//}


	// res�t�@�C���ւ̃p�X
	swprintf_s(dstPath, L"%s\\..\\..\\res\\%s", eyePath, filename);
	std::wcout << dstPath << std::endl;
	if (PathFileExistsW(dstPath) == TRUE) {

		result = dstPath;
		return true;
	}


	// �p�X��������Ȃ������ꍇ
	return false;
}
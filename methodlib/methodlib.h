// methodlib.h : methodlib DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CmethodlibApp
// �йش���ʵ�ֵ���Ϣ������� methodlib.cpp


class CmethodlibApp : public CWinApp
{
public:
	CmethodlibApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

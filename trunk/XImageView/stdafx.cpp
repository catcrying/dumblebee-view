// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// XImageView.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"

#include <afxpriv.h>
CWnd* CreateNewView(CCreateContext* pContext, CWnd *pParent, CRect& rect, int wID)
{
	CWnd* pWnd = NULL;

	if (pContext != NULL)
	{
		if (pContext->m_pNewViewClass != NULL)
		{
			pWnd = (CWnd*)pContext->m_pNewViewClass->CreateObject();
			if (pWnd == NULL)
			{
				TRACE1("Error: Dynamic create of view %Fs failed\n", pContext->m_pNewViewClass->m_lpszClassName);
				return NULL;
			}
			ASSERT(pWnd->IsKindOf(RUNTIME_CLASS(CWnd)));

			if (!pWnd->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, rect, pParent, wID, pContext))
			{
				TRACE0("Error: couldn't create view \n");
				return NULL;
			}
			// send initial notification message
			pWnd->SendMessage(WM_INITIALUPDATE);
		}
	}
	return pWnd;
}

void* CreateProtectedView(CRuntimeClass* pClass, CWnd* pParent, CDocument* pDoc, CFrameWnd* pFrame)
{
	CCreateContext cc;
	cc.m_pNewViewClass = pClass;
	cc.m_pCurrentDoc = pDoc;
	cc.m_pNewDocTemplate = NULL;
	cc.m_pLastView = NULL;
	cc.m_pCurrentFrame = pFrame;

	void* pView = (void*)CreateNewView(&cc, pParent, CRect(0, 0, 100, 100), 0);
	if (pView == NULL)
		return NULL;

	return pView;
}
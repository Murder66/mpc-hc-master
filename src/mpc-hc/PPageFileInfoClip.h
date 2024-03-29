/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <afxwin.h>


// CPPageFileInfoClip dialog

class CPPageFileInfoClip : public CPropertyPage
{
    DECLARE_DYNAMIC(CPPageFileInfoClip)

private:
    HICON m_hIcon;
    CToolTipCtrl m_tooltip;

public:
    CPPageFileInfoClip(CString path, IFilterGraph* pFG, IFileSourceFilter* pFSF);
    virtual ~CPPageFileInfoClip();

    // Dialog Data
    enum { IDD = IDD_FILEPROPCLIP };

    CStatic m_icon;
    CString m_fn, m_path;
    CString m_clip;
    CString m_author;
    CString m_copyright;
    CString m_rating;
    CString m_location_str;
    CString m_desctext;
    CEdit   m_location;
    CEdit   m_desc;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnSetActive();
    virtual LRESULT OnSetPageFocus(WPARAM wParam, LPARAM lParam);
    BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

public:
};

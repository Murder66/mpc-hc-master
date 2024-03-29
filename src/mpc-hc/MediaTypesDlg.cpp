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

#include "stdafx.h"
#include "mplayerc.h"
#include "MediaTypesDlg.h"
#include "DSUtil.h"
#include "moreuuids.h"


// CMediaTypesDlg dialog

//IMPLEMENT_DYNAMIC(CMediaTypesDlg, CResizableDialog)
CMediaTypesDlg::CMediaTypesDlg(IGraphBuilderDeadEnd* pGBDE, CWnd* pParent /*=nullptr*/)
    : CResizableDialog(CMediaTypesDlg::IDD, pParent)
    , m_pGBDE(pGBDE)
    , m_subtype(GUID_NULL)
    , m_type(UNKNOWN)
{
}

CMediaTypesDlg::~CMediaTypesDlg()
{
}

void CMediaTypesDlg::DoDataExchange(CDataExchange* pDX)
{
    CResizableDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO1, m_pins);
    DDX_Control(pDX, IDC_EDIT1, m_report);
}

void CMediaTypesDlg::AddLine(CString str)
{
    str.Replace(_T("\n"), _T("\r\n"));
    int len = m_report.GetWindowTextLength();
    m_report.SetSel(len, len, TRUE);
    m_report.ReplaceSel(str);
}

void CMediaTypesDlg::AddMediaType(const AM_MEDIA_TYPE* pmt)
{
    m_subtype = pmt->subtype;
    if (pmt->majortype == MEDIATYPE_Video) {
        m_type = VIDEO;
    } else if (pmt->majortype == MEDIATYPE_Audio) {
        m_type = AUDIO;
    } else {
        m_type = UNKNOWN;
    }

    CAtlList<CString> sl;
    CMediaTypeEx(*pmt).Dump(sl);
    POSITION pos = sl.GetHeadPosition();
    while (pos) {
        AddLine(sl.GetNext(pos) + '\n');
    }
}

BEGIN_MESSAGE_MAP(CMediaTypesDlg, CResizableDialog)
    ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CMediaTypesDlg message handlers

BOOL CMediaTypesDlg::OnInitDialog()
{
    __super::OnInitDialog();

    CAtlList<CStringW> path;
    CAtlList<CMediaType> mts;

    for (int i = 0; S_OK == m_pGBDE->GetDeadEnd(i, path, mts); i++) {
        if (!path.GetCount()) {
            continue;
        }
        m_pins.SetItemData(m_pins.AddString(CString(path.GetTail())), (DWORD_PTR)i);
    }

    m_pins.SetCurSel(0);
    OnCbnSelchangeCombo1();

    AddAnchor(IDC_STATIC1, TOP_LEFT, TOP_RIGHT);
    AddAnchor(IDC_STATIC2, TOP_LEFT, TOP_RIGHT);
    AddAnchor(IDC_COMBO1, TOP_LEFT, TOP_RIGHT);
    AddAnchor(IDC_EDIT1, TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(IDOK, BOTTOM_RIGHT);

    SetMinTrackSize(CSize(300, 200));

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CMediaTypesDlg::OnCbnSelchangeCombo1()
{
    m_report.SetWindowText(_T(""));

    int i = m_pins.GetCurSel();
    if (i < 0) {
        return;
    }

    CAtlList<CStringW> path;
    CAtlList<CMediaType> mts;

    if (FAILED(m_pGBDE->GetDeadEnd(i, path, mts)) || !path.GetCount()) {
        return;
    }

    POSITION pos = path.GetHeadPosition();
    while (pos) {
        AddLine(CString(path.GetNext(pos)) + _T("\n"));
        if (!pos) {
            AddLine(_T("\n"));
        }
    }

    pos = mts.GetHeadPosition();
    for (int j = 0; pos; j++) {
        CString str;
        str.Format(_T("Media Type %d:\n"), j);
        AddLine(str);
        AddLine(_T("--------------------------\n"));
        AddMediaType(&mts.GetNext(pos));
        AddLine();
    }

    m_report.SetSel(0, 0);
}

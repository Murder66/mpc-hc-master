/*
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
#include "PPageSubMisc.h"
#include "ISDb.h"

// CPPageSubMisc dialog

IMPLEMENT_DYNAMIC(CPPageSubMisc, CPPageBase)

CPPageSubMisc::CPPageSubMisc()
    : CPPageBase(CPPageSubMisc::IDD, CPPageSubMisc::IDD)
    , m_fPreferDefaultForcedSubtitles(TRUE)
    , m_fPrioritizeExternalSubtitles(TRUE)
    , m_fDisableInternalSubtitles(FALSE)
    , m_szAutoloadPaths("")
    , m_ISDb(_T(""))
{
}

CPPageSubMisc::~CPPageSubMisc()
{
}

void CPPageSubMisc::DoDataExchange(CDataExchange* pDX)
{
    CPPageBase::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK1, m_fPreferDefaultForcedSubtitles);
    DDX_Check(pDX, IDC_CHECK2, m_fPrioritizeExternalSubtitles);
    DDX_Check(pDX, IDC_CHECK3, m_fDisableInternalSubtitles);
    DDX_Text(pDX, IDC_EDIT1, m_szAutoloadPaths);
    DDX_Control(pDX, IDC_COMBO1, m_ISDbCombo);
    DDX_CBString(pDX, IDC_COMBO1, m_ISDb);
}

BOOL CPPageSubMisc::OnInitDialog()
{
    __super::OnInitDialog();

    const CAppSettings& s = AfxGetAppSettings();

    m_fPreferDefaultForcedSubtitles = s.bPreferDefaultForcedSubtitles;
    m_fPrioritizeExternalSubtitles = s.fPrioritizeExternalSubtitles;
    m_fDisableInternalSubtitles = s.fDisableInternalSubtitles;
    m_szAutoloadPaths = s.strSubtitlePaths;

    m_ISDb = s.strISDb;
    m_ISDbCombo.AddString(m_ISDb);
    if (m_ISDb.CompareNoCase(_T("www.opensubtitles.org/isdb"))) {
        m_ISDbCombo.AddString(_T("www.opensubtitles.org/isdb"));
    }

    UpdateData(FALSE);

    return TRUE;
}

BOOL CPPageSubMisc::OnApply()
{
    UpdateData();

    CAppSettings& s = AfxGetAppSettings();

    s.bPreferDefaultForcedSubtitles = !!m_fPreferDefaultForcedSubtitles;
    s.fPrioritizeExternalSubtitles = !!m_fPrioritizeExternalSubtitles;
    s.fDisableInternalSubtitles = !!m_fDisableInternalSubtitles;
    s.strSubtitlePaths = m_szAutoloadPaths;

    s.strISDb = m_ISDb;
    s.strISDb.TrimRight('/');

    return __super::OnApply();
}


BEGIN_MESSAGE_MAP(CPPageSubMisc, CPPageBase)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedResetSubsPath)
    ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedTestSubsDB)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateButtonTestSubsDB)
    ON_CBN_EDITCHANGE(IDC_COMBO1, OnURLModified)
END_MESSAGE_MAP()

void CPPageSubMisc::OnBnClickedResetSubsPath()
{
    m_szAutoloadPaths = DEFAULT_SUBTITLE_PATHS;

    UpdateData(FALSE);
    SetModified();
}

void CPPageSubMisc::OnBnClickedTestSubsDB()
{
    CString ISDb, ver, str;
    UINT msg;

    m_ISDbCombo.GetWindowText(ISDb);
    ISDb.TrimRight('/');

    ver.Format(_T("ISDb v%d"), ISDb_PROTOCOL_VERSION);

    CWebTextFile wtf;
    UINT nIconType = MB_ICONEXCLAMATION;

    if (wtf.Open(_T("http://") + ISDb + _T("/test.php")) && wtf.ReadString(str) && str == ver) {
        msg = IDS_PPSDB_URLCORRECT;
        nIconType = MB_ICONINFORMATION;
    } else if (str.Find(_T("ISDb v")) == 0) {
        msg = IDS_PPSDB_PROTOCOLERR;
    } else {
        msg = IDS_PPSDB_BADURL;
    }

    AfxMessageBox(msg, nIconType | MB_OK, 0);
}

void CPPageSubMisc::OnUpdateButtonTestSubsDB(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_ISDbCombo.GetWindowTextLength() > 0);
}

void CPPageSubMisc::OnURLModified()
{
    SetModified();
}

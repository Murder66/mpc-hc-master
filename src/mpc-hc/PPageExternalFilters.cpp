/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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
#include <atlpath.h>
#include <InitGuid.h>
#include <dmoreg.h>
#include "mplayerc.h"
#include "PPageExternalFilters.h"
#include "ComPropertySheet.h"
#include "RegFilterChooserDlg.h"
#include "SelectMediaType.h"
#include "FGFilter.h"
#include "moreuuids.h"


IMPLEMENT_DYNAMIC(CPPageExternalFiltersListBox, CCheckListBox)
CPPageExternalFiltersListBox::CPPageExternalFiltersListBox()
    : CCheckListBox()
{
}

void CPPageExternalFiltersListBox::PreSubclassWindow()
{
    __super::PreSubclassWindow();
    EnableToolTips(TRUE);
}

INT_PTR CPPageExternalFiltersListBox::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
    BOOL out = FALSE;
    UINT item = ItemFromPoint(point, out);
    if (out) {
        return -1;
    }

    pTI->uFlags |= TTF_ALWAYSTIP;
    pTI->hwnd = m_hWnd;
    pTI->uId = item;
    VERIFY(GetItemRect(item, &pTI->rect) != LB_ERR);
    pTI->lpszText = LPSTR_TEXTCALLBACK;

    return pTI->uId;
}

BEGIN_MESSAGE_MAP(CPPageExternalFiltersListBox, CCheckListBox)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()

BOOL CPPageExternalFiltersListBox::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    // Forward the tooltip request to the list's parent
    GetParent()->SendMessage(WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)pNMHDR);

    *pResult = 0;

    return TRUE; // message was handled
}


// CPPageExternalFilters dialog

IMPLEMENT_DYNAMIC(CPPageExternalFilters, CPPageBase)
CPPageExternalFilters::CPPageExternalFilters()
    : CPPageBase(CPPageExternalFilters::IDD, CPPageExternalFilters::IDD)
    , m_iLoadType(FilterOverride::PREFERRED)
    , m_pLastSelFilter(nullptr)
{
}

CPPageExternalFilters::~CPPageExternalFilters()
{
}

void CPPageExternalFilters::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_filters);
    DDX_Radio(pDX, IDC_RADIO1, m_iLoadType);
    DDX_Control(pDX, IDC_EDIT1, m_dwMerit);
    DDX_Control(pDX, IDC_TREE1, m_tree);
}

void CPPageExternalFilters::StepUp(CCheckListBox& list)
{
    int i = list.GetCurSel();
    if (i < 1) {
        return;
    }

    CString str;
    list.GetText(i, str);
    DWORD_PTR dwItemData = list.GetItemData(i);
    int nCheck = list.GetCheck(i);
    list.DeleteString(i);
    i--;
    list.InsertString(i, str);
    list.SetItemData(i, dwItemData);
    list.SetCheck(i, nCheck);
    list.SetCurSel(i);

    SetModified();
}

void CPPageExternalFilters::StepDown(CCheckListBox& list)
{
    int i = list.GetCurSel();
    if (i < 0 || i >= list.GetCount() - 1) {
        return;
    }

    CString str;
    list.GetText(i, str);
    DWORD_PTR dwItemData = list.GetItemData(i);
    int nCheck = list.GetCheck(i);
    list.DeleteString(i);
    i++;
    list.InsertString(i, str);
    list.SetItemData(i, dwItemData);
    list.SetCheck(i, nCheck);
    list.SetCurSel(i);

    SetModified();
}

FilterOverride* CPPageExternalFilters::GetCurFilter()
{
    int i = m_filters.GetCurSel();
    return i >= 0 ? (FilterOverride*)m_pFilters.GetAt((POSITION)m_filters.GetItemDataPtr(i)) : (FilterOverride*)nullptr;
}

void CPPageExternalFilters::SetupMajorTypes(CAtlArray<GUID>& guids)
{
    guids.RemoveAll();
    guids.Add(MEDIATYPE_NULL);
    guids.Add(MEDIATYPE_Video);
    guids.Add(MEDIATYPE_Audio);
    guids.Add(MEDIATYPE_Text);
    guids.Add(MEDIATYPE_Midi);
    guids.Add(MEDIATYPE_Stream);
    guids.Add(MEDIATYPE_Interleaved);
    guids.Add(MEDIATYPE_File);
    guids.Add(MEDIATYPE_ScriptCommand);
    guids.Add(MEDIATYPE_AUXLine21Data);
    guids.Add(MEDIATYPE_VBI);
    guids.Add(MEDIATYPE_Timecode);
    guids.Add(MEDIATYPE_LMRT);
    guids.Add(MEDIATYPE_URL_STREAM);
    guids.Add(MEDIATYPE_MPEG1SystemStream);
    guids.Add(MEDIATYPE_AnalogVideo);
    guids.Add(MEDIATYPE_AnalogAudio);
    guids.Add(MEDIATYPE_MPEG2_PACK);
    guids.Add(MEDIATYPE_MPEG2_PES);
    guids.Add(MEDIATYPE_MPEG2_SECTIONS);
    guids.Add(MEDIATYPE_DVD_ENCRYPTED_PACK);
    guids.Add(MEDIATYPE_DVD_NAVIGATION);
}

void CPPageExternalFilters::SetupSubTypes(CAtlArray<GUID>& guids)
{
    guids.RemoveAll();
    guids.Add(MEDIASUBTYPE_None);
    guids.Add(MEDIASUBTYPE_CLPL);
    guids.Add(MEDIASUBTYPE_YUYV);
    guids.Add(MEDIASUBTYPE_IYUV);
    guids.Add(MEDIASUBTYPE_YVU9);
    guids.Add(MEDIASUBTYPE_Y411);
    guids.Add(MEDIASUBTYPE_Y41P);
    guids.Add(MEDIASUBTYPE_YUY2);
    guids.Add(MEDIASUBTYPE_YVYU);
    guids.Add(MEDIASUBTYPE_UYVY);
    guids.Add(MEDIASUBTYPE_Y211);
    guids.Add(MEDIASUBTYPE_CLJR);
    guids.Add(MEDIASUBTYPE_IF09);
    guids.Add(MEDIASUBTYPE_CPLA);
    guids.Add(MEDIASUBTYPE_MJPG);
    guids.Add(MEDIASUBTYPE_MJPA);
    guids.Add(MEDIASUBTYPE_MJPB);
    guids.Add(MEDIASUBTYPE_TVMJ);
    guids.Add(MEDIASUBTYPE_WAKE);
    guids.Add(MEDIASUBTYPE_CFCC);
    guids.Add(MEDIASUBTYPE_IJPG);
    guids.Add(MEDIASUBTYPE_Plum);
    guids.Add(MEDIASUBTYPE_DVCS);
    guids.Add(MEDIASUBTYPE_DVSD);
    guids.Add(MEDIASUBTYPE_MDVF);
    guids.Add(MEDIASUBTYPE_RGB1);
    guids.Add(MEDIASUBTYPE_RGB4);
    guids.Add(MEDIASUBTYPE_RGB8);
    guids.Add(MEDIASUBTYPE_RGB565);
    guids.Add(MEDIASUBTYPE_RGB555);
    guids.Add(MEDIASUBTYPE_RGB24);
    guids.Add(MEDIASUBTYPE_RGB32);
    guids.Add(MEDIASUBTYPE_ARGB1555);
    guids.Add(MEDIASUBTYPE_ARGB4444);
    guids.Add(MEDIASUBTYPE_ARGB32);
    guids.Add(MEDIASUBTYPE_A2R10G10B10);
    guids.Add(MEDIASUBTYPE_A2B10G10R10);
    guids.Add(MEDIASUBTYPE_AYUV);
    guids.Add(MEDIASUBTYPE_AI44);
    guids.Add(MEDIASUBTYPE_IA44);
    guids.Add(MEDIASUBTYPE_RGB32_D3D_DX7_RT);
    guids.Add(MEDIASUBTYPE_RGB16_D3D_DX7_RT);
    guids.Add(MEDIASUBTYPE_ARGB32_D3D_DX7_RT);
    guids.Add(MEDIASUBTYPE_ARGB4444_D3D_DX7_RT);
    guids.Add(MEDIASUBTYPE_ARGB1555_D3D_DX7_RT);
    guids.Add(MEDIASUBTYPE_RGB32_D3D_DX9_RT);
    guids.Add(MEDIASUBTYPE_RGB16_D3D_DX9_RT);
    guids.Add(MEDIASUBTYPE_ARGB32_D3D_DX9_RT);
    guids.Add(MEDIASUBTYPE_ARGB4444_D3D_DX9_RT);
    guids.Add(MEDIASUBTYPE_ARGB1555_D3D_DX9_RT);
    guids.Add(MEDIASUBTYPE_YV12);
    guids.Add(MEDIASUBTYPE_NV12);
    guids.Add(MEDIASUBTYPE_IMC1);
    guids.Add(MEDIASUBTYPE_IMC2);
    guids.Add(MEDIASUBTYPE_IMC3);
    guids.Add(MEDIASUBTYPE_IMC4);
    guids.Add(MEDIASUBTYPE_S340);
    guids.Add(MEDIASUBTYPE_S342);
    guids.Add(MEDIASUBTYPE_Overlay);
    guids.Add(MEDIASUBTYPE_MPEG1Packet);
    guids.Add(MEDIASUBTYPE_MPEG1Payload);
    guids.Add(MEDIASUBTYPE_MPEG1AudioPayload);
    guids.Add(MEDIASUBTYPE_MPEG1System);
    guids.Add(MEDIASUBTYPE_MPEG1VideoCD);
    guids.Add(MEDIASUBTYPE_MPEG1Video);
    guids.Add(MEDIASUBTYPE_MPEG1Audio);
    guids.Add(MEDIASUBTYPE_Avi);
    guids.Add(MEDIASUBTYPE_Asf);
    guids.Add(MEDIASUBTYPE_QTMovie);
    guids.Add(MEDIASUBTYPE_QTRpza);
    guids.Add(MEDIASUBTYPE_QTSmc);
    guids.Add(MEDIASUBTYPE_QTRle);
    guids.Add(MEDIASUBTYPE_QTJpeg);
    guids.Add(MEDIASUBTYPE_PCMAudio_Obsolete);
    guids.Add(MEDIASUBTYPE_PCM);
    guids.Add(MEDIASUBTYPE_WAVE);
    guids.Add(MEDIASUBTYPE_AU);
    guids.Add(MEDIASUBTYPE_AIFF);
    guids.Add(MEDIASUBTYPE_dvsd);
    guids.Add(MEDIASUBTYPE_dvhd);
    guids.Add(MEDIASUBTYPE_dvsl);
    guids.Add(MEDIASUBTYPE_dv25);
    guids.Add(MEDIASUBTYPE_dv50);
    guids.Add(MEDIASUBTYPE_dvh1);
    guids.Add(MEDIASUBTYPE_Line21_BytePair);
    guids.Add(MEDIASUBTYPE_Line21_GOPPacket);
    guids.Add(MEDIASUBTYPE_Line21_VBIRawData);
    guids.Add(MEDIASUBTYPE_TELETEXT);
    guids.Add(MEDIASUBTYPE_DRM_Audio);
    guids.Add(MEDIASUBTYPE_IEEE_FLOAT);
    guids.Add(MEDIASUBTYPE_DOLBY_AC3_SPDIF);
    guids.Add(MEDIASUBTYPE_RAW_SPORT);
    guids.Add(MEDIASUBTYPE_SPDIF_TAG_241h);
    guids.Add(MEDIASUBTYPE_DssVideo);
    guids.Add(MEDIASUBTYPE_DssAudio);
    guids.Add(MEDIASUBTYPE_VPVideo);
    guids.Add(MEDIASUBTYPE_VPVBI);
    guids.Add(MEDIASUBTYPE_ATSC_SI);
    guids.Add(MEDIASUBTYPE_DVB_SI);
    guids.Add(MEDIASUBTYPE_MPEG2DATA);
    guids.Add(MEDIASUBTYPE_MPEG2_VIDEO);
    guids.Add(MEDIASUBTYPE_MPEG2_PROGRAM);
    guids.Add(MEDIASUBTYPE_MPEG2_TRANSPORT);
    guids.Add(MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE);
    guids.Add(MEDIASUBTYPE_MPEG2_AUDIO);
    guids.Add(MEDIASUBTYPE_DOLBY_AC3);
    guids.Add(MEDIASUBTYPE_DVD_SUBPICTURE);
    guids.Add(MEDIASUBTYPE_DVD_LPCM_AUDIO);
    guids.Add(MEDIASUBTYPE_DTS);
    guids.Add(MEDIASUBTYPE_SDDS);
    guids.Add(MEDIASUBTYPE_DVD_NAVIGATION_PCI);
    guids.Add(MEDIASUBTYPE_DVD_NAVIGATION_DSI);
    guids.Add(MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER);
    guids.Add(MEDIASUBTYPE_I420);
    guids.Add(MEDIASUBTYPE_WAVE_DOLBY_AC3);
    guids.Add(MEDIASUBTYPE_WAVE_DTS);
}

BEGIN_MESSAGE_MAP(CPPageExternalFilters, CPPageBase)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateFilter)
    ON_UPDATE_COMMAND_UI(IDC_RADIO1, OnUpdateFilter)
    ON_UPDATE_COMMAND_UI(IDC_RADIO2, OnUpdateFilter)
    ON_UPDATE_COMMAND_UI(IDC_RADIO3, OnUpdateFilter)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON3, OnUpdateFilterUp)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON4, OnUpdateFilterDown)
    ON_UPDATE_COMMAND_UI(IDC_EDIT1, OnUpdateFilterMerit)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON5, OnUpdateFilter)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON6, OnUpdateSubType)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON7, OnUpdateDeleteType)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON8, OnUpdateFilter)
    ON_BN_CLICKED(IDC_BUTTON1, OnAddRegistered)
    ON_BN_CLICKED(IDC_BUTTON2, OnRemoveFilter)
    ON_BN_CLICKED(IDC_BUTTON3, OnMoveFilterUp)
    ON_BN_CLICKED(IDC_BUTTON4, OnMoveFilterDown)
    ON_LBN_DBLCLK(IDC_LIST1, OnDoubleClickFilter)
    ON_WM_VKEYTOITEM()
    ON_BN_CLICKED(IDC_BUTTON5, OnAddMajorType)
    ON_BN_CLICKED(IDC_BUTTON6, OnAddSubType)
    ON_BN_CLICKED(IDC_BUTTON7, OnDeleteType)
    ON_BN_CLICKED(IDC_BUTTON8, OnResetTypes)
    ON_LBN_SELCHANGE(IDC_LIST1, OnFilterSelectionChange)
    ON_CLBN_CHKCHANGE(IDC_LIST1, OnFilterCheckChange)
    ON_BN_CLICKED(IDC_RADIO1, OnClickedMeritRadioButton)
    ON_BN_CLICKED(IDC_RADIO2, OnClickedMeritRadioButton)
    ON_BN_CLICKED(IDC_RADIO3, OnClickedMeritRadioButton)
    ON_EN_CHANGE(IDC_EDIT1, OnChangeMerit)
    ON_NOTIFY(NM_DBLCLK, IDC_TREE1, OnDoubleClickType)
    ON_NOTIFY(TVN_KEYDOWN, IDC_TREE1, OnKeyDownType)
    ON_WM_DROPFILES()
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()


// CPPageExternalFilters message handlers

BOOL CPPageExternalFilters::OnInitDialog()
{
    __super::OnInitDialog();

    DragAcceptFiles(TRUE);

    const CAppSettings& s = AfxGetAppSettings();

    m_pFilters.RemoveAll();

    POSITION pos = s.m_filters.GetHeadPosition();
    while (pos) {
        CAutoPtr<FilterOverride> f(DEBUG_NEW FilterOverride(s.m_filters.GetNext(pos)));

        CString name(_T("<unknown>"));

        if (f->type == FilterOverride::REGISTERED) {
            name = CFGFilterRegistry(f->dispname).GetName();
            if (name.IsEmpty()) {
                name = f->name + _T(" <not registered>");
            }
        } else if (f->type == FilterOverride::EXTERNAL) {
            name = f->name;
            if (f->fTemporary) {
                name += _T(" <temporary>");
            }
            if (!CPath(MakeFullPath(f->path)).FileExists()) {
                name += _T(" <not found!>");
            }
        }

        int i = m_filters.AddString(name);
        m_filters.SetCheck(i, f->fDisabled ? 0 : 1);
        m_filters.SetItemDataPtr(i, m_pFilters.AddTail(f));
    }

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageExternalFilters::OnApply()
{
    UpdateData();

    CAppSettings& s = AfxGetAppSettings();

    s.m_filters.RemoveAll();

    for (int i = 0; i < m_filters.GetCount(); i++) {
        if (POSITION pos = (POSITION)m_filters.GetItemData(i)) {
            CAutoPtr<FilterOverride> f(DEBUG_NEW FilterOverride(m_pFilters.GetAt(pos)));
            f->fDisabled = !m_filters.GetCheck(i);
            s.m_filters.AddTail(f);
        }
    }

    s.SaveExternalFilters();

    return __super::OnApply();
}

void CPPageExternalFilters::OnUpdateFilter(CCmdUI* pCmdUI)
{
    if (FilterOverride* f = GetCurFilter()) {
        pCmdUI->Enable(!(pCmdUI->m_nID == IDC_RADIO2 && f->type == FilterOverride::EXTERNAL));
    } else {
        pCmdUI->Enable(FALSE);
    }
}

void CPPageExternalFilters::OnUpdateFilterUp(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_filters.GetCurSel() > 0);
}

void CPPageExternalFilters::OnUpdateFilterDown(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_filters.GetCurSel() >= 0 && m_filters.GetCurSel() < m_filters.GetCount() - 1);
}

void CPPageExternalFilters::OnUpdateFilterMerit(CCmdUI* pCmdUI)
{
    UpdateData();
    pCmdUI->Enable(m_iLoadType == FilterOverride::MERIT);
}

void CPPageExternalFilters::OnUpdateSubType(CCmdUI* pCmdUI)
{
    HTREEITEM node = m_tree.GetSelectedItem();
    pCmdUI->Enable(node != nullptr && m_tree.GetItemData(node) == NULL);
}

void CPPageExternalFilters::OnUpdateDeleteType(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!!m_tree.GetSelectedItem());
}

void CPPageExternalFilters::OnAddRegistered()
{
    CRegFilterChooserDlg dlg(this);
    if (dlg.DoModal() == IDOK) {
        while (!dlg.m_filters.IsEmpty()) {
            if (FilterOverride* f = dlg.m_filters.RemoveHead()) {
                CAutoPtr<FilterOverride> p(f);

                CString name = f->name;

                if (f->type == FilterOverride::EXTERNAL) {
                    if (!CPath(MakeFullPath(f->path)).FileExists()) {
                        name += _T(" <not found!>");
                    }
                }

                int i = m_filters.AddString(name);
                m_filters.SetItemDataPtr(i, m_pFilters.AddTail(p));
                m_filters.SetCheck(i, 1);

                if (dlg.m_filters.IsEmpty()) {
                    m_filters.SetCurSel(i);
                    OnFilterSelectionChange();
                }

                SetModified();
            }
        }
    }
}

void CPPageExternalFilters::OnRemoveFilter()
{
    int i = m_filters.GetCurSel();
    m_pFilters.RemoveAt((POSITION)m_filters.GetItemDataPtr(i));
    m_filters.DeleteString(i);

    if (i >= m_filters.GetCount()) {
        i--;
    }
    m_filters.SetCurSel(i);
    OnFilterSelectionChange();

    SetModified();
}

void CPPageExternalFilters::OnMoveFilterUp()
{
    StepUp(m_filters);
}

void CPPageExternalFilters::OnMoveFilterDown()
{
    StepDown(m_filters);
}

void CPPageExternalFilters::OnDoubleClickFilter()
{
    if (FilterOverride* f = GetCurFilter()) {
        CComPtr<IBaseFilter> pBF;
        CString name;

        if (f->type == FilterOverride::REGISTERED) {
            CStringW namew;
            if (CreateFilter(f->dispname, &pBF, namew)) {
                name = namew;
            }
        } else if (f->type == FilterOverride::EXTERNAL) {
            if (SUCCEEDED(LoadExternalFilter(f->path, f->clsid, &pBF))) {
                name = f->name;
            }
        }

        if (CComQIPtr<ISpecifyPropertyPages> pSPP = pBF) {
            CComPropertySheet ps(name, this);
            if (ps.AddPages(pSPP) > 0) {
                CComPtr<IFilterGraph> pFG;
                if (SUCCEEDED(pFG.CoCreateInstance(CLSID_FilterGraph))) {
                    pFG->AddFilter(pBF, L"");
                }

                ps.DoModal();
            }
        }
    }
}

int CPPageExternalFilters::OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex)
{
    if (nKey == VK_DELETE) {
        OnRemoveFilter();
        return -2;
    }

    return __super::OnVKeyToItem(nKey, pListBox, nIndex);
}

void CPPageExternalFilters::OnAddMajorType()
{
    FilterOverride* f = GetCurFilter();
    if (!f) {
        return;
    }

    CAtlArray<GUID> guids;
    SetupMajorTypes(guids);

    CSelectMediaType dlg(guids, MEDIATYPE_NULL, this);
    if (dlg.DoModal() == IDOK) {
        POSITION pos = f->guids.GetHeadPosition();
        while (pos) {
            if (f->guids.GetNext(pos) == dlg.m_guid) {
                AfxMessageBox(IDS_EXTERNAL_FILTERS_ERROR_MT, MB_ICONEXCLAMATION | MB_OK, 0);
                return;
            }
            f->guids.GetNext(pos);
        }

        f->guids.AddTail(dlg.m_guid);
        pos = f->guids.GetTailPosition();
        f->guids.AddTail(GUID_NULL);

        CString major = GetMediaTypeName(dlg.m_guid);
        CString sub = GetMediaTypeName(GUID_NULL);

        HTREEITEM node = m_tree.InsertItem(major);
        m_tree.SetItemData(node, NULL);

        node = m_tree.InsertItem(sub, node);
        m_tree.SetItemData(node, (DWORD_PTR)pos);

        SetModified();
    }
}

void CPPageExternalFilters::OnAddSubType()
{
    FilterOverride* f = GetCurFilter();
    if (!f) {
        return;
    }

    HTREEITEM node = m_tree.GetSelectedItem();
    if (!node) {
        return;
    }

    HTREEITEM child = m_tree.GetChildItem(node);
    if (!child) {
        return;
    }

    POSITION pos = (POSITION)m_tree.GetItemData(child);
    GUID major = f->guids.GetAt(pos);

    CAtlArray<GUID> guids;
    SetupSubTypes(guids);

    CSelectMediaType dlg(guids, MEDIASUBTYPE_NULL, this);
    if (dlg.DoModal() == IDOK) {
        for (child = m_tree.GetChildItem(node); child; child = m_tree.GetNextSiblingItem(child)) {
            pos = (POSITION)m_tree.GetItemData(child);
            f->guids.GetNext(pos);
            if (f->guids.GetAt(pos) == dlg.m_guid) {
                AfxMessageBox(IDS_EXTERNAL_FILTERS_ERROR_MT, MB_ICONEXCLAMATION | MB_OK, 0);
                return;
            }
        }

        f->guids.AddTail(major);
        pos = f->guids.GetTailPosition();
        f->guids.AddTail(dlg.m_guid);

        CString sub = GetMediaTypeName(dlg.m_guid);

        node = m_tree.InsertItem(sub, node);
        m_tree.SetItemData(node, (DWORD_PTR)pos);

        SetModified();
    }
}

void CPPageExternalFilters::OnDeleteType()
{
    if (FilterOverride* f = GetCurFilter()) {
        HTREEITEM node = m_tree.GetSelectedItem();
        if (!node) {
            return;
        }

        POSITION pos = (POSITION)m_tree.GetItemData(node);

        if (pos == nullptr) {
            for (HTREEITEM child = m_tree.GetChildItem(node); child; child = m_tree.GetNextSiblingItem(child)) {
                pos = (POSITION)m_tree.GetItemData(child);

                POSITION pos1 = pos;
                f->guids.GetNext(pos);
                POSITION pos2 = pos;
                f->guids.GetNext(pos);

                f->guids.RemoveAt(pos1);
                f->guids.RemoveAt(pos2);
            }

            m_tree.DeleteItem(node);
        } else {
            HTREEITEM parent = m_tree.GetParentItem(node);

            POSITION pos1 = pos;
            f->guids.GetNext(pos);
            POSITION pos2 = pos;
            f->guids.GetNext(pos);

            m_tree.DeleteItem(node);

            if (!m_tree.ItemHasChildren(parent)) {
                f->guids.SetAt(pos2, GUID_NULL);
                node = m_tree.InsertItem(GetMediaTypeName(GUID_NULL), parent);
                m_tree.SetItemData(node, (DWORD_PTR)pos1);
            } else {
                f->guids.RemoveAt(pos1);
                f->guids.RemoveAt(pos2);
            }
        }

        SetModified();
    }
}

void CPPageExternalFilters::OnResetTypes()
{
    if (FilterOverride* f = GetCurFilter()) {
        if (f->type == FilterOverride::REGISTERED) {
            CFGFilterRegistry fgf(f->dispname);
            if (!fgf.GetName().IsEmpty()) {
                f->guids.RemoveAll();
                f->backup.RemoveAll();

                f->guids.AddTailList(&fgf.GetTypes());
                f->backup.AddTailList(&fgf.GetTypes());
            } else {
                f->guids.RemoveAll();
                f->guids.AddTailList(&f->backup);
            }
        } else {
            f->guids.RemoveAll();
            f->guids.AddTailList(&f->backup);
        }

        m_pLastSelFilter = nullptr;
        OnFilterSelectionChange();

        SetModified();
    }
}

void CPPageExternalFilters::OnFilterSelectionChange()
{
    if (FilterOverride* f = GetCurFilter()) {
        if (m_pLastSelFilter == f) {
            return;
        }
        m_pLastSelFilter = f;

        m_iLoadType = f->iLoadType;
        UpdateData(FALSE);
        m_dwMerit = f->dwMerit;

        HTREEITEM dummy_item = m_tree.InsertItem(_T(""), 0, 0, nullptr, TVI_FIRST);
        if (dummy_item)
            for (HTREEITEM item = m_tree.GetNextVisibleItem(dummy_item); item; item = m_tree.GetNextVisibleItem(dummy_item)) {
                m_tree.DeleteItem(item);
            }

        CMapStringToPtr map;

        POSITION pos = f->guids.GetHeadPosition();
        while (pos) {
            POSITION tmp = pos;
            CString major = GetMediaTypeName(f->guids.GetNext(pos));
            CString sub = GetMediaTypeName(f->guids.GetNext(pos));

            HTREEITEM node = nullptr;

            void* val = nullptr;
            if (map.Lookup(major, val)) {
                node = (HTREEITEM)val;
            } else {
                map[major] = node = m_tree.InsertItem(major);
            }
            m_tree.SetItemData(node, NULL);

            node = m_tree.InsertItem(sub, node);
            m_tree.SetItemData(node, (DWORD_PTR)tmp);
        }

        m_tree.DeleteItem(dummy_item);

        for (HTREEITEM item = m_tree.GetFirstVisibleItem(); item; item = m_tree.GetNextVisibleItem(item)) {
            m_tree.Expand(item, TVE_EXPAND);
        }

        m_tree.EnsureVisible(m_tree.GetRootItem());
    } else {
        m_pLastSelFilter = nullptr;

        m_iLoadType = FilterOverride::PREFERRED;
        UpdateData(FALSE);
        m_dwMerit = 0;

        m_tree.DeleteAllItems();
    }
}

void CPPageExternalFilters::OnFilterCheckChange()
{
    SetModified();
}

void CPPageExternalFilters::OnClickedMeritRadioButton()
{
    UpdateData();

    FilterOverride* f = GetCurFilter();
    if (f && f->iLoadType != m_iLoadType) {
        f->iLoadType = m_iLoadType;

        SetModified();
    }
}

void CPPageExternalFilters::OnChangeMerit()
{
    UpdateData();
    if (FilterOverride* f = GetCurFilter()) {
        DWORD dw;
        if (m_dwMerit.GetDWORD(dw) && f->dwMerit != dw) {
            f->dwMerit = dw;

            SetModified();
        }
    }
}

void CPPageExternalFilters::OnDoubleClickType(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = 0;

    if (FilterOverride* f = GetCurFilter()) {
        HTREEITEM node = m_tree.GetSelectedItem();
        if (!node) {
            return;
        }

        POSITION pos = (POSITION)m_tree.GetItemData(node);
        if (!pos) {
            return;
        }

        f->guids.GetNext(pos);
        if (!pos) {
            return;
        }

        CAtlArray<GUID> guids;
        SetupSubTypes(guids);

        CSelectMediaType dlg(guids, f->guids.GetAt(pos), this);
        if (dlg.DoModal() == IDOK) {
            f->guids.SetAt(pos, dlg.m_guid);
            m_tree.SetItemText(node, GetMediaTypeName(dlg.m_guid));

            SetModified();
        }
    }
}

void CPPageExternalFilters::OnKeyDownType(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);

    if (pTVKeyDown->wVKey == VK_DELETE) {
        OnDeleteType();
    }

    *pResult = 0;
}

void CPPageExternalFilters::OnDropFiles(HDROP hDropInfo)
{
    SetActiveWindow();

    UINT nFiles = ::DragQueryFile(hDropInfo, UINT_MAX, nullptr, 0);
    for (UINT iFile = 0; iFile < nFiles; iFile++) {
        TCHAR szFileName[MAX_PATH];
        ::DragQueryFile(hDropInfo, iFile, szFileName, MAX_PATH);

        CFilterMapper2 fm2(false);
        fm2.Register(szFileName);

        while (!fm2.m_filters.IsEmpty()) {
            if (FilterOverride* f = fm2.m_filters.RemoveHead()) {
                CAutoPtr<FilterOverride> p(f);
                int i = m_filters.AddString(f->name);
                m_filters.SetItemDataPtr(i, m_pFilters.AddTail(p));
                m_filters.SetCheck(i, 1);

                if (fm2.m_filters.IsEmpty()) {
                    m_filters.SetCurSel(i);
                    OnFilterSelectionChange();
                }

                SetModified();
            }
        }
    }
    ::DragFinish(hDropInfo);
}

BOOL CPPageExternalFilters::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;

    int nIndex = (int)pNMHDR->idFrom;
    if (0 <= nIndex && nIndex < m_filters.GetCount()) {
        if (POSITION pos = (POSITION)m_filters.GetItemData(nIndex)) {
            CAutoPtr<FilterOverride>& f = m_pFilters.GetAt(pos);

            pTTT->lpszText = (f->type == FilterOverride::EXTERNAL) ? (LPTSTR)(LPCTSTR)f->path : _T("Registered filter");
        } else {
            ASSERT(FALSE);
        }
    } else {
        ASSERT(FALSE);
    }

    *pResult = 0;

    return TRUE; // message was handled
}

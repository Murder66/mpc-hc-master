/*
 * (C) 2010-2013 see Authors.txt
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
#include "resource.h"
#include "MpcAudioRendererSettingsWnd.h"
#include "../../../DSUtil/DSUtil.h"

#include "../../../mpc-hc/InternalFiltersConfig.h"


CMpcAudioRendererSettingsWnd::CMpcAudioRendererSettingsWnd()
{
}

bool CMpcAudioRendererSettingsWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    ASSERT(!m_pMAR);

    m_pMAR.Release();

    POSITION pos = pUnks.GetHeadPosition();
    while (pos && !(m_pMAR = pUnks.GetNext(pos))) {
        ;
    }

    if (!m_pMAR) {
        return false;
    }

    return true;
}

void CMpcAudioRendererSettingsWnd::OnDisconnect()
{
    m_pMAR.Release();
}

bool CALLBACK DSEnumProc(LPGUID lpGUID,
                         LPCTSTR lpszDesc,
                         LPCTSTR lpszDrvName,
                         LPVOID lpContext)
{
    CComboBox* pCombo = (CComboBox*)lpContext;
    ASSERT(pCombo);
    LPGUID lpTemp = nullptr;

    if (lpGUID != nullptr) { // NULL only for "Primary Sound Driver".
        if ((lpTemp = (LPGUID)malloc(sizeof(GUID))) == nullptr) {
            return TRUE;
        }
        memcpy(lpTemp, lpGUID, sizeof(GUID));
    }
    pCombo->AddString(lpszDesc);
    free(lpTemp);
    return TRUE;
}

bool CMpcAudioRendererSettingsWnd::OnActivate()
{
    ASSERT(IPP_FONTSIZE == 13);
    DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
    CPoint p(10, 10);

    m_cbWasapiMode.Create(ResStr(IDS_ARS_WASAPI_MODE), dwStyle | BS_AUTOCHECKBOX | BS_LEFTTEXT, CRect(p, CSize(IPP_SCALE(320), m_fontheight)), this, IDC_PP_WASAPI_MODE);

    p.y += IPP_SCALE(20);
    m_cbMuteFastForward.Create(ResStr(IDS_ARS_MUTE_FAST_FORWARD), dwStyle | BS_AUTOCHECKBOX | BS_LEFTTEXT, CRect(p, CSize(IPP_SCALE(320), m_fontheight)), this, IDC_PP_MUTE_FAST_FORWARD);

    p.y += IPP_SCALE(25);
    m_txtSoundDevice.Create(ResStr(IDS_ARS_SOUND_DEVICE), WS_VISIBLE | WS_CHILD, CRect(p, CSize(IPP_SCALE(115), m_fontheight)), this, (UINT)IDC_STATIC);
    m_cbSoundDevice.Create(dwStyle | CBS_DROPDOWNLIST | WS_VSCROLL, CRect(p + CPoint(IPP_SCALE(120), -4), CSize(IPP_SCALE(200), 200)), this, IDC_PP_SOUND_DEVICE);

    SetClassLongPtr(GetDlgItem(IDC_PP_SOUND_DEVICE)->m_hWnd, GCLP_HCURSOR, (LONG_PTR)AfxGetApp()->LoadStandardCursor(IDC_HAND));

    DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumProc, (VOID*)&m_cbSoundDevice);

    if (m_cbSoundDevice.GetCount() > 0) {
        int idx = m_cbSoundDevice.FindString(0, m_pMAR->GetSoundDevice());
        if (idx < 0) {
            m_cbSoundDevice.SetCurSel(0);
        } else {
            m_cbSoundDevice.SetCurSel(m_cbSoundDevice.FindString(0, m_pMAR->GetSoundDevice()));
        }
    }

    m_cbWasapiMode.SetCheck(m_pMAR->GetWasapiMode());
    m_cbMuteFastForward.SetCheck(m_pMAR->GetMuteFastForward());

    for (CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
        pWnd->SetFont(&m_font, FALSE);
    }

    CorrectComboListWidth(m_cbSoundDevice);

    return true;
}

void CMpcAudioRendererSettingsWnd::OnDeactivate()
{
}

bool CMpcAudioRendererSettingsWnd::OnApply()
{
    OnDeactivate();

    if (m_pMAR) {
        m_pMAR->SetWasapiMode(m_cbWasapiMode.GetCheck());
        m_pMAR->SetMuteFastForward(m_cbMuteFastForward.GetCheck());
        CString str;
        int idx = m_cbSoundDevice.GetCurSel();
        if (!(idx < 0)) {
            m_cbSoundDevice.GetLBText(idx, str);
            m_pMAR->SetSoundDevice(str);
        }
        m_pMAR->Apply();
    }

    return true;
}


BEGIN_MESSAGE_MAP(CMpcAudioRendererSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()

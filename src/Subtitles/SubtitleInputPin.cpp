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
#include "SubtitleInputPin.h"
#include "VobSubFile.h"
#include "RTS.h"
#include "DVBSub.h"
#include "PGSSub.h"

#include <InitGuid.h>
#include <uuids.h>
#include "moreuuids.h"

// our first format id
#define __GAB1__ "GAB1"

// our tags for __GAB1__ (ushort) + size (ushort)

// "lang" + '0'
#define __GAB1_LANGUAGE__ 0
// (int)start+(int)stop+(char*)line+'0'
#define __GAB1_ENTRY__ 1
// L"lang" + '0'
#define __GAB1_LANGUAGE_UNICODE__ 2
// (int)start+(int)stop+(WCHAR*)line+'0'
#define __GAB1_ENTRY_UNICODE__ 3

// same as __GAB1__, but the size is (uint) and only __GAB1_LANGUAGE_UNICODE__ is valid
#define __GAB2__ "GAB2"

// (BYTE*)
#define __GAB1_RAWTEXTSUBTITLE__ 4

CSubtitleInputPin::CSubtitleInputPin(CBaseFilter* pFilter, CCritSec* pLock, CCritSec* pSubLock, HRESULT* phr)
    : CBaseInputPin(NAME("CSubtitleInputPin"), pFilter, pLock, phr, L"Input")
    , m_pSubLock(pSubLock)
{
    m_bCanReconnectWhenActive = true;
}

HRESULT CSubtitleInputPin::CheckMediaType(const CMediaType* pmt)
{
    return pmt->majortype == MEDIATYPE_Text && (pmt->subtype == MEDIASUBTYPE_NULL || pmt->subtype == FOURCCMap((DWORD)0))
           || pmt->majortype == MEDIATYPE_Subtitle && pmt->subtype == MEDIASUBTYPE_UTF8
           || pmt->majortype == MEDIATYPE_Subtitle && (pmt->subtype == MEDIASUBTYPE_SSA || pmt->subtype == MEDIASUBTYPE_ASS || pmt->subtype == MEDIASUBTYPE_ASS2)
           || pmt->majortype == MEDIATYPE_Subtitle && (pmt->subtype == MEDIASUBTYPE_VOBSUB)
           || IsRLECodedSub(pmt)
           ? S_OK
           : E_FAIL;
}

HRESULT CSubtitleInputPin::CompleteConnect(IPin* pReceivePin)
{
    if (m_mt.majortype == MEDIATYPE_Text) {
        if (!(m_pSubStream = DEBUG_NEW CRenderedTextSubtitle(m_pSubLock))) {
            return E_FAIL;
        }
        CRenderedTextSubtitle* pRTS = (CRenderedTextSubtitle*)(ISubStream*)m_pSubStream;
        pRTS->m_name = CString(GetPinName(pReceivePin)) + _T(" (embeded)");
        pRTS->m_dstScreenSize = CSize(384, 288);
        pRTS->CreateDefaultStyle(DEFAULT_CHARSET);
    } else if (m_mt.majortype == MEDIATYPE_Subtitle) {
        SUBTITLEINFO* psi = (SUBTITLEINFO*)m_mt.pbFormat;
        DWORD   dwOffset = 0;
        CString name;
        LCID    lcid = 0;

        if (psi != nullptr) {
            dwOffset = psi->dwOffset;

            name = ISO6392ToLanguage(psi->IsoLang);
            lcid = ISO6392ToLcid(psi->IsoLang);

            CString trackName(psi->TrackName);
            trackName.Trim();
            if (!trackName.IsEmpty()) {
                if (!name.IsEmpty()) {
                    if (trackName[0] != _T('(') && trackName[0] != _T('[')) {
                        name += _T(",");
                    }
                    name += _T(" ");
                }
                name += trackName;
            }
            if (name.IsEmpty()) {
                name = _T("Unknown");
            }
        }

        name.Replace(_T(""), _T(""));
        name.Replace(_T(""), _T(""));

        if (m_mt.subtype == MEDIASUBTYPE_UTF8
                /*|| m_mt.subtype == MEDIASUBTYPE_USF*/
                || m_mt.subtype == MEDIASUBTYPE_SSA
                || m_mt.subtype == MEDIASUBTYPE_ASS
                || m_mt.subtype == MEDIASUBTYPE_ASS2) {
            if (!(m_pSubStream = DEBUG_NEW CRenderedTextSubtitle(m_pSubLock))) {
                return E_FAIL;
            }
            CRenderedTextSubtitle* pRTS = (CRenderedTextSubtitle*)(ISubStream*)m_pSubStream;
            pRTS->m_name = name;
            pRTS->m_lcid = lcid;
            pRTS->m_dstScreenSize = CSize(384, 288);
            pRTS->CreateDefaultStyle(DEFAULT_CHARSET);

            if (dwOffset > 0 && m_mt.cbFormat - dwOffset > 0) {
                CMediaType mt = m_mt;
                if (mt.pbFormat[dwOffset + 0] != 0xef
                        && mt.pbFormat[dwOffset + 1] != 0xbb
                        && mt.pbFormat[dwOffset + 2] != 0xfb) {
                    dwOffset -= 3;
                    mt.pbFormat[dwOffset + 0] = 0xef;
                    mt.pbFormat[dwOffset + 1] = 0xbb;
                    mt.pbFormat[dwOffset + 2] = 0xbf;
                }

                pRTS->Open(mt.pbFormat + dwOffset, mt.cbFormat - dwOffset, DEFAULT_CHARSET, pRTS->m_name);
            }
        } else if (m_mt.subtype == MEDIASUBTYPE_VOBSUB) {
            if (!(m_pSubStream = DEBUG_NEW CVobSubStream(m_pSubLock))) {
                return E_FAIL;
            }
            CVobSubStream* pVSS = (CVobSubStream*)(ISubStream*)m_pSubStream;
            pVSS->Open(name, m_mt.pbFormat + dwOffset, m_mt.cbFormat - dwOffset);
        } else if (IsRLECodedSub(&m_mt)) {
            if (m_mt.subtype == MEDIASUBTYPE_DVB_SUBTITLES) {
                m_pSubStream = DEBUG_NEW CDVBSub(m_pSubLock, name, lcid);
            } else {
                m_pSubStream = DEBUG_NEW CPGSSub(m_pSubLock, name, lcid);
            }
            if (!m_pSubStream) {
                return E_FAIL;
            }
        }
    }

    AddSubStream(m_pSubStream);

    return __super::CompleteConnect(pReceivePin);
}

HRESULT CSubtitleInputPin::BreakConnect()
{
    RemoveSubStream(m_pSubStream);
    m_pSubStream = nullptr;

    ASSERT(IsStopped());

    return __super::BreakConnect();
}

STDMETHODIMP CSubtitleInputPin::ReceiveConnection(IPin* pConnector, const AM_MEDIA_TYPE* pmt)
{
    if (m_Connected) {
        RemoveSubStream(m_pSubStream);
        m_pSubStream = nullptr;

        m_Connected->Release();
        m_Connected = nullptr;
    }

    return __super::ReceiveConnection(pConnector, pmt);
}

STDMETHODIMP CSubtitleInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    CAutoLock cAutoLock(&m_csReceive);

    if (m_mt.majortype == MEDIATYPE_Text
            || m_mt.majortype == MEDIATYPE_Subtitle
            && (m_mt.subtype == MEDIASUBTYPE_UTF8
                /*|| m_mt.subtype == MEDIASUBTYPE_USF*/
                || m_mt.subtype == MEDIASUBTYPE_SSA
                || m_mt.subtype == MEDIASUBTYPE_ASS
                || m_mt.subtype == MEDIASUBTYPE_ASS2)) {
        CAutoLock cAutoLock2(m_pSubLock);
        CRenderedTextSubtitle* pRTS = (CRenderedTextSubtitle*)(ISubStream*)m_pSubStream;
        pRTS->RemoveAll();
        pRTS->CreateSegments();
    } else if (m_mt.majortype == MEDIATYPE_Subtitle && (m_mt.subtype == MEDIASUBTYPE_VOBSUB)) {
        CAutoLock cAutoLock2(m_pSubLock);
        CVobSubStream* pVSS = (CVobSubStream*)(ISubStream*)m_pSubStream;
        pVSS->RemoveAll();
    } else if (IsRLECodedSub(&m_mt)) {
        CAutoLock cAutoLock2(m_pSubLock);
        CRLECodedSubtitle* pRLECodedSubtitle = (CRLECodedSubtitle*)(ISubStream*)m_pSubStream;
        pRLECodedSubtitle->NewSegment(tStart, tStop, dRate);
    }

    TRACE(_T("NewSegment: InvalidateSubtitle(%I64d, ...)\n"), tStart);
    // IMPORTANT: m_pSubLock must not be locked when calling this
    InvalidateSubtitle(tStart, m_pSubStream);

    return __super::NewSegment(tStart, tStop, dRate);
}

interface __declspec(uuid("D3D92BC3-713B-451B-9122-320095D51EA5"))
IMpeg2DemultiplexerTesting :
public IUnknown {
    STDMETHOD(GetMpeg2StreamType)(ULONG * plType) PURE;
    STDMETHOD(toto)() PURE;
};

STDMETHODIMP CSubtitleInputPin::Receive(IMediaSample* pSample)
{
    HRESULT hr;

    hr = __super::Receive(pSample);
    if (FAILED(hr)) {
        return hr;
    }

    CAutoLock cAutoLock(&m_csReceive);

    REFERENCE_TIME tStart, tStop;
    pSample->GetTime(&tStart, &tStop);
    tStart += m_tStart;
    tStop += m_tStart;

    BYTE* pData = nullptr;
    hr = pSample->GetPointer(&pData);
    if (FAILED(hr) || pData == nullptr) {
        return hr;
    }

    int len = pSample->GetActualDataLength();

    bool fInvalidate = false;

    if (m_mt.majortype == MEDIATYPE_Text) {
        CAutoLock cAutoLock2(m_pSubLock);
        CRenderedTextSubtitle* pRTS = (CRenderedTextSubtitle*)(ISubStream*)m_pSubStream;

        if (!strncmp((char*)pData, __GAB1__, strlen(__GAB1__))) {
            char* ptr = (char*)&pData[strlen(__GAB1__) + 1];
            char* end = (char*)&pData[len];

            while (ptr < end) {
                WORD tag = *((WORD*)(ptr));
                ptr += 2;
                WORD size = *((WORD*)(ptr));
                ptr += 2;

                if (tag == __GAB1_LANGUAGE__) {
                    pRTS->m_name = CString(ptr);
                } else if (tag == __GAB1_ENTRY__) {
                    pRTS->Add(AToW(&ptr[8]), false, *(int*)ptr, *(int*)(ptr + 4));
                    fInvalidate = true;
                } else if (tag == __GAB1_LANGUAGE_UNICODE__) {
                    pRTS->m_name = (WCHAR*)ptr;
                } else if (tag == __GAB1_ENTRY_UNICODE__) {
                    pRTS->Add((WCHAR*)(ptr + 8), true, *(int*)ptr, *(int*)(ptr + 4));
                    fInvalidate = true;
                }

                ptr += size;
            }
        } else if (!strncmp((char*)pData, __GAB2__, strlen(__GAB2__))) {
            char* ptr = (char*)&pData[strlen(__GAB2__) + 1];
            char* end = (char*)&pData[len];

            while (ptr < end) {
                WORD tag = *((WORD*)(ptr));
                ptr += 2;
                DWORD size = *((DWORD*)(ptr));
                ptr += 4;

                if (tag == __GAB1_LANGUAGE_UNICODE__) {
                    pRTS->m_name = (WCHAR*)ptr;
                } else if (tag == __GAB1_RAWTEXTSUBTITLE__) {
                    pRTS->Open((BYTE*)ptr, size, DEFAULT_CHARSET, pRTS->m_name);
                    fInvalidate = true;
                }

                ptr += size;
            }
        } else if (pData != 0 && len > 1 && *pData != 0) {
            CStringA str((char*)pData, len);

            str.Replace("\r\n", "\n");
            str.Trim();

            if (!str.IsEmpty()) {
                pRTS->Add(AToW(str), false, (int)(tStart / 10000), (int)(tStop / 10000));
                fInvalidate = true;
            }
        }
    } else if (m_mt.majortype == MEDIATYPE_Subtitle) {
        CAutoLock cAutoLock2(m_pSubLock);

        if (m_mt.subtype == MEDIASUBTYPE_UTF8) {
            CRenderedTextSubtitle* pRTS = (CRenderedTextSubtitle*)(ISubStream*)m_pSubStream;

            CStringW str = UTF8To16(CStringA((LPCSTR)pData, len)).Trim();
            if (!str.IsEmpty()) {
                pRTS->Add(str, true, (int)(tStart / 10000), (int)(tStop / 10000));
                fInvalidate = true;
            }
        } else if (m_mt.subtype == MEDIASUBTYPE_SSA || m_mt.subtype == MEDIASUBTYPE_ASS || m_mt.subtype == MEDIASUBTYPE_ASS2) {
            CRenderedTextSubtitle* pRTS = (CRenderedTextSubtitle*)(ISubStream*)m_pSubStream;

            CStringW str = UTF8To16(CStringA((LPCSTR)pData, len)).Trim();
            if (!str.IsEmpty()) {
                STSEntry stse;

                int fields = m_mt.subtype == MEDIASUBTYPE_ASS2 ? 10 : 9;

                CAtlList<CStringW> sl;
                Explode(str, sl, ',', fields);
                if (sl.GetCount() == (size_t)fields) {
                    stse.readorder = wcstol(sl.RemoveHead(), nullptr, 10);
                    stse.layer = wcstol(sl.RemoveHead(), nullptr, 10);
                    stse.style = sl.RemoveHead();
                    stse.actor = sl.RemoveHead();
                    stse.marginRect.left = wcstol(sl.RemoveHead(), nullptr, 10);
                    stse.marginRect.right = wcstol(sl.RemoveHead(), nullptr, 10);
                    stse.marginRect.top = stse.marginRect.bottom = wcstol(sl.RemoveHead(), nullptr, 10);
                    if (fields == 10) {
                        stse.marginRect.bottom = wcstol(sl.RemoveHead(), nullptr, 10);
                    }
                    stse.effect = sl.RemoveHead();
                    stse.str = sl.RemoveHead();
                }

                if (!stse.str.IsEmpty()) {
                    pRTS->Add(stse.str, true, (int)(tStart / 10000), (int)(tStop / 10000),
                              stse.style, stse.actor, stse.effect, stse.marginRect, stse.layer, stse.readorder);
                    fInvalidate = true;
                }
            }
        } else if (m_mt.subtype == MEDIASUBTYPE_VOBSUB) {
            CVobSubStream* pVSS = (CVobSubStream*)(ISubStream*)m_pSubStream;
            pVSS->Add(tStart, tStop, pData, len);
        } else if (IsRLECodedSub(&m_mt)) {
            CRLECodedSubtitle* pRLECodedSubtitle = (CRLECodedSubtitle*)(ISubStream*)m_pSubStream;
            pRLECodedSubtitle->ParseSample(pSample);
        }
    }

    if (fInvalidate) {
        TRACE(_T("Receive: InvalidateSubtitle(%I64d, ...)\n"), tStart);
        // IMPORTANT: m_pSubLock must not be locked when calling this
        InvalidateSubtitle(tStart, m_pSubStream);
    }

    hr = S_OK;

    return hr;
}

STDMETHODIMP CSubtitleInputPin::EndOfStream(void)
{
    HRESULT hr = __super::EndOfStream();

    if (SUCCEEDED(hr) && IsRLECodedSub(&m_mt)) {
        CAutoLock cAutoLock(m_pSubLock);
        CRLECodedSubtitle* pRLECodedSubtitle = (CRLECodedSubtitle*)(ISubStream*)m_pSubStream;
        pRLECodedSubtitle->EndOfStream();
    }

    return hr;
}

bool CSubtitleInputPin::IsRLECodedSub(const CMediaType* pmt) const
{
    return !!(pmt->majortype == MEDIATYPE_Subtitle
              && (pmt->subtype == MEDIASUBTYPE_HDMVSUB                                                // Blu-Ray presentation graphics
                  || pmt->subtype == MEDIASUBTYPE_DVB_SUBTITLES                                       // DVB subtitles
                  || (pmt->subtype == MEDIASUBTYPE_NULL && pmt->formattype == FORMAT_SubtitleInfo))); // Workaround : support for Haali PGS
}

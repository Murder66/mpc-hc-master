/*
 * (C) 2009-2014 see Authors.txt
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

#define FORMAT_VERSION_0       0
#define FORMAT_VERSION_1       1
#define FORMAT_VERSION_2       2
#define FORMAT_VERSION_3       3
#define FORMAT_VERSION_CURRENT 4

#define DVB_MAX_AUDIO    10
#define DVB_MAX_SUBTITLE 10

struct EventDescriptor {
    CString eventName;
    CString eventDesc;
    time_t startTime;
    time_t duration;
    CString strStartTime;
    CString strEndTime;
    CArray<CString> extendedDescriptorsItemsDesc;
    CArray<CString> extendedDescriptorsItemsContent;
    CStringList extendedDescriptorsTexts;
    int parentalRating;
    CString content;

    EventDescriptor()
        : startTime(0)
        , duration(0)
        , parentalRating(-1)
    {};
};

enum DVB_STREAM_TYPE {
    DVB_MPV      = 0x00,
    DVB_H264     = 0x01,
    DVB_MPA      = 0x02,
    DVB_AC3      = 0x03,
    DVB_EAC3     = 0x04,
    DVB_LATM     = 0x11,
    DVB_PSI      = 0x80,
    DVB_TIF      = 0x81,
    DVB_EPG      = 0x82,
    //DVB_PMT    = 0x83,
    DVB_SUB      = 0x83,
    DVB_SUBTITLE = 0xFE,
    DVB_UNKNOWN  = 0xFF
};

enum DVB_CHROMA_TYPE {
    DVB_Chroma_NONE  = 0x00,
    DVB_Chroma_4_2_0 = 0x01,
    DVB_Chroma_4_2_2 = 0x02,
    DVB_Chroma_4_4_4 = 0x03
};

enum DVB_FPS_TYPE {
    DVB_FPS_NONE   = 0x00,
    DVB_FPS_23_976 = 0x01,
    DVB_FPS_24_0   = 0x02,
    DVB_FPS_25_0   = 0x03,
    DVB_FPS_29_97  = 0x04,
    DVB_FPS_30_0   = 0x05,
    DVB_FPS_50_0   = 0x06,
    DVB_FPS_59_94  = 0x07,
    DVB_FPS_60_0   = 0x08
};

enum DVB_AspectRatio_TYPE {
    DVB_AR_NULL   = 0x00,
    DVB_AR_1      = 0x01,
    DVB_AR_3_4    = 0x02,
    DVB_AR_9_16   = 0x03,
    DVB_AR_1_2_21 = 0x04
};

struct DVBStreamInfo {
    ULONG           PID;
    DVB_STREAM_TYPE Type;
    PES_STREAM_TYPE PesType;
    CString         Language;

    LCID GetLCID() { return ISO6392ToLcid(CStringA(Language)); };
};

class CDVBChannel
{
public:
    CDVBChannel();
    ~CDVBChannel();

    bool FromString(CString strValue);
    CString ToString();
    /**
     * @brief Output a JSON representation of a DVB channel.
     * @note The object contains two elements : "index", which corresponds to
     * @c m_nPrefNumber, and "name", which contains @c m_strName.
     * @returns A string representing a JSON object containing the
     * aforementioned elements.
     */
    CStringA ToJSON() const;

    LPCTSTR GetName() const { return m_strName; };
    ULONG GetFrequency() const { return m_ulFrequency; };
    int GetPrefNumber() const { return m_nPrefNumber; };
    int GetOriginNumber() const { return m_nOriginNumber; };
    ULONG GetONID() const { return m_ulONID; };
    ULONG GetTSID() const { return m_ulTSID; };
    ULONG GetSID() const { return m_ulSID; };
    ULONG GetPMT() const { return m_ulPMT; };
    ULONG GetPCR() const { return m_ulPCR; };
    ULONG GetVideoPID() const { return m_ulVideoPID; };
    DVB_FPS_TYPE GetVideoFps() const { return m_nVideoFps; }
    CString GetVideoFpsDesc();
    DVB_CHROMA_TYPE GetVideoChroma() const { return m_nVideoChroma; }
    ULONG GetVideoWidth() const {return m_nVideoWidth; }
    ULONG GetVideoHeight() const {return m_nVideoHeight; }
    DVB_AspectRatio_TYPE GetVideoAR() {return m_nVideoAR; }
    DWORD GetVideoARx();
    DWORD GetVideoARy();
    DVB_STREAM_TYPE GetVideoType() const { return m_nVideoType; }
    ULONG GetDefaultAudioPID() const { return m_Audios[GetDefaultAudio()].PID; };
    DVB_STREAM_TYPE GetDefaultAudioType() const { return m_Audios[GetDefaultAudio()].Type; }
    ULONG GetDefaultSubtitlePID() const { return m_Subtitles[GetDefaultSubtitle()].PID; };
    int GetAudioCount() const { return m_nAudioCount; };
    int GetDefaultAudio() const { return m_nDefaultAudio; };
    int GetSubtitleCount() const { return m_nSubtitleCount; };
    int GetDefaultSubtitle() const { return m_nDefaultSubtitle; };
    DVBStreamInfo* GetAudio(int nIndex) { return &m_Audios[nIndex]; };
    const DVBStreamInfo* GetAudio(int nIndex) const { return &m_Audios[nIndex]; };
    DVBStreamInfo* GetSubtitle(int nIndex) { return &m_Subtitles[nIndex]; };
    const DVBStreamInfo* GetSubtitle(int nIndex) const { return &m_Subtitles[nIndex]; };
    bool HasName() const { return !m_strName.IsEmpty(); };
    bool IsEncrypted() const { return m_bEncrypted; };
    bool GetNowNextFlag() const { return m_bNowNextFlag; };
    REFERENCE_TIME GetAvgTimePerFrame();

    void SetName(LPCTSTR Value) { m_strName = Value; };
    void SetFrequency(ULONG Value) { m_ulFrequency = Value; };
    void SetPrefNumber(int Value) { m_nPrefNumber = Value; };
    void SetOriginNumber(int Value) { m_nOriginNumber = m_nPrefNumber = Value; };
    void SetEncrypted(bool Value) { m_bEncrypted = Value; };
    void SetNowNextFlag(bool Value) { m_bNowNextFlag = Value; };
    void SetONID(ULONG Value) { m_ulONID = Value; };
    void SetTSID(ULONG Value) { m_ulTSID = Value; };
    void SetSID(ULONG Value) { m_ulSID = Value; };
    void SetPMT(ULONG Value) { m_ulPMT = Value; };
    void SetPCR(ULONG Value) { m_ulPCR = Value; };
    void SetVideoPID(ULONG Value) { m_ulVideoPID = Value; };
    void SetVideoFps(DVB_FPS_TYPE Value) { m_nVideoFps = Value; };
    void SetVideoChroma(DVB_CHROMA_TYPE Value) { m_nVideoChroma = Value; };
    void SetVideoWidth(ULONG Value) { m_nVideoWidth = Value; };
    void SetVideoHeight(ULONG Value) { m_nVideoHeight = Value; };
    void SetVideoAR(DVB_AspectRatio_TYPE Value) { m_nVideoAR = Value; };
    void SetDefaultAudio(int Value) { m_nDefaultAudio = Value; }
    void SetDefaultSubtitle(int Value) { m_nDefaultSubtitle = Value; }

    void AddStreamInfo(ULONG ulPID, DVB_STREAM_TYPE nType, PES_STREAM_TYPE nPesType, LPCTSTR strLanguage);

private:
    CString m_strName;
    ULONG m_ulFrequency;
    int m_nPrefNumber;
    int m_nOriginNumber;
    bool m_bEncrypted;
    bool m_bNowNextFlag;
    ULONG m_ulONID;
    ULONG m_ulTSID;
    ULONG m_ulSID;
    ULONG m_ulPMT;
    ULONG m_ulPCR;
    ULONG m_ulVideoPID;
    DVB_STREAM_TYPE m_nVideoType;
    DVB_FPS_TYPE m_nVideoFps;
    DVB_CHROMA_TYPE m_nVideoChroma;
    ULONG m_nVideoWidth;
    ULONG m_nVideoHeight;
    DVB_AspectRatio_TYPE m_nVideoAR;
    int m_nAudioCount;
    int m_nDefaultAudio;
    int m_nSubtitleCount;
    int m_nDefaultSubtitle;
    DVBStreamInfo m_Audios[DVB_MAX_AUDIO];
    DVBStreamInfo m_Subtitles[DVB_MAX_SUBTITLE];
};

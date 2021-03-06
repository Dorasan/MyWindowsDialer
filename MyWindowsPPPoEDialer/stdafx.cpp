
// stdafx.cpp : 只包括标准包含文件的源文件
// MyWindowsPPPoEDialer.pch 将作为预编译标头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"


#include "MyWindowsPPPoEDialerDlg.h"
#define DialName L"Dora's PPPoEDialer"

// Create RAS Connection
DWORD createEntry() {
    LPRASENTRY lpRasEntry = new RASENTRY;

    ZeroMemory(lpRasEntry, sizeof(RASENTRY));
    lpRasEntry->dwSize = sizeof(RASENTRY);
    wcscpy(lpRasEntry->szDeviceName, L"WAN Miniport (PPPOE)");
    wcscpy(lpRasEntry->szDeviceType, L"pppoe");
    wcscpy(lpRasEntry->szLocalPhoneNumber, L"");

    lpRasEntry->dwFramingProtocol = RASFP_Ppp;
    lpRasEntry->dwfOptions =
        RASEO_RemoteDefaultGateway |
        RASEO_ModemLights |
        RASEO_SecureLocalFiles |
        RASEO_RequirePAP |
        RASEO_PreviewUserPw |
        RASEO_ShowDialingProgress |
        RASEO_RequireCHAP |
        RASEO_RequireMsCHAP2;
    lpRasEntry->dwfNetProtocols = RASNP_Ip | RASNP_Ipv6;
    lpRasEntry->dwType = RASET_Broadband;
    lpRasEntry->dwEncryptionType = ET_Optional;
    lpRasEntry->dwfOptions2 =
        RASEO2_SecureFileAndPrint |
        RASEO2_SecureClientForMSNet |
        RASEO2_DontNegotiateMultilink |
        RASEO2_DontUseRasCredentials |
        RASEO2_Internet |
        RASEO2_DisableNbtOverIP |
        RASEO2_ReconnectIfDropped |
        RASEO2_IPv6RemoteDefaultGateway;

    return RasSetEntryProperties(NULL, DialName, lpRasEntry, sizeof(RASENTRY), NULL, 0);
}
DWORD createConnection(wchar_t *wsName, wchar_t *wsPassword, LPHRASCONN lphRasConn) {
    LPRASDIALEXTENSIONS lpRasDialExtensions = NULL;
    LPCTSTR             lpszPhonebook = NULL;
    LPRASDIALPARAMS     lpRasDialParams = new RASDIALPARAMS;
    DWORD               dwNotifierType = NULL;
    LPVOID              lpvNotifier = NULL;
    *lphRasConn = NULL;

    ZeroMemory(lpRasDialParams, sizeof(RASDIALPARAMS));
    lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);
    wcscpy(lpRasDialParams->szEntryName, DialName);
    wcscpy(lpRasDialParams->szPhoneNumber, L"");
    wcscpy(lpRasDialParams->szCallbackNumber, L"");
    wcscpy(lpRasDialParams->szUserName, wsName);
    wcscpy(lpRasDialParams->szPassword, wsPassword);
    wcscpy(lpRasDialParams->szDomain, L"");

    return RasDial(lpRasDialExtensions, lpszPhonebook, lpRasDialParams, dwNotifierType, lpvNotifier, lphRasConn);
}
DWORD checkConnection(LPHRASCONN lphRasConn) {
    RAS_STATS *RasStats = new RAS_STATS;
    RasStats->dwSize = sizeof(RAS_STATS);
    LPRASCONNSTATUS lprasconnstatus = new RASCONNSTATUS;
    lprasconnstatus->dwSize = sizeof(RASCONNSTATUS);
    DWORD ErrCode = ERROR_SUCCESS;
    return RasGetConnectStatus(*lphRasConn, lprasconnstatus);
}
DWORD WINAPI Thread_checkConnection(LPVOID argv)
{
    ((CMyWindowsPPPoEDialerDlg*)argv)->ErrCode = ERROR_SUCCESS;
    RAS_STATS *RasStats = new RAS_STATS;
    RasStats->dwSize = sizeof(RAS_STATS);
    while (1) {
        if (0 == ((CMyWindowsPPPoEDialerDlg*)argv)->connected) return 0;
        DWORD ErrCode = RasGetConnectionStatistics(*(LPHRASCONN)(((CMyWindowsPPPoEDialerDlg*)argv)->lphRasConn), RasStats);
        if (ERROR_SUCCESS != ErrCode) {
            ((CMyWindowsPPPoEDialerDlg*)argv)->msgAlert = true;
            ((CMyWindowsPPPoEDialerDlg*)argv)->setMessage(L"连接异常断开. Error Code: ");
            ((CMyWindowsPPPoEDialerDlg*)argv)->appendMessage(int2str(ErrCode));
            ((CMyWindowsPPPoEDialerDlg*)argv)->appendMessage(L"\n3秒后自动重连");
            ((CMyWindowsPPPoEDialerDlg*)argv)->btnConnectToggle.SetWindowTextW(L"连接");
            ((CMyWindowsPPPoEDialerDlg*)argv)->connected = 0;
            Sleep(3000);
            if (2 == ((CMyWindowsPPPoEDialerDlg*)argv)->connected) return 0;
            ((CMyWindowsPPPoEDialerDlg*)argv)->OnBnClickedConnectToggle();
            return 0;
        }
        Sleep(1000);
    }
}
DWORD WINAPI Thread_Connect(LPVOID argv)
{
    CMyWindowsPPPoEDialerDlg* dlg = (CMyWindowsPPPoEDialerDlg*)argv;
    if (1 == dlg->connected) return 0;
    dlg->ErrCode = createEntry();
    dlg->lphRasConn = new HRASCONN(NULL);
    if (ERROR_SUCCESS == dlg->ErrCode) dlg->ErrCode = createConnection(dlg->sName.AllocSysString(), dlg->sPass.AllocSysString(), dlg->lphRasConn);
    else {
        dlg->msgAlert = TRUE;
        dlg->setMessage(L"创建连接错误! Error Code: ");
        dlg->appendMessage(int2str(dlg->ErrCode));
        dlg->btnConnectToggle.SetWindowTextW(L"连接");
        dlg->btnConnectToggle.EnableWindow(TRUE);
        dlg->connected = 0;
        return 1;
    }
    if (ERROR_SUCCESS != dlg->ErrCode) {
        dlg->msgAlert = TRUE;
        dlg->setMessage(L"连接错误! Error Code: ");
        dlg->appendMessage(int2str(dlg->ErrCode));
        dlg->btnConnectToggle.SetWindowTextW(L"连接");
        dlg->btnConnectToggle.EnableWindow(TRUE);
        dlg->connected = 0;
        return 1;
    }
    dlg->msgAlert = FALSE;
    dlg->setMessage(L"连接成功!");
    dlg->connected = 1;
    dlg->btnConnectToggle.EnableWindow(TRUE);
    dlg->btnConnectToggle.SetWindowTextW(L"断开连接");
    CreateThread(NULL, 0, Thread_checkConnection, dlg, 0, NULL);
    return 0;
}
// End

// Useful Functions
int char2Hex(const char &c) {// Transform a hex charactor into number
    if ('0' < c && c < '9')return c - '0';
    if ('a' < c && c < 'f')return c - 'a' + 10;
    if ('A' < c && c < 'F')return c - 'A' + 10;
    return 0x10;
}
int char2Hex(const wchar_t &c) {// Transform a long hex charactor into number
    if (L'0' < c && c < L'9')return c - L'0';
    if (L'a' < c && c < L'f')return c - L'a' + 10;
    if (L'A' < c && c < L'F')return c - L'A' + 10;
    return 0x10;
}
char* handleString(const char *s, unsigned int len) {// Replace \\n into \n, \\t into \t, \r into \r, \\xnn into \xnn
    int j = 0;
    if (-1 == len)len = strlen(s);
    char *hs = new char[len + 1];
    for (unsigned int i = 0; i < len; i++) {
        if (L'\\' == s[i] && i < len - 1) {
            if (L'n' == s[i + 1]) {
                hs[j++] = '\n';
            }
            else if (L'r' == s[i + 1]) {
                hs[j++] = '\r';
            }
            else if (L't' == s[i + 1]) {
                hs[j++] = '\t';
            }
            else if (L'x' == s[i + 1] && i<len - 3) {
                char c = 0x00;
                c += 0x10 * char2Hex(s[i + 2]);
                c += 0x01 * char2Hex(s[i + 3]);
                hs[j++] = c;
            }
            else
                hs[j++] = s[i];
        }
        else
            hs[j++] = s[i];
    }
    hs[j] = 0;
    return hs;
}
wchar_t* handleString(const wchar_t *s, unsigned int len) {// Replace \\n into \n, \\t into \t, \r into \r, \\xnn into \xnn in wchar_t
    int j = 0;
    if (-1 == len)len = lstrlen(s);
    wchar_t *hs = new wchar_t[len + 1];
    for (unsigned int i = 0; i < len; i++) {
        if (L'\\' == s[i] && i < len - 1) {
            if (L'n' == s[i + 1]) {
                hs[j++] = '\n';
                i += 1;
            }
            else if (L'r' == s[i + 1]) {
                hs[j++] = '\r';
                i += 1;
            }
            else if (L't' == s[i + 1]) {
                hs[j++] = '\t';
                i += 1;
            }
            else if (L'x' == s[i + 1] && i<len - 3) {
                char c = 0x00;
                c += 0x10 * char2Hex(s[i + 2]);
                c += 0x01 * char2Hex(s[i + 3]);
                hs[j++] = c;
                i += 3;
            }
            else
                hs[j++] = s[i];
        }
        else
            hs[j++] = s[i];
    }
    hs[j] = 0;
    return hs;
}
CString int2str(int n) {// Transform a number into a DEC string
    CString s = L"";
    do {
        s = char('0' + n % 10) + s;
        n /= 10;
    } while (n);
    return s;
}
// End
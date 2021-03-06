#include "stdafx.h"
#pragma comment(lib,"Rasapi32.lib")

DWORD Dial(LPHRASCONN);
void log(const char* s);
void log(const wchar_t* s);
void log(const int s);

wchar_t DialName[] = L"Dora's PPPoEDialer";
wchar_t *UserName, *Password;

int main(int argc, char *argv[])
{
    unsigned int n = strlen(argv[0]);
    int i = 0;
    switch (argc) {
    case 1:case 2:
        char *fname;
        while (('\\' != argv[0][--n]) && ++i);
        fname = new char[i];
        fname[i - 1] = 0;
        for (unsigned int j = 1; j < n + i; j++) fname[j - 1] = argv[0][n + j];
        printf("Usage: %s [usename] [password]\n", fname);
        return 1;
    case 3:
        UserName = new wchar_t[strlen(argv[1])];
        for (unsigned int i = 0; i < strlen(argv[1]); i++)UserName[i] = (wchar_t)argv[1][i];
        Password = new wchar_t[strlen(argv[2])];
        for (unsigned int i = 0; i < strlen(argv[2]); i++)UserName[i] = (wchar_t)argv[2][i];
        break;
    default:
        UserName = new wchar_t[strlen(argv[1])];
        for (unsigned int i = 0; i < strlen(argv[1]); i++)UserName[i] = (wchar_t)argv[1][i];
        Password = new wchar_t[strlen(argv[2])];
        for (unsigned int i = 0; i < strlen(argv[2]); i++)UserName[i] = (wchar_t)argv[2][i];
        break;
    }

    log("Creating RAS Entry...\n");
    LPCTSTR    lpszPhonebook = NULL; // 字符串指针，表示一个电话簿(PBK)文件的完整路径和文件名. 如果参数为 NULL, 函数会使用默认电话簿文件.
    LPCTSTR    lpszEntry = DialName; // 存储入口点名称的字符串.
    LPRASENTRY lpRasEntry = new RASENTRY;
    DWORD      dwEntryInfoSize = sizeof(RASENTRY);
    LPBYTE     lpbDeviceInfo = NULL;
    DWORD      dwDeviceInfoSize = 0;

    // Init
    ZeroMemory(lpRasEntry, sizeof(RASENTRY));
    lpRasEntry->dwSize = sizeof(RASENTRY);
    wcscpy(lpRasEntry->szDeviceName, L"WAN Miniport (PPPOE)");
    wcscpy(lpRasEntry->szDeviceType, RASDT_PPPoE);      // 表示由szDeviceName引用的RAS设备类型的字符串.见下表.
                                                     /*
                                                     字符串	            含义
                                                     RASDT_Modem	        A modem accessed through a COM port.
                                                     RASDT_Isdn	        An ISDN card with corresponding NDISWAN driver installed.
                                                     RASDT_X25	        An X.25 card with corresponding NDISWAN driver installed.
                                                     RASDT_Vpn	        私有专用网络连接. 参考 VPN Connections: https://msdn.microsoft.com/en-us/library/windows/desktop/aa382345(v=vs.85).aspx
                                                     RASDT_Pad	        A Packet Assembler/Disassembler.
                                                     RASDT_Generic	    Generic.
                                                     RASDT_Serial	    Direct serial connection through a serial port.
                                                     RASDT_FrameRelay	Frame Relay.
                                                     RASDT_Atm	        Asynchronous Transfer Mode (ATM).
                                                     RASDT_Sonet	        Sonet.
                                                     RASDT_SW56	        Switched 56K Access.
                                                     RASDT_Irda	        Infrared Data Association (IrDA) compliant device.
                                                     RASDT_Parallel	    Direct parallel connection through a parallel port.
                                                     RASDT_PPPoE	        基于以太网的点对点通讯协议. 基于以太网的点对点通讯协议. 参考 PPPoE Connections: https://msdn.microsoft.com/en-us/library/windows/desktop/aa376686(v=vs.85).aspx
                                                     */
    wcscpy(lpRasEntry->szLocalPhoneNumber, L"");
    lpRasEntry->dwFramingProtocol = RASFP_Ppp;
    lpRasEntry->dwfOptions = 755302672;                           // A set of bit flags that specify connection options. 见下表
                                                                  // RASEO_UseCountryAndAreaCodes |    // 如果设置此标志, dwCountryID, dwCountryCode, 和 szAreaCode 都会在构造电话号码时使用. 如果未设置, 这些都会被忽略.
                                                                  // RASEO_SpecificIpAddr	      |    // 如果设置此标志, RAS 会尝试使用 ipaddr 声明的 IP 地址作为拨号连接的 IP 地址. 如果未设置, ipaddr 会被忽略.
                                                                  // RASEO_SpecificNameServers	  |    // 如果设置此标志, RAS 会使用 ipaddrDns, ipaddrDnsAlt, ipaddrWins, 和 ipaddrWinsAlt 来表示拨号连接服务器地址. 如果未设置, RAS 会忽略这些项.
                                                                  // RASEO_IpHeaderCompression	  |    // 如果设置此标志, RAS negotiates to use IP header compression on PPP connections. 如果未设置, IP header compression is not negotiated.
                                                                  // RASEO_RemoteDefaultGateway   |    // 如果设置此标志, the default route for IP packets is through the dial-up adapter when the connection is active. 如果此标志被清空, 默认路线不会修改.
                                                                  // RASEO_DisableLcpExtensions	  |    // 如果设置此标志, RAS disables the PPP LCP extensions defined in RFC 1570. This may be necessary to connect to certain older PPP implementations, but interferes with features such as server callback. Do not set this flag unless specifically required.
                                                                  // RASEO_TerminalAfterDial      |    // 如果设置此标志, RAS 在拨号时显示终端窗口让用户输入. 如果设置了拨号执行脚本不要设置此标志, 因为脚本会有自己的终端.此标志只在 RasDialDlg 函数执行之后生效.如果是 RasDial 函数则不会生效.
                                                                  // RASEO_ModemLights            |    // 如果设置此标志, 任务栏会显示一个状态监视器.
                                                                  // RASEO_SwCompression          |    // 如果设置此标志, software compression is negotiated on the link. 设置此标志会导致 PPP 驱动尝试与服务器传输 CCP. 此标志应该默认选中, but clearing it can reduce the negotiation period if the server does not support a compatible compression protocol.
                                                                  // RASEO_RequireEncryptedPw	  |    // 如果设置此标志, only secure password schemes can be used to authenticate the client with the server. 此选项会阻止 PPP 驱动使用 PAP plain-text 认证协议认证客户端. MD5-CHAP 和 SPAP 协议也受支持. Clear this flag for increased interoperability, and set it for increased security. 设置 RASEO_RequireEncryptedPw 也会设置 RASEO_RequireSPAP, RASEO_RequireCHAP, RASEO_RequireMsCHAP, 和 RASEO_RequireMsCHAP2. 此标签相当于在 Security 对话框中的 Require Encrypted Password 复选框.
                                                                  // RASEO_RequireMsEncryptedPw	  |    // 如果设置此标志, only the Microsoft secure password scheme, MSCHAP, can be used to authenticate the client with the server. This flag prevents the PPP driver from using the PPP plain-text authentication protocol (PAP), MD5-CHAP, or SPAP. See Also the following flags: RASEO_RequireMsCHAP, RASEO_RequireMsCHAP2, and RASEO_RequireW95MsCHAP. Setting RASEO_RequireMsEncryptedPw also sets RASEO_RequireMsCHAP and RASEO_RequireMsCHAP2.This flag corresponds to the Require Microsoft Encrypted Password check box in the Security dialog box. See also RASEO_RequireDataEncryption.
                                                                  // RASEO_RequireDataEncryption  |    // 如果设置此标志, data encryption must be negotiated successfully or the connection should be dropped. This flag is ignored unless RASEO_RequireMsEncryptedPw is also set.This flag corresponds to the Require Data Encryption check box in the Security dialog box.
                                                                  // RASEO_NetworkLogon	          |    // 如果设置此标志, RAS logs on to the network after the point-to-point connection is established.
                                                                  // RASEO_UseLogonCredentials	  |    // 如果设置此标志, RAS 拨号时使用当前登录用户的用户名, 密码, 和域名. 此标志会被忽略除非 RASEO_RequireMsEncryptedPw 被启用.此设置被 RasDial 函数默认忽略, where specifying empty strings for the szUserName and szPassword members of the RASDIALPARAMS structure gives the same result.This flag corresponds to the Use Current Username and Password check box in the Security dialog box.
                                                                  // RASEO_PromoteAlternates	  |    // 此标志位在当轮流电话号码被 dwAlternateOffset 定义时生效. 如果设置此标志, 当一个电话号码连接成功时成为主号码, 当前主号码会被移动到轮流列表中.This flag corresponds to the check box in the Alternate Numbers dialog box. The successful number is moved to the top of the list.
                                                                  // RASEO_SecureLocalFiles	      |    // 如果设置此标志, RAS 检查存在的远程文件系统和远程打印机 bindings 在连接之前. 通常, set this flag is set on phone-book entries for public networks to remind users to break connections to their private network before connecting to a public network.If either Client for Microsoft Networks or File and Print Sharing is unchecked for a given connection, calling RasGetEntryProperties returns dwfOptions with RASEO_SecureLocalFiles set. Calling RasSetEntryProperties with RASEO_SecureLocalFiles set unchecks both Client for Microsoft Networks and File and Print Sharing; resetting the bit checks both items.
                                                                  // RASEO_RequireEAP	          |    // 如果设置此标志, 认证必须支持 Extensible Authentication Protocol (EAP).
                                                                  // RASEO_RequirePAP	          |    // 如果设置此标志, 认证必须支持 Password Authentication Protocol.
                                                                  // RASEO_RequireSPAP	          |    // 如果设置此标志, 认证必须支持 Shiva's Password Authentication Protocol.
                                                                  // RASEO_Custom	              |    // 如果设置此标志, 连接使用自定义加密.
                                                                  // RASEO_PreviewPhoneNumber     |    // 如果设置此标志, the remote access dialer 显示拨号的号码.
                                                                  // RASEO_SharedPhoneNumbers     |    // 如果设置此标志, 所有在本地计算机安装的 modems 共用同一个电话号码.
                                                                  // RASEO_PreviewUserPw          |    // 如果设置此标志, the remote access dialer 在拨号之前显示用户名和密码.
                                                                  // RASEO_PreviewDomain	      |    // 如果设置此标志, the remote access dialer 在拨号之前显示域名.
                                                                  // RASEO_ShowDialingProgress	  |    // 如果设置此标志, the remote access dialer 在建立连接时显示进度.
                                                                  // RASEO_RequireCHAP	          |    // 如果设置此标志, 认证必须支持 Challenge Handshake Authentication Protocol.
                                                                  // RASEO_RequireMsCHAP	      |    // 如果设置此标志, 认证必须支持 Microsoft Challenge Handshake Authentication Protocol.
                                                                  // RASEO_RequireMsCHAP2         |    // 如果设置此标志, 认证必须支持 Microsoft Challenge Handshake Authentication Protocol version 2.
                                                                  // RASEO_RequireW95MSCHAP	      |    // 如果设置此标志, RASEO_RequireMsCHAP 必须同时被设置并且 MS-CHAP 必须发送 LanManager-hashed 密码.
                                                                  // RASEO_CustomScript	          |    // 为了使 RAS 在服务器建立连接之后执行一个自定义 DLL, 此标志必须设置.
    lpRasEntry->dwfNetProtocols = RASNP_Ip | RASNP_Ipv6;
    lpRasEntry->dwType = RASET_Broadband;
    lpRasEntry->dwEncryptionType = ET_Optional;
    // lpRasEntry->dwfOptions2=8559;

    // Create
    DWORD ErrCode = RasSetEntryProperties(lpszPhonebook, lpszEntry, lpRasEntry, dwEntryInfoSize, lpbDeviceInfo, dwDeviceInfoSize);
    if (ERROR_SUCCESS != ErrCode) {
        log("Create RAS Entry Failed! Error Code: "); log(ErrCode); printf("\n");
        _getch();
        exit(0);
    }
    log("Create RAS Entry Succeed!\n");

    // Dial
    LPHRASCONN lphRasConn = new HRASCONN;
    *lphRasConn = NULL;
    log("Connecting...\n");
    ErrCode = Dial(lphRasConn);
    log("Connection Succeed!\n");
    RAS_STATS *RasStats = new RAS_STATS;
    RasStats->dwSize = sizeof(RAS_STATS);
    while (1) {
        ErrCode = RasGetConnectionStatistics(*lphRasConn, RasStats);
        if (ERROR_SUCCESS != ErrCode) {
            log("Disconnected! This program will connect again after 5 seconds. Error Code: "); log(ErrCode); printf("\n");
            log("Reconnecting...\n");
            Sleep(3000);
            ErrCode = Dial(lphRasConn);
            log("Connection Succeed!\n");
        }
        Sleep(1000);
    }
    return 0;
}

void log(const char* s) {
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    printf("[%02d-%02d %02d:%02d:%02d.%03d] ", sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
    printf("%s", s);
}
void log(const int s) {
    printf("%d", s);
}
void log(const wchar_t* s) {
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    printf("[%02d-%02d %02d:%02d:%02d.%03d] ", sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
    printf("%ls", s);
}
DWORD Dial(LPHRASCONN lphRasConn) {
    // Vars
    DWORD ErrCode = ERROR_SUCCESS;
    do {
        LPRASDIALEXTENSIONS lpRasDialExtensions = NULL;                 // RASDIALEXTENSIONS 指针，让程序使用 RasDial 扩展属性. 如果不需要的话可以为 NULL.
        LPCTSTR             lpszPhonebook = NULL;                       // 字符串指针，表示一个电话簿(PBK)文件的完整路径和文件名. 如果参数为 NULL, 函数会使用默认电话簿文件.
        LPRASDIALPARAMS     lpRasDialParams = new RASDIALPARAMS;        // RASDIALPARAMS 指针，表示 RAS 连接的参数. 使用 RasGetEntryDialParams 函数来获取某个电话簿的 structure 拷贝.
        DWORD               dwNotifierType = NULL;                      // 表示 the nature of the lpvNotifier 参数. 如果为 NULL, dwNotifierType 会被忽略. 如果不为 NULL, dwNotifierType 值和含义如下.
                                                                        /*
                                                                        值	含义
                                                                        0   lpvNotifier 指向一个 RasDialFunc 回调函数.
                                                                        1   lpvNotifier 指向一个 RasDialFunc1 回调函数.
                                                                        2   lpvNotifier 指向一个 RasDialFunc2 回调函数.
                                                                        */
        LPVOID              lpvNotifier = NULL;                         // https://msdn.microsoft.com/en-us/library/windows/desktop/aa377004(v=vs.85).aspx
        *lphRasConn = NULL;                            // HRASCONN 指针. 在调用 RasDial 之前将此变量设置为 NULL. 如果 RasDial 成功, RAS 连接句柄会保存在此变量.

                                                       // Init
        ZeroMemory(lpRasDialParams, sizeof(RASDIALPARAMS));           // 
        lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);              // 用bytes表示此结构大小.
        wcscpy(lpRasDialParams->szEntryName, DialName);               // 存储用来建立连接的 phone-book entry 字符串. 空串 ("") 表示可以在第一个可用的modem端口简单建立 modem 连接, 并且 szPhoneNumber 不能为空.
        wcscpy(lpRasDialParams->szPhoneNumber, L"");                  // 存储电话号码的字符串. 空串 ("") 使用电话簿中的号码. 如果 szEntryName 是 "", 此变量不能为 "".
        wcscpy(lpRasDialParams->szCallbackNumber, L"");               // 存储回调电话号码的字符串. 空串 ("") 表示回调不可用. 此变量会被忽略除非用户有 "Set By Caller" RAS服务器权限.
        wcscpy(lpRasDialParams->szUserName, UserName);                // 存储用户名的字符串. 用于向远程服务器验证用户.
        wcscpy(lpRasDialParams->szPassword, Password);                // 存储密码的字符串. 用于向远程服务器验证用户.
        wcscpy(lpRasDialParams->szDomain, L"");                       // 存储认证服务器域名的字符串. 空串 ("") 表示 the domain in which the remote access server is a member. An asterisk specifies the domain stored in the phone book for the entry.

                                                                      // Execute
        DWORD ErrCode = RasDial(lpRasDialExtensions, lpszPhonebook, lpRasDialParams, dwNotifierType, lpvNotifier, lphRasConn);

        // Handle Error
        if (ErrCode != ERROR_SUCCESS) {
            log("Dial Failed! Error Code: "); log(ErrCode); printf("\n"); log("The Program will try again in 5 seconds.Press any key to exit.");
            Sleep(1000);
            if (_kbhit())exit(1);
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b4 seconds.Press any key to exit.");
            Sleep(1000);
            if (_kbhit())exit(1);
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b3 seconds.Press any key to exit.");
            Sleep(1000);
            if (_kbhit())exit(1);
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b2 seconds.Press any key to exit.");
            Sleep(1000);
            if (_kbhit())exit(1);
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b1 seconds.Press any key to exit.");
            Sleep(1000);
            if (_kbhit())exit(1);
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b0 seconds.Press any key to exit.\n");
        }
    } while (ErrCode != ERROR_SUCCESS);
    return ERROR_SUCCESS;
}
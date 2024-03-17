#pragma once
#include <map>
#include <string>

class Dict {
public:
    // 常见端口的服务
    const std::map<int, std::string> portServiceDict = {
        {1, "tcpmux"},
        {7, "echo"},
        {9, "discard"},
        {11, "systat"},
        {13, "daytime"},
        {17, "qotd"},
        {19, "chargen"},
        {20, "ftp-data"},
        {21, "ftp"},
        {22, "ssh"},
        {23, "telnet"},
        {25, "smtp"},
        {37, "time"},
        {42, "nameserver"},
        {43, "whois"},
        {49, "tacacs"},
        {53, "domain"},
        {70, "gopher"},
        {79, "finger"},
        {80, "http"},
        {103, "x400"},
        {108, "sna"},
        {109, "pop2"},
        {110, "pop3"},
        {115, "sftp"},
        {118, "sqlserv"},
        {119, "nntp"},
        {137, "netbios-ns"},
        {139, "netbios-ssn"},
        {143, "imap4"},
        {150, "sql-net"},
        {156, "sqlsrv"},
        {161, "snmp"},
        {179, "bgp"},
        {194, "irc"},
        {201, "at-rtmp"},
        {209, "qmtp"},
        {213, "ipx"},
        {220, "imap3"},
        {389, "ldap"},
        {443, "https"},
        {444, "snpp"},
        {445, "microsoft-ds"},
        {465, "smtps"},
        {513, "who"},
        {514, "syslog"},
        {515, "printer"},
        {520, "efs"},
        {521, "ripng"},
        {540, "uucp"},
        {554, "rtsp"},
        {546, "dhcpv6-client"},
        {547, "dhcpv6-server"},
        {554, "rtsp"},
        {587, "submission"},
        {631, "ipp"},
        {636, "ldaps"},
        {873, "rsync"},
        {902, "ideafarm-door"},
        {989, "ftps-data"},
        {990, "ftps"},
        {993, "imaps"},
        {995, "pop3s"},
        {2049, "nfs"},
        {3389, "ms-wbt-server"},
        {5631, "pcanywhere-data"},
        {5632, "pcanywhere-status"},
        {5900, "vnc"},
        {6000, "x11"},
        {8080, "http-proxy"},
        {8081, "blackice-icecap"},
        {9100, "jetdirect"},
        {10000, "webmin"},
        {12345, "netbus"},
        {12346, "netbus"},
        {12349, "tsirc"},
        {27374, "asp"},
        {31337, "elite"}
    };

    std::string getServiceByPort(int port) {
        if (portServiceDict.find(port) != portServiceDict.end()) {
            return portServiceDict.at(port);
        } else {
            return "unknown";
        }
    }
};

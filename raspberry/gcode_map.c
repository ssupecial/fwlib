#include "gcode_map.h"

const char* mapGCode(int type, unsigned char g_data) {
    // type과 g_data 값에 따른 G코드 매핑
    switch(type) {
        case 0:
            switch(value) {
                case 0: return "G00";
                case 1: return "G01";
                case 2: return "G02";
                case 3: return "G03";
                case 4: return "G33";
                case 5: return "G75";
                case 6: return "G77";
                case 7: return "G78";
                case 8: return "G79";
                case 10: return "G02.2";
                case 11: return "G03.2";
                case 12: return "G02.3";
                case 13: return "G03.3";
                case 14: return "G06.2";
                case 15: return "G02.4";
                case 16: return "G03.4";
                case 22: return "G35";
                case 23: return "G36";
                case 24: return "G34";
                default: return "Unknown";
            }
            break;
        case 1:
            switch (value) {
                case 0: return "G17";
                case 4: return "G19";
                case 8: return "G18";
                case 10: return "G17.1";
                default: return "Unknown";
            }
            break;
        case 2:
            switch(value) {
                case 0: return "G90";
                case 1: return "G91";
                default: return "Unknown";
            }
            break;
        case 3:
            switch(value) {
                case 0: return "G23";
                case 1: return "G22";
                default: return "Unknown";
            }
            break;
        case 4:
            switch(value) {
                case 0: return "G94";
                case 1: return "G95";
                case 2: return "G93";
                case 3: return "G93.2";
                default: return "Unknown";
            }
            break;
        case 5:
            switch (value) {
                case 0: return "G20(G70)";
                case 1: return "G21(G71)";
                default: return "Unknown";
            }
            break;
        case 6:
            switch(value) {
                case 0: return "G40";
                case 1: return "G41";
                case 2: return "G42";
                case 3: return "G41.2";
                case 4: return "G42.2";
                case 5: return "G41.3";
                case 6: return "G41.4";
                case 7: return "G42.4";
                case 8: return "G41.5";
                case 9: return "G42.5";
                case 10: return "G41.6";
                case 11: return "G42.6";
                default: return "Unknown";
            }
            break;
        case 7:
            switch(value) {
                case 0: return "G49(G49.1)";
                case 1: return "G43";
                case 2: return "G44";
                case 3: return "G43.1";
                case 4: return "G43.4";
                case 5: return "G43.5";
                case 6: return "G43.2";
                case 7: return "G43.3";
                default: return "Unknown";
            }
            break; 
        case 8:
            switch(value) {
                case 0: return "G80";
                case 1: return "G81";
                case 2: return "G82";
                case 3: return "G83";
                case 4: return "G84";
                case 5: return "G85";
                case 6: return "G86";
                case 7: return "G87";
                case 8: return "G88";
                case 9: return "G89";
                case 10: return "G73";
                case 11: return "G74";
                case 12: return "G76";
                case 13: return "G84.2";
                case 14: return "G84.3";
                case 15: return "G81.2";
                default: return "Unknown";
            }
            break;
        case 9:
            switch(value) {
                case 0: return "G98";
                case 1: return "G99";
                default: return "Unknown";
            }
            break;
        case 10:
            switch(value) {
                case 0: return "G50";
                case 1: return "G51";
                default: return "Unknown";
            }
            break;
        case 11:
            switch(value) {
                case 0: return "G67";
                case 1: return "G66";
                case 2: return "G66.1";
                default: return "Unknown";
            }
            break;
        case 12:
            switch(value) {
                case 0: return "G97";
                case 1: return "G96";
                default: return "Unknown";
            }
            break;
        case 13:
            switch(value) {
                case 0: return "G54(G54.1)";
                case 1: return "G55";
                case 2: return "G56";
                case 3: return "G57";
                case 4: return "G58";
                case 5: return "G59";
                default: return "Unknown";
            }
            break;
        case 14:
            switch(value) {
                case 0: return "G64";
                case 1: return "G61";
                case 2: return "G62";
                case 3: return "G63";
                default: return "Unknown";
            }
            break;
        case 15:
            switch(value) {
                case 0: return "G69";
                case 1: return "G68";
                case 2: return "G68.2";
                case 3: return "G68.3";
                default: return "Unknown";
            }
            break;
        case 16:
            switch(value) {
                case 0: return "G15";
                case 1: return "G16";
                default: return "Unknown";
            }
            break;
        case 17:
            switch(value) {
                case 0: return "G40.1(G150)";
                case 1: return "G41.1(G151)";
                case 2: return "G42.1(G152)";
                default: return "Unknown";
            }
            break;
        case 18:
            switch(value) {
                case 0: return "G25";
                case 1: return "G26";
                default: return "Unknown";
            }
            break;
        case 19:
            switch(value) {
                case 0: return "G160";
                case 1: return "G161";
                default: return "Unknown";
            }
            break;
        case 20:
            switch(value) {
                case 0: return "G13.1(G113)";
                case 1: return "G12.1(G112)";
                default: return "Unknown";
            }
            break;
    }
    
    return "Unknown";  // 매핑되지 않은 경우
}
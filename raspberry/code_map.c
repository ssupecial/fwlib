#include "code_map.h"

const char* map_one_shot_gcode(int type, unsigned char value) {
    switch(type) {
        case 300:
            switch(value) {
                case 0: return "G04";
                case 1: return "G10";
                case 4: return "G27";
                case 5: return "G28";
                case 6: return "G29";
                case 7: return "G30";
                case 9: return "G39";
                case 14: return "G92";
                case 16: return "G31(G31.1)";
                case 17: return "G60";
                case 18: return "G65";
                case 20: return "G05";
                case 21: return "G11";
                case 22: return "G52";
                case 24: return "G37";
                case 25: return "G07.1(G107)";
                case 26: return "G30.1";
                case 27: return "G10.6";
                case 28: return "G72.1";
                case 29: return "G72.2";
                case 30: return "G92.1";
                case 31: return "G08";
                case 100: return "G81.1";
                case 104: return "G05.1";
                case 105: return "G07";
                case 106: return "G31.8";
                case 107: return "G31.9";
                case 108: return "G12.4";
                case 109: return "G13.4";
                case 110: return "G05.4";
                case 111: return "G10.9";
                case 112: return "G31.2";
                case 113: return "G31.3";
                case 114: return "G31.4";
                case 116: return "G91.1";
                case 125: return "G37.1";
                case 126: return "G37.2";
                case 127: return "G37.3";
                default: return "Unknown";
            }
            break;
        default:
            return "Unknown";
    }
}
const char* map_modal_gcode(int type, unsigned char value) {
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
const char* map_other_code(int type) {
    const char* address;
    switch (type) {
        case 100:
            address = "B";
            break;
        case 101:
            address = "D";
            break;
        case 102:
            address = "-";
            break;
        case 103:
            address = "F";
            break;
        case 104:
            address = "H[M]";
            break;
        case 105:
            address = "L";
            break;
        case 106:
            address = "M";
            break;
        case 107:
            address = "S";
            break;
        case 108:
            address = "T";
            break;
        case 109:
            address = "R[M]";
            break;
        case 110:
            address = "P[M]";
            break;
        case 111:
            address = "Q[M]";
            break;
        case 112:
            address = "A";
            break;
        case 113:
            address = "C";
            break;
        case 114:
            address = "I";
            break;
        case 115:
            address = "J";
            break;
        case 116:
            address = "K";
            break;
        case 117:
            address = "N";
            break;
        case 118:
            address = "O";
            break;
        case 119:
            address = "U";
            break;
        case 120:
            address = "V";
            break;
        case 121:
            address = "W";
            break;
        case 122:
            address = "X";
            break;
        case 123:
            address = "Y";
            break;
        case 124:
            address = "Z";
            break;
        case 125:
            address = "M";
            break;
        case 126:
            address = "M";
            break;
        default:
            address = "Unknown";
            break;
    }

    return address;
}


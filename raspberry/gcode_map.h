#ifndef GCODE_MAPPING_H
#define GCODE_MAPPING_H

#ifdef __cplusplus
extern "C" {
#endif

// G코드 매핑 함수 선언
// 입력: type - G코드 타입 번호
//       g_data - FOCAS에서 받은 g_data 값
// 반환: 매핑된 G코드 문자열 (예: "G00", "G01" 등)
extern const char* mapGcode(int type, unsigned char g_data);

#ifdef __cplusplus
}
#endif

#endif // GCODE_MAPPING_H
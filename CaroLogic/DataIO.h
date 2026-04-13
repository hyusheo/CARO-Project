#pragma once

// Khai báo các hàm đọc/ghi file nhị phân (thuần thủ tục)
// Trả về true nếu thành công, false nếu thất bại (không tìm thấy file, lỗi quyền ghi...)
bool SaveBinary(const char* filepath, float timeLeft, int isPlayerTurn);
bool LoadBinary(const char* filepath, float* outTimeLeft, int* outIsPlayerTurn);
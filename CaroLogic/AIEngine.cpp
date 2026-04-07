#include "AIEngine.h"
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "json.hpp"
#include <string>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

// Bạn cần lấy API Key miễn phí từ Google AI Studio
const std::string GEMINI_API_KEY = "AIzaSyD25PZHBu7n_pE7exmdhY227sfVSy4couk";


std::string CallGeminiAPI(const std::string& prompt, bool isLevel3) {
    // Chọn Model: Mức 3 dùng Pro (thông minh hơn), Mức 2 dùng Flash (nhanh hơn)
    std::string model = isLevel3 ? "gemini-1.5-pro" : "gemini-1.5-flash";

    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/" + model + ":generateContent?key=" + GEMINI_API_KEY;

    // Xóa các ký tự có thể làm hỏng chuỗi JSON (như " hoặc \n) trong prompt
    std::string safePrompt = prompt;
    size_t pos = 0;
    while ((pos = safePrompt.find("\"", pos)) != std::string::npos) {
        safePrompt.replace(pos, 1, "\\\"");
        pos += 2;
    }
    pos = 0;
    while ((pos = safePrompt.find("\n", pos)) != std::string::npos) {
        safePrompt.replace(pos, 1, "\\n");
        pos += 2;
    }

    // Tạo file JSON chứa Payload để gửi đi
    std::string requestJson = R"({
        "contents": [{
            "parts": [{"text": ")" + safePrompt + R"("}]
        }],
        "generationConfig": {
            "temperature": 0.2
        }
    })";

    // Lưu payload ra file tạm (vì truyền qua command line dễ bị lỗi giới hạn độ dài)
    std::ofstream reqFile("request.json");
    reqFile << requestJson;
    reqFile.close();

    // Dùng cURL của Windows để gọi API và lưu kết quả vào file response.json
    std::string command = "curl -s -X POST -H \"Content-Type: application/json\" -d @request.json " + url + " > response.json";
    // In câu lệnh ra màn hình Console để kiểm tra xem nó có ghép đúng không
    std::cout << "DEBUG CMD: " << command << std::endl;

    // Dừng màn hình lại 1 chút để bạn kịp đọc lỗi của cURL (nếu có)
    std::string debugCommand = command + " & pause";
    system(debugCommand.c_str());
    system(command.c_str());

    // Đọc kết quả trả về từ file
    std::ifstream resFile("response.json");
    std::string responseString((std::istreambuf_iterator<char>(resFile)), std::istreambuf_iterator<char>());
    resFile.close();

    // Phân tích JSON để lấy câu trả lời (Tọa độ)
    try {
        json responseJson = json::parse(responseString);
        if (responseJson.contains("candidates") && responseJson["candidates"].size() > 0) {
            std::string textOutput = responseJson["candidates"][0]["content"]["parts"][0]["text"];
            return textOutput;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Loi Parse JSON: " << e.what() << std::endl;
    }

    return ""; // Trả về rỗng nếu lỗi để kích hoạt Fallback
}
// Mảng điểm Tấn công (Khi AI tự xây chuỗi của mình)
const long attackScores[7] = { 0, 9, 54, 162, 1458, 13112, 118008 };

// Mảng điểm Phòng ngự (Khi AI chặn chuỗi của người chơi)
const long defenseScores[7] = { 0, 3, 27, 99, 729, 6561, 59049 };

// ==========================================
// 1. HÀM TÍNH ĐIỂM CHO 1 HƯỚNG
// ==========================================
long ScoreDirection(int boardCopy[30][30], int boardSize, int x, int y, int dx, int dy, int playerMode, int aiMode) {
    long attackScore = 0;
    long defenseScore = 0;
    int allyCount = 0;
    int blockCount = 0;

    // Quét Tới
    for (int i = 1; i <= 5; i++) {
        int nx = x + i * dx;
        int ny = y + i * dy;
        if (nx < 0 || nx >= boardSize || ny < 0 || ny >= boardSize) { blockCount++; break; }
        if (boardCopy[nx][ny] == aiMode) { allyCount++; }
        else if (boardCopy[nx][ny] == playerMode) { blockCount++; break; }
        else { break; }
    }

    // Quét Lui
    for (int i = 1; i <= 5; i++) {
        int nx = x - i * dx;
        int ny = y - i * dy;
        if (nx < 0 || nx >= boardSize || ny < 0 || ny >= boardSize) { blockCount++; break; }
        if (boardCopy[nx][ny] == aiMode) { allyCount++; }
        else if (boardCopy[nx][ny] == playerMode) { blockCount++; break; }
        else { break; }
    }

    // Đánh giá Tấn công
    if (blockCount == 2) attackScore = 0;
    else {
        if (allyCount >= 5) allyCount = 5;
        attackScore = attackScores[allyCount];
        if (blockCount == 0 && allyCount > 0) attackScore *= 2;
    }

    // Tính Phòng Ngự
    int enemyCount = 0;
    blockCount = 0;

    // Quét Tới (Phòng ngự)
    for (int i = 1; i <= 5; i++) {
        int nx = x + i * dx;
        int ny = y + i * dy;
        if (nx < 0 || nx >= boardSize || ny < 0 || ny >= boardSize) { blockCount++; break; }
        if (boardCopy[nx][ny] == playerMode) { enemyCount++; }
        else if (boardCopy[nx][ny] == aiMode) { blockCount++; break; }
        else { break; }
    }

    // Quét Lui (Phòng ngự)
    for (int i = 1; i <= 5; i++) {
        int nx = x - i * dx;
        int ny = y - i * dy;
        if (nx < 0 || nx >= boardSize || ny < 0 || ny >= boardSize) { blockCount++; break; }
        if (boardCopy[nx][ny] == playerMode) { enemyCount++; }
        else if (boardCopy[nx][ny] == aiMode) { blockCount++; break; }
        else { break; }
    }

    // Đánh giá Phòng ngự
    if (blockCount == 2) defenseScore = 0;
    else {
        if (enemyCount >= 5) enemyCount = 5;
        defenseScore = defenseScores[enemyCount];
        if (blockCount == 0 && enemyCount > 0) defenseScore *= 2;
    }

    return attackScore + defenseScore;
}

// ==========================================
// 2. HÀM TÍNH TỔNG ĐIỂM CHO 1 Ô TRỐNG
// ==========================================
long CalculateCellScore(int boardCopy[30][30], int boardSize, int x, int y, int playerMode, int aiMode) {
    long totalScore = 0;
    totalScore += ScoreDirection(boardCopy, boardSize, x, y, 1, 0, playerMode, aiMode);
    totalScore += ScoreDirection(boardCopy, boardSize, x, y, 0, 1, playerMode, aiMode);
    totalScore += ScoreDirection(boardCopy, boardSize, x, y, 1, 1, playerMode, aiMode);
    totalScore += ScoreDirection(boardCopy, boardSize, x, y, 1, -1, playerMode, aiMode);
    return totalScore;
}

// ==========================================
// 4. AI CẤP ĐỘ TRUNG BÌNH (Heuristic)
// ==========================================
void MoveMedium(int boardCopy[30][30], int boardSize, int playerMode, int aiMode, int* outX, int* outY) {
    long maxScore = -1;
    *outX = -1;
    *outY = -1;

    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (boardCopy[i][j] == 0) {
                long score = CalculateCellScore(boardCopy, boardSize, i, j, playerMode, aiMode);
                if (score > maxScore) {
                    maxScore = score;
                    *outX = i;
                    *outY = j;
                }
            }
        }
    }
}

// Hàm dịch bàn cờ thành Text (Prompt)
std::string GenerateCaroPrompt(int boardCopy[30][30], int boardSize) {
    std::string prompt = "Bạn là đại kiện tướng cờ Caro (Gomoku). Bàn cờ kích thước " + std::to_string(boardSize) + "x" + std::to_string(boardSize) + ". ";
    prompt += "Nhiệm vụ của bạn là tìm ra nước đi tốt nhất tiếp theo để thắng hoặc chặn đối thủ.\n";
    prompt += "Tuyệt đối CHỈ trả về tọa độ X và Y cách nhau bởi dấu phẩy (VD: 15,14). Không giải thích gì thêm.\n\n";
    prompt += "Danh sách các nước cờ hiện tại trên bàn:\n";

    int pieceCount = 0;
    for (int x = 0; x < boardSize; ++x) {
        for (int y = 0; y < boardSize; ++y) {
            if (boardCopy[x][y] == 1) { // 1 là Người chơi
                prompt += "- Người chơi (X) tại: " + std::to_string(x) + "," + std::to_string(y) + "\n";
                pieceCount++;
            }
            else if (boardCopy[x][y] == 2) { // 2 là AI
                prompt += "- AI (O) tại: " + std::to_string(x) + "," + std::to_string(y) + "\n";
                pieceCount++;
            }
        }
    }

    if (pieceCount == 0) {
        prompt += "(Bàn cờ đang trống, bạn được đi nước đầu tiên. Hãy chọn ô ở ngay giữa bàn cờ).\n";
    }

    return prompt;
}

void MoveAPI(int boardCopy[30][30], int boardSize, int level, int* outX, int* outY) {
    std::string prompt = GenerateCaroPrompt(boardCopy, boardSize);

    // Nếu Level 3, thêm chỉ thị để AI nghĩ kỹ hơn (Mẹo Prompt Engineering)
    if (level == 3) {
        prompt += "\nHãy phân tích thật cẩn thận để tìm nước đi chí mạng hoặc chặn thế cờ bí của đối thủ trước khi xuất ra tọa độ.";
    }

    // Xác định có phải Level 3 không để gọi Model phù hợp
    bool isLevel3 = (level == 3);
    std::string apiResponse = CallGeminiAPI(prompt, isLevel3);

    // Tách chuỗi...
    size_t commaPos = apiResponse.find(',');
    if (commaPos != std::string::npos) {
        try {
            int parsedX = std::stoi(apiResponse.substr(0, commaPos));
            int parsedY = std::stoi(apiResponse.substr(commaPos + 1));

            if (parsedX >= 0 && parsedX < boardSize &&
                parsedY >= 0 && parsedY < boardSize &&
                boardCopy[parsedX][parsedY] == 0)
            {
                *outX = parsedX;
                *outY = parsedY;
                return;
            }
        }
        catch (...) {}
    }

    // Phương án dự phòng (Fallback) gọi Medium
    MoveMedium(boardCopy, boardSize, 1, 2, outX, outY);
}

// ==========================================
// 5. HÀM TỔNG
// ==========================================
void CalculateBestMove(int boardCopy[30][30], int boardSize, int level, int* outX, int* outY) {
    int playerMode = 1;
    int aiMode = 2;

    if (level == 0) {
        MoveMedium(boardCopy, boardSize, playerMode, aiMode, outX, outY);
    }
    else if (level == 1) {
        MoveMedium(boardCopy, boardSize, playerMode, aiMode, outX, outY);
    }
    else if (level == 2 || level == 3) {
        MoveAPI(boardCopy, boardSize, level, outX, outY);
    }
}
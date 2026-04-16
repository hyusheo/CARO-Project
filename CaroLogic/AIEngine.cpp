
#include "AIEngine.h"
#include <vector>
#include <algorithm>
#include <climits>
#include <iostream>

// Mảng điểm Tấn công (Khi AI tự xây chuỗi của mình)
const long attackScores[7] = { 0, 9, 54, 162, 1458, 13112, 118008 };
// Mảng điểm Phòng ngự (Khi AI chặn chuỗi của người chơi)
const long defenseScores[7] = { 0, 3, 27, 99, 729, 6561, 59049 };

// ==========================================
// 1. HÀM TÍNH ĐIỂM HEURISTIC 
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

    for (int i = 1; i <= 5; i++) {
        int nx = x + i * dx;
        int ny = y + i * dy;
        if (nx < 0 || nx >= boardSize || ny < 0 || ny >= boardSize) { blockCount++; break; }
        if (boardCopy[nx][ny] == playerMode) { enemyCount++; }
        else if (boardCopy[nx][ny] == aiMode) { blockCount++; break; }
        else { break; }
    }

    for (int i = 1; i <= 5; i++) {
        int nx = x - i * dx;
        int ny = y - i * dy;
        if (nx < 0 || nx >= boardSize || ny < 0 || ny >= boardSize) { blockCount++; break; }
        if (boardCopy[nx][ny] == playerMode) { enemyCount++; }
        else if (boardCopy[nx][ny] == aiMode) { blockCount++; break; }
        else { break; }
    }

    if (blockCount == 2) defenseScore = 0;
    else {
        if (enemyCount >= 5) enemyCount = 5;
        defenseScore = defenseScores[enemyCount];
        if (blockCount == 0 && enemyCount > 0) defenseScore *= 2;
    }

    return attackScore + defenseScore;
}

long CalculateCellScore(int boardCopy[30][30], int boardSize, int x, int y, int playerMode, int aiMode) {
    long totalScore = 0;
    totalScore += ScoreDirection(boardCopy, boardSize, x, y, 1, 0, playerMode, aiMode);
    totalScore += ScoreDirection(boardCopy, boardSize, x, y, 0, 1, playerMode, aiMode);
    totalScore += ScoreDirection(boardCopy, boardSize, x, y, 1, 1, playerMode, aiMode);
    totalScore += ScoreDirection(boardCopy, boardSize, x, y, 1, -1, playerMode, aiMode);
    return totalScore;
}


// ==========================================
// 2. KỸ THUẬT MOVE ORDERING CHO MINIMAX
// ==========================================
struct MoveInfo {
    int x, y;
    long heuristicScore;
};

// Hàm sắp xếp điểm giảm dần để Alpha-Beta tỉa cành nhanh nhất
bool CompareMoves(const MoveInfo& a, const MoveInfo& b) {
    return a.heuristicScore > b.heuristicScore;
}

// Lọc các nước đi tiềm năng (Chỉ lấy ô trống quanh các quân cờ đã đánh bán kính 2 ô)
std::vector<MoveInfo> GetCandidateMoves(int boardCopy[30][30], int boardSize, int playerMode, int aiMode) {
    std::vector<MoveInfo> candidates;
    int maxCandidates = 15; // Chỉ xét top 15 nước đi nguy hiểm nhất để chống giật lag

    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (boardCopy[i][j] == 0) {
                // Kiểm tra xem xung quanh bán kính 2 ô có quân cờ nào không
                bool hasNeighbor = false;
                for (int di = -2; di <= 2 && !hasNeighbor; di++) {
                    for (int dj = -2; dj <= 2 && !hasNeighbor; dj++) {
                        if (di == 0 && dj == 0) continue;
                        int ni = i + di;
                        int nj = j + dj;
                        if (ni >= 0 && ni < boardSize && nj >= 0 && nj < boardSize) {
                            if (boardCopy[ni][nj] != 0) {
                                hasNeighbor = true;
                            }
                        }
                    }
                }

                // Nếu có hàng xóm, tính điểm Heuristic và đưa vào danh sách
                if (hasNeighbor) {
                    long score = CalculateCellScore(boardCopy, boardSize, i, j, playerMode, aiMode);
                    candidates.push_back({ i, j, score });
                }
            }
        }
    }

    // Nếu bàn cờ trống trơn, đi ngay giữa bàn
    if (candidates.empty()) {
        candidates.push_back({ boardSize / 2, boardSize / 2, 0 });
        return candidates;
    }

    // Sắp xếp các nước đi có điểm cao nhất lên đầu
    std::sort(candidates.begin(), candidates.end(), CompareMoves);

    // Cắt bớt danh sách chỉ giữ lại Top nước đi tốt nhất
    if (candidates.size() > maxCandidates) {
        candidates.resize(maxCandidates);
    }
    return candidates;
}


// ==========================================
// 3. THUẬT TOÁN ĐỆ QUY MINIMAX ALPHA-BETA
// ==========================================
long Minimax(int boardCopy[30][30], int boardSize, int depth, long alpha, long beta, bool isMaximizing, int playerMode, int aiMode) {
    std::vector<MoveInfo> candidates = GetCandidateMoves(boardCopy, boardSize, playerMode, aiMode);

    // Điều kiện dừng: Chạm đáy độ sâu hoặc hết nước đi
    if (depth == 0 || candidates.empty()) {
        // Đánh giá trạng thái tại điểm dừng bằng nước cờ tốt nhất hiện có
        return candidates.empty() ? 0 : candidates[0].heuristicScore;
    }

    if (isMaximizing) {
        long maxEval = -INT_MAX;
        for (const auto& move : candidates) {
            boardCopy[move.x][move.y] = aiMode; // Thử đánh AI

            // Nếu đánh nước này mà điểm >= 100000 (Thắng chắc), return luôn không cần duyệt sâu thêm
            if (move.heuristicScore >= attackScores[5]) {
                boardCopy[move.x][move.y] = 0;
                return move.heuristicScore + depth; // Ưu tiên thắng nhanh nhất
            }

            long eval = Minimax(boardCopy, boardSize, depth - 1, alpha, beta, false, playerMode, aiMode);
            boardCopy[move.x][move.y] = 0; // Trả lại bàn cờ

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break; // Tỉa cành Beta (Cắt bỏ nhánh không cần thiết)
        }
        return maxEval;
    }
    else {
        long minEval = INT_MAX;
        for (const auto& move : candidates) {
            boardCopy[move.x][move.y] = playerMode; // Giả sử Người chơi đánh để chặn

            long eval = Minimax(boardCopy, boardSize, depth - 1, alpha, beta, true, playerMode, aiMode);
            boardCopy[move.x][move.y] = 0; // Trả lại bàn cờ

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break; // Tỉa cành Alpha
        }
        return minEval;
    }
}


// ==========================================
// 4. CÁC HÀM XỬ LÝ THEO CẤP ĐỘ
// ==========================================

// CẤP ĐỘ 1: DỄ (Chỉ tính Heuristic 1 bước)
void MoveEasy(int boardCopy[30][30], int boardSize, int playerMode, int aiMode, int* outX, int* outY) {
    long maxScore = -1;
    *outX = boardSize / 2;
    *outY = boardSize / 2;

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

// CẤP ĐỘ 2 & 3: TRUNG BÌNH VÀ KHÓ (Dùng Minimax)
void MoveAdvanced(int boardCopy[30][30], int boardSize, int playerMode, int aiMode, int targetDepth, int* outX, int* outY) {
    std::vector<MoveInfo> candidates = GetCandidateMoves(boardCopy, boardSize, playerMode, aiMode);

    if (candidates.empty()) return;

    long bestScore = -INT_MAX;
    *outX = candidates[0].x;
    *outY = candidates[0].y;

    for (const auto& move : candidates) {
        boardCopy[move.x][move.y] = aiMode; // AI đánh thử ô này

        // Nếu nước này đem lại chiến thắng trực tiếp, chọn luôn không chần chừ
        if (move.heuristicScore >= attackScores[5]) {
            *outX = move.x; *outY = move.y;
            boardCopy[move.x][move.y] = 0;
            break;
        }

        // Đẩy xuống cây Minimax để đối thủ (Player) đánh thử
        long score = Minimax(boardCopy, boardSize, targetDepth - 1, -INT_MAX, INT_MAX, false, playerMode, aiMode);

        boardCopy[move.x][move.y] = 0; // Xóa nước thử

        if (score > bestScore) {
            bestScore = score;
            *outX = move.x;
            *outY = move.y;
        }
    }
}


// ==========================================
// 5. HÀM TỔNG ĐIỀU HƯỚNG
// ==========================================
void CalculateBestMove(int boardCopy[30][30], int boardSize, int level, int* outX, int* outY) {
    int playerMode = 1;
    int aiMode = 2;

    if (level == 1) { // Mức Dễ
        MoveEasy(boardCopy, boardSize, playerMode, aiMode, outX, outY);
    }
    else if (level == 2) { // Mức Trung bình (Nhìn trước 2 bước)
        MoveAdvanced(boardCopy, boardSize, playerMode, aiMode, 2, outX, outY);
    }
    else if (level == 3) { // Mức Khó (Nhìn trước 4 bước)
        MoveAdvanced(boardCopy, boardSize, playerMode, aiMode, 4, outX, outY);
    }
    else {
        // Fallback mặc định
        MoveEasy(boardCopy, boardSize, playerMode, aiMode, outX, outY);
    }
}


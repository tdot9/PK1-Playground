#include <array>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

using ChessBoard = std::array<std::array<char, 8>, 8>;

std::stack<std::tuple<ChessBoard, bool>> previousBoards;

ChessBoard SchachBrett = {{
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
}};

bool isValidPromotionPiece(char piece) {
  return (piece == 'Q' || piece == 'N' || piece == 'R' || piece == 'B' || piece == 'q' || piece == 'n' ||
          piece == 'r' || piece == 'b');
}

bool isValidSquare(int rank, int file) { return (rank >= 1 && rank <= 8 && file >= 1 && file <= 8); }

inline bool whitesTurn = true;

bool isValidPiece(char piece) {
  return whitesTurn ? (piece == 'P' || piece == 'K' || piece == 'N' || piece == 'R' || piece == 'Q' || piece == 'B')
                    : (piece == 'p' || piece == 'k' || piece == 'n' || piece == 'r' || piece == 'q' || piece == 'b');
}

inline bool isValidMove2(int fromRank, int fromFile, char piece) {
  return SchachBrett[fromRank][fromFile] == piece && isValidPiece(piece);
}

inline bool isCaptureMove(int toRank, int toFile) { return SchachBrett[toRank][toFile] != ' '; }

inline bool isSameColor(char piece1, char piece2) {
  return (piece1 >= 'a' && piece1 <= 'z' && piece2 >= 'a' && piece2 <= 'z') ||
         (piece1 >= 'A' && piece1 <= 'Z' && piece2 >= 'A' && piece2 <= 'Z');
}

inline void switchTurn() { whitesTurn = !whitesTurn; }

void promotePawn(int toRank, int toFile, char promotionPiece) {
  if (isValidSquare(toRank, toFile) && (whitesTurn ? toRank == 0 : toRank == 7)) {
    SchachBrett[toRank][toFile] = promotionPiece;
  }
}

void convertInput(const std::string& input, int& fromRank, int& fromFile, int& toRank, int& toFile) {
  fromFile = input[1] - 'a';
  fromRank = 8 - (input[2] - '0');

  char piece = input[0];

  if (!isValidPiece(piece)) {
    fromRank = fromFile = toRank = toFile = -1;
    return;
  }

  if (input[3] == 'x') {
    toFile = input[4] - 'a';
    toRank = 8 - (input[5] - '0');
  } else {
    toFile = input[3] - 'a';
    toRank = 8 - (input[4] - '0');
  }

  int rankDiff = std::abs(toRank - fromRank);
  int fileDiff = std::abs(toFile - fromFile);

  if ((piece == 'B' || piece == 'b') && rankDiff != fileDiff) {
    fromRank = fromFile = toRank = toFile = -1;
  }

  std::string promotionString = input.substr(input.find('=') + 1);
  char promotionPiece = promotionString.empty() ? ' ' : promotionString[0];
}

class ChessPiece {
 public:
  ChessPiece(bool isWhite, int initialRank, int initialFile)
      : isWhite(isWhite), currentRank(initialRank), currentFile(initialFile) {}

  virtual bool isValidMove(int toRank, int toFile, const ChessBoard& board) const = 0;

  bool isOpponentPiece(char piece) const { return (isWhite && islower(piece)) || (!isWhite && isupper(piece)); }

  int getRank() const { return currentRank; }
  int getFile() const { return currentFile; }
  int getPieceType() const { return currentRank + currentFile; }

  bool isWhite;
  int currentRank;
  int currentFile;
};

class King : public ChessPiece {
 public:
  King(bool isWhite, int initialRank, int initialFile) : ChessPiece(isWhite, initialRank, initialFile) {}

  bool isValidMove(int toRank, int toFile, const ChessBoard& board) const override;
};

class Pawn : public ChessPiece {
 public:
  char promotionPieceType = ' ';
  char getPromotionPieceType() const { return promotionPieceType; }
  void setPromotionPieceType(char promotionPiece) { promotionPieceType = promotionPiece; }
  Pawn(bool isWhite, int initialRank, int initialFile)
      : ChessPiece(isWhite, initialRank, initialFile) /*, promotionPieceType(promotionPieceType)*/ {}
  bool isValidMove(int toRank, int toFile, const ChessBoard& board) const override;

  int getDirection() const { return isWhite ? -1 : 1; }

  bool isOpponentPiece(char piece) const { return (isWhite && islower(piece)) || (!isWhite && isupper(piece)); }

  bool isPawnCaptureMove(int toRank, int toFile, const ChessBoard& board) const;

  bool isValidForwardMove(int rankDiff, const ChessBoard& board) const;

  bool isValidDoubleMove(int rankDiff, const ChessBoard& board) const;
  void handlePromotion(int toRank, int toFile, const ChessBoard& board) const {
    if ((isWhite && toRank == 0) || (!isWhite && toRank == 7)) {
      char promotionPiece = getPromotionPieceType();
      if (promotionPiece != ' ') {
        SchachBrett[toRank][toFile] = promotionPiece;
      }
    }
  }
};

class Rook : public ChessPiece {
 public:
  Rook(bool isWhite, int initialRank, int initialFile) : ChessPiece(isWhite, initialRank, initialFile) {}

  bool isValidMove(int toRank, int toFile, const ChessBoard& board) const override;

  bool isPathClear(int toRank, int toFile, const ChessBoard& board) const;
};

class Bishop : public ChessPiece {
 public:
  Bishop(bool isWhite, int initialRank, int initialFile) : ChessPiece(isWhite, initialRank, initialFile) {}

  bool isValidMove(int toRank, int toFile, const ChessBoard& board) const override;
};

class Knight : public ChessPiece {
 public:
  Knight(bool isWhite, int initialRank, int initialFile) : ChessPiece(isWhite, initialRank, initialFile) {}
  bool isValidMove(int toRank, int toFile, [[maybe_unused]] const ChessBoard& board) const override;
};

class Queen : public ChessPiece {
 public:
  Queen(bool isWhite, int initialRank, int initialFile) : ChessPiece(isWhite, initialRank, initialFile) {}

  bool isValidMove(int toRank, int toFile, const ChessBoard& board) const override;
};

bool King::isValidMove(int toRank, int toFile, const ChessBoard& board) const {
  if (toRank < 0 || toRank >= 8 || toFile < 0 || toFile >= 8) {
    return false;
  }

  int rankDiff = std::abs(toRank - currentRank);
  int fileDiff = std::abs(toFile - currentFile);

  if ((rankDiff == 1 && fileDiff <= 1) || (rankDiff <= 1 && fileDiff == 1)) {
    return board[toRank][toFile] == ' ' || isOpponentPiece(board[toRank][toFile]);
  }

  return false;
}

bool Pawn::isValidForwardMove(int rankDiff, const ChessBoard& board) const {
  if (isWhite && rankDiff == -1 && board[currentRank - 1][currentFile] == ' ') {
    return true;
  } else if (!isWhite && rankDiff == 1 && board[currentRank + 1][currentFile] == ' ') {
    return true;
  }
  return false;
}

bool Pawn::isValidDoubleMove(int rankDiff, const ChessBoard& board) const {
  if ((isWhite && currentRank == 6 && std::abs(rankDiff) == 2 &&
       board[currentRank + rankDiff / 2][currentFile] == ' ' && board[currentRank + rankDiff][currentFile] == ' ') ||
      (!isWhite && currentRank == 1 && std::abs(rankDiff) == 2 &&
       board[currentRank + rankDiff / 2][currentFile] == ' ' && board[currentRank + rankDiff][currentFile] == ' ')) {
    return true;
  }
  return false;
}

bool Pawn::isValidMove(int toRank, int toFile, const ChessBoard& board) const {
  if (toRank < 0 || toRank >= 8 || toFile < 0 || toFile >= 8) {
    return false;
  }

  int rankDiff = toRank - currentRank;
  int fileDiff = std::abs(toFile - currentFile);

  char targetPiece = board[toRank][toFile];

  if (fileDiff == 1 && rankDiff == getDirection()) {
    if (targetPiece != ' ' && isOpponentPiece(targetPiece)) {
      return true;
    }
  }

  if (fileDiff == 0) {
    if (isValidForwardMove(rankDiff, board) && toFile == currentFile) {
      return true;
    }
    if (isValidDoubleMove(rankDiff, board) && toFile == currentFile) {
      return true;
    }
  }
  return false;
}

/*bool Pawn::isPawnCaptureMove(int toRank, int toFile, const ChessBoard& board) const {
  if (!isValidSquare(toRank, toFile)) {
    return false;
  }

  char targetPiece = board[toRank][toFile];

  if (targetPiece != ' ' && isOpponentPiece(targetPiece)) {
    int rankDiff = abs(toRank - getRank());
    int fileDiff = abs(toFile - getFile());

    if (rankDiff == 1 && fileDiff == 1) {
      return true;
    }
  }

  return false;
}*/

bool Rook::isValidMove(int toRank, int toFile, const ChessBoard& board) const {
  if (toRank < 0 || toRank >= 8 || toFile < 0 || toFile >= 8) {
    return false;
  }

  int rankDiff = std::abs(toRank - currentRank);
  int fileDiff = std::abs(toFile - currentFile);

  if ((rankDiff > 0 && fileDiff == 0) || (rankDiff == 0 && fileDiff > 0)) {
    if (isPathClear(toRank, toFile, board)) {
      return board[toRank][toFile] == ' ' || isOpponentPiece(board[toRank][toFile]);
    }
  }

  return false;
}

bool Rook::isPathClear(int toRank, int toFile, const ChessBoard& board) const {
  int rankDiff = toRank - currentRank;
  int fileDiff = toFile - currentFile;

  if (rankDiff != 0) {
    int rankStep = (rankDiff > 0) ? 1 : -1;
    for (int rank = currentRank + rankStep; rank != toRank; rank += rankStep) {
      if (board[rank][currentFile] != ' ') {
        // std::cout << "Path not clear at: " << rank << ", " << currentFile << std::endl;
        return false;
      }
    }
  } else if (fileDiff != 0) {
    int fileStep = (fileDiff > 0) ? 1 : -1;
    for (int file = currentFile + fileStep; file != toFile; file += fileStep) {
      if (board[currentRank][file] != ' ') {
        // std::cout << "Path not clear at: " << file << ", " << currentRank << std::endl;
        return false;
      }
    }
  }

  return true;
}

bool Bishop::isValidMove(int toRank, int toFile, const ChessBoard& board) const {
  if (toRank < 0 || toRank >= 8 || toFile < 0 || toFile >= 8) {
    return false;
  }

  int rankDiff = toRank - currentRank;
  int fileDiff = toFile - currentFile;

  if (std::abs(rankDiff) != std::abs(fileDiff)) {
    return false;
  }

  int rankStep = (rankDiff > 0) ? 1 : -1;
  int fileStep = (fileDiff > 0) ? 1 : -1;

  for (int rank = currentRank + rankStep, file = currentFile + fileStep; rank != toRank && file != toFile;
       rank += rankStep, file += fileStep) {
    if (board[rank][file] != ' ') {
      return false;
    }
  }

  return true;
}

bool Knight::isValidMove(int toRank, int toFile, [[maybe_unused]] const ChessBoard& board) const {
  if (toRank < 0 || toRank >= 8 || toFile < 0 || toFile >= 8) {
    return false;
  }
  int rankDiff = std::abs(toRank - currentRank);
  int fileDiff = std::abs(toFile - currentFile);

  return (rankDiff == 2 && fileDiff == 1) || (rankDiff == 1 && fileDiff == 2);
}

bool Queen::isValidMove(int toRank, int toFile, const ChessBoard& board) const {
  if (toRank < 0 || toRank >= 8 || toFile < 0 || toFile >= 8) {
    return false;
  }

  int rankDiff = std::abs(toRank - currentRank);
  int fileDiff = std::abs(toFile - currentFile);

  if ((rankDiff == 0 || fileDiff == 0 || rankDiff == fileDiff) &&
      (board[toRank][toFile] == ' ' || (isWhite ? islower(board[toRank][toFile]) : isupper(board[toRank][toFile])))) {
    int rankStep = (toRank > currentRank) ? 1 : (toRank < currentRank) ? -1 : 0;
    int fileStep = (toFile > currentFile) ? 1 : (toFile < currentFile) ? -1 : 0;

    for (int rank = currentRank + rankStep, file = currentFile + fileStep; rank != toRank || file != toFile;
         rank += rankStep, file += fileStep) {
      if (board[rank][file] != ' ') {
        return false;
      }
    }

    return true;
  }

  return false;
}

bool isValidMove(const ChessPiece& piece, int toRank, int toFile, const ChessBoard& board) {
  return piece.isValidMove(toRank, toFile, board);
}

bool InCheck(const ChessPiece& king, const ChessBoard& board) {
  int kingRank = king.getRank();
  int kingFile = king.getFile();

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      char piece = board[rank][file];

      if (king.isOpponentPiece(piece) && isValidPiece(piece)) {
        ChessPiece* opponentPiece = nullptr;

        switch (piece) {
          case 'K':
            opponentPiece = new King(true, rank, file);
            break;
          case 'Q':
            opponentPiece = new Queen(true, rank, file);
            break;
          case 'P':
            opponentPiece = new Pawn(true, rank, file);
            break;
          case 'R':
            opponentPiece = new Rook(true, rank, file);
            break;
          case 'B':
            opponentPiece = new Bishop(true, rank, file);
            break;
          case 'N':
            opponentPiece = new Knight(true, rank, file);
            break;
          case 'k':
            opponentPiece = new King(false, rank, file);
            break;
          case 'q':
            opponentPiece = new Queen(false, rank, file);
            break;
          case 'p':
            opponentPiece = new Pawn(false, rank, file);
            break;
          case 'r':
            opponentPiece = new Rook(false, rank, file);
            break;
          case 'b':
            opponentPiece = new Bishop(false, rank, file);
            break;
          case 'n':
            opponentPiece = new Knight(false, rank, file);
            break;
          default:
            break;
        }

        if (opponentPiece && isValidMove(*opponentPiece, kingRank, kingFile, board)) {
          delete opponentPiece;
          return true;
        }

        delete opponentPiece;
      }
    }
  }

  return false;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    return EXIT_FAILURE;
  }

  std::ifstream inputFile(argv[1]);

  if (!inputFile.is_open()) {
    return EXIT_FAILURE;
  }
  int fromRank, fromFile, toRank, toFile;
  std::string boardConfiguration;

  Queen whiteQueen(true, 7, 3);
  whiteQueen.currentRank = 7;
  whiteQueen.currentFile = 3;

  Queen blackQueen(false, 0, 3);
  blackQueen.currentRank = 0;
  blackQueen.currentFile = 3;

  King whiteKing(true, 7, 4);
  whiteKing.currentRank = 7;
  whiteKing.currentFile = 4;

  King blackKing(false, 0, 4);
  blackKing.currentRank = 0;
  blackKing.currentFile = 4;

  Pawn whitePawns[8] = {Pawn(true, 6, 0), Pawn(true, 6, 1), Pawn(true, 6, 2), Pawn(true, 6, 3),
                        Pawn(true, 6, 4), Pawn(true, 6, 5), Pawn(true, 6, 6), Pawn(true, 6, 7)};

  for (int i = 0; i < 8; ++i) {
    whitePawns[i] = Pawn(true, 6, i);
  }

  Pawn blackPawns[8] = {Pawn(false, 1, 0), Pawn(false, 1, 1), Pawn(false, 1, 2), Pawn(false, 1, 3),
                        Pawn(false, 1, 4), Pawn(false, 1, 5), Pawn(false, 1, 6), Pawn(false, 1, 7)};
  for (int i = 0; i < 8; ++i) {
    blackPawns[i] = Pawn(false, 1, i);
  }

  Rook whiteRooks[2] = {Rook(true, 7, 0), Rook(true, 7, 7)};
  for (int i = 0; i < 2; ++i) {
    whiteRooks[i].currentRank = 7;
    whiteRooks[i].currentFile = i * 7;
  }

  Rook blackRooks[2] = {Rook(false, 0, 0), Rook(false, 0, 7)};
  for (int i = 0; i < 2; ++i) {
    blackRooks[i].currentRank = 0;
    blackRooks[i].currentFile = i * 7;
  }
  Bishop whiteBishops[2] = {Bishop(true, 7, 2), Bishop(true, 7, 5)};
  for (int i = 0; i < 2; ++i) {
    whiteBishops[i].currentRank = 7;
    whiteBishops[i].currentFile = 2 + 3 * i;
  }

  Bishop blackBishops[2] = {Bishop(false, 0, 2), Bishop(false, 0, 5)};
  for (int i = 0; i < 2; ++i) {
    blackBishops[i].currentRank = 0;
    blackBishops[i].currentFile = 2 + 3 * i;
  }
  Knight whiteKnights[2] = {Knight(true, 7, 1), Knight(true, 7, 6)};
  for (int i = 0; i < 2; ++i) {
    whiteKnights[i].currentRank = 7;
    whiteKnights[i].currentFile = 1 + 5 * i;
  }

  Knight blackKnights[2] = {Knight(false, 0, 1), Knight(false, 0, 6)};
  for (int i = 0; i < 2; ++i) {
    blackKnights[i].currentRank = 0;
    blackKnights[i].currentFile = 1 + 5 * i;
  }

  ChessBoard SchachBrett;

  std::stack<std::tuple<ChessBoard, bool>> previousBoards;

  std::string input;

  while (std::getline(inputFile >> std::ws, input)) {
    if (input[0] == 'B') {
      std::string boardConfiguration = input.substr(1);
      int k = 0;
      for (int i = 0; i < 8 && k < boardConfiguration.size(); ++i) {
        for (int j = 0; j < 8 && k < boardConfiguration.size(); ++j) {
          if (boardConfiguration[k] != ' ') {
            SchachBrett[i][j] = boardConfiguration[k];
          } else {
            SchachBrett[i][j] = ' ';
          }
          k++;
        }
      }
      if (InCheck(whiteKing, SchachBrett) || InCheck(blackKing, SchachBrett)) {
        std::cout << "yes\n";
      }
      if (!InCheck(whiteKing, SchachBrett) && !InCheck(blackKing, SchachBrett)) {
        std::cout << "no\n";
      }
    }

    if (input[0] == 'M') {
      input = input.substr(1);
      convertInput(input, fromRank, fromFile, toRank, toFile);

      ChessPiece* piece = nullptr;
      switch (SchachBrett[fromRank][fromFile]) {
        case 'K':
          piece = &whiteKing;
          break;
        case 'k':
          piece = &blackKing;
          break;
        case 'Q':
          piece = &whiteQueen;
          break;
        case 'q':
          piece = &blackQueen;
          break;
        case 'P':
          for (int i = 0; i < 8; ++i) {
            if (whitePawns[i].getFile() == fromFile) {
              piece = &whitePawns[i];
              break;
            }
          }
          break;
        case 'p':
          for (int i = 0; i < 8; i++) {
            if (blackPawns[i].getFile() == fromFile) {
              piece = &blackPawns[i];
              break;
            }
          }
          break;
        case 'R':
          for (int i = 0; i < 2; ++i) {
            if (whiteRooks[i].getFile() == fromFile) {
              piece = &whiteRooks[i];
              break;
            }
          }
          break;
        case 'r':
          for (int i = 0; i < 2; ++i) {
            if (blackRooks[i].getFile() == fromFile) {
              piece = &blackRooks[i];
              break;
            }
          }
          break;
        case 'B':
          for (int i = 0; i < 2; ++i) {
            if (whiteBishops[i].getFile() == fromFile) {
              piece = &whiteBishops[i];
              break;
            }
          }
          break;
        case 'b':
          for (int i = 0; i < 2; ++i) {
            if (blackBishops[i].getFile() == fromFile) {
              piece = &blackBishops[i];
              break;
            }
          }
          break;
        case 'N':
          for (int i = 0; i < 2; ++i) {
            if (whiteKnights[i].getFile() == fromFile) {
              piece = &whiteKnights[i];
              break;
            }
          }
          break;
        case 'n':
          for (int i = 0; i < 2; ++i) {
            if (blackKnights[i].getFile() == fromFile) {
              piece = &blackKnights[i];
              break;
            }
          }
          break;
      }

      if (input.size() >= 7 && input[5] == '=') {
        char promotionPiece = input[6];
        if ((whitesTurn && input[4] != '8') || (!whitesTurn && input[4] != '1')) {
          std::cout << "invalid\n";
          continue;
        }
        if ((whitesTurn && toRank == 0) || (!whitesTurn && toRank == 7)) {
          if (piece && isValidPiece(SchachBrett[fromRank][fromFile]) && isValidPromotionPiece(promotionPiece)) {
            dynamic_cast<Pawn*>(piece)->setPromotionPieceType(promotionPiece);
            SchachBrett[toRank][toFile] = promotionPiece;
            SchachBrett[fromRank][fromFile] = ' ';
            if (InCheck(whiteKing, SchachBrett) || InCheck(blackKing, SchachBrett)) {
              std::cout << "yes\n";
            }
            if (!InCheck(whiteKing, SchachBrett) && !InCheck(blackKing, SchachBrett)) {
              std::cout << "no\n";
            }
          } else {
            std::cout << "invalid\n";
          }
        }
      }

      if (input.size() >= 7 && input[6] == '=' && input[3] == 'x') {
        char promotionPiece = input[7];
        if (piece && isValidPiece(SchachBrett[fromRank][fromFile])) {
          if ((whitesTurn && toRank == 0) || (!whitesTurn && toRank == 7)) {
            if (isCaptureMove(toRank, toFile)) {
            }
            if (piece && isValidPromotionPiece(promotionPiece)) {
              dynamic_cast<Pawn*>(piece)->setPromotionPieceType(promotionPiece);
              SchachBrett[toRank][toFile] = promotionPiece;
              SchachBrett[fromRank][fromFile] = ' ';
              if (InCheck(whiteKing, SchachBrett) || InCheck(blackKing, SchachBrett)) {
                std::cout << "yes\n";
              }
              if (!InCheck(whiteKing, SchachBrett) && !InCheck(blackKing, SchachBrett)) {
                std::cout << "no"
                          << "\n";
              }
            }
          } else {
            std::cout << "invalid\n";
          }
        }
      }

      if (input.size() < 7) {
        if (piece && !isValidMove(*piece, toRank, toFile, SchachBrett)) {
          std::cout << "invalid"
                    << "1"
                    << "\n";
          continue;
        }

        
        if (input[3] != 'x' && SchachBrett[toRank][toFile] != ' ') {
            std::cout << "invalid"
                      << "5"
                      << "\n";
            continue;
          }

        if (piece && isValidMove(*piece, toRank, toFile, SchachBrett)) {
          piece->currentRank = toRank;
          piece->currentFile = toFile;
        }

        previousBoards.push({SchachBrett, whitesTurn});

        SchachBrett[toRank][toFile] = SchachBrett[fromRank][fromFile];
        SchachBrett[fromRank][fromFile] = ' ';

        if (piece) {
          piece->currentRank = toRank;
          piece->currentFile = toFile;
        }

        if (input[3] == 'x') {
          if (SchachBrett[toRank][toFile] == ' ') {
          std::cout << "invalid"
                    << "7"
                    << "\n";
          continue;
        }
          if (isSameColor(SchachBrett[fromRank][fromFile], SchachBrett[toRank][toFile])) {
            std::cout << "invalid"
                      << "2"
                      << "\n";
            continue;
          }
          if (SchachBrett[toRank][toFile] == ' ') {
            std::cout << "invalid"
                      << "17"
                      << "\n";
            continue;
          } 
        }

        if (InCheck(whiteKing, SchachBrett) || InCheck(blackKing, SchachBrett)) {
          std::cout << "yes\n";
        }
        if (!InCheck(whiteKing, SchachBrett) && !InCheck(blackKing, SchachBrett)) {
          std::cout << "no" << /*": tofile " << toFile << " toRank " << toRank <<*/ "\n";
        }
      }
      switchTurn();
    }
    if (input == "print") {
      for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
          std::cout << SchachBrett[i][j];
        }
        std::cout << '\n';
      }
      continue;
    }
  }

  return EXIT_SUCCESS;
}

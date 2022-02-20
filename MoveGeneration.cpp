#include "MoveGeneration.hpp"

#include <ostream>
#include <cassert>
#include <cmath>
#include <memory>
#include <vector>
#include <iostream>

void MoveGeneration::generatePseudoLegalMoves(const Board& board, MoveVec& moves, const std::optional<Square>& from)
{
    if(from == std::nullopt){
        for (auto const& [key, val] : board.pieceMap()){
            Square startSquare = (Square)* Square::fromIndex(key);
            Piece piece = *val;
            if(piece.color() == board.turn()) generatePieceMoves(board, startSquare, piece, moves);
        }
    }
    else {
            Square startSquare = (Square)* from;
            try {
                Piece piece = (Piece)* board.pieceMap().at(startSquare.index());
                generatePieceMoves(board, startSquare, piece, moves);
            } catch(const std::exception& e) {}
    }
}

void MoveGeneration::generatePieceMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves){
    switch(piece.type()){
        case PieceType::Pawn:
            generatePawnMoves(board, startSquare, piece, moves);
            break;
        case PieceType::King:
            generateKingMoves(board, startSquare, piece, moves);
            break;
        case PieceType::Queen:
            generateQueenMoves(board, startSquare, piece, moves);
            break;
        case PieceType::Rook:
            generateRookMoves(board, startSquare, piece, moves);
            break;
        case PieceType::Knight:
            generateKnightMoves(board, startSquare, piece, moves);
            break;
        case PieceType::Bishop:
            generateBishopMoves(board, startSquare, piece, moves);
            break;
    }
}

//OK
void MoveGeneration::generatePawnMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves) {
    // define moveOffsetDirections.
    std::tuple<int,int> moveOffsetsBlack[4] = {{0,-1},{0,-2},{-1,-1},{1,-1}};
    std::tuple<int,int> moveOffsetsWhite[4] = {{0,1},{0,2},{-1,1},{1,1}};

    std::tuple<int,int>* moveOffsets;
    if(piece.color() == PieceColor::Black) moveOffsets = moveOffsetsBlack;
    else moveOffsets = moveOffsetsWhite;
    
    // loop over moveOffsets.
    for(int i = 0; i < 4; i++){
        // check weater they return a valid Square by calling index using moveOffsets.
        Square::Optional targetSquareOpt = Square::fromCoordinates(startSquare.file()+std::get<0>(moveOffsets[i]),
                                                                startSquare.rank()+std::get<1>(moveOffsets[i]));
        if(targetSquareOpt == std::nullopt) continue;
        Square targetSquare = (Square)* targetSquareOpt;
        
        // check if diagonal moves possible
        if(i > 1){
            // check if other piece on targetSquare. If so move on it.
            Piece::Optional otherPiece = board.piece(targetSquare);
            if(otherPiece == std::nullopt) continue;
            if(piece.color() != ((Piece)* otherPiece).color()){
                // check promotion;
                if((targetSquare.rank() == 0 && piece.color() == PieceColor::Black) || (targetSquare.rank() == 7 && piece.color() == PieceColor::White))
                    moves.push_back(Move(startSquare,targetSquare,PieceType::Queen));
                else moves.push_back(Move(startSquare,targetSquare));
            }
        }
        
        // check if vertical moves possible
        if(i < 2){
            // check if other piece on targetSquare. If so move blocked.
            Piece::Optional otherPiece = board.piece(targetSquare);
            if(otherPiece != std::nullopt) continue;
            // if 1 vertical move OK
            if(i == 0){
                // check promotion
                if((targetSquare.rank() == 0 && piece.color() == PieceColor::Black) || (targetSquare.rank() == 7 && piece.color() == PieceColor::White))
                    moves.push_back(Move(startSquare,targetSquare,PieceType::Queen));
                else moves.push_back(Move(startSquare,targetSquare));
            }
            // if 2 vertical moves at once only OK when rank == 1 and PieceColor is White or when rank == 6 and PieceColor is Black. (promotion not possible)
            // and if no piece blocking
            else {
                if(((piece.color() == PieceColor::White && startSquare.rank() == 1) || (piece.color() == PieceColor::Black && startSquare.rank() == 6))
                    && !board.pieceMap().count(targetSquare.index()+(std::get<1>(moveOffsets[0])*(-8))))
                    moves.push_back(Move(startSquare,targetSquare));
            }
        }
    }
    
    // check for en passant OK
    //check adjacent square
    for(int verticalMove = -1; verticalMove < 2; verticalMove+=2){
        // check if there is a pawn of opposing color next to you
        Square::Optional adjacentSquareOpt = Square::fromCoordinates(startSquare.file()+verticalMove, startSquare.rank());
        if(adjacentSquareOpt == std::nullopt) continue;
        Square adjacentSquare = (Square)* adjacentSquareOpt;
        
        Piece::Optional adjacentPieceOpt = board.piece(adjacentSquare);
        if(adjacentPieceOpt == std::nullopt) continue;
        Piece adjacentPiece = (Piece)* adjacentPieceOpt;
        
        if(adjacentPiece.color() == piece.color() || adjacentPiece.type() != PieceType::Pawn) continue;
        
        // check if capturing pawn has advanced exactly three ranks
        if((piece.color() == PieceColor::White && startSquare.rank() != 4) || (piece.color() == PieceColor::Black && startSquare.rank() != 3)) continue;
        
        // check if previous move was where adjacent pawn moved two squares => i.e. check if enPassantSquare is adjacentSquare.
        Square::Optional enPassantSquareOpt = board.enPassantSquare();
        if(enPassantSquareOpt == std::nullopt) continue;
        //Square enPassantSquare = (Square)* enPassantSquareOpt;
        //if(!(enPassantSquare == adjacentSquare)) continue;
        
        // calculate move diagonal offset of en passant
        int diagonalOffset;
        if(piece.color() == PieceColor::Black) diagonalOffset = -8+verticalMove;
        else diagonalOffset = 8+verticalMove;
        Square::Optional targetSquare = Square::fromIndex(startSquare.index()+diagonalOffset);
        if(targetSquare == std::nullopt) continue;
        moves.push_back(Move(startSquare,(Square)* targetSquare));
    }
}

void MoveGeneration::generateKingMoves(Board board, Square startSquare, Piece piece, MoveVec& moves) {
    // define moveOffsetDirections.
    int moveOffsets[8] = {-1,1,-8,8,-7,7,-9,9};
    int numberOffSquaresToEdge[8] = {(int) startSquare.file(), (int) (7-startSquare.file()), (int) startSquare.rank(), (int) (7-startSquare.rank()),
        std::min((int) (7-startSquare.file()), (int) startSquare.rank()),
        std::min((int) startSquare.file(), (int) (7-startSquare.rank())),
        std::min((int) startSquare.file(), (int) startSquare.rank()),
        std::min((int) (7-startSquare.rank()), (int) (7-startSquare.file()))
    };
    
    // loop over all offset directions.
    for(int i = 0; i < 8; i++){
        // loop over all positions of this direction.
        for(int j = 0; j < numberOffSquaresToEdge[i]; j++){
            // check weater they return a valid Square by calling index using moveOffsets.
            Square::Optional targetSquare = Square::fromIndex(startSquare.index()+moveOffsets[i]*(j+1));  
            if(targetSquare == std::nullopt) continue;
            // check if move to targetSquare results in a check.
            //if(board.inCheck((Square)* targetSquare)) continue;
            // check if other piece on targetSquare. If same color piece continue, else capture it.
            Piece::Optional otherPiece = board.piece((Square)* targetSquare);
            if(otherPiece != std::nullopt){
                // if enemy piece has been encountered capture and break search in this direction. else just break.
                if(piece.color() != ((Piece)* otherPiece).color()){
                moves.push_back(Move(startSquare,(Square)* targetSquare));
                }
                break;
            }
            moves.push_back(Move(startSquare,(Square)* targetSquare));
            break;
        }
    }
    /*
    // check castling positions
    // if board has no castling rights => return
    if (board.castlingRightsHave(CastlingRights::None)) return;
    // if color has no castling rights => return
    if((board.turn() == PieceColor::White && board.castlingRightsHave(CastlingRights::Black))
        || (board.turn() == PieceColor::Black && board.castlingRightsHave(CastlingRights::White))) return;
    
    // check if there is a piece between king and rook
    int kingIndices[2] = {4, 60};
    int rankOffsets[2] = {4,3};
    int rankStartCount[2] = {-1,1};
    int rankEndCount[2] = {3,2};
    int i = 0;
    if(piece.color() == PieceColor::Black) i = 1;
    //std::cout << "CastlingRights: " << board.castlingRights() << "--------------------------------" << '\n';
    if (board.castlingRightsHave(CastlingRights::BlackQueenside)) {
        //std::cout << "IN IF CastlingRights: " << board.castlingRights() << "--------------------------------" << '\n';
    }

    for(int j = 0; j < 2; j++){
        // check castling rights
        //if(board.turn() == PieceColor::White && (board.castlingRights() & CastlingRights::White) != CastlingRights::None))
        
        // check if king exists
        if(!board.pieceMap().count(kingIndices[i])) continue; 
        Piece king = *board.pieceMap().at(kingIndices[i]);
        if(king.type() != PieceType::King) continue;

        // check if rook exist
        if(!board.pieceMap().count(kingIndices[i]+(rankStartCount[j]*rankOffsets[j]))) continue;
        Piece rook = *board.pieceMap().at(kingIndices[i]+(rankStartCount[j]*rankOffsets[j]));
        if(rook.type() != PieceType::Rook) continue;
        // check squares from king to rook for pieces
        for(int verticalOffset = 1; verticalOffset < rankOffsets[j]; verticalOffset++){
            // if there exists another piece in between => continue;
            if(board.pieceMap().count(kingIndices[i]+(rankStartCount[j]*verticalOffset))) continue;
            // if there exists no piece in between and verticalOffset is at end => add extra possible castling move;
            if(rankStartCount[j]*verticalOffset == rankStartCount[j]*rankEndCount[j]){
                // if king in check now => continue
                if(board.isCheck(kingIndices[i])) continue;
                    //if(board.turn() == PieceColor::White) Board::setCastlingRights(board, ); continue; //OK
                // if king in check after castling => continue
                if(board.isCheck(startSquare.rank() * 8 + startSquare.file()+(rankStartCount[j]*2))) continue; //OK
                // if rook in capturing zone => continue
                if(Board::pieceInCapturingZone(board, startSquare.rank() * 8 + startSquare.file()+(rankStartCount[j]))) continue; //OK
                
                moves.push_back(Move(startSquare, (Square)* Square::fromCoordinates(startSquare.file()+(rankStartCount[j]*2), startSquare.rank())));
            }
        }
    }*/
}

//OK
void MoveGeneration::generateQueenMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves)
{
    // define moveOffsetDirections.
    //std::cout << "in queen, ";
    int moveOffsetDirections[8] = {-1,1,-8,8,-7,7,-9,9};
    int numberOffSquaresToEdge[8] = {(int) startSquare.file(), (int) (7-startSquare.file()), (int) startSquare.rank(), (int) (7-startSquare.rank()),
        std::min((int) (7-startSquare.file()), (int) startSquare.rank()),
        std::min((int) startSquare.file(), (int) (7-startSquare.rank())),
        std::min((int) startSquare.file(), (int) startSquare.rank()),
        std::min((int) (7-startSquare.rank()), (int) (7-startSquare.file()))
    };

    // loop over all offset directions.
    for(int i = 0; i < 8; i++){
        // loop over all positions of this direction.
        for(int j = 0; j < numberOffSquaresToEdge[i]; j++){
            // check weater they return a valid Square by calling index using moveOffsetDirections*j.
            Square::Optional targetSquare = Square::fromIndex(startSquare.index()+moveOffsetDirections[i]*(j+1));  
            if(targetSquare == std::nullopt) continue;
            // check if other piece on targetSquare. If same color piece continue, else capture it.
            Piece::Optional otherPiece = board.piece((Square)* targetSquare);
            if(otherPiece != std::nullopt){
                // if enemy piece has been encountered capture and break search in this direction. else just break.
                if(piece.color() != ((Piece)* otherPiece).color()){
                moves.push_back(Move(startSquare,(Square)* targetSquare));
                }
                break;
            }
            moves.push_back(Move(startSquare,(Square)* targetSquare));
        }
    }
}

//OK
void MoveGeneration::generateRookMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves) {
    // define moveOffsetDirections.
    int moveOffsetDirections[4] = {-1,1,-8,8};
    int numberOffSquaresToEdge[4] = {(int) startSquare.file(), (int) (7-startSquare.file()), (int) startSquare.rank(), (int) (7-startSquare.rank())};
    
    // loop over all offset directions.
    for(int i = 0; i < 4; i++){
        // loop over all positions of this direction.
        for(int j = 0; j < numberOffSquaresToEdge[i]; j++){
            // check weater they return a valid Square by calling index using moveOffsetDirections*j.
            Square::Optional targetSquare = Square::fromIndex(startSquare.index()+moveOffsetDirections[i]*(j+1));  
            if(targetSquare == std::nullopt) continue;
            // check if other piece on targetSquare. If same color piece continue, else capture it.
            Piece::Optional otherPiece = board.piece((Square)* targetSquare);
            if(otherPiece != std::nullopt){
                // if enemy piece has been encountered capture and break search in this direction. else just break.
                if(piece.color() != ((Piece)* otherPiece).color()){
                moves.push_back(Move(startSquare,(Square)* targetSquare));
                }
                break;
            }
            moves.push_back(Move(startSquare,(Square)* targetSquare));
        }
    }  
}

//OK
void MoveGeneration::generateKnightMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves)
{
    // define moveOffsets.
    std::tuple<int,int> moveOffsets[8] = {{-1,-2},{1,-2},{-2,-1},{-2,1},{-1,2},{1,2},{2,1},{2,-1}};
    
    // loop over all offset positions.
    for(int i = 0; i < 8; i++){
        // check weater they return a valid Square by calling index using moveOffsets.
        Square::Optional targetSquare = Square::fromCoordinates(startSquare.file()+std::get<0>(moveOffsets[i]),
                                                                startSquare.rank()+std::get<1>(moveOffsets[i]));
        if(targetSquare == std::nullopt) continue;
        // check if other piece on targetSquare. If same color piece continue, else capture it.
        Piece::Optional otherPiece = board.piece((Square)* targetSquare);
        if(otherPiece != std::nullopt){
            // if enemy piece has been encountered capture and break search in this direction. else just break.
            if(piece.color() != ((Piece)* otherPiece).color()){
            moves.push_back(Move(startSquare,(Square)* targetSquare));
            }
            else continue;
        }
        moves.push_back(Move(startSquare,(Square)* targetSquare));
    }
}

//OK
void MoveGeneration::generateBishopMoves(const Board& board, Square startSquare, Piece piece, MoveVec& moves) {
    // define moveOffsetDirections.
    int moveOffsetDirections[4] = {-7,7,-9,9};
    int numberOffSquaresToEdge[4] = {
        std::min((int) (7-startSquare.file()), (int) startSquare.rank()),
        std::min((int) startSquare.file(), (int) (7-startSquare.rank())),
        std::min((int) startSquare.file(), (int) startSquare.rank()),
        std::min((int) (7-startSquare.rank()), (int) (7-startSquare.file()))};
        
    // loop over all offset directions.
    for(int i = 0; i < 4; i++){
        // loop over all positions of this direction.
        for(int j = 0; j < numberOffSquaresToEdge[i]; j++){
            // check weater they return a valid Square by calling index using moveOffsetDirections*j.
            Square::Optional targetSquare = Square::fromIndex(startSquare.index()+moveOffsetDirections[i]*(j+1));  
            if(targetSquare == std::nullopt) continue;
            // check if other piece on targetSquare. If same color piece continue, else capture it.
            Piece::Optional otherPiece = board.piece((Square)* targetSquare);
            if(otherPiece != std::nullopt){
                // if enemy piece has been encountered capture and break search in this direction. else just break.
                if(piece.color() != ((Piece)* otherPiece).color()){
                moves.push_back(Move(startSquare,(Square)* targetSquare));
                }
                break;
            }
            moves.push_back(Move(startSquare,(Square)* targetSquare));
        }
    }  
}


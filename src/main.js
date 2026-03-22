/**
 * Chess Move Calculator - Main Entry
 * Uses chess.js for move generation/validation.
 * Supports drag-to-move and drag-off-board to delete.
 */

import { Chess } from 'chess.js';

// Piece type to asset path mapping
const PIECE_ASSETS = {
  w: { k: 'king', q: 'queen', r: 'rook', b: 'bishop', n: 'knight', p: 'pawn' },
  b: { k: 'king', q: 'queen', r: 'rook', b: 'bishop', n: 'knight', p: 'pawn' },
};

const boardEl = document.getElementById('chess-board');
const movesOutput = document.getElementById('moves-output');
const turnDisplay = document.getElementById('turn-display');
const fenDisplay = document.getElementById('fen-display');

let game = new Chess();
let selectedSquare = null;
let dragLegalMoves = []; // Legal move targets during drag
let boardFlipped = false; // true = Black at bottom (Black's perspective)

function getPieceAsset(color, piece) {
  const colorDir = color === 'w' ? 'white' : 'black';
  const pieceName = PIECE_ASSETS[color][piece];
  return `/assets/chess/${colorDir}/${pieceName}.svg`;
}

function parseFenToSquares(fenPosition) {
  const squares = [];
  for (const row of fenPosition.split('/')) {
    for (const char of row) {
      if (/[1-8]/.test(char)) {
        for (let i = 0; i < parseInt(char, 10); i++) squares.push(null);
      } else {
        squares.push(char);
      }
    }
  }
  return squares;
}

function renderBoard() {
  boardEl.innerHTML = '';
  const fen = game.fen();
  const fenPosition = fen.split(' ')[0];
  const squares = parseFenToSquares(fenPosition);

  // When flipped: row 0 = rank 1 (top), row 7 = rank 8 (bottom) — Black's perspective
  const getLogicalRow = (r) => (boardFlipped ? 7 - r : r);

  for (let row = 0; row < 8; row++) {
    for (let col = 0; col < 8; col++) {
      const logicalRow = getLogicalRow(row);
      const logicalCol = col;
      const square = document.createElement('div');
      square.className = `chess-square ${(logicalRow + logicalCol) % 2 === 0 ? 'light' : 'dark'}`;
      square.dataset.square = String.fromCharCode(97 + logicalCol) + (8 - logicalRow);

      const char = squares[logicalRow * 8 + logicalCol];
      if (char && /[KQRBNPkqrbnp]/.test(char)) {
        const color = char === char.toUpperCase() ? 'w' : 'b';
        const piece = char.toLowerCase();
        const img = document.createElement('img');
        img.src = getPieceAsset(color, piece);
        img.alt = `${color === 'w' ? 'White' : 'Black'} ${piece}`;
        img.className = 'chess-piece';
        img.draggable = true;
        img.addEventListener('dragstart', (e) => handleDragStart(e, square.dataset.square));
        img.addEventListener('dragend', () => {
          document.querySelectorAll('.dragging').forEach((el) => el.classList.remove('dragging'));
          boardEl.querySelectorAll('.drag-over-legal').forEach((el) => el.classList.remove('drag-over-legal'));
        });
        square.appendChild(img);
      }

      square.addEventListener('dragover', handleDragOver);
      square.addEventListener('dragleave', handleDragLeave);
      square.addEventListener('drop', (e) => handleDrop(e, square.dataset.square));
      square.addEventListener('click', () => handleSquareClick(square.dataset.square));
      boardEl.appendChild(square);
    }
  }

  turnDisplay.textContent = game.turn() === 'w' ? 'White' : 'Black';
  fenDisplay.textContent = fen;
}

function getLegalMovesForSquare(square) {
  return game.moves({ square, verbose: true });
}

function handleDragStart(e, fromSquare) {
  const piece = game.get(fromSquare);
  if (!piece) {
    e.preventDefault();
    return;
  }
  // Only current player can move; anyone can delete (drag off board)
  dragLegalMoves = piece.color === game.turn() ? getLegalMovesForSquare(fromSquare) : [];
  e.dataTransfer.setData('text/plain', fromSquare);
  e.dataTransfer.effectAllowed = 'move';
  e.target.classList.add('dragging');
}

function handleDragOver(e) {
  if (e.dataTransfer.types.includes('text/plain')) {
    e.preventDefault();
    e.dataTransfer.dropEffect = 'move';
    const toSquare = e.currentTarget.dataset.square;
    const isLegal = dragLegalMoves.some((m) => m.to === toSquare);
    e.currentTarget.classList.toggle('drag-over-legal', isLegal);
  }
}

function handleDragLeave(e) {
  e.currentTarget.classList.remove('drag-over-legal');
}

function handleDrop(e, toSquare) {
  e.preventDefault();
  e.stopPropagation();
  e.currentTarget.classList.remove('drag-over-legal');
  document.querySelectorAll('.dragging').forEach((el) => el.classList.remove('dragging'));

  const fromSquare = e.dataTransfer.getData('text/plain');
  if (!fromSquare) return;

  const move = dragLegalMoves.find((m) => m.to === toSquare);
  if (move) {
    game.move(move);
    renderBoard();
    updateMovesList([]);
  }
}

function handleDocumentDragOver(e) {
  if (e.dataTransfer.types.includes('text/plain')) {
    e.preventDefault();
    e.dataTransfer.dropEffect = 'move';
  }
}

function handleDocumentDrop(e) {
  if (!e.dataTransfer.types.includes('text/plain')) return;
  e.preventDefault();
  const fromSquare = e.dataTransfer.getData('text/plain');
  if (!fromSquare) return;

  // Only remove if dropped outside the board
  if (!boardEl.contains(e.target)) {
    game.remove(fromSquare);
    selectedSquare = null;
    clearHighlights();
    renderBoard();
    updateMovesList([]);
  }
  document.querySelectorAll('.dragging').forEach((el) => el.classList.remove('dragging'));
}

function handleSquareClick(square) {
  const moves = getLegalMovesForSquare(square);
  const piece = game.get(square);

  // If we had a selection, try to make a move
  if (selectedSquare && selectedSquare !== square) {
    const movesFromSelected = getLegalMovesForSquare(selectedSquare);
    const move = movesFromSelected.find((m) => m.to === square);
    if (move) {
      game.move(move);
      selectedSquare = null;
      clearHighlights();
      renderBoard();
      updateMovesList([]);
      return;
    }
  }

  selectedSquare = piece ? square : null;
  clearHighlights();

  if (piece && piece.color === game.turn()) {
    document.querySelector(`[data-square="${square}"]`)?.classList.add('selected');
    moves.forEach((m) => {
      const el = document.querySelector(`[data-square="${m.to}"]`);
      if (el) el.classList.add(m.captured ? 'legal-capture' : 'legal-move');
    });
    updateMovesList(moves);
  } else {
    updateMovesList([]);
  }
}

function clearHighlights() {
  document.querySelectorAll('.chess-square').forEach((el) => {
    el.classList.remove('selected', 'legal-move', 'legal-capture');
  });
}

function updateMovesList(moves) {
  movesOutput.innerHTML = '';
  if (moves.length === 0) {
    movesOutput.innerHTML = '<li class="hint">No moves</li>';
    return;
  }
  moves.forEach((m) => {
    const li = document.createElement('li');
    li.textContent = m.san || m.from + '-' + m.to;
    movesOutput.appendChild(li);
  });
}

function setBoardOrientation(flipped) {
  boardFlipped = flipped;
  document.getElementById('orientation-white').classList.toggle('active', !flipped);
  document.getElementById('orientation-black').classList.toggle('active', flipped);

  // Update whose turn it is
  const fen = game.fen().split(' ');
  fen[1] = flipped ? 'b' : 'w';
  game.load(fen.join(' '));

  renderBoard();
  updateMovesList([]);
  clearHighlights();
}

function resetBoard() {
  game.reset();
  boardFlipped = false;
  selectedSquare = null;
  document.getElementById('orientation-white').classList.add('active');
  document.getElementById('orientation-black').classList.remove('active');
  clearHighlights();
  renderBoard();
  updateMovesList([]);
}

// Document drop zone for removing pieces (drag off board)
document.addEventListener('dragover', handleDocumentDragOver);
document.addEventListener('drop', handleDocumentDrop);

// Orientation controls
document.getElementById('orientation-white').addEventListener('click', () => setBoardOrientation(false));
document.getElementById('orientation-black').addEventListener('click', () => setBoardOrientation(true));
document.getElementById('reset-board').addEventListener('click', resetBoard);

// Initialize
renderBoard();

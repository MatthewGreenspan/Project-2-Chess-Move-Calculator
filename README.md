# Chess Move Calculator

A frontend chess move calculator that displays legal moves for selected pieces. Built with Vite and [chess.js](https://github.com/jhlywa/chess.js).

## Setup

```bash
npm install
npm run dev
```

Open http://localhost:5173 in your browser.

## Project structure

```
├── index.html           # Entry HTML
├── src/
│   ├── main.js          # App logic, chess.js integration
│   └── style.css        # Styles (customize here)
├── public/
│   └── assets/chess/    # Chess piece SVGs and board
│       ├── white/       # White pieces (king, queen, rook, bishop, knight, pawn)
│       ├── black/       # Black pieces
│       └── board/       # Board SVG
├── package.json
└── vite.config.js
```

## Chess assets

Chess piece SVGs are from [Sashité Chess Assets](https://sashite.dev/assets/chess/), released under CC0 1.0 (public domain). You may use them for any purpose without attribution.

## Dependencies

- **chess.js** – Move generation, validation, FEN parsing
- **Vite** – Build tool and dev server

## Customization

- **Styles**: Edit `src/style.css` (CSS variables in `:root` for colors)
- **Logic**: Edit `src/main.js` to change move display, interactions, or add features
- **Assets**: Replace SVGs in `public/assets/chess/` with your own piece set

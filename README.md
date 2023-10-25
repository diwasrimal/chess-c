# Chess

Chess made with raylib. Allows playing v/s human or computer. The AI is not quite powerful yet.
Keep your expectations low.

<img width="657" alt="board" src="https://github.com/diwasrimal/chess-c/assets/84910758/d8209192-436a-4b09-afc2-d1bc6bb5299b">

## Binaries for Windows
https://github.com/diwasrimal/chess-c/releases/tag/v1.0.0

## Build

### POSIX
Requires [raylib](https://www.raylib.com/) and `pkg-config`

```sh
git clone https://github.com/diwasrimal/chess-c.git
cd chess-c
./build.sh    # or `make`
./chess
```

### Cross compilation to Windows via mingw-w64.
Requires [mingw-w64](https://www.mingw-w64.org/)

```sh
git clone https://github.com/diwasrimal/chess-c.git
cd chess-c
./build_mingw.sh
wine64 chess.exe
```

## Controls
* `Esc` to exit
* `R` to restart

## TODO
- [x] Promotions
- [x] Implement en passant
- [ ] Better Bot

## Contributing
- Feel free to report any bugs
- Feel free to send pull requests

## References
Icons designed and provided by [Gyan Lakhwani](https://github.com/gyanl)

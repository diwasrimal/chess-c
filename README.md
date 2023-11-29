# Chess

Chess made with raylib. Allows playing human v/s human

<img width="654" alt="Screenshot 2023-11-29 at 6 23 28 PM" src="https://github.com/diwasrimal/chess-c/assets/84910758/ade2975a-d9d4-4935-b297-d5daca6dc5f6">

## Binaries for Windows
https://github.com/diwasrimal/chess-c/releases/tag/v1.1.0

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
- Nothing right now! :)

## Contributing
- Feel free to report any bugs
- Feel free to send pull requests

## References
Icons designed and provided by [Gyan Lakhwani](https://github.com/gyanl)


# Clibean

## A command line tool for Membean

### Information

- Clibean is very early in development and the program may not compile at the moment

- I will update this README when a working version is ready

- Feel free to contribute or ask questions!

  

### Build and run instructions (Linux and MacOS only)

1. Install the prerequisites:

- Ubuntu: `sudo apt install build-essential libssl-dev git`

- Arch: `sudo pacman -S base-devel openssl git`

- MacOS:
  1. Install the command line tools for [Xcode](https://developer.apple.com/xcode/) and [brew](https://brew.sh/)
  2. Run `ln -s /usr/local/opt/openssl/include/openssl /usr/local/include`
  3. Run `ln -s /usr/local/opt/openssl/lib/libssl.dylib /usr/local/opt/openssl/lib/libcrypto.dylib /usr/local/lib/`

2. Clone the repository: `git clone https://github.com/campital/Clibean.git`

3. Run `make` inside the source directory

4. Run `./clibean` to start a training session!
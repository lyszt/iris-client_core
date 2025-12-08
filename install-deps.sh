#!/bin/bash
# IrisCLI Dependency Installation Script

set -e

echo "Installing dependencies for IrisCLI..."

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo "Unable to detect OS"
    exit 1
fi

# Install based on OS
case "$OS" in
    ubuntu|debian)
        echo "Detected Debian/Ubuntu"
        sudo apt update
        sudo apt install -y \
            build-essential \
            cmake \
            libcurl4-openssl-dev
        ;;

    fedora|rhel|centos)
        echo "Detected Fedora/RHEL/CentOS"
        sudo dnf install -y \
            gcc \
            make \
            cmake \
            libcurl-devel
        ;;

    arch|manjaro)
        echo "Detected Arch Linux"
        sudo pacman -S --needed \
            base-devel \
            cmake \
            curl
        ;;

    *)
        echo "Unsupported OS: $OS"
        echo "Please install manually: build-essential, cmake, libcurl-dev"
        exit 1
        ;;
esac

echo ""
echo "✓ Dependencies installed successfully!"
echo ""
echo "Next steps:"
echo "  make        # Build the project"
echo "  make run    # Build and run"

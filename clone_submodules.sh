#!/bin/bash
# This file is used for development only.
# Clones the subdirectories needed when developing this submodule when isolated from the real rn installation.
git clone https://github.com/superpoweredSDK/Low-Latency-Android-iOS-Linux-Windows-tvOS-macOS-Interactive-Audio-Platform Superpowered && cd Superpowered && git checkout 49ded315b92c3c7287cd17f558d5842d051735e3 && git filter-branch --prune-empty --subdirectory-filter Superpowered HEAD
